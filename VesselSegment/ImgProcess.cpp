#include "ImgProcess.h"


Mat ImgProcess::CannyThreshold(Mat src, int lowThreshold)
{
	const int max_lowThreshold = 100;
	const int ratio = 3;
	const int kernel_size = 3;	
	Mat src_gray;
	Mat dst, detected_edges;
	dst.create(src.size(), src.type());
	cvtColor(src, src_gray, COLOR_BGR2GRAY);
	blur(src_gray, detected_edges, Size(3, 3));
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*3, kernel_size);
	dst = Scalar::all(0);
	src.copyTo(dst, detected_edges);
#ifdef DEBUG
	imshow("Edge Map", dst);
#endif // DEBUG
	return dst;

	
}
int ImgProcess::LoadImage(string szInput)
{
	m_Img = imread(szInput, COLOR_BGR2GRAY);

	if (m_Img.empty()){
		cout << "Could not read the image: " << szInput << endl;
		return -1;
	}
	return 0;
}


int ImgProcess::BoundaryDetction_Frangi()
{
	Mat InputImg = m_Img;
	
	//set default frangi opts
	frangi2d_opts_t opts;
	frangi2d_createopts(&opts);

	Mat input_img_fl;
	InputImg.convertTo(input_img_fl, CV_32FC1);
	Mat vesselness, scale, angles;
	frangi2d(input_img_fl, vesselness, scale, angles, opts);

	vesselness = vesselness * 255;

	Mat vesselgray;
	cvtColor(vesselness, vesselgray, COLOR_BGR2GRAY);
	Mat img_edge, labels, centroids, img_color, stats;

	threshold(vesselgray, img_edge, 20, 255, THRESH_BINARY);

	Mat img_edge_gray;
	//imwrite(szOutput, img_edge);
	Mat img_edge_8s;
	img_edge.convertTo(img_edge_8s, CV_8S);
	int i, nccomps = connectedComponentsWithStats(img_edge_8s, labels, stats, centroids);
	cout << "Total Connected Components Detected: " << nccomps << endl;

	vector<Vec3b> colors(nccomps + 1);
	// Ordered map
	std::map<int, int> order;

	colors[0] = Vec3b(0, 0, 0); // background pixels remain black.
	for (i = 1; i <= nccomps; i++) {

		// Mapping values to keys
		order[(int)stats.at<int>(i - 1, CC_STAT_AREA)] = i;

	}

	// Iterating the map and
	// printing ordered values
	for (auto i = order.begin(); i
		!= order.end(); i++) {
		std::cout << i->first
			<< " : "
			<< i->second << '\n';
	}

	//remove the biggest area, which is the background.
	auto it = order.end();
	it--;
	order.erase(it);

	it = order.end();
	it--;
	cout << "vessel: " << it->second << endl;
	int reslabel = it->second;

	img_color = Mat::zeros(img_edge_8s.size(), CV_8UC3);
	for (int y = 0; y < img_color.rows; y++)
		for (int x = 0; x < img_color.cols; x++){
			int label = labels.at<int>(y, x);
			CV_Assert(0 <= label && label <= nccomps);

			if (label == (reslabel - 1)) {
				img_color.at<Vec3b>(y, x) = Vec3b(0, 0, 255);//set red color for the vessel.
			}
			else {
				img_color.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
			}

		}
#ifdef DEBUG
	imshow("Labeled map", img_color);
	waitKey();
#endif // DEBUG

	
	m_mask = img_color;
	//imwrite(szOutput, img_color);

	Mat togray, resbw;
	cvtColor(img_color, togray, COLOR_BGR2GRAY);
	threshold(togray, resbw, 0, 255, THRESH_BINARY | THRESH_OTSU);
	thinning(resbw);
#ifdef DEBUG
	imshow("centerline:", resbw);
	waitKey();
#endif // DEBUG
		
	m_centerline = resbw;

	Mat dstwithcl;
	Mat Imggray;
	cvtColor(m_Img, Imggray, COLOR_RGB2GRAY);
	
	addWeighted(Imggray, 0.7, resbw, 0.3, 0, dstwithcl);
#ifdef DEBUG
	imshow("centerlinecolor:", dstwithcl);
	waitKey();
#endif // DEBUG

	return 0;
}
int ImgProcess::BoundaryDetction_Canny()
{
		
	//namedWindow(window_name, WINDOW_AUTOSIZE);
	//createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);
	const int thred = 22;
	auto dst = CannyThreshold(m_Img, thred);
	

	Mat resImg;
#ifdef DEBUG
	imshow("dst", dst);
	waitKey(0);
#endif // DEBUG

	
	cvtColor(dst, resImg, COLOR_BGR2GRAY);
	Mat cropImg;
	Mat img_bw;
	Mat grayImg;
	cvtColor(m_Img, grayImg, COLOR_BGR2GRAY);
	
	//invert intensity of the input image, make the vessle bright.
	for (int j = 0; j < grayImg.rows; j++)
	{
		for (int i = 0; i < grayImg.cols; i++)
		{
			grayImg.at<uchar>(j, i) = 255 - (int)grayImg.at<uchar>(j, i);
		}
	}
#ifdef DEBUG
	imshow("inverse image", grayImg);
	waitKey(0);
#endif // DEBUG	

	//Adaptive threshold.
	int cropsize = 20;
	int f = 0;
	Mat cropdst;
	Mat dst_gray;
	cvtColor(dst, dst_gray, COLOR_BGR2GRAY);
	for (int j = 0; j < dst.rows; j++){
		for (int i = 0; i < dst.cols; i++){
			int intensity = (int)dst_gray.at<uchar>(j, i);
			if (intensity > 0){
				//find the local area and use otsu threshold to fill the hole.
				if (i - cropsize >= 0 && i + cropsize < dst.cols && j - cropsize >= 0 && j + cropsize < dst.rows) {
					cropImg = grayImg(Range(j - cropsize, j + cropsize), Range(i - cropsize, i + cropsize));
					cropdst = dst_gray(Range(j - cropsize, j + cropsize), Range(i - cropsize, i + cropsize));
					
					threshold(cropImg, img_bw, 0, 255, THRESH_BINARY | THRESH_OTSU);

					//update the pixel value.
					int sx = 0;
					int sy;
					for (int n = (j - cropsize); n < (j + cropsize); n++){
						sy = 0;
						for (int m = (i - cropsize); m < (i + cropsize); m++){
							resImg.at<uchar>(n, m) = (int)img_bw.at<uchar>(sx, sy);
							sy++;
						}
						sx++;
					}
					f++;
				}
			}
		}
	}
#ifdef DEBUG
	imshow("Otsu image", resImg);
	waitKey(0);
#endif // DEBUG	
	
	Mat img_edge, labels, centroids, img_color, stats;

	threshold(resImg, img_edge, 120, 255, THRESH_BINARY);
#ifdef DEBUG
	imshow("Otsu image", img_edge);
	waitKey(0);
#endif // DEBUG

	
	Mat img_edge_gray;
	//imwrite(szOutput, img_edge);
	Mat img_edge_8s;
	img_edge.convertTo(img_edge_8s, CV_8S);
	int i, nccomps = connectedComponentsWithStats(img_edge_8s, labels, stats, centroids);
	cout << "Total Connected Components Detected: " << nccomps << endl;

	vector<Vec3b> colors(nccomps + 1);
	// Ordered map
	std::map<int, int> order;

	colors[0] = Vec3b(0, 0, 0); // background pixels remain black.
	for (i = 1; i <= nccomps; i++) {		
		// Mapping values to keys
		order[(int)stats.at<int>(i - 1, CC_STAT_AREA)] = i;

	}

	// Iterating the map and
	// printing ordered values
	for (auto i = order.begin(); i
		!= order.end(); i++) {
		std::cout << i->first
			<< " : "
			<< i->second << '\n';
	}

	//remove the biggest area, which is the background.
	auto it = order.end();
	it--;
	order.erase(it);

	it = order.end();
	it--;
	cout << "vessel: " << it->second << endl;
	int reslabel = it->second;

	img_color = Mat::zeros(img_edge_8s.size(), CV_8UC3);
	for (int y = 0; y < img_color.rows; y++)
		for (int x = 0; x < img_color.cols; x++){
			int label = labels.at<int>(y, x);
			CV_Assert(0 <= label && label <= nccomps);

			if (label == (reslabel - 1)) {

				img_color.at<Vec3b>(y, x) = Vec3b(0, 0, 255);//set red color for the vessel.
			}
			else {
				img_color.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
			}
			
		}
#ifdef DEBUG
	imshow("Labeled map", img_color);
	waitKey();
#endif // DEBUG

	m_mask = img_color;

	Mat togray, resbw;
	cvtColor(img_color, togray, COLOR_BGR2GRAY);
	threshold(togray, resbw, 0, 255, THRESH_BINARY | THRESH_OTSU);
	thinning(resbw);
#ifdef DEBUG
	imshow("centerline:", resbw);
	waitKey();
#endif // DEBUG	
	m_centerline = resbw;

	Mat dstwithcl;
	addWeighted(grayImg, 0.7, resbw, 0.3, 0, dstwithcl);
#ifdef DEBUG
	imshow("centerlinecolor:", dstwithcl);
	waitKey();
#endif // DEBUG

	//imwrite(szOutput, vesselness*255);

	//imwrite(szOutput, resImg);
	return 0;
}

void ImgProcess::thinningIteration(Mat& im, int iter)
{
	Mat marker = Mat::zeros(im.size(), CV_8UC1);

	for (int i = 1; i < im.rows - 1; i++)
	{
		for (int j = 1; j < im.cols - 1; j++)
		{
			uchar p2 = im.at<uchar>(i - 1, j);
			uchar p3 = im.at<uchar>(i - 1, j + 1);
			uchar p4 = im.at<uchar>(i, j + 1);
			uchar p5 = im.at<uchar>(i + 1, j + 1);
			uchar p6 = im.at<uchar>(i + 1, j);
			uchar p7 = im.at<uchar>(i + 1, j - 1);
			uchar p8 = im.at<uchar>(i, j - 1);
			uchar p9 = im.at<uchar>(i - 1, j - 1);

			int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
				(p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
				(p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
				(p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
			int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
			int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
			int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

			if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
				marker.at<uchar>(i, j) = 1;
		}
	}

	im &= ~marker;
}

/**
 * Function for thinning the given binary image
 */
void ImgProcess::thinning(Mat& im)
{
	im /= 255;

	Mat prev = Mat::zeros(im.size(), CV_8UC1);
	Mat diff;

	do {
		thinningIteration(im, 0);
		thinningIteration(im, 1);
		absdiff(im, prev, diff);
		im.copyTo(prev);
	} while (countNonZero(diff) > 0);

	im *= 255;
}
