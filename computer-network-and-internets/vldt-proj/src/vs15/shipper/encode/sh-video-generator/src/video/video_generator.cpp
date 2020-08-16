// video-generator.cpp  29/03/2020
// J_Code
// @brief: ��������Ƶ���ɻ�
// @author: Zhenlang Li

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <jc.h>
#include <file-dealer.h>
#include "frames_generator.h"
#include "video_generator.h"

using namespace std;
using namespace cv;

int saveFramesToVideo(const char videoFilename[], Mat frames[], int frame_num, int fps, int videoWidth, int videoHeight) {

	int isColor=1;
	int i;
	
	VideoWriter writer;
	while (writer.isOpened() == false) {
		writer.open(videoFilename, -1, fps, Size(videoWidth, videoHeight), isColor);
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
		for (i = 0; i < 3; i++) {
			writer << tmp;
		} 
	}

	// 循环写入 
	for (i = 0; i < frame_num; i++) {
		if (!frames[i].data) {	//判断图片调入是否成功
			cout << "SH_ERROR: Could not load image file of number:" << i << endl;
		}
		if (frames[i].size() != Size(videoWidth, videoHeight)) {
			resize(frames[i], frames[i], Size(videoWidth, videoHeight), INTER_LINEAR);
		}
		writer << frames[i];
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
		 << " fps: " << fps << endl;
	// Add judgement
	if (fps > 10) {
		cout 	<< " WARNING: fps is larger than 10/second, "
				<< "errors may occur VERY frequently in decoding process! "
				<< "However, the error rate may subject to devices." << endl;
	}
	cout << "-------------------------" << endl;
	
	return 1;
}



<<<<<<< HEAD:src/vs15/shipper/encode/sh-video-generator/src/video/video_generator.cpp
// ����֯��������һ���ļ��ڵ�����ת��Ϊ��Ƶ�����ṩ��⼰�ܾ�����
int generateDataVideo(const char filePath[], const char videoPath[], int length, int videoWidth, int videoHeight) {
	// �õ��ļ���
	SH_Pack* pack = fileImport(filePath, length); 
=======
// 总组织函数：将一个文件内的内容转换为视频
int generateDataVideo(const char filePath[], const char videoPath[], int length, int videoWidth, int videoHeight) {
	// 拿到文件包
	SH_Pack* pack = fileImport(filePath); 
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/encoder/video_generator/video_generator.cpp
	
	if (!pack) {
		cout << "SH_ERROR: Encoder: Error occured while dealing with imported file." << endl;
		return 0;
	}
	// 得到文件包讯息，并计算fps 
	int framesCount = pack->packs;
	int fps = (3 + framesCount + 3) / (length / 1000);
	// 限制：fps最大为28！ 
	if (fps > 28) fps = 28;
	
	// 开始写入视频
	cout << "SH_INFO: Encoder: Writing Video... " << endl;

	convertPackToVideo(pack, videoPath, pack->packs, fps, videoWidth, videoHeight);
	// 程序完成 
	destroyFilePack(pack);
	cout << "SH_INFO: Encoder: Output complete." << endl;
	// 自行析构Mat[]
	return 1; 
}
