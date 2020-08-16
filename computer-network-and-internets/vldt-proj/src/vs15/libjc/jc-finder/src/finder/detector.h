// detector.h    03/03/2020
// J_Code
// brief: J_Code symbol detector functions

#ifndef detector_hpp
#define detector_hpp

#include <jc.h>

// Detector
extern J_Found_Symbol* detectMaster( Bitmap* bitmap, Bitmap* ch[]);
extern J_Alignment_Pattern findAlignmentPattern(Bitmap* ch[], Float x, Float y, Float module_size, Int32 ap_type);
extern Int32 confirmSymbolSize(Bitmap* ch[], J_Finder_Pattern* fps, J_Decoded_Symbol* symbol);

#endif /* detector_hpp */
