// jc-decoder-main    09/03/2020
// J_Code
// brief: the decoder functions ports
// Han Li

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dll.h"
#include "jc_decoder.h"

#define FATAL_ERROR -2	//e.g. out of memory
#define SKIP 255		// 不符合需要的马
#define LOST 254
#define FORCESTOP 256

/**
 * @brief Print usage of JCode reader
 */
void printUsage() {
    printf("\n");
    printf("J_Code Decoder Library (Version %s Build date: %s)\n  - Adapted by Han Li from Fraunhofer SIT\n  - Build by MSVC Version %d\n\n\n", VERSION, BUILD_DATE, _MSC_VER);
    printf("Usage:\n\n");
    printf("jc-decoder input-image(png) [--output output-file]\n");
    printf("\n");
    printf("--output\tOutput file for decoded data.\n");
    printf("--help\t\tPrint this help.\n");
    printf("\n");
}


/**
 * @brief JCode reader main function in console
 * @return 0: success | 255: not detectable | other positive-non-zero: decoding failed
 */
int decoder_console_using_pngs_main(int argc, char **argv) {
    if(argc < 2 || (0 == strcmp(argv[1],"--help"))) {
        printUsage();
        return 255;
    }
    
    Boolean output_as_file = 0;
    if(argc > 2) {
        if(0 == strcmp(argv[2], "--output"))
            output_as_file = 1;
        else {
            printf("Unknown parameter: %s\n", argv[2]);
            return 255;
        }
    }
    
    // find and decode JCode in the image
    Int32 decode_status;
    J_Decoded_Symbol symbols[MAX_SYMBOL_NUMBER];
    J_Data* decoded_data = decodeJCodeEx(argv[1], NORMAL_DECODE, &decode_status, symbols, 0);
    if(decoded_data == NULL) {
        J_REPORT_ERROR(("Decoding JCode failed"));
        if (decode_status > 0)
            return (Int32)(symbols[0].module_size + 0.5f);
        else
            return 255;
    }
    
    
    // output result
    if(output_as_file) {
        // 文件末尾追加内容 26/3/2020适应VS安全检查机制
        /*FILE* output_file = fopen_s(argv[3], "wb");*/
		FILE* output_file;
		errno_t err = fopen_s(&output_file, argv[3], "wb");
        if(err) {
           printf("Can not open the output file");
           return 255;
        }
        // 将第1个字节开始才写入。第0字节将会被认为是定界符
        fwrite(decoded_data->data + 1, decoded_data->length, 1, output_file);
        fclose(output_file);
    }
    else {
        for(Int32 i = 1; i<decoded_data->length; i++)
            printf("%c", decoded_data->data[i]);
        printf("\n");
    }
    
    // free(bitmap);
    free(decoded_data);
    return 0;
}


/**
 JCode reader main function, using found symbols to decode

 @param found_ms found symbols accepted from libjc-finder
 @param _correspond_no the number that is expected to correspond with that is decoded (0-255)
 @param _dst_file write-to-file source path
 @return 0: success | 255: not detectable | other positive-non-zero: decoding failed | -1: not correspond | -2: force stop | -3: lost detected
 */
Int32 decoder_using_found_ms_main(J_Found_Symbol* found_ms,
<<<<<<< HEAD:src/vs15/libjc/jc-decoder/src/jc-decoder-main.c
                                  int *_correspond_no,
=======
                                  int *_correspond_no,			// �Ľ���21/3/2020 
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/jcode/jc_decoder/jc-decoder-main.c
                                  const Char _dst_file[],
								  const Char _dst_val_file[]) {
    Int32 decode_status;
    J_Decoded_Symbol symbols[MAX_SYMBOL_NUMBER];
    J_Data* decoded_data = decodeJCodeEx_using_found_symbol(found_ms, NORMAL_DECODE, &decode_status, symbols, *_correspond_no);
    
    if(decoded_data == NULL) {	// 有两种可能，可能跳过了，有可能是紧急停止，有可能是错误
		if (decode_status == SKIP) {
			// 被跳过
			return -1;	// not correspond
		} else if (decode_status == FORCESTOP) {
			return -2;	// force stop
		} else {
			// 解码失败
			// J_REPORT_ERROR(("Decoding JCode failed"));
			if (decode_status > 0)
				return (Int32)(symbols[0].module_size + 0.5f);	// 输出失败位置
			else
				return 255;
        }
    }
    
    // printf(" DEBUG_INFO: decoded_data is not null");
    if (decode_status == LOST) {
		//printf(" DEBUG_INFO: detected a lost data, doing records...\n");
    	// 丢包了。置位为0，并且输出错误讯息
		FILE* output_file;
		errno_t err = fopen_s(&output_file, _dst_file, "ab+");
		if(err) {
        	J_REPORT_ERROR(("Can not open the output file"))
        	free(decoded_data);
        	return 255;
    	}
    	//printf(" DEBUG_INFO: fetching new_frame_no...\n");
    	// 拿到最新帧帧号 
    	int new_frame_no = *((int *)decoded_data->data);
    	//printf(" DEBUG_INFO: opening output_file...\n");
    	// 失效帧填0 
    	BYTE* zero_buff = (BYTE *)malloc(decoded_data->length - sizeof(int));
    	memset(zero_buff, 0, decoded_data->length - sizeof(int));
    	// LH 改进于28/3/2020
		if (*_correspond_no == 0) {
			// 首帧丢失，则不管首帧，只需要非0帧填0就好
			for (int i = 1; i < new_frame_no; i++) {
				fwrite(zero_buff, decoded_data->length - sizeof(int), 1, output_file);
			}
		}
		else {
			// 非首帧丢失，严格填0
			for (int i = *_correspond_no; i < new_frame_no; i++) {
				fwrite(zero_buff, decoded_data->length - sizeof(int), 1, output_file);
			}
		}

		// 写入本帧
		fwrite(decoded_data->data + sizeof(int), decoded_data->length - sizeof(int), 1, output_file);
		fclose(output_file);
		
		//printf(" DEBUG_INFO: opening_val_file...\n");
		// 打开valid_file，填写信息
		FILE* valid_file;
		err = fopen_s(&valid_file, _dst_val_file, "ab+");
		if(err) {
        	J_REPORT_ERROR(("Can not open the verify check file"))
        	free(zero_buff);
        	free(decoded_data);
        	return 255;
    	}
    	// 失效帧填0：失败 
    	//printf(" DEBUG_INFO: writing 0 for errors...\n");
		if (*_correspond_no == 0) {
			for (int i = 1; i < new_frame_no; i++) {
				fwrite(zero_buff, decoded_data->length - sizeof(int), 1, valid_file);
			}
		}
		else {
			for (int i = *_correspond_no; i < new_frame_no; i++) {
				fwrite(zero_buff, decoded_data->length - sizeof(int), 1, valid_file);
			}
		}
    	
		//printf(" DEBUG_INFO: setting 0xff to current frame...\n");
		memset(zero_buff, 0xFF, decoded_data->length - sizeof(int));
		// 写入本帧
		//printf(" DEBUG_INFO: writing 0xff to current frame...\n");
		fwrite(zero_buff, decoded_data->length - sizeof(int), 1, valid_file);
    	fclose(valid_file);
    	// 修改最新帧号
		*_correspond_no = new_frame_no;
		free(zero_buff);
		free(decoded_data);
		return -3;	// 帧丢失 
	}
    
    // success:
    
    // 文件末尾追加内容，当且仅当定界符匹配才追加（9/3/2020）
    // TODO - 能否让解码器看到这玩意不correspond就停止解码？（19/3/2020）
	// 26.3.2020 适应MSVC安全检查
	FILE* output_file; 
	errno_t err_1 = fopen_s(&output_file, _dst_file, "ab+");
	FILE* valid_file; 
	errno_t err_2 = fopen_s(&valid_file, _dst_val_file, "ab+");
	if(err_1 || err_2) {
        J_REPORT_ERROR(("Can not open the output file or the verify check file"))
        free(decoded_data);
        return 255;
    }
    
    BYTE* one_buff = (BYTE *)malloc(decoded_data->length - sizeof(int));
    memset(one_buff, 0xFF, decoded_data->length - sizeof(int));
	// 将第1个字节开始才写入。第0字节将会被认为是定界符。但解码时已帮我们扔掉（19/3/2020否决）
    fwrite(decoded_data->data + sizeof(int), decoded_data->length - sizeof(int), 1, output_file);
    fwrite(one_buff, decoded_data->length - sizeof(int), 1, valid_file);
    
    fclose(output_file);
    fclose(valid_file);
    free(decoded_data);
	free(one_buff);
    return 0;
}
