// bitmap-to-mat.cpp   29/03/2020
// J_Code
// @brief: ��Matת���ɱ�����ͳһ��Bitmap����

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <jc.h>
#include <file-dealer.h>
#include "frames_generator.h"
using namespace std;
using namespace cv;

// bitmap至OpenCV Mat 
void bitmapToMat(Mat& mat, Bitmap* bitmap) {
	if (!bitmap || bitmap->channel_count != 4) {
		return;
	}
	// 创建图像 
	mat.create(bitmap->height, bitmap->width, CV_8UC4);
	// 拷贝信息
	memcpy(mat.data, bitmap->pixel, bitmap->width * bitmap->height * 4);
	cvtColor(mat, mat, COLOR_RGBA2BGRA);
}
