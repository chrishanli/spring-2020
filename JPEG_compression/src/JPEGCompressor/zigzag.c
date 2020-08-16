// zigzag.c
// Han Li 20/6/2020

#include <stdio.h>
#include <stdlib.h>
#include "JPEGComprssor.h"

static const int SCAN_INDICES[][2] = {
    {0, 0}, {0, 1}, {1, 0}, {2, 0},
    {1, 1}, {0, 2}, {0, 3}, {1, 2},
    {2, 1}, {3, 0}, {4, 0}, {3, 1},
    {2, 2}, {1, 3}, {0, 4}, {0, 5},
    {1, 4}, {2, 3}, {3, 2}, {4, 1},
    {5, 0}, {6, 0}, {5, 1}, {4, 2},
    {3, 3}, {2, 4}, {1, 5}, {0, 6},
    {0, 7}, {1, 6}, {2, 5}, {3, 4},
    {4, 3}, {5, 2}, {6, 1}, {7, 0},
    {7, 1}, {6, 2}, {5, 3}, {4, 4},
    {3, 5}, {2, 6}, {1, 7}, {2, 7},
    {3, 6}, {4, 5}, {5, 4}, {6, 3},
    {7, 2}, {7, 3}, {6, 4}, {5, 5},
    {4, 6}, {3, 7}, {4, 7}, {5, 6},
    {6, 5}, {7, 4}, {7, 5}, {6, 6},
    {5, 7}, {6, 7}, {7, 6}, {7, 7}
};

// (3) zig-zag scan to produce several 1x64 vectors
JVects* zigZagImage(JBlocks* dctImg, JStatus* status) {
    if (!dctImg) {
        *status = JPG_NULL_POINTER;
        return NULL;
    }
    
    JVects* zigzag = (JVects *)malloc(sizeof(JVects));
    if (!zigzag) {
        *status = JPG_OUT_OF_MEMORY;
        return NULL;
    }
    
    // 1. allocate vectors
    zigzag->vectCount = dctImg->blocksCount;
    zigzag->vect = (int (*)[64])malloc(dctImg->blocksCount * 64 * sizeof(int));
    if (!zigzag->vect) {
        *status = JPG_OUT_OF_MEMORY;
        free(zigzag);
        return NULL;
    }
    
    // 2. zig-zag scan
    int (*vect)[64], (*block)[8][8];
    for (int i = 0; i < zigzag->vectCount; i++) {
        vect = &zigzag->vect[i];
        block = &dctImg->blocks[i];
        for (int j = 0; j < 64; j++) {
            (*vect)[j] = (*block)[SCAN_INDICES[j][0]][SCAN_INDICES[j][1]];
        }
    }
    
    *status = JPG_SUCCESS;
    return zigzag;
}
