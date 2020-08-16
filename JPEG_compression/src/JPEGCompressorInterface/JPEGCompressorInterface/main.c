// main.c

#include <stdio.h>
#include "libjpgcomp.h"

void printHelp(const char appName[]) {
    printf("Usage: %s [input-file] [output-file]\n", appName);
    printf("Arguments:\n");
    printf("\tinput-file\timage data to be input, typically ends with '.data'\n");
    printf("\toutput-file\tfile to store the compressed data\n");
    printf("\n");
    printf("Options:\n");
    printf("\t-h\t\tprint this help\n");
    printf("\n");
    printf("Version:\n");
    printf("\tlibjpgcomp version 1.0, built on %s %s\n", __DATE__, __TIME__);
    printf("\n");
    printf("Copyright:\n");
    printf("\t(c) 2020 Han Li Studios. All rights reserved.\n");
    printf("\tCourse design of the Multimedia Technology Course.\n");
    printf("\n");
}

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        printHelp(argv[0]);
        return 1;
    }
    
    JStatus status = compress(argv[1], argv[2]);
    checkStatus(status);
    return status;
}
