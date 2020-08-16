
// dll.h    08/03/2020
// J_Code
// brief: the jc-finder dll header
// Han Li

#ifndef _DLL_H_
#define _DLL_H_

// 26.3.2020 移植至vs
#define DLLIMPORT __declspec(dllimport)

#ifdef __cplusplus
extern "C" {
#endif
#include "jc.h"
	// 汇入图像，并进行初始扫描 
#pragma once 
	DLLIMPORT extern J_Found_Symbol* importImage(const char src[]);
	// 如果初始扫描发现尺寸不符、颜色不符、图像有问题，进阶扫描
#pragma once 
	DLLIMPORT extern Bitmap* sampleSymbolByAlignmentPattern(Bitmap* bitmap, Bitmap* ch[], J_Decoded_Symbol* symbol, J_Finder_Pattern* fps);
	// 扔掉一个found symbol 
#pragma once
	DLLIMPORT extern void clean_foundSymbol(J_Found_Symbol* found_symbol);
	// 拿到RGB3个值的最值 
#pragma once
	DLLIMPORT extern void getRGBMinMax(BYTE* rgb, BYTE* min, BYTE* mid, BYTE* max, Int32* index_min, Int32* index_mid, Int32* index_max);
	// 拿到RGB的平均值
#pragma once
	DLLIMPORT extern void getRGBAveVar(BYTE* rgb, Double* ave, Double* var);
	// 防止OpenCV失效而使用 
	extern Boolean saveImage(Bitmap* bitmap, Char* filename);
#ifdef __cplusplus
}

#include <opencv2/core/core.hpp>
#pragma once
DLLIMPORT extern J_Found_Symbol* importMat_cpp(cv::Mat& src);

#endif
#endif 
