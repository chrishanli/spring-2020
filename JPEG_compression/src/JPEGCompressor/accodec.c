// accodec.c
// encoding methods for AC arguments using run-length codec
// Han Li 20/6/2020

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "JPEGComprssor.h"

// (5) ac codec, extract ac args and encode via differential & VLC
JACEncode* ACEncode(JVects* inVect, JStatus* status) {
    if (!inVect) {
        *status = JPG_NULL_POINTER;
        return NULL;
    }
    
    // 0. prepare
    JACEncode* data = (JACEncode *)malloc(sizeof(JACEncode));
    if (!data) {
        *status = JPG_OUT_OF_MEMORY;
        return NULL;
    }
    data->length = 0;
    data->blocksCount = 0;
    data->blocksLength = (int*)malloc(inVect->vectCount * sizeof(int) * 63);
    if (!data->blocksLength) {
        *status = JPG_OUT_OF_MEMORY;
        free(data);
        return NULL;
    }
    data->blocksForVects = (int*)malloc(inVect->vectCount * sizeof(int));
    if (!data->blocksForVects) {
        *status = JPG_OUT_OF_MEMORY;
        free(data->blocksLength);
        free(data);
        return NULL;
    }
    data->data = (JByte *)malloc(inVect->vectCount * 1006);  // 126 = 63*2*8
    if (!data->data) {
        *status = JPG_OUT_OF_MEMORY;
        free(data->blocksLength);
        free(data->blocksForVects);
        free(data);
        return NULL;
    }
    
    // do run-length coding for every 1x64 vector
    JByte* offset = data->data;
    int i, j, zeroCount = 0, (*thisVect)[64], rsCodeLen, acCodeLen, blocksOfVect;
    const char* rsCode;
    char acCode[11];
    for (i = 0; i < inVect->vectCount; i++) {
        j = 1;
        blocksOfVect = 0;
        thisVect = &inVect->vect[i];
        // check all values
        while (j < 64) {
            // 1. check 0 counts;
            while (j < 64 && (*thisVect)[j] == 0) {
                zeroCount++;
                j++;
            }
            // 2. if has more than 1 zero, do some prepare
            if (zeroCount > 0) {
                if (j == 64) {
                    // the rest are all zeroes, write (0, 0)
                    rsCode = AC_RS_TABLE[0][0];
                    rsCodeLen = (int)strlen(rsCode);
                    data->blocksLength[data->blocksCount++] = rsCodeLen;
                    memcpy(offset, rsCode, rsCodeLen);
                    blocksOfVect++;
                    data->length += rsCodeLen;
                    offset += rsCodeLen;
                    zeroCount = 0;
                    break; // go to check next vector;
                }
                // j != 64, and thisVect[j] != 0
                while (zeroCount > 15) {
                    // write (15/0)
                    rsCode = AC_RS_TABLE[15][0];
                    rsCodeLen = (int)strlen(rsCode);
                    data->blocksLength[data->blocksCount++] = rsCodeLen;
                    memcpy(offset, rsCode, rsCodeLen);
                    blocksOfVect++;
                    data->length += rsCodeLen;
                    offset += rsCodeLen;
                    zeroCount -= 16;
                }
            }
            // 3. if no zero, or zeroCount < 15
            // write (r/s, ac) = (zeroCount/size(thisVect[j]), ac)
            acCodeLen = binary((*thisVect)[j], acCode); // *thisVect[j] = ac
            rsCode = AC_RS_TABLE[zeroCount][acCodeLen];
            rsCodeLen = (int)strlen(rsCode);
            memcpy(offset, rsCode, rsCodeLen);
            offset += rsCodeLen;
            memcpy(offset, acCode, acCodeLen);
            offset += acCodeLen;
            
            data->blocksLength[data->blocksCount++] = rsCodeLen + acCodeLen;
            data->length += rsCodeLen + acCodeLen;
            zeroCount = 0;
            blocksOfVect++;
            
            // 4. increment j so it could deal with next digit, or over
            j++;
        } // while
        
        // record blocks occupied by this vector
        data->blocksForVects[i] = blocksOfVect;
    }
    
    // finally, adjust memory size to its actual occupied size
    data->data = realloc(data->data, data->length);
    data->blocksLength = realloc(data->blocksLength, sizeof(int) * data->blocksCount);
    
    return data;
}
