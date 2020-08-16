#include "head.h"

//测试通过样例：（注：图片路径需要更换成本地图片路径）


int main(int argc, char** argv) {
    Mat src = imread("C:\\Users\\40962\\Pictures\\高中军训.jpg");   //读入原图
   
	if (src.empty()) {
		printf("can not load image...");
		return -1;
	}
    imshow("原图：", src);     //显示原图

/*
··下面是测试RGB图像三通道分离函数split()部分：
*/
    Mat dst_b, dst_g, dst_r;
    Mat r_thres, g_thres, b_thres;
    split(src, dst_r, dst_g, dst_b, r_thres, g_thres, b_thres,0);

    //显示分离后的图像：
    imshow("分离后R通道图像：", dst_r);
    imshow("分离后G通道图像：", dst_g);
    imshow("分离后B通道图像：", dst_b);
    imshow("二极化后R通道图像：", r_thres);
    imshow("二极化后G通道图像：", g_thres);
    imshow("二极化后B通道图像：", b_thres);

    //把分离后的图像写入指定的路径：
    imwrite("C:\\Users\\40962\\Pictures\\分离后R通道图像.jpg", dst_r);
    imwrite("C:\\Users\\40962\\Pictures\\分离后G通道图像.jpg", dst_g);
    imwrite("C:\\Users\\40962\\Pictures\\分离后B通道图像.jpg", dst_b);
    imwrite("C:\\Users\\40962\\Pictures\\二级化后R通道图像.jpg", r_thres);
    imwrite("C:\\Users\\40962\\Pictures\\二级化后G通道图像.jpg", g_thres);
    imwrite("C:\\Users\\40962\\Pictures\\二级化后B通道图像.jpg", b_thres);
    
/*
··下面是测试直方图拉伸函数histStretch部分：
*/

    

// 读入图像，此时是3通道的RGB图像
    Mat image = imread("C:\\Users\\40962\\Pictures\\MambaForever.jpg");
    if (image.empty())
    {
        return -1;
    }
     
    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);// 转换为单通道的灰度图
    Mat grayImageStretched = grayImage.clone();
    histStretch(grayImageStretched, grayImage, 128);
    imshow("直方图拉伸后：", grayImageStretched);

/*
··下面是提高单/三通道图像对比度函数kernel( )部分：
*/
    Mat Img = imread("C:\\Users\\40962\\Pictures\\test1.jpg");
    Mat dst;
    Mat Kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);//局部变量
    filter2D(Img, dst, src.depth(), Kernel);
    imshow("原图：", Img);
    imshow("处理后：", dst);
    waitKey(0);
    return 0;
}