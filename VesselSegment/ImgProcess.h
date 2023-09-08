#pragma once


#include <string.h>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <cstring>
#include <cstdlib>
#include <vector>
#include "frangi.h"

using namespace cv;
using namespace std;

class ImgProcess
{
public:
			
	int LoadImage(string szInput);
	int BoundaryDetction_Canny();
	int BoundaryDetction_Frangi();
	Mat GetMask() {
		return m_mask;
	};
	Mat GetCenterline(){
		return m_centerline;
	};

protected:
	void thinning(Mat& im);
	void thinningIteration(Mat& im, int iter);
	Mat  CannyThreshold(Mat src, int lowThreshold);

private:
	Mat m_Img;
	Mat m_mask;
	Mat m_centerline;
		
};

