#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2\imgproc\types_c.h>
using namespace cv;
using namespace std;
typedef unsigned char BYTE;

typedef struct {
    int	width;
    int	height;
    int bits_per_pixel;//默认为32
    int bits_per_channel;//默认为8
    int channel_count;//默认为4
    BYTE pixel[];
} Bitmap;

/*
    使用说明：
    Bitmap格式转Mat函数
    BitmapConverseMat()
    参数1：const Bitmap bitImg是Bitmap结构体；
    参数2：Mat matImg是转换成的Mat对象；
*/
void BitmapConverseMat(const Bitmap bitImg,Mat matImg){
    
    //根据Bitmap对象的宽和高创建一个matImgRGB三通道对象：
    matImg = Mat::zeros(bitImg.height, bitImg.width, CV_8UC3);
    
    int rows = bitImg.height;
    int cols = bitImg.width;
    int index=0;//将bitImg像素下标置为0；
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
           
            matImg.at<Vec3b>(row, col)[2] = bitImg.pixel[index];    //Mat对象：B、G、R<——>Bitmap对象：R、G、B、A
            index++;
            matImg.at<Vec3b>(row, col)[1] = bitImg.pixel[index];
            index++;
            matImg.at<Vec3b>(row, col)[0] = bitImg.pixel[index];
            index += 2;//自动跳过A通道赋值。
             
        }
    }


}

/*
    使用说明：
    Mat格式转Bitmap函数
    MatConverseBitmap()
    参数1：const Mat matImg是Mat类对象；
    参数2：Bitmap bitImg是转换成的Bitmap结构体；
*/
void MatConverseBitmap(const Mat matImg, Bitmap bitImg) {
    bitImg.width = matImg.cols;
    bitImg.height = matImg.rows;
    int index = 0;
    int rows = bitImg.height;
    int cols = bitImg.width;
    //将bitImg像素下标置为0；
    int index = 0;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {

            bitImg.pixel[index] = matImg.at<Vec3b>(row, col)[2];   //Mat对象：B、G、R<——>Bitmap对象：R、G、B、A
            index++;
            bitImg.pixel[index] = matImg.at<Vec3b>(row, col)[1];  
            index++;
            bitImg.pixel[index] = matImg.at<Vec3b>(row, col)[0];
            index += 2;//自动跳过A通道赋值。
        }
    }
   
}