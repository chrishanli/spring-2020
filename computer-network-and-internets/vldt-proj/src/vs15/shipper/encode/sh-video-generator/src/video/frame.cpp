// frame.cpp  29/03/2020
// J_Code
// @brief: 将文件包直接一个一个扔到视频中去 

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

// 灏嗘枃浠跺寘杞崲涓轰竴绯诲垪甯�.榛樿锛歮at[]鏁扮粍鏄凡缁忓噯澶囧ソ鐨勶紝澶у皬涓嶄細瓒婄晫
int convertPackToFrames(SH_Pack* pack, Mat mat[]) {
	int i = 0;
	// 澶勭悊姣忎釜pack涓殑鏁版嵁 
	for (i = 0; i < pack->packs; i++) {
		// 鎷垮埌bitmap 
		Bitmap* btmp = encoder_using_J_Data(pack->data[i]);
		if (btmp == NULL) {
			return 0;	// 鍙戠敓閿欒 
		}
		// 鎷垮埌Mat
		bitmapToMat(mat[i], btmp);
		// 鎵旀帀bitmap
		free(btmp); 
	}
	return 1; 
}

// 灏嗘枃浠跺寘鐩存帴涓�涓竴涓墧鍒拌棰戜腑鍘� 
int convertPackToVideo(SH_Pack* pack, const char videoFilename[], int frame_num, int fps, int videoWidth, int videoHeight) {
	int isColor=1;
	int i;
	
	// 鎵撳紑瑙嗛 
	VideoWriter writer;
	while (writer.isOpened() == false) {
		writer.open(videoFilename, -1, fps, Size(videoWidth, videoHeight), isColor);
	}
	
	/* 鎷垮埌棣栧抚锛屼互渚跨‘瀹氬抚澶у皬 */ 
	// 鎷垮埌bitmap 
	Bitmap* btmp = encoder_using_J_Data(pack->data[0]);	// 鎮ㄥ啓i骞插暐 
	if (btmp == NULL) {
		return 0;	// 鍙戠敓閿欒 
	}
	Mat first;
	// 鎷垮埌Mat
	bitmapToMat(first, btmp);
	// 鎵旀帀bitmap
	free(btmp); 
	
	// 濡傛灉鐢ㄦ埛娌℃湁鎻愪緵锛� 灏辩敓鎴愮収鍥剧墖鐨勫昂瀵稿畨鎺� 
	if (videoWidth == 0 || videoHeight == 0) {
		videoWidth = first.size().width;
		videoHeight = first.size().height;
	} 

	// 鍐欏叆澶村抚锛�8鍊峟ps锛� 
	Mat tmp = imread("./res/startFrame.png");
	if (tmp.empty()) {
		cout << "SH_ERROR: Can't find startFrame.png!" << endl;
	} else {
		if (tmp.size() != Size(videoWidth, videoHeight) ) {
			// 绾挎�ф彃鍊兼敼鍙樺浘鍍忓ぇ灏� 
			resize(tmp, tmp, Size(videoWidth, videoHeight), INTER_LINEAR);
		}
		for (i = 0; i < 5; i++) {
			writer << tmp;
		} 
	}

	// 寰幆鍐欏叆 
	writer << first;	// 涓嶈蹇樹簡鍐欏叆澶村抚 
	for (i = 1; i < frame_num; i++) {
		// 鎷垮埌bitmap 
		Bitmap* btmp = encoder_using_J_Data(pack->data[i]);
		if (btmp == NULL) {
			return 0;	// 鍙戠敓閿欒 
		}
		// 鎷垮埌Mat
		bitmapToMat(tmp, btmp);
		// 鎵旀帀bitmap
		free(btmp); 
		if (!tmp.data) {	//鍒ゆ柇鍥剧墖璋冨叆鏄惁鎴愬姛
			cout << "SH_ERROR: Could not load image file of number:" << i << endl;
		}
		// 鍐欏叆 
		writer << tmp;
		// writer.write(frames[i]);
	}
	
	// 鍐欏叆灏惧抚锛�5鍊峟ps锛� 
	tmp = imread("./res/endFrame.png");
	if (tmp.empty()) {
		cout << "SH_ERROR: Can't find endFrame.png!" << endl;
	} else {
		if (tmp.size() != Size(videoWidth, videoHeight) ) {
			// 绾挎�ф彃鍊兼敼鍙樺浘鍍忓ぇ灏� 
			resize(tmp, tmp, Size(videoWidth, videoHeight), INTER_LINEAR);
		} 
		for (i = 0; i < 5; i++) {
//			writer.write(tmp);
			writer << tmp;
		} 
	}
	
	// 鎴戞睏棰滀簡銆傘�傘�傘�傘�� 
	writer.release();
	
	cout << "----Output Video Info----" << endl;
	cout << " Path:" << videoFilename << endl
		 << " Size: " << videoWidth << " * " << videoHeight << endl
		 << " FPS: " << fps << endl;
	cout << "-------------------------" << endl;
	
	return 1;
}

