
#include <iostream>

#include <string>

#include "opencv2/highgui/highgui.hpp"

#include "opencv2/core/core.hpp"

#include<Windows.h>
using namespace std;

using namespace cv;



int saveFramesToVideo(string videoFilename, Mat frames[], int frame_num, int fps, int videoWidth, int videoHeight) {

	int isColor=1;
	int i;
	cv::VideoWriter writer;
	writer = VideoWriter(videoFilename,	VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(videoWidth, videoHeight), isColor);


	cout << "info:" << endl
		<< videoFilename << endl
		<< "Size:" << videoWidth << "*" << videoHeight << endl
		<< "fps:" << fps << endl;

	for (i = 0; i < frame_num; i++) {
		if (!frames[i].data)	//判断图片调入是否成功
		{
			cout << "Could not load image file...\n" << endl;
		}
		imshow("image to video",frames[i]);		//写入
		writer.write(frames[i]);
		if (cv::waitKey(30) == 27 || i == 160)
		{
			cout << "按下ESC键" << endl;
			break;
		}		
			
	}
	return 1;
}

int main(int argc, char** argv)
{
	Mat frames[30];
	char image_name[20];
	int fps = 1, videowidth = 480, videoheight = 720;
	string videofilename = "D:\\FFMPEGTEST.avi";
	for (int i = 0; i < 5; i++)
	{
		sprintf_s(image_name, "D:\\PICTURE\\L%d.jpg", i);
		frames[i] = imread(image_name, 1);
	}

	saveFramesToVideo(videofilename, frames, 5, fps, videowidth, videoheight);
	return 0;
}


