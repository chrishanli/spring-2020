// dccodec.c
// encoding methods for DC coefficients using DPCM codec
// Han Li 20/6/2020

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "JPEGComprssor.h"

// (4) dc codec, extract dc args and encode via differential & VLC
JDCEncode* DCEncode(JVects* inVect, JStatus* status) {
    if (!inVect) {
        *status = JPG_NULL_POINTER;
        return NULL;
    }
    
    JDCEncode* data = (JDCEncode *)malloc(sizeof(JDCEncode));
    if (!data) {
        *status = JPG_OUT_OF_MEMORY;
        return NULL;
    }
    data->length = 0;
    data->blocksCount = inVect->vectCount;
    data->blocksLength = (int*)malloc(inVect->vectCount * sizeof(int));
    if (!data->blocksLength) {
        *status = JPG_OUT_OF_MEMORY;
        free(data);
        return NULL;
    }
    data->data = (JByte *)malloc(inVect->vectCount * 16);
    if (!data->data) {
        *status = JPG_OUT_OF_MEMORY;
        free(data->blocksLength);
        free(data);
        return NULL;
    }
    
    int i, diff, ssss, pair[2][2], whichPair, diffCodeSize, blockSize;
    const char *ssssCode;
    char diffCode[9];
    // 1. extract the first (pair[0]'s) DC coefficients
    diff = pair[0][1] = inVect->vect[0][0];
    ssss = pair[0][0] = diff == 0 ? 0 : log2(JPG_ABS(diff)) + 1;
    ssssCode = DC_SSSS_TABLE[ssss];
    diffCodeSize = binary(diff, diffCode);
    
    JByte* offset = data->data;
    memcpy(offset, ssssCode, DC_SSSS_TABLE_LENS[ssss]);
    offset += DC_SSSS_TABLE_LENS[ssss];
    memcpy(offset, diffCode, diffCodeSize);
    offset += diffCodeSize;
    
    blockSize = DC_SSSS_TABLE_LENS[ssss] + diffCodeSize;
    data->blocksLength[0] = blockSize;
    data->length += blockSize;
    
    // 2. extract the rest DC coefficients
    for (i = 1; i < inVect->vectCount; ++i) {
        // delta = DC(0, 0)_i - DC(0, 0)_(i-1)
        whichPair = i % 2;
        pair[whichPair][1] = inVect->vect[i][0];
        diff = inVect->vect[i][0] - pair[!whichPair][1];
        // calculate ssss according to diff
        ssss = pair[whichPair][0] = diff == 0 ? 0 : log2(JPG_ABS(diff)) + 1;
        
        // write (ssss, diff) as huffman
        ssssCode = DC_SSSS_TABLE[ssss];
        diffCodeSize = binary(diff, diffCode);
        
        memcpy(offset, ssssCode, DC_SSSS_TABLE_LENS[ssss]);
        offset += DC_SSSS_TABLE_LENS[ssss];
        memcpy(offset, diffCode, diffCodeSize);
        offset += diffCodeSize;
        
        // record this DC coefficient's actual count of bits
        blockSize = DC_SSSS_TABLE_LENS[ssss] + diffCodeSize;
        data->blocksLength[i] = blockSize;
        data->length += blockSize;
    }
    
    // 3. adjust memory size to its actual occupied size
    data->data = realloc(data->data, data->length);
    
    return data;
}
