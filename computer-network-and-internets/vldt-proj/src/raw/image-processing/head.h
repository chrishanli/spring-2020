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
    int bits_per_pixel;//Ĭ��Ϊ32
    int bits_per_channel;//Ĭ��Ϊ8
    int channel_count;//Ĭ��Ϊ4
    BYTE pixel[];
} Bitmap;




//����R��G��B����ͨ��ͼ��ĺ���,�������������
void split( Mat& src, Mat& dst_r, Mat& dst_g, Mat& dst_b, Mat& r_thres, Mat& g_thres, Mat& b_thres, int number);

// ֱ��ͼ���캯��:
void histStretch(Mat& grayImageStretched, const Mat& grayImage, int minValue);

// ���ͼ��ԱȶȺ�����
void kernel(Mat &src,Mat &dst);
