// video-import.h   29/03/2020
// J_Code
// @brief: ��������Ƶ���������

#ifndef SH_VIDEO_IMPORT_H
#define SH_VIDEO_IMPORT_H

#define STOP 128	// 停止帧读取讯号
#define CONTINUE 127	// 未有效读取，而继续帧读取讯号
#define LOST 256	// 帧丢失讯号 

extern int dealWithVideo(	int fps_need, const char videofilename[], 
							const char datafilename[], const char validfilename[],
							int (*foo)(cv::Mat, int*, int*, const char*, const char*));

#endif