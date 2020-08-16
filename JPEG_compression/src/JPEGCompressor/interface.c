// interface.c
// sealed interface(s) of libjpgcomp
// Han Li 19/6/2020

#include <stdio.h>
#include <stdlib.h>
#include "JPEGComprssor.h"
#ifdef _WIN32 
#include "libjpgcomp_dll.h"
#else 
#include "libjpgcomp.h"
#endif

JStatus compress(const char inData[], const char outData[]) {
    JStatus state;
    
    // 1. read image
    JImage* img = readImage(inData, &state);
    if (!img) {
        return state;
    }
    
    // 2. do dct
    JBlocks* dct = dctImage(img, &state);
    free(img->data);
    free(img);
    if (!dct) {
        return state;
    }
    
    // 3. do zig zag
    JVects* zigzag = zigZagImage(dct, &state);
    free(dct->blocks);
    free(dct);
    if (!zigzag) {
        return state;
    }
    
    // 4. encode DC coefficients
    JDCEncode* dcCode = DCEncode(zigzag, &state);
    if (!dcCode) {
        free(zigzag->vect);
        free(zigzag);
        return state;
    }
    
    // 5. encode AC coefficients
    JACEncode* acCode = ACEncode(zigzag, &state);
    free(zigzag->vect);
    free(zigzag);
    if (!acCode) {
        free(dcCode->data);
        free(dcCode->blocksLength);
        free(dcCode);
        return state;
    }
    
    // 6. save encoded to file
    writeImage(dcCode, acCode, outData, &state);
    if (state != JPG_SUCCESS) {
        free(dcCode->data);
        free(dcCode->blocksLength);
        free(dcCode);
        free(acCode->data);
        free(acCode->blocksLength);
        free(acCode->blocksForVects);
        free(acCode);
        return state;
    }
    
    // 7. procedure over
    return JPG_SUCCESS;
}

void checkStatus(JStatus exceptionNumber) {
    switch (exceptionNumber) {
        case JPG_NO_SUCH_FILE:
            fprintf(stderr, "Exception-2: No such file or directory\n");
            break;
        case JPG_OUT_OF_MEMORY:
            fprintf(stderr, "Exception-3: Out of memory\n");
            break;
        case JPG_NULL_POINTER:
            fprintf(stderr, "Exception-4: Null pointer\n");
            break;
        case JPG_FILE_ACCESS_DENIED:
            fprintf(stderr, "Exception-5: File access denied when output\n");
            break;
        case JPG_SUCCESS:
            printf("Encode succeeded.\n");
            break;
        default:
            fprintf(stderr, "Exception-%d: Unknown exception number\n", exceptionNumber);
            break;
    }
}
