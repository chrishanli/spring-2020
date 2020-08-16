// output.c
// outputs encoded DC & AC coefficients into a file
// Han Li 21/6/2020

#include <stdio.h>
#include "JPEGComprssor.h"

void writeImage(JDCEncode* dc, JACEncode* ac, const char fileName[], JStatus* status) {
    if (!dc || !ac) {
        *status = JPG_NULL_POINTER;
        return;
    }
    
    FILE* out = fopen(fileName, "wb");
    if (!out) {
        *status = JPG_FILE_ACCESS_DENIED;
        return;
    }
    
    // gather up all coefficients of every 8x8 blocks and write
    int i, j, dcWroteSize = 0, acWroteSize = 0, acWroteBlocks = 0;
    for (i = 0; i < dc->blocksCount; i++) {
        // 1. write dc coefficient of this 8x8 block
        fwrite(dc->data + dcWroteSize,
               dc->blocksLength[i], 1, out);
        dcWroteSize += dc->blocksLength[i];
        // 2. write ac coefficients of this 8x8 block
        for (j = 0; j < ac->blocksForVects[i]; j++) {
            fwrite(ac->data + acWroteSize,
                   ac->blocksLength[acWroteBlocks], 1, out);
            acWroteSize += ac->blocksLength[acWroteBlocks];
            acWroteBlocks++;
        }
    }
    
    // save to file
    fclose(out);
}
