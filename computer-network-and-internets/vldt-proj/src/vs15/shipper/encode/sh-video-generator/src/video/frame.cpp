// frame.cpp  29/03/2020
// J_Code
// @brief: ���ļ���ֱ��һ��һ���ӵ���Ƶ��ȥ 

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <jc.h>
#include <jc_encoder.h>
#include <file-dealer.h>
#include "frames_generator.h"

using namespace std;
using namespace cv;

// 将文件包转换为一系列帧.默认：mat[]数组是已经准备好的，大小不会越界
int convertPackToFrames(SH_Pack* pack, Mat mat[]) {
	int i = 0;
	// 处理每个pack中的数据 
	for (i = 0; i < pack->packs; i++) {
		// 拿到bitmap 
		Bitmap* btmp = encoder_using_J_Data(pack->data[i]);
		if (btmp == NULL) {
			return 0;	// 发生错误 
		}
		// 拿到Mat
		bitmapToMat(mat[i], btmp);
		// 扔掉bitmap
		free(btmp); 
	}
	return 1; 
}

// 将文件包直接一个一个扔到视频中去 
int convertPackToVideo(SH_Pack* pack, const char videoFilename[], int frame_num, int fps, int videoWidth, int videoHeight) {
	int isColor=1;
	int i;
	
	// 打开视频 
	VideoWriter writer;
	while (writer.isOpened() == false) {
		writer.open(videoFilename, -1, fps, Size(videoWidth, videoHeight), isColor);
	}
	
	/* 拿到首帧，以便确定帧大小 */ 
	// 拿到bitmap 
	Bitmap* btmp = encoder_using_J_Data(pack->data[0]);	// 您写i干啥 
	if (btmp == NULL) {
		return 0;	// 发生错误 
	}
	Mat first;
	// 拿到Mat
	bitmapToMat(first, btmp);
	// 扔掉bitmap
	free(btmp); 
	
	// 如果用户没有提供， 就生成照图片的尺寸安排 
	if (videoWidth == 0 || videoHeight == 0) {
		videoWidth = first.size().width;
		videoHeight = first.size().height;
	} 

	// 写入头帧（8倍fps） 
	Mat tmp = imread("./res/startFrame.png");
	if (tmp.empty()) {
		cout << "SH_ERROR: Can't find startFrame.png!" << endl;
	} else {
		if (tmp.size() != Size(videoWidth, videoHeight) ) {
			// 线性插值改变图像大小 
			resize(tmp, tmp, Size(videoWidth, videoHeight), INTER_LINEAR);
		}
		for (i = 0; i < 5; i++) {
			writer << tmp;
		} 
	}

	// 循环写入 
	writer << first;	// 不要忘了写入头帧 
	for (i = 1; i < frame_num; i++) {
		// 拿到bitmap 
		Bitmap* btmp = encoder_using_J_Data(pack->data[i]);
		if (btmp == NULL) {
			return 0;	// 发生错误 
		}
		// 拿到Mat
		bitmapToMat(tmp, btmp);
		// 扔掉bitmap
		free(btmp); 
		if (!tmp.data) {	//判断图片调入是否成功
			cout << "SH_ERROR: Could not load image file of number:" << i << endl;
		}
		// 写入 
		writer << tmp;
		// writer.write(frames[i]);
	}
	
	// 写入尾帧（5倍fps） 
	tmp = imread("./res/endFrame.png");
	if (tmp.empty()) {
		cout << "SH_ERROR: Can't find endFrame.png!" << endl;
	} else {
		if (tmp.size() != Size(videoWidth, videoHeight) ) {
			// 线性插值改变图像大小 
			resize(tmp, tmp, Size(videoWidth, videoHeight), INTER_LINEAR);
		} 
		for (i = 0; i < 5; i++) {
//			writer.write(tmp);
			writer << tmp;
		} 
	}
	
	// 我汗颜了。。。。。 
	writer.release();
	
	cout << "----Output Video Info----" << endl;
	cout << " Path:" << videoFilename << endl
		 << " Size: " << videoWidth << " * " << videoHeight << endl
		 << " FPS: " << fps << endl;
	cout << "-------------------------" << endl;
	
	return 1;
}

