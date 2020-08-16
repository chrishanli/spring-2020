// frame-dealer.cpp    09/03/2020
// J_Code
// @brief: ��������Ƶ����������

#include <iostream>
// OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
// 本工程
#include <jc.h>
#include <jc_finder.h>
#include "file_dealer.h"
#include "video-import.h"

extern "C" {
#include <jc_decoder.h>
} 

#include "../dll.h"

using namespace std;
using namespace cv;


// 处理某个帧
// 返回：STOP : 停止帧读取讯号 | CONTINUE : 未有效读取，而继续帧读取讯号 | LOST : 帧丢失讯号 | SUCCESS : 成功读取讯号
int frameDealer(	Mat frame, int *expected_frame_no, 
					int* total_frame_no, 
					const char datafilename[], const char validfilename[]) {
	// HAN-test显示图像 
	//namedWindow("frame");
	//imshow("frame", frame);
	//waitKey(0);
	//destroyAllWindows();
	// 检测有效性
	if (*total_frame_no != 0 && *expected_frame_no >= *total_frame_no) {
		return STOP;
	}
	// 处理图像
	if (frame.empty()) {
		return CONTINUE;
	}
	J_Found_Symbol* found_ms = importMat_cpp(frame);
	if (!found_ms) {	// 找不到马
		return CONTINUE;
	}
	// 找到马，现在开始解码
	int result = decoder_using_found_ms_main(	found_ms,
												expected_frame_no,
												datafilename,
												validfilename);
	
	// 扔掉阿，宁干啥呢
	clean_foundSymbol(found_ms);
	// 找到头帧了呢，然后更新有关数据
	if (*total_frame_no == 0 && *expected_frame_no == 0 && result == 0) {
<<<<<<< HEAD:src/vs15/shipper/decode/sh-video-dealer/src/video-dealer/frame-dealer.cpp
		// �򿪸մ���ļ�	// ������26.3.2020
		FILE* fp;
		errno_t err = fopen_s(&fp, datafilename, "rb");
		FILE* fp_s;
		errno_t err_s = fopen_s(&fp_s, "tmp.bin", "wb");
		if (err || err_s) {
			cerr << "SH_FATAL_ERROR: File could not access during runtime! Abort all." << endl;
			// kill myself
			exit(255);
		}
=======
		// 打开刚存的文件
		FILE* fp = fopen(datafilename, "rb");
		FILE* fp_s = fopen("tmp.bin", "wb");
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/decoder/video_dealer/frame-dealer.cpp
		
		SH_FileInfo fileinfo;
		fread(&fileinfo, sizeof(fileinfo), 1, fp);
		fclose(fp);
		// 删除首文件
		remove(datafilename);
		// 删除valid文件		21/3/2020
		remove(validfilename);
		// 保存进tmp文件
		fwrite(&fileinfo, sizeof(fileinfo), 1, fp_s);
		fclose(fp_s);
		
		// 更新有关数据
		int pack_mod =  fileinfo.file_size % MAXIMUM_FILE_SIZE;		// 最后一帧的长度 
		int pack_int =  fileinfo.file_size / MAXIMUM_FILE_SIZE;
		int pack_num =  pack_int + 
						(pack_mod ? 1 : 0);
		*total_frame_no = pack_num + 1;
		return SUCCESS;
	}
	// 找到普通帧
	if (result == 0) {
		// Success
		return SUCCESS;
	} else if (result == 255) {
		// Not Detectable
		return CONTINUE;
	} else if (result == -3) {
		// lost
		return LOST;
	} else if (result >= -1) {
		// Failed
		return CONTINUE;
	} else {
		// Not Correspond
		return STOP;
	}
}


int doVideoDecoding(int fps, const char videofilename[], const char datafilename[], const char validfilename[]) {
	return dealWithVideo(fps, videofilename, datafilename, validfilename, frameDealer);
}

