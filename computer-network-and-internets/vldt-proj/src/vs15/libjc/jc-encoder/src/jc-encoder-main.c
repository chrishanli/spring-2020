// jc-encoder-main    09/03/2020
// J_Code
// brief: the encoder functions ports
// Han Li

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jc.h"
#include "encoder/encoder.h"
#include "dll.h"

#define SPECIFIED_COLOUR_NUMBER 8
#define SPECIFIED_SYMBOL_NUMBER 1
#define SPECIFIED_MODULE_SIZE   6
#define SPECIFIED_VERSION_X     26
#define SPECIFIED_VERSION_Y     16
#define SPECIFIED_ECC_LEVEL     2


J_Data*       data = 0;
Char*         filename = 0;
Int32         color_number = 0;
Int32         symbol_number = 0;
Int32         module_size = 0;
Int32         master_symbol_width = 0;
Int32         master_symbol_height = 0;
Vector2D*     symbol_versions = 0;
Int32         symbol_versions_number = 0;
Int32*        symbol_ecc_levels = 0;
Int32         color_space = 0;

/**
* @brief Print usage of JABCode writer
*/
void printUsage() {
	printf("\n");
	printf("J_Code Decoder Library (Version %s Build date: %s)\n  - Adapted by Han Li from Fraunhofer SIT\n  - Build by MSVC %d\n\n\n", VERSION, BUILD_DATE, _MSC_VER);
	printf("Usage:\n\n");
	printf("jc-encoder (--input message-to-encode)| (--input-file file-path) --output output-image [options]\n");
	printf("\n");
	printf("--input\t\t\tInput data (message typed to be encoded).\n");
	printf("--input-file\t\tInput data file.\n");
	printf("--output\t\tOutput image file.\n");
	printf("--color-number\t\tNumber of colors (4 or 8, default:8).\n");
	printf("--module-size\t\tModule size in pixel (default 12 pixels).\n");
	printf("--symbol-width\t\tMaster symbol width in pixel.\n");
	printf("--symbol-height\t\tMaster symbol height in pixel.\n");
	printf("--ecc-level\t\tError correction levels (1-10, default:3(6%%)). \n");
	printf("--symbol-version\tSide-Version of the symbol, for example: 6 4. \n\t\t\t"
		"Default: 29 20.\n");
	printf("--help\t\t\tPrint this help.\n");
	printf("\n");
	printf("Example for automatically generated code: \n");
	printf("jc-encoder --input 'Hello world' --output test.png\n");
	printf("\n");
	printf("Example for custom-made code: \n");
	printf("jc-encoder --input 'Hello world' --output test.png  --symbol-version 6 4\n");
	printf("\n");
}

/**
* @brief Parse command line parameters
* @return 1: success | 0: failure
*/
Boolean parseCommandLineParameters(Int32 para_number, Char* para[]) {
	// 第一次扫描
	for (Int32 loop = 1; loop < para_number; loop++) {
		// 人打字输入讯息
		if (0 == strcmp(para[loop], "--input")) {
			// 不容许重复写入，防止内存碎片（LH优化于13/3/2020）
			if (!data) {
				if (loop + 1 > para_number - 1) {
					printf("Value for option '%s' missing.\n", para[loop]);
					return 0;
				}
				// 加载输入讯息（人打字）
				Char* data_string = para[++loop];
				data = (J_Data *)malloc(sizeof(J_Data) + strlen(data_string) * sizeof(Char));
				if (!data) {
					J_REPORT_ERROR(("Memory allocation for input data failed"))
						return 0;
				}
				data->length = strlen(data_string);
				memcpy(data->data, data_string, strlen(data_string));
			}
			else {
				J_REPORT_ERROR(("Duplicate input commands found"))
			}
		}
		// 汇入文件
		else if (0 == strcmp(para[loop], "--input-file")) {
			if (!data) {
				if (loop + 1 > para_number - 1) {
					printf("Value for option '%s' missing.\n", para[loop]);
					return 0;
				}
				FILE* fp;
				errno_t err = fopen_s(&fp, para[++loop], "rb");
				if (err) {
					J_REPORT_ERROR(("Opening input data file failed"))
						return 0;
				}
				size_t file_size;
				fseek(fp, 0, SEEK_END);
				file_size = ftell(fp);
				data = (J_Data *)malloc(sizeof(J_Data) + file_size * sizeof(Char));
				if (!data) {
					J_REPORT_ERROR(("Memory allocation for input data failed"))
						return 0;
				}
				fseek(fp, 0, SEEK_SET);
				if (fread(data->data, 1, file_size, fp) != file_size) {
					J_REPORT_ERROR(("Reading input data file failed"))
						free(data);
					fclose(fp);
					return 0;
				}
				fclose(fp);
				data->length = file_size;
			}
			else {
				J_REPORT_ERROR(("Duplicate input commands found"))
			}
		}
		// 输出图像文件
		else if (0 == strcmp(para[loop], "--output")) {
			if (loop + 1 > para_number - 1) {
				J_REPORT_ERROR(("Value for option '%s' missing.\n", para[loop]))
					return 0;
			}
			filename = para[++loop];
		}
		// 色彩种类个数（2、4或8）
		else if (0 == strcmp(para[loop], "--color-number")) {
			char* option = para[loop];
			if (loop + 1 > para_number - 1) {
				J_REPORT_ERROR(("Value for option '%s' missing.\n", option))
					return 0;
			}
			char* endptr;
			color_number = (int)strtol(para[++loop], &endptr, 10);
			if (*endptr) {
				J_REPORT_ERROR(("Invalid or missing values for option '%s'.\n", option))
					return 0;
			}
			if (color_number != 4 && color_number != 8) {
				J_REPORT_ERROR(("Invalid color number. Valid color number includes 4 and 8."))
					return 0;
			}
		}
		// 每个module的边长（像素）
		else if (0 == strcmp(para[loop], "--module-size")) {
			char* option = para[loop];
			if (loop + 1 > para_number - 1) {
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
			char* endptr;
			module_size = (int)strtol(para[++loop], &endptr, 10);
			if (*endptr || module_size < 0) {
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
		}
		// symbol期望宽（像素）
		else if (0 == strcmp(para[loop], "--symbol-width")) {
			char* option = para[loop];
			if (loop + 1 > para_number - 1) {
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
			char* endptr;
			master_symbol_width = (int)strtol(para[++loop], &endptr, 10);
			if (*endptr || master_symbol_width < 0) {
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
		}
		// symbol期望高（像素）
		else if (0 == strcmp(para[loop], "--symbol-height")) {
			char* option = para[loop];
			if (loop + 1 > para_number - 1) {
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
			char* endptr;
			master_symbol_height = (int)strtol(para[++loop], &endptr, 10);
			if (*endptr || master_symbol_height < 0) {
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
		}
	}

	// 检查输入
	if (!data) {
		J_REPORT_ERROR(("Input data missing"))
			return 0;
	}
	else if (data->length == 0) {
		J_REPORT_ERROR(("Input data is empty"))
			return 0;
	}
	if (!filename) {
		J_REPORT_ERROR(("Output file missing"))
			return 0;
	}
	if (symbol_number == 0) {
		symbol_number = 1;
	}

	// 第二次扫描（检查ecc、版本号）
	for (Int32 loop = 1; loop<para_number; loop++) {
		// ECC扫描
		if (0 == strcmp(para[loop], "--ecc-level")) {
			char* option = para[loop];
			if (loop + 1 > para_number - 1) {
				J_REPORT_ERROR(("Value for option '%s' missing.\n", option))
					return 0;
			}
			symbol_ecc_levels = (Int32 *)calloc(symbol_number, sizeof(Int32));
			if (!symbol_ecc_levels) {
				J_REPORT_ERROR(("Memory allocation for symbol ecc levels failed"))
					return 0;
			}
			if (loop + 1 > para_number - 1)
				break;
			char* endptr;
			symbol_ecc_levels[0] = (int)strtol(para[++loop], &endptr, 10);
			if (*endptr) {
				loop--;
				break;
			} if (symbol_ecc_levels[0] < 0 || symbol_ecc_levels[0] > 10) {
				J_REPORT_ERROR(("Invalid error correction level (must be 0 or 1 - 10)."))
					return 0;
			}
		}
		// 边长版本
		else if (0 == strcmp(para[loop], "--symbol-version")) {
			char* option = para[loop];
			if (loop + 1 > para_number - 1) {
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
			// 一对一对扫描
			symbol_versions = (Vector2D *)calloc(symbol_number, sizeof(Vector2D));
			if (!symbol_versions) {
				J_REPORT_ERROR(("Memory allocation for symbol versions failed"))
					return 0;
			}
			for (Int32 j = 0; j < symbol_number; j++) {
				if (loop + 1 > para_number - 1) {
					printf("Too few values for option '%s'.\n", option);
					return 0;
				}
				char* endptr;
				symbol_versions[j].x = (int)strtol(para[++loop], &endptr, 10);
				if (*endptr) {
					printf("Invalid or missing values for option '%s'.\n", option);
					return 0;
				}
				if (loop + 1 > para_number - 1) {
					printf("Too few values for option '%s'.\n", option);
					return 0;
				}
				symbol_versions[j].y = (int)strtol(para[++loop], &endptr, 10);
				if (*endptr) {
					J_REPORT_ERROR(("Invalid or missing values for option '%s'.\n", option))
						return 0;
				}
				if (symbol_versions[j].x < 1 || symbol_versions[j].x > 32 || symbol_versions[j].y < 1 || symbol_versions[j].y > 32) {
					J_REPORT_ERROR(("Invalid symbol side version (must be 1 - 32)."))
						return 0;
				}
				symbol_versions_number++;
			}
		}
	}

	// 检查输入
	if (symbol_number > 1 || symbol_versions_number > 1) {
		// 不容许多symbol
		J_REPORT_ERROR(("Multi-symbol not yet implemented."))
	}
	return 1;
}

/**
* @brief 削除使用过的记忆体
*/
void cleanMemory() {
	free(data);
	free(symbol_versions);
	free(symbol_ecc_levels);
}

/**
* @brief JCode writer main function in console
* @return 0: success | 1: failure
*/
int encoder_console_using_pngs_main(int argc, char *argv[]) {
	if (argc < 2 || (0 == strcmp(argv[1], "--help"))) {
		printUsage();
		return 1;
	}
	if (!parseCommandLineParameters(argc, argv)) {
		return 1;
	}

	// 制作Encode对象
	J_Encode* enc = createEncode(color_number, symbol_number);
	if (enc == NULL) {
		cleanMemory();
		J_REPORT_ERROR(("Creating encode parameter failed"))
			return 1;
	}
	if (module_size > 0) {
		enc->module_size = module_size;
	}
	if (master_symbol_width > 0) {
		enc->master_symbol_width = master_symbol_width;
	}
	if (master_symbol_height > 0) {
		enc->master_symbol_height = master_symbol_height;
	}
	if (symbol_ecc_levels)
		enc->symbol_ecc_levels[0] = symbol_ecc_levels[0];
	if (symbol_versions)
		enc->symbol_versions[0] = symbol_versions[0];

	// 转换成Code
	if (generateJCode(enc, data) != 0) {
		J_REPORT_ERROR(("Creating jcode failed"))
			destroyEncode(enc);
		cleanMemory();
		return 1;
	}

	// 保存bitmap至文件
	Int32 result = 0;
	if (color_space == 0) {
		if (!saveImage(enc->bitmap, filename)) {
			J_REPORT_ERROR(("Saving png image failed"))
				result = 1;
		}
	}
	else if (color_space == 1) {
		J_REPORT_ERROR(("%s", "Save to CMYK deprecated."))
	}

	// 削除Encode对象
	// 削除暂存记忆体
	destroyEncode(enc);
	cleanMemory();
	return result;
}


/**
Encoder a J_Data format data and return a Bitmap

@param data J_Data数据文件
@return Bitmap
*/
Bitmap* encoder_using_J_Data(J_Data* data) {
	if (!data) {
		J_REPORT_ERROR(("Imported j_data failed\n"));
		return NULL;
	}
	// 制作Encode对象
	J_Encode* enc = createEncode(color_number, symbol_number);
	if (enc == NULL) {
		cleanMemory();
		J_REPORT_ERROR(("Creating encode parameter failed"))
			return NULL;
	}
	// 应用缺省方案
	Vector2D* symbol_version = (Vector2D *)malloc(sizeof(Vector2D));
	symbol_version[0].x = SPECIFIED_VERSION_X;
	symbol_version[0].y = SPECIFIED_VERSION_Y;

	Int32 ecc_version = SPECIFIED_ECC_LEVEL;
	enc->symbol_versions = symbol_version;
	enc->symbol_ecc_levels[0] = ecc_version;

	// 转换成JCode
	if (generateJCode(enc, data) != 0) {
		J_REPORT_ERROR(("Creating j_code failed"))
			destroyEncode(enc);
		return NULL;
	}

	// 削除Encode对象
	Bitmap* ret = destroyAndExtractBitmapFromEncode(enc);
	// 削除暂存记忆体
	cleanMemory();
	return ret;
}
