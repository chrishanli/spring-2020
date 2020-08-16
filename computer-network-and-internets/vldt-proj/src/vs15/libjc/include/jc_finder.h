// jc_finder.h    08/03/2020
// J_Code
// brief: the jc-finder header
// Han Li

#ifndef jc_finder_h
#define jc_finder_h

#ifdef __cplusplus
extern "C" {
#endif
    #include "jc.h"
    // ����ͼ�񣬲����г�ʼɨ�� 
    extern J_Found_Symbol* importImage(const char src[]);
    // �����ʼɨ�跢�ֳߴ粻������ɫ������ͼ�������⣬����ɨ��
    extern Bitmap* sampleSymbolByAlignmentPattern(Bitmap* bitmap, Bitmap* ch[], J_Decoded_Symbol* symbol, J_Finder_Pattern* fps);
    // �ӵ�һ��found symbol 
	extern void clean_foundSymbol(J_Found_Symbol* found_symbol);
	// �õ�RGB3��ֵ����ֵ 
    extern void getRGBMinMax(BYTE* rgb, BYTE* min, BYTE* mid, BYTE* max, Int32* index_min, Int32* index_mid, Int32* index_max);
    // �õ�RGB��ƽ��ֵ 
	extern void getRGBAveVar(BYTE* rgb, Double* ave, Double* var);
#ifdef __cplusplus
}
/* ����C++ʹ�� */ 
#include <opencv2/core/core.hpp>
extern J_Found_Symbol* importMat_cpp(cv::Mat& src);

#endif

#endif /* jc_finder_h */
