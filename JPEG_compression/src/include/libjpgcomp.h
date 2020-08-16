// libjpgcomp.h
// Han Li 19/6/2020

#ifndef libjpgcomp_h
#define libjpgcomp_h

#ifndef JPG_INTERNAL
#define JPG_TRUE 1
#define JPG_FALSE 0

#define JPG_SUCCESS 0
#define JPG_FAILED 1
#define JPG_NO_SUCH_FILE  2
#define JPG_OUT_OF_MEMORY  3
#define JPG_NULL_POINTER  4
#define JPG_FILE_ACCESS_DENIED  5
#define JPG_INVALID_FILE_SIZE  6
typedef unsigned char JStatus;
#endif

// compress data into JPEG format (simplified)
extern JStatus compress(const char inData[], const char outData[]);

// default exceptions / info reporter
extern void checkStatus(JStatus statusNumber);

#endif /* libjpgcomp_h */
