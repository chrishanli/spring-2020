// dct.c
// DCT codec
// Han Li 20/6/2020

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "JPEGComprssor.h"

#define PI 3.14159
#define SQRTROOT_2 1.41421

// (2.1) prepare an image for DCT
static JBlocks* prepDCT(JImage* img, JStatus* status) {
    if (!img) {
        *status = JPG_NULL_POINTER;
        return NULL;
    }
    // 1. allocate a buffer to store all of the 8x8 blocks
    JBlocks* prepObj = (JBlocks *)malloc(sizeof(JBlocks));
    if (!prepObj) {
        *status = JPG_OUT_OF_MEMORY;
        return 0;
    }
    int size = img->h * img->w;
    prepObj->blocksCount = size / 64;
    prepObj->blocks = (int (*)[8][8])malloc(size * sizeof(int));
    if (!prepObj->blocks) {
        *status = JPG_OUT_OF_MEMORY;
        free(prepObj);
        return 0;
    }
    // 2. assign the image's data into blocks;
    int (*currBlock)[8][8], maxBlocksPerLine = img->w / 8;
    for (int i = 0; i < img->h / 8; i++) {
        for (int j = 0; j < maxBlocksPerLine; j++) {
            // the block is at blocks[i][j]
            currBlock = prepObj->blocks + i * maxBlocksPerLine + j;
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    // the element is at *currBlock[x][y]
                    JByte* truePlace =
                    img->data + img->w * (8 * i + x) +
                    j * 8 + y; // trueColumn
                    // turn to range between [-2^(n-1), 2^(n-1) - 1)]
                    (*currBlock)[x][y] = (int)*truePlace - 64;
                }
            }
        }
    }
    
    return prepObj;
}

// (2.2) do DCT of one block
JStatus blockDCT(int (*inBlock)[8][8], double (*outBlock)[8][8]) {
    if (outBlock == NULL || inBlock == NULL) {
        return JPG_NULL_POINTER;
    }
    double avgI = 0, avgJ = 0;
    int i, j, u, v;
    for (u = 0; u < 8; ++u) {
        for (v = 0; v < 8; ++v) {
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    // f(i, j)
                    avgJ += (*inBlock)[i][j] * cos((2 * j + 1) * v * PI / 16);
                }
                // G(i, v)
                avgJ *= (v == 0) ? ( 0.5f / SQRTROOT_2 ) : 0.5f;
                avgI += avgJ * cos((2 * i + 1) * u * PI / 16);
            }
            // F(u, v)
            avgI *= (u == 0) ? ( 0.5f / SQRTROOT_2 ) : 0.5f;
            (*outBlock)[u][v] = avgI;
            // clear
            avgJ = avgI = 0;
        }
    }
    return JPG_SUCCESS;
}

// (2.3) do quantization after DCT of one block using (5-7)
JStatus blockQuant(double (*inBlock)[8][8], int (*outBlock)[8][8]) {
    if (outBlock == NULL || inBlock == NULL) {
        return JPG_NULL_POINTER;
    }
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            // round(a) = (int)(a + 0.5)
            (*outBlock)[i][j] = (*inBlock)[i][j] / QUANT_TABLE[i][j] + 0.5f;
        }
    }
    return JPG_SUCCESS;
}

// (3) do DCT and quantization of all the blocks in prepImg
JBlocks* dctImage(JImage* img, JStatus* status) {
    JBlocks* dctImg = prepDCT(img, status);
    if (*status != JPG_SUCCESS) {
        return NULL;
    }
    
    double dctResult[8][8];
    for (int i = 0; i < dctImg->blocksCount; i++) {
        // 1. DCT block
        *status = blockDCT(&dctImg->blocks[i], &dctResult);
        if (*status != JPG_SUCCESS) {
            return NULL;
        }
        // 2. quantization
        *status = blockQuant(&dctResult, &dctImg->blocks[i]);
        if (*status != JPG_SUCCESS) {
            return NULL;
        }
    }
    return dctImg;
}

