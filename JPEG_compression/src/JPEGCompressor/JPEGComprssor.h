// JPEGCompressor.h
// internal interfaces of JPEGCompressor
// Han Li 19/6/2020

#ifndef JPEGComprssor_h
#define JPEGComprssor_h

#define JPG_INTERNAL 1

#define JPG_TRUE 1
#define JPG_FALSE 0

#define JPG_SUCCESS 0
#define JPG_FAILED 1
#define JPG_NO_SUCH_FILE  2
#define JPG_OUT_OF_MEMORY  3
#define JPG_NULL_POINTER  4
#define JPG_FILE_ACCESS_DENIED  5
#define JPG_INVALID_FILE_SIZE  6

#define JPG_ABS(x) x < 0 ? -x : x

typedef unsigned char JBool;
typedef unsigned char JByte;
typedef unsigned char JStatus;

// an image from file
typedef struct {
    int h;
    int w;
    JByte* data;
} JImage;

// preparation or result of DCT
typedef struct {
    int blocksCount;    // count of 8x8 blocks
    int (*blocks)[8][8];
} JBlocks;

// result vectors of zig-zag scan
typedef struct {
    int vectCount;    // count of 1x64 blocks
    int (*vect)[64];
} JVects;

// result of DC coding
typedef struct {
    int length;
    int blocksCount;    // one block represents a (x, y) pair
    int *blocksLength;  // length for every block
    JByte *data; // one byte represents one bit
} JDCEncode;

// result of AC coding
typedef struct {
    int length;
    int blocksCount;    // one block represents a (x, y) pair
    int *blocksLength;  // length for every block
    int *blocksForVects;// count of blocks that every vector administers
    JByte *data; // one byte represents one bit
} JACEncode;

// (1) read image (allocated)
extern JImage* readImage(const char inFile[], JStatus* status);

// (2) do dct and quantization of the image
extern JBlocks* dctImage(JImage* dctImg, JStatus* status);

// (3) zig-zag scan
extern JVects* zigZagImage(JBlocks* dctImg, JStatus* status);

// (4) dc codec, extract dc args and encode via differential & VLC
extern JDCEncode* DCEncode(JVects* inVect, JStatus* status);

// (5) ac codec, extract ac args and encode via differential & VLC
extern JACEncode* ACEncode(JVects* inVect, JStatus* status);

// (6) write a compressed image through their dc & ac coeffitients
extern void writeImage(JDCEncode* dc, JACEncode* ac, const char fileName[], JStatus* status);

// (-1) binary codec for DC & AC codecs
extern int binary(int byteInt, char* buffer);

// tables
extern const char* AC_RS_TABLE[][11];   // AC's R/S -> huffman table
extern const char* DC_SSSS_TABLE[];     // DC's SSSS-> huffman table
extern const int DC_SSSS_TABLE_LENS[];  // DC's SSSS-> huffman length
extern const int QUANT_TABLE[][8];      // quantization table


#endif /* JPEGComprssor_h */
