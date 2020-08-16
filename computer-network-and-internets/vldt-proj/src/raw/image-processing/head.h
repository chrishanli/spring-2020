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




//分离R、G、B三个通道图像的函数,并二极化输出：
void split( Mat& src, Mat& dst_r, Mat& dst_g, Mat& dst_b, Mat& r_thres, Mat& g_thres, Mat& b_thres, int number);

// 直方图拉伸函数:
void histStretch(Mat& grayImageStretched, const Mat& grayImage, int minValue);

// 提高图像对比度函数：
void kernel(Mat &src,Mat &dst);
