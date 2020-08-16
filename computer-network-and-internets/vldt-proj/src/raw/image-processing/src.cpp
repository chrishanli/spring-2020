#include "head.h"


//���Է���R��G��B����ͨ��ͼ��ĺ���,�������������
/*
ʹ��˵����
��һ������src    ��Ҫ����RGB�����ͼ��
�ڶ�������dst_r  �Ƿ������Rͨ��ͼ��
����������dst_g  �Ƿ������Gͨ��ͼ��
���ĸ�����dst_b  �Ƿ������Bͨ��ͼ��
���������r_thres�Ƕ�Rͨ��ͼ����ж��������ͼ��
����������g_thres�Ƕ�Gͨ��ͼ����ж��������ͼ��
���߸�����b_thres�Ƕ�Bͨ��ͼ����ж��������ͼ��
�ڰ˸�����type   �Ƕ�ͼ����ж�����������ѡ�Ĭ��Ϊ0��һ���������
*/
void split(Mat& src,Mat& dst_r,Mat& dst_g,Mat& dst_b, Mat& r_thres,Mat& g_thres, Mat& b_thres,int number) {
    //�Զ�����ֵ��
    const int thres = 127;
    //������ֵ����ѡ��(Ĭ��Ϊnumber=0����ֵ��������
   //0,1,2,3,4,5,6,7
    int type = THRESH_BINARY;
    switch (number) {
    case 0:type = THRESH_BINARY; break;         //��ֵ������(������ֵ�Ĳ��ֱ���Ϊ255��С�ڲ��ֱ���Ϊ0)��
    case 1:type = THRESH_BINARY_INV; break;     //��ֵ��������(������ֵ���ֱ���Ϊ0��С�ڲ��ֱ���Ϊ255)��
    case 2:type = THRESH_TRUNC; break;          //������ֵ���ֱ���Ϊthreshold��С�ڲ��ֱ���ԭ��;
    case 3:type = THRESH_TOZERO; break;         //С����ֵ���ֱ���Ϊ0�����ڲ��ֱ��ֲ���;
    case 4:type = THRESH_TOZERO_INV; break;     //������ֵ���ֱ���Ϊ0��С�ڲ��ֱ��ֲ��� ;
    case 5:type = THRESH_MASK; break;
    case 6:type = THRESH_OTSU; break;           //ϵͳ����ͼ���Զ�ѡ��������ֵ�������ж�������
    case 7:type = THRESH_TRIANGLE; break;       //ϵͳ����ͼ���Զ�ѡ��������ֵ�������ж�������

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


// ֱ��ͼ���캯��
/*
grayImageStretched - �����õ���ͼ��
grayImage - Ҫ����ֱ��ͼ�����ͼ��
minValue - ������������С�ڴ�ֵ�ĻҶȼ���
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
    // ����߽�
    uchar grayMin = 0;
    for (int i = 0; i < histImg.rows; ++i)
    {
        if (histImg.at<float>(i, 0) > minValue)
        {
            grayMin = static_cast<uchar>(i);
            break;
        }
    }

    // ���ұ߽�
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


//��߸����ؼ�ԱȶȺ�����
//src:ԭͼ��
//dst:������ͼƬ��
//void kernel(Mat& src, Mat& dst) {
//    Mat dst;
//    Mat Kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);//�ֲ�����
//    filter2D(src, dst, src.depth(), Kernel);
//}
