#pragma once
#include<opencv2/opencv.hpp>
#include<iostream>

using namespace std;
using namespace cv;

class EdgeDetection
{
    cv::Mat m_img;
    cv::Mat m_canny;
public:
    string FilePath = "C:\\Users\\40962\\Pictures\\";
    EdgeDetection(cv::Mat iamge);
    bool cannyProcess(unsigned int downThreshold, unsigned int upThreshold);    //进行边缘处理的函数；
    bool getContours();                                                         //进行矩形轮廓顶点确定，并分离出矩形图片；
    ~EdgeDetection();                                                           
};
