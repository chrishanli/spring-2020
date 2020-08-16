// sample.h    03/03/2020
// J_Code
// brief: J_Code symbol sampling functions

#ifndef jc_finder_sample_h
#define jc_finder_sample_h

#include "detector.h"
#include "transform.h"

Bitmap* sampleSymbol(Bitmap* bitmap, J_Perspective_Transform* pt, Vector2D side_size);

Bitmap* sampleSymbolByAlignmentPattern(Bitmap* bitmap, Bitmap* ch[], J_Decoded_Symbol* symbol, J_Finder_Pattern* fps);

#endif
