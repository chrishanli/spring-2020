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
    // 汇入图像，并进行初始扫描 
    extern J_Found_Symbol* importImage(const char src[]);
    // 如果初始扫描发现尺寸不符、颜色不符、图像有问题，进阶扫描
    extern Bitmap* sampleSymbolByAlignmentPattern(Bitmap* bitmap, Bitmap* ch[], J_Decoded_Symbol* symbol, J_Finder_Pattern* fps);
    // 扔掉一个found symbol 
	extern void clean_foundSymbol(J_Found_Symbol* found_symbol);
	// 拿到RGB3个值的最值 
    extern void getRGBMinMax(BYTE* rgb, BYTE* min, BYTE* mid, BYTE* max, Int32* index_min, Int32* index_mid, Int32* index_max);
    // 拿到RGB的平均值 
	extern void getRGBAveVar(BYTE* rgb, Double* ave, Double* var);
#ifdef __cplusplus
}
/* 仅当C++使用 */ 
#include <opencv2/core/core.hpp>
extern J_Found_Symbol* importMat_cpp(cv::Mat& src);

#endif

#endif /* jc_finder_h */
