#include "head.h"

//����ͨ����������ע��ͼƬ·����Ҫ�����ɱ���ͼƬ·����


int main(int argc, char** argv) {
    Mat src = imread("C:\\Users\\40962\\Pictures\\���о�ѵ.jpg");   //����ԭͼ
   
	if (src.empty()) {
		printf("can not load image...");
		return -1;
	}
    imshow("ԭͼ��", src);     //��ʾԭͼ

/*
���������ǲ���RGBͼ����ͨ�����뺯��split()���֣�
*/
    Mat dst_b, dst_g, dst_r;
    Mat r_thres, g_thres, b_thres;
    split(src, dst_r, dst_g, dst_b, r_thres, g_thres, b_thres,0);

    //��ʾ������ͼ��
    imshow("�����Rͨ��ͼ��", dst_r);
    imshow("�����Gͨ��ͼ��", dst_g);
    imshow("�����Bͨ��ͼ��", dst_b);
    imshow("��������Rͨ��ͼ��", r_thres);
    imshow("��������Gͨ��ͼ��", g_thres);
    imshow("��������Bͨ��ͼ��", b_thres);

    //�ѷ�����ͼ��д��ָ����·����
    imwrite("C:\\Users\\40962\\Pictures\\�����Rͨ��ͼ��.jpg", dst_r);
    imwrite("C:\\Users\\40962\\Pictures\\�����Gͨ��ͼ��.jpg", dst_g);
    imwrite("C:\\Users\\40962\\Pictures\\�����Bͨ��ͼ��.jpg", dst_b);
    imwrite("C:\\Users\\40962\\Pictures\\��������Rͨ��ͼ��.jpg", r_thres);
    imwrite("C:\\Users\\40962\\Pictures\\��������Gͨ��ͼ��.jpg", g_thres);
    imwrite("C:\\Users\\40962\\Pictures\\��������Bͨ��ͼ��.jpg", b_thres);
    
/*
���������ǲ���ֱ��ͼ���캯��histStretch���֣�
*/

    

// ����ͼ�񣬴�ʱ��3ͨ����RGBͼ��
    Mat image = imread("C:\\Users\\40962\\Pictures\\MambaForever.jpg");
    if (image.empty())
    {
        return -1;
    }
     
    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);// ת��Ϊ��ͨ���ĻҶ�ͼ
    Mat grayImageStretched = grayImage.clone();
    histStretch(grayImageStretched, grayImage, 128);
    imshow("ֱ��ͼ�����", grayImageStretched);

/*
������������ߵ�/��ͨ��ͼ��ԱȶȺ���kernel( )���֣�
*/
    Mat Img = imread("C:\\Users\\40962\\Pictures\\test1.jpg");
    Mat dst;
    Mat Kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);//�ֲ�����
    filter2D(Img, dst, src.depth(), Kernel);
    imshow("ԭͼ��", Img);
    imshow("�����", dst);
    waitKey(0);
    return 0;
}