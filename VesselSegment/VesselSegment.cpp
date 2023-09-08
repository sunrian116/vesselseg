// VesselSegment.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ImgProcess.h"

int main(int argc, char* argv[])
{
    //std::cout << "Hello World!\n";
    ImgProcess ImgPs;
    //ImgPs.LoadImage(argv[1]);
    ImgPs.LoadImage("..//data//Coronary.png");
    ImgPs.BoundaryDetction_Frangi();
    //ImgPs.BoundaryDetction_Canny();

    Mat mask = ImgPs.GetMask();    
    Mat centerline = ImgPs.GetCenterline();
    imwrite("..//data//mask.png", mask);
    imwrite("..//data//centerline.png", centerline);


}