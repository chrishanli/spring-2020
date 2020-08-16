// decoder.h    09/03/2020
// J_Code
// brief: the decoder functions
// Han Li adapted from Fraunhofer SIT


#ifndef J_DECODER_H
#define J_DECODER_H

#define DECODE_METADATA_FAILED -1
#define FATAL_ERROR -2	//e.g. out of memory
#define SKIP 255		// 不符合需要的马
#define LOST 254
#define FORCESTOP 256	

#define MASTER_METADATA_X	6
#define MASTER_METADATA_Y	1

#define MASTER_METADATA_PART1_LENGTH 6			//master metadata part 1 encoded length
#define MASTER_METADATA_PART2_LENGTH 38			//master metadata part 2 encoded length
#define MASTER_METADATA_PART1_MODULE_NUMBER 4	//the number of modules used to encode master metadata part 1

/**
 * @brief Decoding tables
*/
static const BYTE j_decoding_table_upper[27]   = {32, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90};
static const BYTE j_decoding_table_lower[27]   = {32, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122};
static const BYTE j_decoding_table_numeric[13] = {32, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 44, 46};
static const BYTE j_decoding_table_punct[16]   = {33, 34, 36, 37, 38, 39, 40, 41, 44, 45, 46, 47, 58, 59, 63, 64};
static const BYTE j_decoding_table_mixed[32]   = {35, 42, 43, 60, 61, 62, 91, 92, 93, 94, 95, 96, 123, 124, 125, 126, 9, 10, 13, 0, 0, 0, 0, 164, 167, 196, 214, 220, 223, 228, 246, 252};
static const BYTE j_decoding_table_alphanumeric[63] = {32, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
															 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122};

/**
 * @brief 编码种类表
*/
typedef enum {
	None = -1,
	Upper = 0,
	Lower,
	Numeric,
	Punct,
	Mixed,
	Alphanumeric,
	Byte,
	ECI,
	FNC1
} j_encode_mode;

// In Decoder
extern J_Data* decodeData(J_Data* bits);
extern void deinterleaveData(J_Data* data);
extern void getNextMetadataModuleInMaster(Int32 matrix_height, Int32 matrix_width, Int32 next_module_count, Int32* x, Int32* y);
extern Int32 readColorPaletteInMaster(Bitmap* matrix, J_Decoded_Symbol* symbol, BYTE* data_map, Int32* module_count, Int32* x, Int32* y);


// In Mask.c
extern void demaskSymbol(J_Data* data, BYTE* data_map, Vector2D symbol_size, Int32 mask_type, Int32 color_number);

// In image_tests.c
extern Boolean saveImage(Bitmap* bitmap, Char* filename);

#endif
