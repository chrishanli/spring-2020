/**
 * libjc - J_Code Encoding/Decoding Library
 * adapted and optimized by Han Li from original work of Fraunhofer SIT
 * Copyright 2020 by Han Li & Fraunhofer SIT. All rights reserved.
 * 
 * @file jc.h  (22/3/2020)
 * @brief Main procedures defination
 *
 */

#ifndef JC_H
#define JC_H

// HAN
#define MAXIMUM_FILE_SIZE 2010

#define VERSION "2.0.0"
#define BUILD_DATE __DATE__

#define MAX_SYMBOL_NUMBER       1
#define MAX_COLOR_NUMBER        256
#define MAX_SIZE_ENCODING_MODE  256
#define J_ENCODING_MODES      6
#define ENC_MAX                 1000000
#define NUMBER_OF_MASK_PATTERNS	8

#define DEFAULT_SYMBOL_NUMBER 			1
#define DEFAULT_MODULE_SIZE				12
#define DEFAULT_COLOR_NUMBER 			8
#define DEFAULT_MODULE_COLOR_MODE 		2
#define DEFAULT_ECC_LEVEL				3
#define DEFAULT_MASKING_REFERENCE 		7


#define DISTANCE_TO_BORDER      4
#define MAX_ALIGNMENT_NUMBER    9
#define COLOR_PALETTE_NUMBER	4

#define BITMAP_BITS_PER_PIXEL	32
#define BITMAP_BITS_PER_CHANNEL	8
#define BITMAP_CHANNEL_COUNT	4

#define	SUCCESS		1
#define	FAILURE		0

#define NORMAL_DECODE		0
#define COMPATIBLE_DECODE	1

#define VERSION2SIZE(x)		(x * 4 + 17)
#define SIZE2VERSION(x)		((x - 17) / 4)

//#ifdef 
//#ifndef MAX
//#define MAX(a,b) 			({__typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b;})
//#ifndef MIN
//#define MIN(a,b) 			({__typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b;})
//#endif
//#endif

// HAN LI at 26.3.2020 移植至Visual Studio
#ifndef MAX
#define MAX(a,b) 			(a > b ? a : b)
#ifndef MIN
#define MIN(a,b) 			(a > b ? b : a)
#endif
#endif


#define TEST_MODE            0

#define MAX_MODULES         145    //the number of modules in side-version 32
#define MAX_SYMBOL_ROWS        3
#define MAX_SYMBOL_COLUMNS    3
#define MAX_FINDER_PATTERNS 500
#define PI                     3.14159265

#define DIST(x1, y1, x2, y2) (Float)(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)))

#define J_REPORT_ERROR(x)    { printf("JC_ERR: "); printf x; printf("\n"); }
#define J_REPORT_INFO(x)    { printf("JC_INFO: "); printf x; printf("\n"); }


typedef unsigned char 		BYTE;
typedef char 				Char;
typedef unsigned char 		Boolean;
typedef int 				Int32;
typedef unsigned int 		UInt32;
typedef short 				Int16;
typedef unsigned short 		UInt16;
typedef long long 			Int64;
typedef unsigned long long	UInt64;
typedef float				Float;
typedef double              Double;

/**
 * @brief 一个二维整数向量，即二维整数坐标
*/
typedef struct {
	Int32	x;
	Int32	y;
} Vector2D;

/**
 * @brief 一个二维浮点数向量，即二维浮点数坐标
*/
typedef struct {
	Float	x;
	Float	y;
} J_POINT;

/**
 * @brief 一组数据
*/
typedef struct {
	Int32	length;
	Char	data[];
} J_Data;

/**
 * @brief Code Bitmap
*/
typedef struct {
   Int32	width;
   Int32	height;
   Int32	bits_per_pixel;
   Int32	bits_per_channel;
   Int32	channel_count;
   BYTE		pixel[];
} Bitmap;

/**
 * @brief Symbol（未解码）
*/
typedef struct {
	Int32		index;
	Vector2D	side_size;
	Int32		host;
	Int32 		wcwr[2];
	J_Data*		data;
	BYTE*		data_map;
	J_Data*		metadata;
	BYTE*		matrix;
} J_Symbol;

/**
 * @brief 编码参数（Encode Parameters）
*/
typedef struct {
	Int32		color_number;
	Int32		symbol_number;
	Int32		module_size;
	Int32		master_symbol_width;
	Int32		master_symbol_height;
	BYTE*		palette;                /// < Palette holding used module colors in format RGB
	Vector2D*	symbol_versions;
	BYTE* 		symbol_ecc_levels;
	Int32*		symbol_positions;
	J_Symbol*	symbols;				///< Pointer to internal representation of J Code symbols
	Bitmap*		bitmap;
} J_Encode;

/**
 * @brief Metadata（已经解码的）
*/
typedef struct {
	Boolean default_mode;
	BYTE Nc;
	BYTE mask_type;
	BYTE docked_position;
	Vector2D side_version;
	Vector2D ecl;
} J_Metadata;

/**
 * @brief Symbol（已经解码的）
*/
typedef struct {
	Int32 index;
	Int32 host_index;
	Int32 host_position;
	Vector2D side_size;
	Float module_size;
	J_POINT pattern_positions[4];
	J_Metadata metadata;
	BYTE* palette;
	J_Data* data;
} J_Decoded_Symbol;


/**
 * @brief Detection modes
 */
typedef enum {
    QUICK_DETECT = 0,
    NORMAL_DETECT,
    INTENSIVE_DETECT
} J_Detect_Mode;

/**
 * @brief Finder pattern, alignment pattern
 */
typedef struct {
    Int32        type;
    Float        module_size;
    J_POINT        center;            // Finder Pattern的中心所在坐标
    Int32        found_count;
    Int32        direction;
} J_Finder_Pattern, J_Alignment_Pattern;

/**
 * @brief Symbol（已经找到但并未解码的Symbol）
 */
typedef struct {
    Int32 index;
    Int32 host_index;
    Vector2D side_size;
    Float module_size;
    J_Finder_Pattern* fps;
    J_Metadata metadata;
    Bitmap* sampled_img;
	// 以下内容于19/3/2020增加，为应对图形变质
	Bitmap* bitmap;
	Bitmap* ch[3];
} J_Found_Symbol;


#endif
