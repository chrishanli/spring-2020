
// dll.h    08/03/2020
// J_Code
// brief: the jc-finder dll header
// Han Li

#ifndef _DLL_H_
#define _DLL_H_

// 26.3.2020 ��ֲ��vs
#define DLLIMPORT __declspec(dllimport)

#ifdef __cplusplus
extern "C" {
#endif
#include "jc.h"
	// ����ͼ�񣬲����г�ʼɨ�� 
#pragma once 
	DLLIMPORT extern J_Found_Symbol* importImage(const char src[]);
	// �����ʼɨ�跢�ֳߴ粻������ɫ������ͼ�������⣬����ɨ��
#pragma once 
	DLLIMPORT extern Bitmap* sampleSymbolByAlignmentPattern(Bitmap* bitmap, Bitmap* ch[], J_Decoded_Symbol* symbol, J_Finder_Pattern* fps);
	// �ӵ�һ��found symbol 
#pragma once
	DLLIMPORT extern void clean_foundSymbol(J_Found_Symbol* found_symbol);
	// �õ�RGB3��ֵ����ֵ 
#pragma once
	DLLIMPORT extern void getRGBMinMax(BYTE* rgb, BYTE* min, BYTE* mid, BYTE* max, Int32* index_min, Int32* index_mid, Int32* index_max);
	// �õ�RGB��ƽ��ֵ
#pragma once
	DLLIMPORT extern void getRGBAveVar(BYTE* rgb, Double* ave, Double* var);
	// ��ֹOpenCVʧЧ��ʹ�� 
	extern Boolean saveImage(Bitmap* bitmap, Char* filename);
#ifdef __cplusplus
}

#include <opencv2/core/core.hpp>
#pragma once
DLLIMPORT extern J_Found_Symbol* importMat_cpp(cv::Mat& src);

#endif
#endif 
