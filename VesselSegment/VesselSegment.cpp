// VesselSegment.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ImgProcess.h"

int main(int argc, char* argv[])
{
    //std::cout << "Hello World!\n";
    ImgProcess ImgPs;
    Mat boundary;
    //ImgPs.LoadImage(argv[1]);
    ImgPs.LoadImage("..//data//Coronary.png");
    boundary = ImgPs.BoundaryDetction_Frangi();
    //boundary = ImgPs.BoundaryDetction_Canny();
    ImgPs.FindVessel(boundary);

    Mat mask = ImgPs.GetMask();    
    Mat centerline = ImgPs.GetCenterline();
    imwrite("..//data//mask_frangi.png", mask);
    imwrite("..//data//centerline_frangi.png", centerline);


}