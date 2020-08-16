#include "head.h"


//可以分离R、G、B三个通道图像的函数,并二极化输出：
/*
使用说明：
第一个参数src    是要进行RGB分离的图像；
第二个参数dst_r  是分离出的R通道图像；
第三个参数dst_g  是分离出的G通道图像；
第四个参数dst_b  是分离出的B通道图像；
第五个参数r_thres是对R通道图像进行二极化后的图像；
第六个参数g_thres是对G通道图像进行二极化后的图像；
第七个参数b_thres是对B通道图像进行二极化后的图像；
第八个参数type   是对图像进行二极化的类型选项，默认为0，一般二极化；
*/
void split(Mat& src,Mat& dst_r,Mat& dst_g,Mat& dst_b, Mat& r_thres,Mat& g_thres, Mat& b_thres,int number) {
    //自定义阈值：
    const int thres = 127;
    //设置阈值类型选项(默认为number=0，阈值二极化）
   //0,1,2,3,4,5,6,7
    int type = THRESH_BINARY;
    switch (number) {
    case 0:type = THRESH_BINARY; break;         //阈值二极化(大于阈值的部分被置为255，小于部分被置为0)；
    case 1:type = THRESH_BINARY_INV; break;     //阈值反二极化(大于阈值部分被置为0，小于部分被置为255)；
    case 2:type = THRESH_TRUNC; break;          //大于阈值部分被置为threshold，小于部分保持原样;
    case 3:type = THRESH_TOZERO; break;         //小于阈值部分被置为0，大于部分保持不变;
    case 4:type = THRESH_TOZERO_INV; break;     //大于阈值部分被置为0，小于部分保持不变 ;
    case 5:type = THRESH_MASK; break;
    case 6:type = THRESH_OTSU; break;           //系统根据图像自动选择最优阈值，并进行二极化；
    case 7:type = THRESH_TRIANGLE; break;       //系统根据图像自动选择最优阈值，并进行二极化；

    }
    Mat dst;
    cvtColor(src, dst, CV_BGR2GRAY);
    dst_b.create(dst.size(), dst.type());
    dst_g.create(dst.size(), dst.type());
    dst_r.create(dst.size(), dst.type());
    int rows = src.rows;
    int cols = src.cols;
    int channel = src.channels();
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {

            dst_b.at<uchar>(row, col) = src.at<Vec3b>(row, col)[0];
            dst_g.at<uchar>(row, col) = src.at<Vec3b>(row, col)[1];
            dst_r.at<uchar>(row, col) = src.at<Vec3b>(row, col)[2];


        }
    }
    threshold(dst_b, b_thres, thres, 255, type);
    threshold(dst_g, g_thres, thres, 255, type);
    threshold(dst_r, r_thres, thres, 255, type);
}


// 直方图拉伸函数
/*
grayImageStretched - 拉伸后得到的图像；
grayImage - 要进行直方图拉伸的图像；
minValue - 忽略像数个数小于此值的灰度级；
*/
void histStretch(Mat& grayImageStretched, const Mat& grayImage, int minValue)
{
    Mat histImg;
    int channels[1] = { 0 };
    int histSize = 256;
    float range[2] = { 0, 256 };
    const float* ranges[1] = { range };
    calcHist(&grayImage, 1, channels, Mat(), histImg, 1, &histSize, ranges);
    CV_Assert(!grayImageStretched.empty() && grayImageStretched.channels() == 1 && grayImageStretched.depth() == CV_8U);
    CV_Assert(!histImg.empty() && histImg.rows == 256 && histImg.cols == 1 && histImg.depth() == CV_32F);
    CV_Assert(minValue >= 0);
    // 求左边界
    uchar grayMin = 0;
    for (int i = 0; i < histImg.rows; ++i)
    {
        if (histImg.at<float>(i, 0) > minValue)
        {
            grayMin = static_cast<uchar>(i);
            break;
        }
    }

    // 求右边界
    uchar grayMax = 0;
    for (int i = histImg.rows - 1; i >= 0; --i)
    {
        if (histImg.at<float>(i, 0) > minValue)
        {
            grayMax = static_cast<uchar>(i);
            break;
        }
    }

    if (grayMin >= grayMax)
    {
        return;
    }

    const int w = grayImageStretched.cols;
    const int h = grayImageStretched.rows;
    for (int y = 0; y < h; ++y)
    {
        uchar* imageData = grayImageStretched.ptr<uchar>(y);
        for (int x = 0; x < w; ++x)
        {
            if (imageData[x] < grayMin)
            {
                imageData[x] = 0;
            }
            else if (imageData[x] > grayMax)
            {
                imageData[x] = 255;
            }
            else
            {
                imageData[x] = static_cast<uchar>(round((imageData[x] - grayMin) * 255.0 / (grayMax - grayMin)));
            }
        }
    }
}


//提高各像素间对比度函数：
//src:原图；
//dst:处理后的图片；
//void kernel(Mat& src, Mat& dst) {
//    Mat dst;
//    Mat Kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);//局部变量
//    filter2D(src, dst, src.depth(), Kernel);
//}
