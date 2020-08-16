// video-import.h   29/03/2020
// J_Code
// @brief: ½âÂëÆ÷ÊÓÆµµ¼Èë»úº¯Êı

#ifndef SH_VIDEO_IMPORT_H
#define SH_VIDEO_IMPORT_H

#define STOP 128	// åœæ­¢å¸§è¯»å–è®¯å·
#define CONTINUE 127	// æœªæœ‰æ•ˆè¯»å–ï¼Œè€Œç»§ç»­å¸§è¯»å–è®¯å·
#define LOST 256	// å¸§ä¸¢å¤±è®¯å· 

extern int dealWithVideo(	int fps_need, const char videofilename[], 
							const char datafilename[], const char validfilename[],
							int (*foo)(cv::Mat, int*, int*, const char*, const char*));

#endif