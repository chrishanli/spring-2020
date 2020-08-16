// binary.c
// binary codec
// Han Li 20/6/2020

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "JPEGComprssor.h"

static const int masks[] = {
    1, 1, 3, 3, 7, 7, 7, 7, 15, 15, 15, 15
};

// (-1) writes a int value to its binary form, return bin size
int binary(int value, char* buffer) {
    if (value == 0) {
        strcpy(buffer, "0");
        return 1;
    }
    JByte bin; // the highest bit
    int i = 0, j = 0, absValue = value > 0 ? value : -value, moveBits = (int)log2(absValue), mask = masks[moveBits + 1];
    while (!(bin = absValue >> moveBits)) {
        absValue <<= 1;
        absValue &= mask;
        ++i;
    }
    if (value < 0) {
        for (j = 0; i < 8; ++j, ++i) {
            buffer[j] = !bin + 0x30;
            absValue <<= 1;
            absValue &= mask;
            bin = absValue >> moveBits;
        }
    } else {
        for (j = 0; i < 8; ++j, ++i) {
            buffer[j] = bin + 0x30;
            absValue <<= 1;
            absValue &= mask;
            bin = absValue >> moveBits;
        }
    }
    
    buffer[j] = 0;
    return j;
}
