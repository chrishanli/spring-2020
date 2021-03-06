// frame-dealer.cpp    09/03/2020
// J_Code
// @brief: 解码器视频处理机函数

#include <iostream>
// OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
// 鏈伐绋�
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


// 澶勭悊鏌愪釜甯�
// 杩斿洖锛歋TOP : 鍋滄甯ц鍙栬鍙� | CONTINUE : 鏈湁鏁堣鍙栵紝鑰岀户缁抚璇诲彇璁彿 | LOST : 甯т涪澶辫鍙� | SUCCESS : 鎴愬姛璇诲彇璁彿
int frameDealer(	Mat frame, int *expected_frame_no, 
					int* total_frame_no, 
					const char datafilename[], const char validfilename[]) {
	// HAN-test鏄剧ず鍥惧儚 
	//namedWindow("frame");
	//imshow("frame", frame);
	//waitKey(0);
	//destroyAllWindows();
	// 妫�娴嬫湁鏁堟��
	if (*total_frame_no != 0 && *expected_frame_no >= *total_frame_no) {
		return STOP;
	}
	// 澶勭悊鍥惧儚
	if (frame.empty()) {
		return CONTINUE;
	}
	J_Found_Symbol* found_ms = importMat_cpp(frame);
	if (!found_ms) {	// 鎵句笉鍒伴┈
		return CONTINUE;
	}
	// 鎵惧埌椹紝鐜板湪寮�濮嬭В鐮�
	int result = decoder_using_found_ms_main(	found_ms,
												expected_frame_no,
												datafilename,
												validfilename);
	
	// 鎵旀帀闃匡紝瀹佸共鍟ュ憿
	clean_foundSymbol(found_ms);
	// 鎵惧埌澶村抚浜嗗憿锛岀劧鍚庢洿鏂版湁鍏虫暟鎹�
	if (*total_frame_no == 0 && *expected_frame_no == 0 && result == 0) {
<<<<<<< HEAD:src/vs15/shipper/decode/sh-video-dealer/src/video-dealer/frame-dealer.cpp
		// 打开刚存的文件	// 更新于26.3.2020
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
		// 鎵撳紑鍒氬瓨鐨勬枃浠�
		FILE* fp = fopen(datafilename, "rb");
		FILE* fp_s = fopen("tmp.bin", "wb");
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/decoder/video_dealer/frame-dealer.cpp
		
		SH_FileInfo fileinfo;
		fread(&fileinfo, sizeof(fileinfo), 1, fp);
		fclose(fp);
		// 鍒犻櫎棣栨枃浠�
		remove(datafilename);
		// 鍒犻櫎valid鏂囦欢		21/3/2020
		remove(validfilename);
		// 淇濆瓨杩泃mp鏂囦欢
		fwrite(&fileinfo, sizeof(fileinfo), 1, fp_s);
		fclose(fp_s);
		
		// 鏇存柊鏈夊叧鏁版嵁
		int pack_mod =  fileinfo.file_size % MAXIMUM_FILE_SIZE;		// 鏈�鍚庝竴甯х殑闀垮害 
		int pack_int =  fileinfo.file_size / MAXIMUM_FILE_SIZE;
		int pack_num =  pack_int + 
						(pack_mod ? 1 : 0);
		*total_frame_no = pack_num + 1;
		return SUCCESS;
	}
	// 鎵惧埌鏅�氬抚
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

