// image.cpp    03/03/2020
// J_Code
// brief: J_Code image processing functions
// Miao Xinyu

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

#include "../dll.h"
#include "detector.h"
#include "jc_finder_cpp.h"


using namespace cv;
using namespace std;


/**
 可以分离R、G、B三个通道图像的函数,并二极化输出

 @param src 原图（Mat）
 @param rgb_thres 存放2极化后的 3个通道 的数组
 @param number 选用的二级化种类，默认为0
 */
void split_thres(Mat& src, Mat rgb_thres[], int number = 0) {
    //自定义阈值：
    const int thres = 175;
    //设置阈值类型选项(默认为number=0，阈值二极化）
    //0,1,2,3,4,5,6,7
    int type = THRESH_BINARY | THRESH_OTSU;
    switch (number) {
        case 0:type = THRESH_BINARY; break;         //阈值二极化(大于阈值的部分被置为255，小于部分被置为0)；
        case 1:type = THRESH_BINARY_INV; break;     //阈值反二极化(大于阈值部分被置为0，小于部分被置为255)；
        case 2:type = THRESH_TRUNC; break;          //大于阈值部分被置为threshold，小于部分保持原样;
        case 3:type = THRESH_TOZERO; break;         //小于阈值部分被置为0，大于部分保持不变;
        case 4:type = THRESH_TOZERO_INV; break;     //大于阈值部分被置为0，小于部分保持不变 ;
        case 5:type = THRESH_MASK; break;
        case 6:type = THRESH_OTSU; break;           //系统根据图像自动选择最优阈值，并进行二极化；
            // case 7:type = THRESH_TRIANGLE; break;       //系统根据图像自动选择最优阈值，并进行二极化；
    }
    
    // 分割3个通道
    split(src, rgb_thres);
    
    // 先直方图拉伸
    void histStretch(Mat& grayImage, int minValue);
    histStretch(rgb_thres[0], 20);
    histStretch(rgb_thres[1], 20);
    histStretch(rgb_thres[2], 20);
    // 再二值化
    threshold(rgb_thres[0], rgb_thres[0], thres, 255, type);
    threshold(rgb_thres[1], rgb_thres[1], thres, 255, type);
    threshold(rgb_thres[2], rgb_thres[2], thres, 255, type);
    
}


// 直方图拉伸函数
/*
 grayImageStretched - 拉伸后得到的图像；
 grayImage - 要进行直方图拉伸的图像；
 minValue - 忽略像数个数小于此值的灰度级；
 */
void histStretch(Mat& grayImage, int minValue)
{
    // grayImageStretched.create(grayImage.size(), grayImage.type());
    Mat histImg;
    
    int channels[1] = { 0 };
    int histSize = 256;
    float range[2] = { 0, 256 };
    const float* ranges[1] = { range };
    calcHist(&grayImage, 1, channels, Mat(), histImg, 1, &histSize, ranges);
    //    CV_Assert(!grayImageStretched.empty() && grayImageStretched.channels() == 1 && grayImageStretched.depth() == CV_8U);
    //    CV_Assert(!histImg.empty() && histImg.rows == 256 && histImg.cols == 1 && histImg.depth() == CV_32F);
    //    CV_Assert(minValue >= 0);
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
    
    const int w = grayImage.cols;
    const int h = grayImage.rows;
    for (int y = 0; y < h; ++y)
    {
        uchar* imageData = grayImage.ptr<uchar>(y);
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



/**
 将Mat格式的图像转化为Bitmap

 @param mat Mat格式的图像
 @return Bitmap图像指针
 */
Bitmap* matToBitmap(Mat& mat) {
    Size size = mat.size();
    Bitmap *bitmap = NULL;
    
    if (mat.channels() == 3) {
        Mat destMat;
        cvtColor(mat, destMat, CV_BGR2RGBA);
        int bitmap_len = size.width * size.height;
		bitmap = (Bitmap *)malloc(sizeof(Bitmap) + 4 * bitmap_len);
        // 映射bitmap内存
        memcpy(bitmap->pixel, destMat.data, 4 * bitmap_len);
        bitmap->channel_count = BITMAP_CHANNEL_COUNT;
    } else if (mat.channels() == 4) {
    	Mat destMat;
        cvtColor(mat, destMat, CV_BGRA2RGBA);
        int bitmap_len = size.width * size.height;
		bitmap = (Bitmap *)malloc(sizeof(Bitmap) + 4 * bitmap_len);
        // 映射bitmap内存
        memcpy(bitmap->pixel, destMat.data, 4 * bitmap_len);
        bitmap->channel_count = BITMAP_CHANNEL_COUNT;
	} else {
        // 设为黑白图
        int bitmap_len = size.width * size.height;
        bitmap = (Bitmap *)malloc(sizeof(Bitmap) + bitmap_len);
        // 搬运图像内存内容
        memcpy(bitmap->pixel, mat.data, bitmap_len);
        bitmap->channel_count = 1;
    }
    
    (bitmap)->width = size.width;
    (bitmap)->height = size.height;
    (bitmap)->bits_per_pixel = BITMAP_BITS_PER_PIXEL;
    (bitmap)->bits_per_channel = BITMAP_BITS_PER_CHANNEL;
    
    return bitmap;
}


/**
 对一个Mat对象进行预处理、透视变换、采样

 @param img 原图（Mat）
 @return 采样后的Symbol
 */
J_Found_Symbol* image_processing(Mat& img) {
    Mat rgb_thres[3];
    split_thres(img, rgb_thres);

    Mat img_after_thres;
    merge(rgb_thres, 3, img_after_thres);
    
    Bitmap* bitmap = matToBitmap(img_after_thres);
    Bitmap* ch[3];
    ch[0] = matToBitmap(rgb_thres[2]);
    ch[1] = matToBitmap(rgb_thres[1]);
    ch[2] = matToBitmap(rgb_thres[0]);
    
#if TEST_MODE
    saveImage(bitmap, "bitmap.png");
    saveImage(ch[0], "ch_r.png");
    saveImage(ch[1], "ch_g.png");
    saveImage(ch[2], "ch_b.png");
    
    J_Found_Symbol* found = detectMaster(bitmap, ch);
    saveImage(found->sampled_img, "sampled_fin.png");
    return found;
#endif
    J_Found_Symbol* found_smb = detectMaster(bitmap, ch);
    // 把原图的bitmap和ch删掉
    return found_smb;
}


/**
 入口函数

 @param src 存图片的地址
 @return 返回一个Found_Symbol，要用clear_found_symbol清除
 */
J_Found_Symbol* importImage_cpp(const char src[]) {
    Mat img = imread(src);
    if (img.empty()) {
    	cout << "FATAL ERROR: " << src << ": No such file or directory." << endl;
        return NULL;
    }
    return image_processing(img);
}


J_Found_Symbol* importMat_cpp(Mat& src) {
	if (src.empty()) {
        return NULL;
	}
	// HAN TEST
	// cout << "INFO: Mat imported." << endl;
	return image_processing(src);
}

/**
 清除Found_Symbol

 @param found_symbol Found_Symbol
 */
void clean_foundSymbol_cpp(J_Found_Symbol* found_symbol) {
    if (!found_symbol) return;
    else {
        free(found_symbol->fps);
        free(found_symbol->sampled_img);
        free(found_symbol->bitmap);
		for (int i = 0; i < 3; i++) free(found_symbol->ch[i]);
		free(found_symbol);
    }
}

void swap(Int32* a, Int32* b) {
    Int32 tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * @brief Get the min, middle and max value of three values and the corresponding indexes
 * @param rgb the pixel with RGB values
 * @param min the min value
 * @param mid the middle value
 * @param max the max value
 */
void getRGBMinMax_cpp(BYTE* rgb, BYTE* min, BYTE* mid, BYTE* max, Int32* index_min, Int32* index_mid, Int32* index_max)
{
    *index_min = 0;
    *index_mid = 1;
    *index_max = 2;
    if(rgb[*index_min] > rgb[*index_max])
        swap(index_min, index_max);
    if(rgb[*index_min] > rgb[*index_mid])
        swap(index_min, index_mid);
    if(rgb[*index_mid] > rgb[*index_max])
        swap(index_mid, index_max);
    *min = rgb[*index_min];
    *mid = rgb[*index_mid];
    *max = rgb[*index_max];
}

/**
 * @brief 获取一块像素的RGB的平均值及RGB的方差 (Get the average and variance of RGB values)
 * @param rgb 像素的RGB值
 * @param ave 平均值
 * @param var 方差
 */
void getRGBAveVar_cpp(BYTE* rgb, Double* ave, Double* var) {
    // 计算均值
    *ave = (rgb[0] + rgb[1] + rgb[2]) / 3;
    // 计算方差
    Double sum = 0.0;
    for(Int32 i = 0; i < 3; i++) {
        sum += (rgb[i] - (*ave)) * (rgb[i] - (*ave));
    }
    *var = sum / 3;
}


