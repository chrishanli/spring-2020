// j_finder_cpp.h    03/03/2020
// J_Code
// brief: J_Code finder and sampler (Cpp)

#ifndef j_finder_h
#define j_finder_h

#include "jc.h"
#include <opencv2/core/core.hpp>

extern J_Found_Symbol* importImage_cpp(const char src[]);
// extern J_Found_Symbol* importMat_cpp(cv::Mat& src);
extern void clean_foundSymbol_cpp(J_Found_Symbol* found_symbol);
extern void getRGBMinMax_cpp(BYTE* rgb, BYTE* min, BYTE* mid, BYTE* max, Int32* index_min, Int32* index_mid, Int32* index_max);
extern void getRGBAveVar_cpp(BYTE* rgb, Double* ave, Double* var);


// extern Boolean saveImage(Bitmap* bitmap, Char* filename);

#endif /* j_finder_h */
