// input.c
// file input accessories
// Han Li 19/6/2020

#include <stdio.h>
#include <stdlib.h>
#include "JPEGComprssor.h"

// (1) input a file
JImage* readImage(const char inFile[], JStatus* status) {
    // read in file
    FILE* file = fopen(inFile, "rb");
    if (!file) {
        *status = JPG_NO_SUCH_FILE;
        return 0;
    }
    
    // create a new image object
    JImage* img = (JImage *)malloc(sizeof(JImage));
    if (!img) {
        *status = JPG_OUT_OF_MEMORY;
        return 0;
    }
    
    fread(&img->h, sizeof(int), 1, file); // height
    fread(&img->w, sizeof(int), 1, file); // width
    
    // get image data
    int size = img->h * img->w;
    JByte* pixels = (JByte *)malloc(size);
    if (!pixels) {
        *status = JPG_OUT_OF_MEMORY;
        free(img);
        return 0;
    }
    size_t readSize = fread(pixels, sizeof(JByte), size, file);
    if (readSize != size) {
        // invalid size
        *status = JPG_INVALID_FILE_SIZE;
        free(pixels);
        free(img);
        return 0;
    }
    fclose(file);
    img->data = pixels;
    
    *status = JPG_SUCCESS;
    return img;
}
