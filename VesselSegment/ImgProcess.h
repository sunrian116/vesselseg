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
		Mat GetMask();
		Mat GetCenterline();

	private:
		Mat m_Img;
		Mat m_mask;
		Mat m_centerline;


		void thinning(Mat& im);
		void thinningIteration(Mat& im, int iter);
		
};

