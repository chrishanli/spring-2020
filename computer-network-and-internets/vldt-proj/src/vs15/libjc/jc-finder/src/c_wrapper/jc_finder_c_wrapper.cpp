// jc_finder_c_wrapper.cpp    08/03/2020
// J_Code
// brief: the jc-finder cpp-to-c wrapper
// Han Li

#include "../finder/jc_finder_cpp.h"
#include <stdio.h>
#include "../dll.h"

J_Found_Symbol* importImage(const char src[])
{ return importImage_cpp(src); }

void clean_foundSymbol(J_Found_Symbol* found_symbol)
{ clean_foundSymbol_cpp(found_symbol); }

void getRGBMinMax(BYTE* rgb, BYTE* min, BYTE* mid, BYTE* max, Int32* index_min, Int32* index_mid, Int32* index_max)
{ getRGBMinMax_cpp(rgb, min, mid, max, index_min, index_mid, index_max); }

void getRGBAveVar(BYTE* rgb, Double* ave, Double* var)
{ getRGBAveVar_cpp(rgb, ave, var); }



