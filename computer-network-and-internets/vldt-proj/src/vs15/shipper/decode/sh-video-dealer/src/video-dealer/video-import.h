// video-import.h   29/03/2020
// J_Code
// @brief: 解码器视频导入机函数

#ifndef SH_VIDEO_IMPORT_H
#define SH_VIDEO_IMPORT_H

#define STOP 128	// 鍋滄甯ц鍙栬鍙�
#define CONTINUE 127	// 鏈湁鏁堣鍙栵紝鑰岀户缁抚璇诲彇璁彿
#define LOST 256	// 甯т涪澶辫鍙� 

extern int dealWithVideo(	int fps_need, const char videofilename[], 
							const char datafilename[], const char validfilename[],
							int (*foo)(cv::Mat, int*, int*, const char*, const char*));

#endif