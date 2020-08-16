/**
 * @file encoder.c	编码器 (C)
 * @date 13/3/2020	18/13/2020
 * @brief Symbol encoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jc.h"
#include "encoder.h"
#include "../supports/ldpc.h"

/**
 * @brief 根据index的值计算出［下一个］metadata中的module所处的位置
 * @param matrix_height the height of the matrix
 * @param matrix_width the width of the matrix
 * @param next_module_count ［下一个］metadata中的module的编号
 * @param x 传入目前x坐标，将被更改为下一个module的x坐标
 * @param y 同上：the y coordinate of the current and the next module
 */
void getNextMetadataModuleInMaster(Int32 matrix_height, Int32 matrix_width, Int32 next_module_count, Int32* x, Int32* y) {
    // 细节请参考原文档Figure 8 (page 26) 黑字是metadata的位置
    // 其实是按照模4转圈找点的！
    
    // 如果下一个点在［左上角］或［右下角］，就［先］把取样点按矩阵的水平中线 做 轴对称
    if (next_module_count % 4 == 0 || next_module_count % 4 == 2) {
        (*y) = matrix_height - 1 - (*y);    // 留意：-1是因为(0,0)指的是最左上端的那个点
    }
    // 如果下一个点在［右上角］或［左下角］，就［先］把取样点按矩阵的垂直中线 做 轴对称
    if (next_module_count % 4 == 1 || next_module_count % 4 == 3) {
        (*x) = matrix_width - 1 - (*x);     // 留意：-1是因为(0,0)指的是最左上端的那个点
    }
    // 如果下一个点是模4点，则表示一圈查找结束。应该更换另一个查找圈。
    if (next_module_count % 4 == 0) {
        // 如果下一个index是0～20及44～68的点，其下一个取样点都在原取样点的正下方
        if( next_module_count <= 20 ||
           (next_module_count >= 44  && next_module_count <= 68)  ||
           (next_module_count >= 96  && next_module_count <= 124) ||
           (next_module_count >= 156 && next_module_count <= 172)) {
            (*y) += 1;
        }
        // 如果下一个index是20～44及68～96的点，其下一个取样点都在原取样点的正左方
        else if((next_module_count > 20  && next_module_count < 44) ||
                (next_module_count > 68  && next_module_count < 96) ||
                (next_module_count > 124 && next_module_count < 156)) {
            (*x) -= 1;
        }
    }
    // 如果下一个index是44就要更换位置。更换方法是取原点关于主对角线的对称点
    if(next_module_count == 44 || next_module_count == 96 || next_module_count == 156) {
        Int32 tmp = (*x);
        (*x) = (*y);
        (*y) = tmp;
    }
}


/**
 * @brief 设置本次编码使用的系统色盘
 * @param color_number the number of colors
 * @param palette the color palette
 */
void setDefaultPalette(Int32 color_number, BYTE* palette) {
    // 如果要调出4种颜色（2bit/module）
    if (color_number == 4) {
    	memcpy(palette + 0, J_Default_Palette + FP0_CORE_COLOR * 3, 3);	//黑     000 for 00
    	memcpy(palette + 3, J_Default_Palette + 5 * 3, 3);				//洋红    101 for 01
    	memcpy(palette + 6, J_Default_Palette + FP2_CORE_COLOR * 3, 3);	//黄     110 for 10
    	memcpy(palette + 9, J_Default_Palette + FP3_CORE_COLOR * 3, 3);	//蓝绿    011 for 11
    }
    // 如果要调出8种颜色（4bit/module）
    else {
        // 直接套用整个色盘
        for(Int32 i = 0; i < color_number * 3; i++) {
            palette[i] = J_Default_Palette[i];
        }
    }
    // 如果要调出更多种颜色（xbit/module）（删去）
}

/**
 * @brief 设置纠错等级信息存放的位置 / Set default error correction levels
 * @param symbol_number symbol数量（恒为1）
 * @param ecc_levels the ecc_level for each symbol
 */
void setDefaultEccLevels(Int32 symbol_number, BYTE* ecc_levels) {
    memset(ecc_levels, 0, symbol_number * sizeof(BYTE));
}

/**
 * @brief Swap two integer elements
 * @param a the first element
 * @param b the second element
 */
void swap_int(Int32 * a, Int32 * b)
{
    Int32 temp=*a;
    *a=*b;
    *b=temp;
}

/**
 * @brief Swap two byte elements
 * @param a the first element
 * @param b the second element
 */
void swap_byte(BYTE * a, BYTE * b)
{
    BYTE temp=*a;
    *a=*b;
    *b=temp;
}

/**
 * @brief 将一个十进制整数转为二进制数
 * @param dec 十进制整数
 * @param bin 二进制数，用BYTE表达
 * @param start_position ？the position to write in encoded data array
 * @param length 二进制编码长度
 */
void convert_dec_to_bin(Int32 dec, Char* bin, Int32 start_position, Int32 length) {
    if(dec < 0) dec += 256;
    // 循环取余数
    for (Int32 j=0; j<length; j++) {
        Int32 t = dec % 2;
        bin[start_position+length-1-j] = (Char)t;
        dec /= 2;
    }
}



/**
 * @brief 创建一个JCode对象
 * @param color_number 颜色种类个数 / the number of module colors
 * @param symbol_number Symbol的数量
 * @return JCode对象 | NULL: 错误 (out of memory)
 */
J_Encode* createEncode(Int32 color_number, Int32 symbol_number) {
    J_Encode *enc;
    enc = (J_Encode *)calloc(1, sizeof(J_Encode));
    if(enc == NULL)
        return NULL;

    // 如果用户输入不合法的颜色种类个数，就扔掉，使用系统自定义（8种颜色）？
    if(color_number != 4  && color_number != 8   && color_number != 16 &&
       color_number != 32 && color_number != 64 && color_number != 128 && color_number != 256) {
        color_number = DEFAULT_COLOR_NUMBER;    // （8种颜色）
    }
    // 是否有必要？
    if(symbol_number < 1 || symbol_number > MAX_SYMBOL_NUMBER)
        symbol_number = DEFAULT_SYMBOL_NUMBER;  // 只有1个Master

    // 写入配置
    enc->color_number 		 = color_number;
    enc->symbol_number 		 = symbol_number;
    enc->master_symbol_width = 0;
    enc->master_symbol_height= 0;
    enc->module_size 		 = DEFAULT_MODULE_SIZE; // 12（供图片输出时使用：1个module边长为12）

    // 使用系统缺省色盘调色
	enc->palette = (BYTE *)calloc(color_number * 3, sizeof(BYTE));
    if(enc->palette == NULL) {
		J_REPORT_ERROR(("Memory allocation for palette failed"))
        return NULL;
    }
    setDefaultPalette(enc->color_number, enc->palette); // 调色
    // 准备一个地方存放version / allocate memory for symbol versions
    enc->symbol_versions = (Vector2D *)calloc(symbol_number, sizeof(Vector2D));
    if(enc->symbol_versions == NULL) {
		J_REPORT_ERROR(("Memory allocation for symbol versions failed"))
        return NULL;
    }
    // 设置ECC（查错等级）存储位置
    enc->symbol_ecc_levels = (BYTE *)calloc(symbol_number, sizeof(BYTE));
    if(enc->symbol_ecc_levels == NULL) {
		J_REPORT_ERROR(("Memory allocation for ecc levels failed"))
        return NULL;
    }
    setDefaultEccLevels(enc->symbol_number, enc->symbol_ecc_levels);
    // 准备一个地方存放symmbol位置 / allocate memory for symbol positions
    enc->symbol_positions= (Int32 *)calloc(symbol_number, sizeof(Int32));
    if(enc->symbol_positions == NULL) {
		J_REPORT_ERROR(("Memory allocation for symbol positions failed"))
        return NULL;
    }
    // 准备一个地方存放symbol数据 / allocate memory for symbols
    enc->symbols = (J_Symbol *)calloc(symbol_number, sizeof(J_Symbol));
    if(enc->symbols == NULL) {
		J_REPORT_ERROR(("Memory allocation for symbols failed"))
        return NULL;
    }
    return enc;
}

/**
 * @brief 销毁整个encode对象
 * @param enc encode对象
 */
void destroyEncode(J_Encode* enc) {
    free(enc->palette);
    free(enc->symbol_versions);
    free(enc->symbol_ecc_levels);
    free(enc->symbol_positions);
    free(enc->bitmap);
    if(enc->symbols) {
        for(Int32 i=0; i<enc->symbol_number; i++)
        {
        	free(enc->symbols[i].data);
            free(enc->symbols[i].data_map);
            free(enc->symbols[i].metadata);
            free(enc->symbols[i].matrix);
        }
        free(enc->symbols);
    }
    free(enc);
}

/**
 * @brief 销毁encode对象，但保留其中Bitmap并返回
 * @param enc encode对象
 */
Bitmap* destroyAndExtractBitmapFromEncode(J_Encode* enc) {
    free(enc->palette);
    free(enc->symbol_versions);
    free(enc->symbol_ecc_levels);
    free(enc->symbol_positions);
    Bitmap* bitmap = enc->bitmap;
    if(enc->symbols) {
        for(Int32 i=0; i<enc->symbol_number; i++)
        {
            free(enc->symbols[i].data);
            free(enc->symbols[i].data_map);
            free(enc->symbols[i].metadata);
            free(enc->symbols[i].matrix);
        }
        free(enc->symbols);
    }
    free(enc);
    
    return bitmap;
}

// 敲黑板！！！
/**
 * @brief 分析输入数据，然后对每个【字符的样式】给出【最优编码方案】 / Analyze the input data and determine the optimal encoding modes for each character
 * @param input 输入的【字符或其他类型】数据
 * @param encoded_length 最短的编码长度
 * @return 【最优编码方案】数组 | NULL: fatal error (out of memory)
 */
Int32* analyzeInputData(J_Data* input, Int32* encoded_length) {
    Int32 encode_seq_length = ENC_MAX;  // 被初始化为一个很大的数字
    // 为检视输入数据而制作的临时空间
    Char* seq = (Char *)malloc(sizeof(Char) * input->length);
    if(seq == NULL) {
		J_REPORT_ERROR(("Memory allocation for sequence failed"))
        return NULL;
    }
    Int32* curr_seq_len = (Int32 *)malloc(sizeof(Int32)*((input->length + 2) * 14));
    if(curr_seq_len == NULL){
		J_REPORT_ERROR(("Memory allocation for current sequence length failed"))
        free(seq);
        return NULL;
    }
    Int32* prev_mode = (Int32 *)malloc(sizeof(Int32)*(2*input->length+2)*14);
    if(prev_mode == NULL){
		J_REPORT_ERROR(("Memory allocation for previous mode failed"))
        free(seq);
        free(curr_seq_len);
        return NULL;
    }
    for (Int32 i = 0; i < (2 * input->length + 2) * 14; i++)
        prev_mode[i] = ENC_MAX / 2;

    Int32* switch_mode = (Int32 *)malloc(sizeof(Int32) * 28);
    if(switch_mode == NULL){
		J_REPORT_ERROR(("Memory allocation for mode switch failed"))
        free(seq);
        free(curr_seq_len);
        free(prev_mode);
        return NULL;
    }
    for (Int32 i = 0; i < 28; i++)
        switch_mode[i] = ENC_MAX / 2;
    Int32* temp_switch_mode = (Int32 *)malloc(sizeof(Int32) * 28);
    if(temp_switch_mode == NULL){
		J_REPORT_ERROR(("Memory allocation for mode switch failed"))
        free(seq);
        free(curr_seq_len);
        free(prev_mode);
        free(switch_mode);
        return NULL;
    }
    //printf("analyzing memory allocation succeeded\n");
    //for (Int32 i = 0; i < 28; i++)
        //temp_switch_mode[i] = ENC_MAX/2;
    // LH优化于12/3/2020
    memcpy(temp_switch_mode, switch_mode, 28 * sizeof(Int32));

    // calculate the shortest encoding sequence
    // initialize start in upper case mode; no previous mode available
    for (Int32 k = 0; k < 7; k++) {
        curr_seq_len[k] = curr_seq_len[k + 7] = ENC_MAX;
        prev_mode[k] = prev_mode[k + 7] = ENC_MAX;
    }

	//printf("shortest encoding sequence analyzing succeeded\n");
    curr_seq_len[0]=0;
    BYTE jp_to_nxt_char=0, confirm=0;
    Int32 curr_seq_counter=0;
    Boolean is_shift=0;
    Int32 nb_char=0;
    Int32 end_of_loop = input->length;
    Int32 prev_mode_index=0;
    // 检视每一个字符
    for (Int32 i = 0; i < end_of_loop; i++) {
        Int32 tmp = input->data[nb_char];
        Int32 tmp1 = 0;
        if(nb_char + 1 < input->length)
            tmp1 = input->data[nb_char + 1];
        if(tmp < 0)
            tmp = 256+tmp;
        if(tmp1 < 0)
            tmp1 = 256+tmp1;
        curr_seq_counter++;
        for (Int32 j = 0; j < J_ENCODING_MODES; j++) { // J_ENCODING_MODES = 6
            // check if character is in encoding table
            if (j_enconing_table[tmp][j]>-1 && j_enconing_table[tmp][j]<64)
                curr_seq_len[(i+1)*14+j]=curr_seq_len[(i+1)*14+j+7]=character_size[j];
            else if((j_enconing_table[tmp][j]==-18 && tmp1==10) || (j_enconing_table[tmp][j]<-18 && tmp1==32)) // read next character to decide if encodalbe in current mode
            {
                curr_seq_len[(i+1)*14+j]=curr_seq_len[(i+1)*14+j+7]=character_size[j];
                jp_to_nxt_char=1; //jump to next character
            }
            else // not encodable in this mode
                curr_seq_len[(i+1)*14+j]=curr_seq_len[(i+1)*14+j+7]=ENC_MAX;
        }
        // 用byte mode总是可以编码所有字符
        curr_seq_len[(i+1)*14+6]=curr_seq_len[(i+1)*14+13]=character_size[6];
        is_shift = 0;
        for (Int32 j = 0; j < 14; j++) {
            Int32 prev=-1;
            Int32 len=curr_seq_len[(i+1)*14+j]+curr_seq_len[i*14+j]+latch_shift_to[j][j];
            prev_mode[curr_seq_counter*14+j]=j;
            for (Int32 k = 0; k < 14; k++) {
                if((len>=curr_seq_len[(i+1)*14+j]+curr_seq_len[i*14+k]+latch_shift_to[k][j] && k<13) || (k==13 && prev==j)) {
                    len=curr_seq_len[(i+1)*14+j]+curr_seq_len[i*14+k]+latch_shift_to[k][j];
                    if (temp_switch_mode[2*k]==k)
                        prev_mode[curr_seq_counter*14+j]=temp_switch_mode[2*k+1];
                    else
                        prev_mode[curr_seq_counter*14+j]=k;
                    if (k==13 && prev==j)
                        prev=-1;
                }
            }
            curr_seq_len[(i+1)*14+j]=len;
            // shift back to mode if shift is used
            if (j > 6) {
                if ((curr_seq_len[(i+1)*14+prev_mode[curr_seq_counter*14+j]]>len ||
                    (jp_to_nxt_char==1 && curr_seq_len[(i+1)*14+prev_mode[curr_seq_counter*14+j]]+character_size[(prev_mode[curr_seq_counter*14+j])%7]>len)) &&
                     j != 13) {
                    Int32 index=prev_mode[curr_seq_counter*14+j];
                    Int32 loop=1;
                    while (index>6 && curr_seq_counter-loop >= 0) {
                        index=prev_mode[(curr_seq_counter-loop)*14+index];
                        loop++;
                    }

                    curr_seq_len[(i+1)*14+index]=len;
                    prev_mode[(curr_seq_counter+1)*14+index]=j;
                    switch_mode[2*index]=index;
                    switch_mode[2*index+1]=j;
                    is_shift=1;
                    if(jp_to_nxt_char==1 && j==11) {
                        confirm=1;
                        prev_mode_index=index;
                    }
                }
                else if ((curr_seq_len[(i+1)*14+prev_mode[curr_seq_counter*14+j]]>len ||
                        (jp_to_nxt_char==1 && curr_seq_len[(i+1)*14+prev_mode[curr_seq_counter*14+j]]+character_size[prev_mode[curr_seq_counter*14+j]%7]>len)) && j == 13 )
                   {
                       curr_seq_len[(i+1)*14+prev_mode[curr_seq_counter*14+j]]=len;
                       prev_mode[(curr_seq_counter+1)*14+prev_mode[curr_seq_counter*14+j]]=j;
                       switch_mode[2*prev_mode[curr_seq_counter*14+j]]=prev_mode[curr_seq_counter*14+j];
                       switch_mode[2*prev_mode[curr_seq_counter*14+j]+1]=j;
                       is_shift=1;
                   }
                if(j!=13)
                    curr_seq_len[(i+1)*14+j]=ENC_MAX;
                else
                    prev=prev_mode[curr_seq_counter*14+j];
            }
        }
        for (Int32 j=0;j<28;j++) {
            temp_switch_mode[j]=switch_mode[j];
            switch_mode[j]=ENC_MAX/2;
        }

        if(jp_to_nxt_char==1 && confirm==1) {
            for (Int32 j=0;j<=2*J_ENCODING_MODES+1;j++)
            {
                if(j != prev_mode_index)
                    curr_seq_len[(i+1)*14+j]=ENC_MAX;
            }
            nb_char++;
            end_of_loop--;

        }
        jp_to_nxt_char=0;
        confirm=0;
        nb_char++;
    }

    // 选择最后一步中最小的数字，作为可编码最小容量 / pick smallest number in last step
    Int32 current_mode=0;
    for (Int32 j=0;j <= 2*J_ENCODING_MODES+1;j++) {
        if (encode_seq_length>curr_seq_len[(nb_char-(input->length-end_of_loop))*14+j]) {
            encode_seq_length = curr_seq_len[(nb_char-(input->length-end_of_loop))*14+j];
            current_mode=j;
        }
    }
	//printf("pick smallest number succeeded \n");
	
    if(current_mode > 6)
        is_shift=1;
    if (is_shift && temp_switch_mode[2*current_mode+1]<14)
        current_mode=temp_switch_mode[2*current_mode+1];

    // 创建一个数组，用来存储【每步的编码方法】，长度应该为【当前统计了的数量】
    Int32* encode_seq = (Int32 *)malloc(sizeof(Int32) * (curr_seq_counter + 1 + is_shift));
    if(encode_seq == NULL) {
		J_REPORT_ERROR(("Memory allocation for encode sequence failed"))
        return NULL;
    }
	
	//printf("encode_seq succeeded\n");

    // 检视Byte Mode是否有被用超过15次
    // 若有->> 该段所需编码长度再加13（请参考官方文档，＋13是用来表达接下来Byte Mode将存在的字节数）
    Int32 counter=0;
    Int32 seq_len=0;
    encode_seq[curr_seq_counter]=current_mode;//prev_mode[(curr_seq_counter)*14+current_mode];//prev_mode[(curr_seq_counter+is_shift-1)*14+current_mode];
    seq_len+=character_size[encode_seq[curr_seq_counter]%7];
    for (Int32 i=curr_seq_counter; i > 0; i--) {
        // 我探测到了一个模式
        if (encode_seq[i]==13 || encode_seq[i]==6)
            counter++;
        else {
            // 如果真的大于15
            if(counter > 15) {
                encode_seq_length+=13;
                seq_len+=13;
                counter=0;
            }
        }
        if (encode_seq[i]<14 && i-1!=0) {
            encode_seq[i-1]=prev_mode[i*14+encode_seq[i]];
            seq_len+=character_size[encode_seq[i-1]%7];
            if(encode_seq[i-1]!=encode_seq[i])
                seq_len+=latch_shift_to[encode_seq[i-1]][encode_seq[i]];
        }
        else if (i-1==0) {
            encode_seq[i-1]=0;
            if(encode_seq[i-1]!=encode_seq[i])
                seq_len+=latch_shift_to[encode_seq[i-1]][encode_seq[i]];
            if(counter>15) {
                encode_seq_length+=13;
                seq_len+=13;
                counter=0;
            }
        }
        else
            return NULL;
    }
	// printf("encode succeeded\n");
    *encoded_length=encode_seq_length;
    free(seq);
    free(curr_seq_len);
    free(prev_mode);
    free(switch_mode);
    free(temp_switch_mode);
    return encode_seq;
}

/**
 * @brief 检视Master Symbol是否要被用缺省模式编码 / Check if master symbol shall be encoded in default mode
 * @param enc Encode对象 / the encode parameters
 * @return SUCCESS | FAILURE
*/
Boolean isDefaultMode(J_Encode* enc) {
	if(enc->color_number == 8 && (enc->symbol_ecc_levels[0] == 0 || enc->symbol_ecc_levels[0] == DEFAULT_ECC_LEVEL)) {
		return SUCCESS;
	}
	return FAILURE;
}

/**
 * @brief 计算编码后的metadata长度
 * @param enc encode对象
 * @param index symbol编号
 * @return metadata长度
*/
Int32 getMetadataLength(J_Encode* enc, Int32 index) {
    Int32 length = 0;

    if (index == 0) { 
    	// defaut mode：暂时没有metadata
    	if(isDefaultMode(enc)) {
			length = 0;
		}
		else {
			// Part I
			length += MASTER_METADATA_PART1_LENGTH;
			// Part II
			length += MASTER_METADATA_PART2_LENGTH;
		}
    }
#ifdef MULTY_SYMBOL
    else //slave symbol, the original net length
    {
    	//Part I
        length += 2;
        //Part II
        Int32 host_index = enc->symbols[index].host;
        //V in Part II, compare symbol shape and size with host symbol
        if (enc->symbol_versions[index].x != enc->symbol_versions[host_index].x || enc->symbol_versions[index].y != enc->symbol_versions[host_index].y)
		{
			length += 5;
		}
        //E in Part II
        if (enc->symbol_ecc_levels[index] != enc->symbol_ecc_levels[host_index])
        {
            length += 6;
        }
    }
#else
    else {
        return -1;
    }
#endif
    return length;
}

/**
 * @brief 计算当前symbol设置决定的数据容量
 * @param enc Encode对象
 * @param index Symbol的编号（只会是0）
 * @return 数据容量
 */
Int32 getSymbolCapacity(J_Encode* enc, Int32 index) {
	// finder patterns所需module数量
    Int32 nb_modules_fp;    // 4 * 17 = 68
#ifdef MULTI_SYMBOL
    if(index == 0) {	//master symbol
		nb_modules_fp = 4 * 17;
	}
	else {              //slave symbol
		nb_modules_fp = 4 * 7;
	}
#else
    nb_modules_fp = 4 * 17;
#endif
    // 色盘所需module数量
//    Int32 nb_modules_palette = enc->color_number > 64 ? (64-2)*COLOR_PALETTE_NUMBER : (enc->color_number-2)*COLOR_PALETTE_NUMBER;
    Int32 nb_modules_palette = (enc->color_number-2)*COLOR_PALETTE_NUMBER;
	// alignment pattern 所需module数量
	Int32 side_size_x = VERSION2SIZE(enc->symbol_versions[index].x);
	Int32 side_size_y = VERSION2SIZE(enc->symbol_versions[index].y);
	Int32 number_of_aps_x = j_ap_num[enc->symbol_versions[index].x - 1];
	Int32 number_of_aps_y = j_ap_num[enc->symbol_versions[index].y - 1];
	Int32 nb_modules_ap = (number_of_aps_x * number_of_aps_y - 4) * 7;
	// metadata 所需module数量
	Int32 nb_of_bpm = log(enc->color_number) / log(2);
	Int32 nb_modules_metadata = 0;
#ifdef MULTI_SYMBOL
	if(index == 0)	//master symbol
	{
#endif
        // 获取metadata数据【长度】
		Int32 nb_metadata_bits = getMetadataLength(enc, index);
		if(nb_metadata_bits > 0) {
            // PartII的module
			nb_modules_metadata = (nb_metadata_bits - MASTER_METADATA_PART1_LENGTH) / nb_of_bpm; 
			if((nb_metadata_bits - MASTER_METADATA_PART1_LENGTH) % nb_of_bpm != 0) {
				nb_modules_metadata++;
			}
            // PartI的module
			nb_modules_metadata += MASTER_METADATA_PART1_MODULE_NUMBER; 
		}
#ifdef MULTI_SYMBOL
	}
#endif
	Int32 capacity = (side_size_x*side_size_y - nb_modules_fp - nb_modules_ap - nb_modules_palette - nb_modules_metadata) * nb_of_bpm;
	return capacity;
}

/**
 * @brief 获得最优ECC方案
 * @param capacity Symbol容量
 * @param net_data_length 数据净含量
 * @param wcwr the LPDC parameters wc and wr
 */
void getOptimalECC(Int32 capacity, Int32 net_data_length, Int32* wcwr) {
	Float min = capacity;
	for (Int32 k=3; k<=6+2; k++) {
		for (Int32 j=k+1; j<=6+3; j++) {
			Int32 dist = (capacity/j)*j - (capacity/j)*k - net_data_length; //max_gross_payload = floor(capacity / wr) * wr
			if(dist<min && dist>=0) {
				wcwr[1] = j;
				wcwr[0] = k;
				min = dist;
			}
		}
	}
}

/**
 * @brief 用先前就算好的【最优编码方法】，编码输入的数据
 * @param data 输入的数据
 * @param encoded_length 最优编码长度
 * @param encode_seq 最优编码方法序列
 * @return the encoded data | NULL if failed
 */
J_Data* encodeData(J_Data* data, Int32 encoded_length, Int32* encode_seq) {
    // 编码后数据存放位置
    J_Data* encoded_data = (J_Data *)malloc(sizeof(J_Data) + encoded_length*sizeof(Char));
    if(encoded_data == NULL) {
		J_REPORT_ERROR(("Memory allocation for encoded data failed"))
        return NULL;
    }
    // 编码后数据的长度，用之前就计算出来的那个
    encoded_data->length = encoded_length;

    Int32 counter=0;
    Boolean shift_back=0;
    Int32 position=0;
    Int32 current_encoded_length=0;
    Int32 end_of_loop = data->length; // 将针对每个字元进行编码
    Int32 byte_offset=0;
    Int32 byte_counter=0;
    
    // 编码将开始于大写字母模式，针对每个字元进行编码
    for (Int32 i = 0;i < end_of_loop;i++) {
        Int32 tmp = data->data[current_encoded_length]; // 当前字元（有效位只有8位）
        if (tmp < 0)
            tmp += 256;                                 // 如果是-1，其实就代表255，强制类型转换了
        if (position < encoded_length) {
            Int32 decimal_value;
            // 检视：对于当前及下一组编码，编码模式是否需要更换 / check if mode is switched
            if (encode_seq[counter] != encode_seq[counter + 1]) {
                // 告诉解码器：将会更换模式。查表
                Int32 length = latch_shift_to[encode_seq[counter]][encode_seq[counter+1]];
                if(encode_seq[counter+1] == 6 || encode_seq[counter+1] == 13)
                    length-=4;
                if(length < ENC_MAX)
                    // 将更换模式讯息编码进symbol中
                    convert_dec_to_bin(mode_switch[encode_seq[counter]][encode_seq[counter+1]], encoded_data->data, position, length);
                else {
					J_REPORT_ERROR(("Encoding data failed"))
                    free(encoded_data);
                    return NULL;
                }
                position += latch_shift_to[encode_seq[counter]][encode_seq[counter+1]];
                if(encode_seq[counter+1] == 6 || encode_seq[counter+1] == 13)
                    position -= 4;
                // 检查这个是开开关还是切换 check if latch or shift
                if((encode_seq[counter+1]>6 && encode_seq[counter+1]<=13) || (encode_seq[counter+1]==13 && encode_seq[counter+2]!=13))
                    shift_back=1; // 记得将模式切回调用它的那个模式
            }
            // 如果不用Byte Mode
            if (encode_seq[counter+1] %7 != 6) {
                // 如果该编码方案【可以编码】该字符：
                if(j_enconing_table[tmp][encode_seq[counter+1]%7]>-1 && character_size[encode_seq[counter+1]%7] < ENC_MAX) {
                    // 编码这个字符
                    convert_dec_to_bin(j_enconing_table[tmp][encode_seq[counter+1]%7], encoded_data->data,position, character_size[encode_seq[counter+1]%7]);
                    position += character_size[encode_seq[counter+1]%7];
                    counter++;
                }
                // 如果该编码方案不可编码该字元
                else if (j_enconing_table[tmp][encode_seq[counter+1]%7] < -1) {
                    Int32 tmp1=data->data[current_encoded_length+1];
                    if (tmp1 < 0)
                        tmp1+=256;
                    // 读取下一字元，检视我们是否可以更高效地编码这俩字元（用当前方式方式能否编码在一起）
                    if (((tmp==44 || tmp== 46 || tmp==58) && tmp1==32) || (tmp==13 && tmp1==10))
                        decimal_value=abs(j_enconing_table[tmp][encode_seq[counter+1]%7]);
                    else if (tmp==13 && tmp1!=10)
                        decimal_value=18;
                    else {  // 没法编码，说明检查器算得有问题
						J_REPORT_ERROR(("Encoding data failed"))
                        free(encoded_data);
                        return NULL;
                    }
                    if (character_size[encode_seq[counter+1]%7] < ENC_MAX) // ALWAYS TRUE，考慮移除
                        convert_dec_to_bin(decimal_value,encoded_data->data,position,character_size[encode_seq[counter+1]%7]);
                    position += character_size[encode_seq[counter+1]%7];
                    counter++;
                    end_of_loop--;
                    current_encoded_length++;
                }
                else { // 其他情况一律不予考虑
					J_REPORT_ERROR(("Encoding data failed"))
                    free(encoded_data);
                    return NULL;
                }
            }
            else {  // 若需要使用Byte Mode
                // 若前后两组编码方案不一样，需要检查字元个数
                if(encode_seq[counter] != encode_seq[counter + 1]) {
                    // 从当前字元开始，检视到底有多少个要用Byte Mode的字元需要被编码
                    byte_counter = 0;
                    for(Int32 byte_loop = counter + 1; byte_loop <= end_of_loop; byte_loop++) {
                        // 值为6或13的方案，表达接下来是Byte Mode
                        if(encode_seq[byte_loop] == 6 || encode_seq[byte_loop] == 13)
                            byte_counter++;
                        else
                            break;
                    }
                    // 为方便告诉解码器，将Byte Mode字元个数转为二进制数
                    convert_dec_to_bin(byte_counter > 15 ? 0 : byte_counter, encoded_data->data, position, 4);
                    // 多加4位
                    position += 4;
                    // 如果Byte Mode字元个数太多（大于15），就再增加13位用来表达字元个数
                    if(byte_counter > 15) {
                        convert_dec_to_bin(byte_counter-15-1,encoded_data->data, position, 13);
                        position += 13;
                    }
                    // 更新偏移量
                    byte_offset=byte_counter;
                }
                // 编码当前字元长度？
                if (character_size[encode_seq[counter+1]%7] < ENC_MAX)
                    convert_dec_to_bin(tmp, encoded_data->data, position, character_size[encode_seq[counter+1] % 7]);
                // 不予考虑连Byte Mode都用不了的情况
                else {
					J_REPORT_ERROR(("Encoding data failed"))
                    free(encoded_data);
                    return NULL;
                }
                position+=character_size[encode_seq[counter+1]%7];
                counter++;
                byte_counter--;
            }
            // 切回从调用的模式
            if (shift_back && byte_counter == 0) {
                if(byte_offset == 0)
                    encode_seq[counter]=encode_seq[counter - 1];
                else
                    encode_seq[counter]=encode_seq[counter-byte_offset];
                shift_back=0;
                byte_offset=0;
            }
        }
        else { // 算法错误：Position > Length
			J_REPORT_ERROR(("Encoding data failed: Encode Position < Encoded_length"))
            free(encoded_data);
            return NULL;
        }
        // 已编码源信息中的1个字元，请继续
        current_encoded_length++;
    }
    return encoded_data;
}

/**
 * @brief 根据enc中的参数制作metadata
 * @param enc 参数
 * @return SUCCESS | FAILURE
*/
Boolean encodeMasterMetadata(J_Encode* enc) {
	Int32 partI_length 	= MASTER_METADATA_PART1_LENGTH/2;	// partI net length
	Int32 partII_length	= MASTER_METADATA_PART2_LENGTH/2;	// partII net length
	Int32 V_length = 10;    // 详见解码器
	Int32 E_length = 6;     // 详见解码器
	Int32 MSK_length = 3;   // 详见解码器
	// Master的metadata各种变量
	Int32 Nc = log(enc->color_number)/log(2.0) - 1;
	Int32 V = ((enc->symbol_versions[0].x -1) << 5) + (enc->symbol_versions[0].y - 1);
	Int32 E1 = enc->symbols[0].wcwr[0] - 3;
	Int32 E2 = enc->symbols[0].wcwr[1] - 4;
	Int32 MSK = DEFAULT_MASKING_REFERENCE;

	// 写入metadata的各part
	// Part I：颜色部分
	J_Data* partI = (J_Data *)malloc(sizeof(J_Data) + partI_length*sizeof(Char));
	if(partI == NULL) {
		J_REPORT_ERROR(("Memory allocation for metadata Part I in master symbol failed"))
		return FAILURE;
	}
	partI->length = partI_length;
	convert_dec_to_bin(Nc, partI->data, 0, partI->length);
	// Part II：主要参数（如解码参数、采用的mask号码）部分
	J_Data* partII = (J_Data *)malloc(sizeof(J_Data) + partII_length*sizeof(Char));
	if(partII == NULL) {
		J_REPORT_ERROR(("Memory allocation for metadata Part II in master symbol failed"))
		return FAILURE;
	}
	partII->length = partII_length;
	convert_dec_to_bin(V,   partII->data, 0, V_length);
	convert_dec_to_bin(E1,  partII->data, V_length, 3);
	convert_dec_to_bin(E2,  partII->data, V_length+3, 3);
	convert_dec_to_bin(MSK, partII->data, V_length+E_length, MSK_length);

	// 获取LDPC编码后的Part I、II数据
	Int32 wcwr[2] = {2, -1};    // 对于Part I的wc wr，会是2及-1
	// Part I
	J_Data* encoded_partI   = encodeLDPC(partI, wcwr);
	if(encoded_partI == NULL) {
		J_REPORT_ERROR(("LDPC encoding master metadata Part I failed"))
		return FAILURE;
	}
	// Part II
	J_Data* encoded_partII  = encodeLDPC(partII, wcwr);
	if(encoded_partII == NULL) {
		J_REPORT_ERROR(("LDPC encoding master metadata Part II failed"))
		return FAILURE;
	}

    // 决定metadata长度，方便写入
	Int32 encoded_metadata_length = encoded_partI->length + encoded_partII->length;
    // 分配metadata空间
	enc->symbols[0].metadata = (J_Data *)malloc(sizeof(J_Data) + encoded_metadata_length*sizeof(Char));
	if(enc->symbols[0].metadata == NULL) {
		J_REPORT_ERROR(("Memory allocation for encoded metadata in master symbol failed"))
		return FAILURE;
	}
	enc->symbols[0].metadata->length = encoded_metadata_length;
	// 写入metadata部分
	memcpy(enc->symbols[0].metadata->data, encoded_partI->data, encoded_partI->length);
	memcpy(enc->symbols[0].metadata->data+encoded_partI->length, encoded_partII->data, encoded_partII->length);

	free(partI);
	free(partII);
	free(encoded_partI);
	free(encoded_partII);
    return SUCCESS;
}

/**
 * @brief 如果不用default的掩膜操作，就要重新编码metadata PartII
 * @param enc encode对象
 * @param mask_ref 掩膜操作编号
 * @return SUCCESS | FAILURE
*/
Boolean updateMasterMetadataPartII(J_Encode* enc, Int32 mask_ref) {
	Int32 partII_length	= MASTER_METADATA_PART2_LENGTH/2;	//partII net length
	J_Data* partII = (J_Data *)malloc(sizeof(J_Data) + partII_length*sizeof(Char));
	if(partII == NULL) {
		J_REPORT_ERROR(("Memory allocation for metadata Part II in master symbol failed"))
		return FAILURE;
	}
	partII->length = partII_length;

	// set V and E
	Int32 V_length = 10;
	Int32 E_length = 6;
	Int32 MSK_length = 3;
	Int32 V = ((enc->symbol_versions[0].x -1) << 5) + (enc->symbol_versions[0].y - 1);
	Int32 E1 = enc->symbols[0].wcwr[0] - 3;
	Int32 E2 = enc->symbols[0].wcwr[1] - 4;
	convert_dec_to_bin(V,   partII->data, 0, V_length);
	convert_dec_to_bin(E1,  partII->data, V_length, 3);
	convert_dec_to_bin(E2,  partII->data, V_length+3, 3);

	// 设置PartII种的掩膜种类
	convert_dec_to_bin(mask_ref, partII->data, V_length+E_length, MSK_length);

	// 编码及写入新的PartII
	Int32 wcwr[2] = {2, -1};
	J_Data* encoded_partII = encodeLDPC(partII, wcwr);
	if(encoded_partII == NULL) {
		J_REPORT_ERROR(("LDPC encoding master metadata Part II failed while performing update metadata"))
		return FAILURE;
	}
	// 刷新PartII
	memcpy(enc->symbols[0].metadata->data+MASTER_METADATA_PART1_LENGTH, encoded_partII->data, encoded_partII->length);

	free(partII);
	free(encoded_partII);
	return SUCCESS;
}

/**
 * @brief Update master symbol metadata PartII if the default masking reference is changed
 * @param enc the encode parameter
*/
void placeMasterMetadataPartII(J_Encode* enc) {
    //rewrite metadata in master with mask information
    Int32 nb_of_bits_per_mod = log(enc->color_number)/log(2);
    Int32 x = MASTER_METADATA_X;
    Int32 y = MASTER_METADATA_Y;
    Int32 module_count = 0;
    //skip PartI and color palette
    Int32 color_palette_size = MIN(enc->color_number-2, 64-2);
    Int32 module_offset = MASTER_METADATA_PART1_MODULE_NUMBER + color_palette_size*COLOR_PALETTE_NUMBER;
    for(Int32 i=0; i<module_offset; i++) {
		module_count++;
        getNextMetadataModuleInMaster(enc->symbols[0].side_size.y, enc->symbols[0].side_size.x, module_count, &x, &y);
	}
	//update PartII
	Int32 partII_bit_start = MASTER_METADATA_PART1_LENGTH;
	Int32 partII_bit_end = MASTER_METADATA_PART1_LENGTH + MASTER_METADATA_PART2_LENGTH;
	Int32 metadata_index = partII_bit_start;
	while(metadata_index <= partII_bit_end)
	{
    	BYTE color_index = enc->symbols[0].matrix[y*enc->symbols[0].side_size.x + x];
		for(Int32 j=0; j<nb_of_bits_per_mod; j++)
		{
			if(metadata_index <= partII_bit_end)
			{
				BYTE bit = enc->symbols[0].metadata->data[metadata_index];
				if(bit == 0)
					color_index &= ~(1UL << (nb_of_bits_per_mod-1-j));
				else
					color_index |= 1UL << (nb_of_bits_per_mod-1-j);
				metadata_index++;
			}
			else
				break;
		}
        enc->symbols[0].matrix[y*enc->symbols[0].side_size.x + x] = color_index;
        module_count++;
        getNextMetadataModuleInMaster(enc->symbols[0].side_size.y, enc->symbols[0].side_size.x, module_count, &x, &y);
    }
}

/**
 * @brief 拿到色盘编号的二进制格式
 * @param index 色盘编号
 * @param index_size 色盘编号大小
 * @param color_number 颜色种类
*/
void getColorPaletteIndex(BYTE* index, Int32 index_size, Int32 color_number) {
	for(Int32 i=0; i<index_size; i++) {
		index[i] = i;
	}

	if(color_number < 128)
		return;

	BYTE *tmp = (BYTE *)malloc(color_number);
	for(Int32 i=0; i<color_number; i++) {
		tmp[i] = i;
	}

	if(color_number == 128) {
		memcpy(index + 0,  tmp + 0, 16);
		memcpy(index + 16, tmp + 32, 16);
		memcpy(index + 32, tmp + 80, 16);
		memcpy(index + 48, tmp + 112, 16);
	}
	else if(color_number == 256) {
		memcpy(index + 0, tmp + 0,  4);
		memcpy(index + 4, tmp + 8,  4);
		memcpy(index + 8, tmp + 20, 4);
		memcpy(index + 12,tmp + 28, 4);

		memcpy(index + 16, tmp + 64, 4);
		memcpy(index + 20, tmp + 72, 4);
		memcpy(index + 24, tmp + 84, 4);
		memcpy(index + 28, tmp + 92, 4);

		memcpy(index + 32, tmp + 160, 4);
		memcpy(index + 36, tmp + 168, 4);
		memcpy(index + 40, tmp + 180, 4);
		memcpy(index + 44, tmp + 188, 4);

		memcpy(index + 48, tmp + 224, 4);
		memcpy(index + 52, tmp + 232, 4);
		memcpy(index + 56, tmp + 244, 4);
		memcpy(index + 60, tmp + 252, 4);
	}

	free(tmp);
}

/**
 * @brief 创建数据矩阵，将会写入enc中
 * @param enc encode对象
 * @param index symbol编号
 * @param ecc_encoded_data 编码后的数据
 * @return SUCCESS | FAILURE
*/
Boolean createMatrix(J_Encode* enc, Int32 index, J_Data* ecc_encoded_data) {
    // 创建数据矩阵空间
    enc->symbols[index].matrix = (BYTE *)calloc(enc->symbols[index].side_size.x * enc->symbols[index].side_size.y, sizeof(BYTE));
    if(enc->symbols[index].matrix == NULL) {
        J_REPORT_ERROR(("Memory allocation for symbol matrix failed"))
        return FAILURE;
    }
    // 创建data_map，以供可访问判定等
    enc->symbols[index].data_map = (BYTE *)malloc(enc->symbols[index].side_size.x * enc->symbols[index].side_size.y * sizeof(BYTE));
    if(!enc->symbols[index].data_map) {
        J_REPORT_ERROR(("Memory allocation for data map failed"))
        return FAILURE;
    }
    // data_map全设为true
    memset(enc->symbols[index].data_map, 1, enc->symbols[index].side_size.x * enc->symbols[index].side_size.y * sizeof(BYTE));

    // 填充所有alignment pattern（如有）
    Int32 Nc = log(enc->color_number)/log(2.0) - 1;
	BYTE apx_core_color = apx_core_color_index[Nc];
	BYTE apx_peri_color = apn_core_color_index[Nc];
	Int32 side_ver_x_index = SIZE2VERSION(enc->symbols[index].side_size.x) - 1;
	Int32 side_ver_y_index = SIZE2VERSION(enc->symbols[index].side_size.y) - 1;
    for(Int32 x=0; x<j_ap_num[side_ver_x_index]; x++) {
    	BYTE left;
        if (x%2 == 1)
            left=0;
        else
            left=1;
        for(Int32 y=0; y<j_ap_num[side_ver_y_index]; y++) {
            Int32 x_offset = j_ap_pos[side_ver_x_index][x] - 1;
            Int32 y_offset = j_ap_pos[side_ver_y_index][y] - 1;
            // 左边的 alignment patterns
            if(	left == 1
				&& (x != 0 || y != 0)
				&& (x != 0 || y != j_ap_num[side_ver_y_index]-1)
				&& (x != j_ap_num[side_ver_x_index]-1 || y != 0)
				&& (x != j_ap_num[side_ver_x_index]-1 || y != j_ap_num[side_ver_y_index]-1)) {
            	enc->symbols[index].matrix[(y_offset-1)*enc->symbols[index].side_size.x + x_offset-1]=
				enc->symbols[index].matrix[(y_offset-1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].matrix[(y_offset  )*enc->symbols[index].side_size.x + x_offset-1]=
				enc->symbols[index].matrix[(y_offset  )*enc->symbols[index].side_size.x + x_offset+1]=
				enc->symbols[index].matrix[(y_offset+1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].matrix[(y_offset+1)*enc->symbols[index].side_size.x + x_offset+1]=apx_peri_color;
				enc->symbols[index].matrix[(y_offset  )*enc->symbols[index].side_size.x + x_offset  ]=apx_core_color;

				enc->symbols[index].data_map[(y_offset-1)*enc->symbols[index].side_size.x + x_offset-1]=
				enc->symbols[index].data_map[(y_offset-1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].data_map[(y_offset  )*enc->symbols[index].side_size.x + x_offset-1]=
				enc->symbols[index].data_map[(y_offset  )*enc->symbols[index].side_size.x + x_offset+1]=
				enc->symbols[index].data_map[(y_offset+1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].data_map[(y_offset+1)*enc->symbols[index].side_size.x + x_offset+1]=
				enc->symbols[index].data_map[(y_offset  )*enc->symbols[index].side_size.x + x_offset  ]=0;
            }
            // 右边的 alignment patterns
            else if(left == 0
					&& (x != 0 || y != 0)
					&& (x != 0 || y != j_ap_num[side_ver_y_index]-1)
					&& (x != j_ap_num[side_ver_x_index]-1 || y != 0)
					&& (x != j_ap_num[side_ver_x_index]-1 || y != j_ap_num[side_ver_y_index]-1)) {
            	enc->symbols[index].matrix[(y_offset-1)*enc->symbols[index].side_size.x + x_offset+1]=
				enc->symbols[index].matrix[(y_offset-1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].matrix[(y_offset  )*enc->symbols[index].side_size.x + x_offset-1]=
				enc->symbols[index].matrix[(y_offset  )*enc->symbols[index].side_size.x + x_offset+1]=
				enc->symbols[index].matrix[(y_offset+1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].matrix[(y_offset+1)*enc->symbols[index].side_size.x + x_offset-1]=apx_peri_color;
				enc->symbols[index].matrix[(y_offset  )*enc->symbols[index].side_size.x + x_offset  ]=apx_core_color;

				enc->symbols[index].data_map[(y_offset-1)*enc->symbols[index].side_size.x + x_offset+1]=
				enc->symbols[index].data_map[(y_offset-1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].data_map[(y_offset  )*enc->symbols[index].side_size.x + x_offset-1]=
				enc->symbols[index].data_map[(y_offset  )*enc->symbols[index].side_size.x + x_offset+1]=
				enc->symbols[index].data_map[(y_offset+1)*enc->symbols[index].side_size.x + x_offset  ]=
				enc->symbols[index].data_map[(y_offset+1)*enc->symbols[index].side_size.x + x_offset-1]=
				enc->symbols[index].data_map[(y_offset  )*enc->symbols[index].side_size.x + x_offset  ]=0;
            }
            if (left==0)
                left=1;
            else
                left=0;
        }
    }

    // 设置finder pattern的较外层
#ifdef MULTI_SYMBOL
    if(index == 0) {
#endif
        // k=0 centre, k=1 first layer, k=2 second layer
        for(Int32 k = 0; k<3; k++) {
            for(Int32 i=0; i<k+1;i ++) {
                for(Int32 j=0; j<k+1; j++) {
                    if (i==k || j==k) {
                        BYTE fp0_color_index, fp1_color_index, fp2_color_index, fp3_color_index;
						fp0_color_index = (k%2) ? fp3_core_color_index[Nc] : fp0_core_color_index[Nc];
						fp1_color_index = (k%2) ? fp2_core_color_index[Nc] : fp1_core_color_index[Nc];
						fp2_color_index = (k%2) ? fp1_core_color_index[Nc] : fp2_core_color_index[Nc];
						fp3_color_index = (k%2) ? fp0_core_color_index[Nc] : fp3_core_color_index[Nc];

						// 上面的pattern
                        enc->symbols[index].matrix[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER-j-1]=
						enc->symbols[index].matrix[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER+j-1]=fp0_color_index;
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER-j-1]=
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER+j-1]=0;

                        enc->symbols[index].matrix[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
                        enc->symbols[index].matrix[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=fp1_color_index;
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=0;

                        // 下面的pattern
                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=fp2_color_index;
                        enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
                        enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=0;

                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)-j-1]=
                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)+j-1]=fp3_color_index;
                        enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)-j-1]=
                        enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)+j-1]=0;
                    }
                }
            }
        }
#ifdef MULTI_SYMBOL
    }
    else //finder alignments in slave {
        //if k=0 center, k=1 first layer
        for(Int32 k=0;k<2;k++)
        {
            for(Int32 i=0;i<k+1;i++)
            {
                for(Int32 j=0;j<k+1;j++)
                {
                    if (i==k || j==k)
                    {
                    	BYTE ap0_color_index, ap1_color_index, ap2_color_index, ap3_color_index;
                        ap0_color_index =
						ap1_color_index =
						ap2_color_index =
						ap3_color_index = (k%2) ? apx_core_color_index[Nc] : apn_core_color_index[Nc];
                        //upper pattern
                        enc->symbols[index].matrix[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER-j-1]=
                        enc->symbols[index].matrix[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER+j-1]=ap0_color_index;
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER-j-1]=
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+DISTANCE_TO_BORDER+j-1]=0;

                        enc->symbols[index].matrix[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
                        enc->symbols[index].matrix[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=ap1_color_index;
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER-(i+1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
                        enc->symbols[index].data_map[(DISTANCE_TO_BORDER+(i-1))*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=0;

                        //lower pattern
                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=ap2_color_index;
                        enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)-j-1]=
						enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+enc->symbols[index].side_size.x-(DISTANCE_TO_BORDER-1)+j-1]=0;

                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)-j-1]=
                        enc->symbols[index].matrix[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)+j-1]=ap3_color_index;
                        enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER+i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)-j-1]=
                        enc->symbols[index].data_map[(enc->symbols[index].side_size.y-DISTANCE_TO_BORDER-i)*enc->symbols[index].side_size.x+(DISTANCE_TO_BORDER)+j-1]=0;
                    }
                }
            }
        }
    }
#endif // MULTI_SYMBOL

    // 放置Metadata及色盘
    Int32 nb_of_bits_per_mod = log(enc->color_number) / log(2);
    Int32 color_index;
    Int32 module_count = 0;
    Int32 x;
    Int32 y;

    // 拿到色盘的颜色编号 / get color index for color palette
    Int32 palette_index_size = enc->color_number > 64 ? 64 : enc->color_number;
	BYTE *palette_index = (BYTE *)malloc(palette_index_size);
	getColorPaletteIndex(palette_index, palette_index_size, enc->color_number);
#ifdef MULTI_SYMBOL
    if(index == 0) { //place metadata and color palette in master symbol
#endif
        x = MASTER_METADATA_X;
        y = MASTER_METADATA_Y;
		int metadata_index = 0;
        //metadata Part I
        if(!isDefaultMode(enc))
		{
			while(metadata_index < enc->symbols[index].metadata->length && metadata_index < MASTER_METADATA_PART1_LENGTH)
			{
				//Read 3 bits from encoded PartI each time
				BYTE bit1 = enc->symbols[index].metadata->data[metadata_index + 0];
				BYTE bit2 = enc->symbols[index].metadata->data[metadata_index + 1];
				BYTE bit3 = enc->symbols[index].metadata->data[metadata_index + 2];
				Int32 val = (bit1 << 2) + (bit2 << 1) + bit3;
				//place two modules according to the value of every 3 bits
                for(Int32 i=0; i<2; i++)
				{
					color_index = nc_color_encode_table[val][i] % enc->color_number;
					enc->symbols[index].matrix  [y*enc->symbols[index].side_size.x + x] = color_index;
					enc->symbols[index].data_map[y*enc->symbols[index].side_size.x + x] = 0;
					module_count++;
					getNextMetadataModuleInMaster(enc->symbols[index].side_size.y, enc->symbols[index].side_size.x, module_count, &x, &y);
				}
				metadata_index += 3;
			}
		}
		//color palette
		for(Int32 i=2; i<MIN(enc->color_number, 64); i++)	//skip the first two colors in finder pattern
		{
			enc->symbols[index].matrix  [y*enc->symbols[index].side_size.x+x] = palette_index[master_palette_placement_index[0][i]%enc->color_number];
			enc->symbols[index].data_map[y*enc->symbols[index].side_size.x+x] = 0;
			module_count++;
			getNextMetadataModuleInMaster(enc->symbols[index].side_size.y, enc->symbols[index].side_size.x, module_count, &x, &y);

			enc->symbols[index].matrix  [y*enc->symbols[index].side_size.x+x] = palette_index[master_palette_placement_index[1][i]%enc->color_number];
			enc->symbols[index].data_map[y*enc->symbols[index].side_size.x+x] = 0;
			module_count++;
			getNextMetadataModuleInMaster(enc->symbols[index].side_size.y, enc->symbols[index].side_size.x, module_count, &x, &y);

			enc->symbols[index].matrix  [y*enc->symbols[index].side_size.x+x] = palette_index[master_palette_placement_index[2][i]%enc->color_number];
			enc->symbols[index].data_map[y*enc->symbols[index].side_size.x+x] = 0;
			module_count++;
			getNextMetadataModuleInMaster(enc->symbols[index].side_size.y, enc->symbols[index].side_size.x, module_count, &x, &y);

			enc->symbols[index].matrix  [y*enc->symbols[index].side_size.x+x] = palette_index[master_palette_placement_index[3][i]%enc->color_number];
			enc->symbols[index].data_map[y*enc->symbols[index].side_size.x+x] = 0;
			module_count++;
			getNextMetadataModuleInMaster(enc->symbols[index].side_size.y, enc->symbols[index].side_size.x, module_count, &x, &y);
		}
		//metadata PartII
		if(!isDefaultMode(enc))
		{
			while(metadata_index < enc->symbols[index].metadata->length)
			{
				color_index = 0;
				for(Int32 j=0; j<nb_of_bits_per_mod; j++)
				{
					if(metadata_index < enc->symbols[index].metadata->length)
					{
						color_index += ((Int32)enc->symbols[index].metadata->data[metadata_index]) << (nb_of_bits_per_mod-1-j);
						metadata_index++;
					}
					else
						break;
				}
				enc->symbols[index].matrix  [y*enc->symbols[index].side_size.x + x] = color_index;
				enc->symbols[index].data_map[y*enc->symbols[index].side_size.x + x] = 0;
				module_count++;
				getNextMetadataModuleInMaster(enc->symbols[index].side_size.y, enc->symbols[index].side_size.x, module_count, &x, &y);
			}
		}
#ifdef MULTI_SYMBOL
    }
    else//place color palette in slave symbol
    {
    	//color palette
        Int32 width=enc->symbols[index].side_size.x;
        Int32 height=enc->symbols[index].side_size.y;
        for (Int32 i=2; i<MIN(enc->color_number, 64); i++)	//skip the first two colors in alignment pattern
        {
        	//left
			enc->symbols[index].matrix  [slave_palette_position[i-2].y*width + slave_palette_position[i-2].x] = palette_index[slave_palette_placement_index[i]%enc->color_number];
			enc->symbols[index].data_map[slave_palette_position[i-2].y*width + slave_palette_position[i-2].x] = 0;
			//top
			enc->symbols[index].matrix  [slave_palette_position[i-2].x*width + (width-1-slave_palette_position[i-2].y)] = palette_index[slave_palette_placement_index[i]%enc->color_number];
			enc->symbols[index].data_map[slave_palette_position[i-2].x*width + (width-1-slave_palette_position[i-2].y)] = 0;
			//right
			enc->symbols[index].matrix  [(height-1-slave_palette_position[i-2].y)*width + (width-1-slave_palette_position[i-2].x)] = palette_index[slave_palette_placement_index[i]%enc->color_number];
			enc->symbols[index].data_map[(height-1-slave_palette_position[i-2].y)*width + (width-1-slave_palette_position[i-2].x)] = 0;
			//bottom
			enc->symbols[index].matrix  [(height-1-slave_palette_position[i-2].x)*width + slave_palette_position[i-2].y] = palette_index[slave_palette_placement_index[i]%enc->color_number];
			enc->symbols[index].data_map[(height-1-slave_palette_position[i-2].x)*width + slave_palette_position[i-2].y] = 0;
        }
    }
#endif
#if TEST_MODE
	FILE* fp = fopen("j_enc_module_data.bin", "wb");
#endif // TEST_MODE
    // 放置数据
    Int32 written_mess_part=0;
    Int32 padding=0;
    for(Int32 start_i=0;start_i<enc->symbols[index].side_size.x;start_i++) {
        for(Int32 i=start_i;i<enc->symbols[index].side_size.x*enc->symbols[index].side_size.y;i=i+enc->symbols[index].side_size.x) {
            if (enc->symbols[index].data_map[i]!=0 && written_mess_part<ecc_encoded_data->length) {
                color_index=0;
                for(Int32 j=0;j<nb_of_bits_per_mod;j++) {
                    if(written_mess_part<ecc_encoded_data->length)
                        color_index+=((Int32)ecc_encoded_data->data[written_mess_part]) << (nb_of_bits_per_mod-1-j);//*pow(2,nb_of_bits_per_mod-1-j);
                    else {
                        color_index+=padding << (nb_of_bits_per_mod-1-j);//*pow(2,nb_of_bits_per_mod-1-j);
                        if (padding==0)
                            padding=1;
                        else
                            padding=0;
                    }
                    written_mess_part++;
                }
                enc->symbols[index].matrix[i]=(Char)color_index;//i % enc->color_number;
#if TEST_MODE
				fwrite(&enc->symbols[index].matrix[i], 1, 1, fp);
#endif // TEST_MODE
            }
            else if(enc->symbols[index].data_map[i]!=0) //write padding bits
            {
                color_index=0;
                for(Int32 j=0;j<nb_of_bits_per_mod;j++) {
                    color_index+=padding << (nb_of_bits_per_mod-1-j);//*pow(2,nb_of_bits_per_mod-1-j);
                    if (padding==0)
                        padding=1;
                    else
                        padding=0;
                }
                enc->symbols[index].matrix[i]=(Char)color_index;//i % enc->color_number;
#if TEST_MODE
				fwrite(&enc->symbols[index].matrix[i], 1, 1, fp);
#endif // TEST_MODE
            }
        }
    }
#if TEST_MODE
	fclose(fp);
#endif // TEST_MODE
	free(palette_index);
	return SUCCESS;
}

#ifdef MULTI_SYMBOL
/**
 * @brief Swap two symbols (仅当多SYMBOL模式使用)
 * @param enc the encode parameters
 * @param index1 the index number of the first symbol
 * @param index2 the index number of the second symbol
*/
void swap_symbols(J_Encode* enc, Int32 index1, Int32 index2)
{
	swap_int(&enc->symbol_positions[index1],  &enc->symbol_positions[index2]);
	swap_int(&enc->symbol_versions[index1].x, &enc->symbol_versions[index2].x);
	swap_int(&enc->symbol_versions[index1].y, &enc->symbol_versions[index2].y);
	swap_byte(&enc->symbol_ecc_levels[index1],&enc->symbol_ecc_levels[index2]);
	//swap symbols
	J_Symbol s;
    s = enc->symbols[index1];
    enc->symbols[index1] = enc->symbols[index2];
    enc->symbols[index2] = s;
}
#endif

/**
 * @brief (仅针对多SYMBOL模式！)
 * @param enc Encode对象
 * @return SUCCESS | FAILURE
*/
#ifdef MULTI_SYMBOL
Boolean assignDockedSymbols(J_Encode* enc) {
	// initialize host and slaves
	for(Int32 i = 0; i < enc->symbol_number; i++) {
		// initialize symbol host index
		enc->symbols[i].host = -1;
		// initialize symbol's slave index
		for (Int32 j = 0; j < 4; j++)
			enc->symbols[i].slaves[j] = 0;	//0:no slave
	}
	// assign docked symbols
	Int32 assigned_slave_index = 1;
    for(Int32 i=0; i<enc->symbol_number-1 && assigned_slave_index<enc->symbol_number; i++)
    {
    	for(Int32 j=0; j<4 && assigned_slave_index<enc->symbol_number; j++)
		{
			for(Int32 k=i+1; k<enc->symbol_number && assigned_slave_index<enc->symbol_number; k++)
			{
				if(enc->symbols[k].host == -1)
				{
					Int32 hpos = enc->symbol_positions[i];
					Int32 spos = enc->symbol_positions[k];
					Boolean slave_found = FAILURE;
					switch(j)
					{
					case 0:	//top
						if(j_symbol_pos[hpos].x == j_symbol_pos[spos].x && j_symbol_pos[hpos].y - 1 == j_symbol_pos[spos].y)
						{
							enc->symbols[i].slaves[0] = assigned_slave_index;
							enc->symbols[k].slaves[1] = -1;	//-1:host position
							slave_found = SUCCESS;
						}
						break;
					case 1:	//bottom
						if(j_symbol_pos[hpos].x == j_symbol_pos[spos].x && j_symbol_pos[hpos].y + 1 == j_symbol_pos[spos].y)
						{
							enc->symbols[i].slaves[1] = assigned_slave_index;
							enc->symbols[k].slaves[0] = -1;
							slave_found = SUCCESS;
						}
						break;
					case 2:	//left
						if(j_symbol_pos[hpos].y == j_symbol_pos[spos].y && j_symbol_pos[hpos].x - 1 == j_symbol_pos[spos].x)
						{
							enc->symbols[i].slaves[2] = assigned_slave_index;
							enc->symbols[k].slaves[3] = -1;
							slave_found = SUCCESS;
						}
						break;
					case 3://right
						if(j_symbol_pos[hpos].y == j_symbol_pos[spos].y && j_symbol_pos[hpos].x + 1 == j_symbol_pos[spos].x)
						{
							enc->symbols[i].slaves[3] = assigned_slave_index;
							enc->symbols[k].slaves[2] = -1;
							slave_found = SUCCESS;
						}
						break;
					}
					if(slave_found)
					{
						swap_symbols(enc, k, assigned_slave_index);
						enc->symbols[assigned_slave_index].host = i;
						assigned_slave_index++;
					}
				}
			}
		}
    }
    //check if there is undocked symbol
    for(Int32 i=1; i<enc->symbol_number; i++)
    {
		if(enc->symbols[i].host == -1)
		{
			J_REPORT_ERROR(("Slave symbol at position %d has no host", enc->symbol_positions[i]))
			return FAILURE;
		}
    }
    return SUCCESS;
}
#endif

/**
 * @brief 根据Encode对象，合成、计算及输出一个J_Code对象
 * @param enc Encode对象
 * @return the J_Code对象 / code parameters
*/
J_Code* getCodePara(J_Encode* enc) {
    J_Code* cp = (J_Code *)malloc(sizeof(J_Code)); 
    if(!cp) {
        J_REPORT_ERROR(("Memory allocation for code parameter failed"))
        return NULL;
    }
    // 计算期望的每个module的长宽（像素）
    if(enc->master_symbol_width != 0 || enc->master_symbol_height != 0) {
        Int32 dimension_x = enc->master_symbol_width/enc->symbols[0].side_size.x;
        Int32 dimension_y = enc->master_symbol_height/enc->symbols[0].side_size.y;
        cp->dimension = dimension_x > dimension_y ? dimension_x : dimension_y;
        if(cp->dimension < 1)
            cp->dimension = 1;
    }
    else {
        // 使用缺省设置
        cp->dimension = enc->module_size;
    }

    // 获取symbol的坐标范围
    cp->min_x = 0;
    cp->min_y = 0;
    Int32 max_x=0, max_y=0;
#ifdef MULTI_SYMBOL
    for(Int32 i=0; i<enc->symbol_number; i++) {
#else
        Int32 i = 0;
#endif
        // find the mininal x and y
        if(j_symbol_pos[enc->symbol_positions[i]].x < cp->min_x)
            cp->min_x = j_symbol_pos[enc->symbol_positions[i]].x;
        if(j_symbol_pos[enc->symbol_positions[i]].y < cp->min_y)
            cp->min_y = j_symbol_pos[enc->symbol_positions[i]].y;
        // find the maximal x and y
        if(j_symbol_pos[enc->symbol_positions[i]].x > max_x)
            max_x = j_symbol_pos[enc->symbol_positions[i]].x;
        if(j_symbol_pos[enc->symbol_positions[i]].y > max_y)
            max_y = j_symbol_pos[enc->symbol_positions[i]].y;
#ifdef MULTI_SYMBOL
    }
#endif

    // 计算code大小（我估计1*1）
    cp->rows = max_y - cp->min_y + 1;
    cp->cols = max_x - cp->min_x + 1;
    cp->row_height = (Int32 *)malloc(cp->rows * sizeof(Int32));
    if(!cp->row_height) {
		free(cp);
        J_REPORT_ERROR(("Memory allocation for row height in code parameter failed"))
        return NULL;
    }
    cp->col_width = (Int32 *)malloc(cp->cols * sizeof(Int32));
    if(!cp->col_width) {
		free(cp);
        J_REPORT_ERROR(("Memory allocation for column width in code parameter failed"))
        return NULL;
    }
    cp->code_size.x = 0;
    cp->code_size.y = 0;
    Boolean flag = 0;
    for(Int32 x=cp->min_x; x<=max_x; x++) {
        flag = 0;
        for(Int32 i=0; i<enc->symbol_number; i++) {
            if(j_symbol_pos[enc->symbol_positions[i]].x == x) {
                cp->col_width[x - cp->min_x] = enc->symbols[i].side_size.x;
                cp->code_size.x += cp->col_width[x - cp->min_x];
                flag = 1;
            }
            if(flag) break;
        }
    }
    for(Int32 y=cp->min_y; y<=max_y; y++) {
        flag = 0;
        for(Int32 i=0; i<enc->symbol_number; i++) {
            if(j_symbol_pos[enc->symbol_positions[i]].y == y) {
                cp->row_height[y - cp->min_y] = enc->symbols[i].side_size.y;
                cp->code_size.y += cp->row_height[y - cp->min_y];
                flag = 1;
            }
            if(flag) break;
        }
    }
    return cp;
}

/**
 * @brief 制作一个Encode的Bitmap
 * @param enc Encode对象，生成的Bitmap将会存在这儿
 * @param cp J_Code对象
 * @return SUCCESS | FAILURE
*/
Boolean createBitmap(J_Encode* enc, J_Code* cp) {
    // 创建Bitmap
    Int32 width = cp->dimension * cp->code_size.x;
    Int32 height= cp->dimension * cp->code_size.y;
    Int32 bytes_per_pixel = BITMAP_BITS_PER_PIXEL / 8;
    Int32 bytes_per_row = width * bytes_per_pixel;
    enc->bitmap = (Bitmap *)calloc(1, sizeof(Bitmap) + width*height*bytes_per_pixel*sizeof(BYTE));
    if(enc->bitmap == NULL) {
        J_REPORT_ERROR(("Memory allocation for Bitmap failed"))
        return FAILURE;
    }
    enc->bitmap->width = width;
    enc->bitmap->height= height;
    enc->bitmap->bits_per_pixel = BITMAP_BITS_PER_PIXEL;
    enc->bitmap->bits_per_channel = BITMAP_BITS_PER_CHANNEL;
    enc->bitmap->channel_count = BITMAP_CHANNEL_COUNT;

    // 放置symbol
    for(Int32 k=0; k<enc->symbol_number; k++) {
        // 计算symbol的起始坐标
        Int32 startx = 0, starty = 0;
        Int32 col = j_symbol_pos[enc->symbol_positions[k]].x - cp->min_x;
        Int32 row = j_symbol_pos[enc->symbol_positions[k]].y - cp->min_y;
        for(Int32 c=0; c<col; c++)
            startx += cp->col_width[c];
        for(Int32 r=0; r<row; r++)
            starty += cp->row_height[r];

        // 写入symbol
        Int32 symbol_width = enc->symbols[k].side_size.x;
        Int32 symbol_height= enc->symbols[k].side_size.y;
        for(Int32 x=startx; x<(startx+symbol_width); x++) {
            for(Int32 y=starty; y<(starty+symbol_height); y++) {
                // 写入一个module
                Int32 p_index = enc->symbols[k].matrix[(y-starty)*symbol_width + (x-startx)]; // 该module在bitmap中的坐标
                 for(Int32 i=y*cp->dimension; i<(y*cp->dimension+cp->dimension); i++) {
                    for(Int32 j=x*cp->dimension; j<(x*cp->dimension+cp->dimension); j++) {
                        enc->bitmap->pixel[i*bytes_per_row + j*bytes_per_pixel]     = enc->palette[p_index * 3];	//R
                        enc->bitmap->pixel[i*bytes_per_row + j*bytes_per_pixel + 1] = enc->palette[p_index * 3 + 1];//B
                        enc->bitmap->pixel[i*bytes_per_row + j*bytes_per_pixel + 2] = enc->palette[p_index * 3 + 2];//G
                        enc->bitmap->pixel[i*bytes_per_row + j*bytes_per_pixel + 3] = 255; 							//A
                    }
                }
            }
        }
    }
    return SUCCESS;
}


/**
 * @brief （仅在多SYMBOL模式下！）Checks if the docked symbol sizes are valid
 * @param enc the encode parameters
 * @return SUCCESS | FAILURE
*/
#ifdef MULTI_SYMBOL
Boolean checkDockedSymbolSize(J_Encode* enc)
{
	for(Int32 i=0; i<enc->symbol_number; i++)
	{
		for(Int32 j=0; j<4; j++)
		{
			Int32 slave_index = enc->symbols[i].slaves[j];
			if(slave_index > 0)
			{
				Int32 hpos = enc->symbol_positions[i];
				Int32 spos = enc->symbol_positions[slave_index];
				Int32 x_diff = j_symbol_pos[hpos].x - j_symbol_pos[spos].x;
				Int32 y_diff = j_symbol_pos[hpos].y - j_symbol_pos[spos].y;

				if(x_diff == 0 && enc->symbol_versions[i].x != enc->symbol_versions[slave_index].x)
				{
					J_REPORT_ERROR(("Slave symbol at position %d has different side version in X direction as its host symbol at position %d", spos, hpos))
					return FAILURE;
				}
				if(y_diff == 0 && enc->symbol_versions[i].y != enc->symbol_versions[slave_index].y)
				{
					J_REPORT_ERROR(("Slave symbol at position %d has different side version in Y direction as its host symbol at position %d", spos, hpos))
					return FAILURE;
				}
			}
		}
	}
	return SUCCESS;
}
#endif

/**
 * @brief 设置最小需要的图案长宽
 * @param enc 编码对象
 * @param encoded_data 编码过的数据
 * @return SUCCESS | FAILURE
 */
Boolean setMasterSymbolVersion(J_Encode *enc, J_Data* encoded_data) {
    // 原数据净含量
    Int32 net_data_length = encoded_data->length;
    // 需编码的总数据量
    Int32 payload_length = net_data_length + 5;  // 加上了 S标（尽管没有必要）及 SF 标
    // 确认纠错等级及LDPC wc和wr参数
    if(enc->symbol_ecc_levels[0] == 0)           // 用系统默认ECC水平(3级)
        enc->symbol_ecc_levels[0] = DEFAULT_ECC_LEVEL;
    enc->symbols[0].wcwr[0] = ecclevel2wcwr[enc->symbol_ecc_levels[0]][0];
	enc->symbols[0].wcwr[1] = ecclevel2wcwr[enc->symbol_ecc_levels[0]][1];

	// 确认最小需要的正方形Symbol determine the minimum square symbol to fit data
    Int32 capacity = 0, net_capacity;
	Boolean found_flag = FAILURE;
	for (Int32 i=1; i<=32; i++) {
		enc->symbol_versions[0].x = i;
		enc->symbol_versions[0].y = i;
		capacity = getSymbolCapacity(enc, 0);
        // 数据净搭载量
		net_capacity = (capacity/enc->symbols[0].wcwr[1])*enc->symbols[0].wcwr[1] - (capacity/enc->symbols[0].wcwr[1])*enc->symbols[0].wcwr[0];
        // 倘若找到一个最小version，使得：数据净搭载量 > 需编码的总数据量
		if(net_capacity >= payload_length) {
			found_flag = SUCCESS;
			break;
		}
	}
    // 如果不可以，为用户检视一个更低的ECC Level。如果没有检视到，便提示用户增加SYMBOL。？
	if(!found_flag) {
		Int32 level = -1;
		for (Int32 j=(Int32)enc->symbol_ecc_levels[0]-1; j>0; j--) {
			net_capacity = (capacity/ecclevel2wcwr[j][1])*ecclevel2wcwr[j][1] - (capacity/ecclevel2wcwr[j][1])*ecclevel2wcwr[j][0];
			if(net_capacity >= payload_length)
				level = j;
		}
		if(level > 0)
		{
			J_REPORT_ERROR(("Message does not fit into one symbol with the given ECC level. Please use an ECC level lower than %d with '--ecc-level %d'", level, level))
			return FAILURE;
		}
		else
		{
			J_REPORT_ERROR(("Message does not fit into one symbol. Use more symbols."))
			return FAILURE;
		}
	}
	// 更新长宽
    enc->symbols[0].side_size.x = VERSION2SIZE(enc->symbol_versions[0].x);
	enc->symbols[0].side_size.y = VERSION2SIZE(enc->symbol_versions[0].y);

    return SUCCESS;
}


/**
 * @brief 将数据搞入Symbol中
 * @param enc Encode对象
 * @param encoded_data 被编码了的讯息
 * @return SUCCESS | FAILURE
*/
Boolean fitDataIntoSymbols(J_Encode* enc, J_Data* encoded_data) {
	// 计算总数据搭载量（仅限多Symbol模式）
	Int32 *capacity = (Int32 *)malloc(enc->symbol_number * sizeof(Int32));
	Int32 *net_capacity = (Int32 *)malloc(enc->symbol_number * sizeof(Int32));
	Int32 total_net_capacity = 0;
#ifdef MULTI_SYMBOL
	for(Int32 i=0; i<enc->symbol_number; i++) {
#else
        Int32 i = 0;
#endif
		capacity[i] = getSymbolCapacity(enc, i);
		enc->symbols[i].wcwr[0] = ecclevel2wcwr[enc->symbol_ecc_levels[i]][0];
		enc->symbols[i].wcwr[1] = ecclevel2wcwr[enc->symbol_ecc_levels[i]][1];
		net_capacity[i] = (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[1] - (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[0];
		total_net_capacity += net_capacity[i];
#ifdef MULTI_SYMBOL
	}
#endif
	// 将数据搭载进Symbol中
	Int32 assigned_data_length = 0;
#ifdef MULTI_SYMBOL
	for(Int32 i=0; i<enc->symbol_number; i++) {
#else
        i = 0;
#endif
		// 按比重分割数据 / 只有多SYMBOL模式采用 / divide data proportionally
		Int32 s_data_length;
		if(i == enc->symbol_number - 1) {
			s_data_length = encoded_data->length - assigned_data_length;
		}
		else {
			Float prop = (Float)net_capacity[i] / (Float)total_net_capacity;
			s_data_length = (Int32)(prop * encoded_data->length);
		}
        Int32 s_payload_length = encoded_data->length - assigned_data_length;
		
		// 加入FLAG
		s_payload_length++;
		// add host metadata S length (master metadata Part III or slave metadata Part III)
		if(i == 0)	s_payload_length += 4;
		else		s_payload_length += 3;
#ifdef MULTI_SYMBOL
		//add slave metadata length
		for(Int32 j=0; j<4; j++) {
			if(enc->symbols[i].slaves[j] > 0) {
				s_payload_length += enc->symbols[enc->symbols[i].slaves[j]].metadata->length;
			}
		}
#endif

		// 检查是否有超过该Symbol的容量 check if the full payload exceeds net capacity
		if(s_payload_length > net_capacity[i]) {
			J_REPORT_ERROR(("Use higher symbol version. Info: Net capacity: %d | Playload len: %d.", net_capacity[i], s_payload_length))
				
			free(capacity);
			free(net_capacity);
			return FAILURE;
		}
#ifdef MULTI_SYMBOL
		// add metadata E for slave symbols if free capacity available
		Int32 j = 0;
		while(net_capacity[i] - s_payload_length >= 6 && j < 4)
		{
			if(enc->symbols[i].slaves[j] > 0)
			{
				if(enc->symbols[enc->symbols[i].slaves[j]].metadata->data[1] == 0) //check SE
				{
					if(!addE2SlaveMetadata(&enc->symbols[enc->symbols[i].slaves[j]]))
						return FAILURE;
					s_payload_length += 6;	//add E length
				}
			}
			j++;
		}
#endif
        
		// 拿到编码比最优方案 / get optimal code rate
		Int32 pn_length = s_payload_length;
#ifdef MULTI_SYMBOL
		if(i == 0) {
			if(!isDefaultMode(enc)) {
				getOptimalECC(capacity[i], s_payload_length, enc->symbols[i].wcwr);
				pn_length = (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[1] - (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[0];
			}
			else
				pn_length = net_capacity[i];
		}
		else {
			if(enc->symbols[i].metadata->data[1] == 1)	//SE = 1
			{
				getOptimalECC(capacity[i], pn_length, enc->symbols[i].wcwr);
				pn_length = (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[1] - (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[0];
				updateSlaveMetadataE(enc, enc->symbols[i].host, i);
			}
			else
				pn_length = net_capacity[i];
		}
#else   // 单SYMBOL模式
        if(!isDefaultMode(enc)) {
            getOptimalECC(capacity[i], s_payload_length, enc->symbols[i].wcwr);
            pn_length = (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[1] - (capacity[i]/enc->symbols[i].wcwr[1])*enc->symbols[i].wcwr[0];
        }
        else
            pn_length = net_capacity[i];
#endif
        
        /* 公告：现在是12/3/2020 10:34:01 HKT，下面开始写入数据 */

		// 下面开始写入数据
        enc->symbols[i].data = (J_Data *)calloc(1, sizeof(J_Data) + pn_length*sizeof(Char));
        if(enc->symbols[i].data == NULL) {
			J_REPORT_ERROR(("Memory allocation for data payload in symbol failed"))
			free(capacity);
			free(net_capacity);
			return FAILURE;
		}
		enc->symbols[i].data->length = pn_length;
		// 写入全部数据
		memcpy(enc->symbols[i].data->data, &encoded_data->data[assigned_data_length], s_data_length);
		assigned_data_length += s_data_length;
		// 写入flag数据
		Int32 set_pos = s_payload_length - 1;
		enc->symbols[i].data->data[set_pos--] = 1;
#ifdef MULTI_SYMBOL
		// 写入set host metadata S
		for(Int32 k=0; k<4; k++) {
			if(enc->symbols[i].slaves[k] > 0) {
				enc->symbols[i].data->data[set_pos--] = 1;
			}
			else if(enc->symbols[i].slaves[k] == 0) {
				enc->symbols[i].data->data[set_pos--] = 0;
			}
		}
		// set slave metadata
		for(Int32 k=0; k<4; k++)
		{
			if(enc->symbols[i].slaves[k] > 0)
			{
				for(Int32 m=0; m<enc->symbols[enc->symbols[i].slaves[k]].metadata->length; m++)
				{
					enc->symbols[i].data->data[set_pos--] = enc->symbols[enc->symbols[i].slaves[k]].metadata->data[m];
				}
			}
		}
//#endif
//#ifdef MULTI_SYMBOL
	}
#endif
	free(capacity);
	free(net_capacity);
	return SUCCESS;
}

/**
 * @brief 初始化每一个symbol
 * @param enc Encode对象
 * @return SUCCESS | FAILURE
*/
Boolean InitSymbols(J_Encode* enc) {
    // LH批阅：可以但没必要
#ifdef MULTI_SYMBOL
	// check all information for multi-symbol code are valid
	if(enc->symbol_number > 1) {
		for(Int32 i=0; i<enc->symbol_number; i++) {
			if(enc->symbol_versions[i].x < 1 || enc->symbol_versions[i].x > 32 || enc->symbol_versions[i].y < 1 || enc->symbol_versions[i].y > 32)
			{
				J_REPORT_ERROR(("Incorrect symbol version for symbol %d", i))
				return FAILURE;
			}
			if(enc->symbol_positions[i] < 0 || enc->symbol_positions[i] > MAX_SYMBOL_NUMBER)
			{
				J_REPORT_ERROR(("Incorrect symbol position for symbol %d", i))
				return FAILURE;
			}
		}
	}
	// move the master symbol to the first
    if(enc->symbol_number > 1 && enc->symbol_positions[0] != 0)
	{
		for(Int32 i=0; i<enc->symbol_number; i++)
		{
			if(enc->symbol_positions[i] == 0)
			{
				swap_int (&enc->symbol_positions[i], 	&enc->symbol_positions[0]);
				swap_int (&enc->symbol_versions[i].x, 	&enc->symbol_versions[0].x);
				swap_int (&enc->symbol_versions[i].y, 	&enc->symbol_versions[0].y);
				swap_byte(&enc->symbol_ecc_levels[i], 	&enc->symbol_ecc_levels[0]);
				break;
			}
		}
	}
    //if no master symbol exists in multi-symbol code
    if(enc->symbol_number > 1 && enc->symbol_positions[0] != 0)
    {
		J_REPORT_ERROR(("Master symbol missing"))
		return FAILURE;
    }
#endif
    // 如果仅存的symbol不是master，就扔掉，否则会白做
    if(enc->symbol_number == 1 && enc->symbol_positions[0] != 0)
        enc->symbol_positions[0] = 0;
#ifdef MULTI_SYMBOL
    // check if a symbol position is used twice
	for(Int32 i = 0; i < enc->symbol_number - 1; i++) {
		for(Int32 j=i+1; j<enc->symbol_number; j++)
		{
			if(enc->symbol_positions[i] == enc->symbol_positions[j])
			{
				J_REPORT_ERROR(("Duplicate symbol position"))
				return FAILURE;
			}
		}
	}

	if(!assignDockedSymbols(enc))
		return FAILURE;
    //check if the docked symbol size matches the docked side of its host
    if(!checkDockedSymbolSize(enc))
		return FAILURE;
    //set symbol index and symbol side size
    for(Int32 i = 0; i<enc->symbol_number; i++) {
    	//set symbol index
		enc->symbols[i].index = i;
        //set symbol side size
        enc->symbols[i].side_size.x = VERSION2SIZE(enc->symbol_versions[i].x);
        enc->symbols[i].side_size.y = VERSION2SIZE(enc->symbol_versions[i].y);
    }
#else
    Int32 i = 0;
    // 配置dock选项，尽管没必要
    enc->symbols[i].host = -1;
    // 配置边长和index
    // symbol index
    enc->symbols[0].index = 0;
    //set symbol side size
    enc->symbols[0].side_size.x = VERSION2SIZE(enc->symbol_versions[0].x);
    enc->symbols[0].side_size.y = VERSION2SIZE(enc->symbol_versions[0].y);
#endif
	
    return SUCCESS;
}

#ifdef MULTI_SYMBOL
/**
 * @brief Set metadata for slave symbols
 * @param enc the encode parameters
 * @return SUCCESS | FAILURE
*/
Boolean setSlaveMetadata(J_Encode* enc)
{
	//set slave metadata variables
	for(Int32 i=1; i<enc->symbol_number; i++)
	{
		Int32 SS, SE, V, E1=0, E2=0;
		Int32 metadata_length = 2; //Part I length
		//SS and V
		if(enc->symbol_versions[i].x != enc->symbol_versions[enc->symbols[i].host].x)
		{
		  	SS = 1;
			V = enc->symbol_versions[i].x - 1;
			metadata_length += 5;
		}
		else if(enc->symbol_versions[i].y != enc->symbol_versions[enc->symbols[i].host].y)
		{
			SS = 1;
			V = enc->symbol_versions[i].y - 1;
			metadata_length += 5;
		}
		else
		{
			SS = 0;
		}
		//SE and E
		if(enc->symbol_ecc_levels[i] == 0 || enc->symbol_ecc_levels[i] == enc->symbol_ecc_levels[enc->symbols[i].host])
		{
			SE = 0;
		}
		else
		{
			SE = 1;
			E1 = ecclevel2wcwr[enc->symbol_ecc_levels[i]][0] - 3;
			E2 = ecclevel2wcwr[enc->symbol_ecc_levels[i]][1] - 4;
			metadata_length += 6;
		}
		//write slave metadata
		enc->symbols[i].metadata = (J_Data *)malloc(sizeof(J_Data) + metadata_length*sizeof(Char));
		if(enc->symbols[i].metadata == NULL)
		{
			J_REPORT_ERROR(("Memory allocation for metadata in slave symbol failed"))
			return FAILURE;
		}
		enc->symbols[i].metadata->length = metadata_length;
		//Part I
		enc->symbols[i].metadata->data[0] = SS;
		enc->symbols[i].metadata->data[1] = SE;
		//Part II
		if(SS == 1)
		{
			convert_dec_to_bin(V, enc->symbols[i].metadata->data, 2, 5);
		}
		if(SE == 1)
		{
			Int32 start_pos = (SS == 1) ? 7 : 2;
			convert_dec_to_bin(E1, enc->symbols[i].metadata->data, start_pos, 3);
			convert_dec_to_bin(E2, enc->symbols[i].metadata->data, start_pos+3, 3);
		}
	}
	return SUCCESS;
}
#endif

/**
 * @brief 制造一个J_Code
 * @param enc 编码选项
 * @param data 要被编码的数据
 * @return 0:success | 1: out of memory | 2:no input data | 3:incorrect symbol version or position | 4: input data too long
*/
Int32 generateJCode(J_Encode* enc, J_Data* data) {
    // 检查数据
    if(data == NULL) {
        J_REPORT_ERROR(("No input data specified!"))
        return 2;
    } 
    if(data->length == 0) { // 并无数据传入
		J_REPORT_ERROR(("No input data specified!"))
        return 2;
    } 

    // 初始化要被编码的Symbol，设置index，并设置它们的metadata（边长之类的）
    if(!InitSymbols(enc)) {
    	J_REPORT_ERROR(("Initiate Symbols failed"))
    	return 3;
	}
    // 获得最优编码方案序列？，用NULL表达编码错误
    Int32 encoded_length;
    
    Int32* encode_seq = analyzeInputData(data, &encoded_length);
    if(encode_seq == NULL) {
		J_REPORT_ERROR(("Analyzing input data failed"))
		return 1;
    }
    //printf("data analyzed\n");
    /* 开始编码数据 */
    
    
	// 在最も望ましい模式下编码【数据】
    J_Data* encoded_data = encodeData(data, encoded_length, encode_seq);
    free(encode_seq);
    if(encoded_data == NULL) {
    	J_REPORT_ERROR(("Encoding data failed"))
        return 1;
    }

	//printf("data encoded\n");
    // 如果用户没有给出，就自动计算需要的 master symbol version ?
    if(enc->symbol_number == 1 && (enc->symbol_versions[0].x == 0 || enc->symbol_versions[0].y == 0)) {
        if(!setMasterSymbolVersion(enc, encoded_data)) {
        	J_REPORT_ERROR(("Set master symbol version failed"))
        	free(encoded_data);
            return 4;
        }
    }
    //printf("version set\n");
//	// set metadata for slave symbols
//	if(!setSlaveMetadata(enc)) {
//		free(encoded_data);
//		return 1;
//	}
	// 将数据搞入symbol中
	if(!fitDataIntoSymbols(enc, encoded_data)) {
		J_REPORT_ERROR(("Fitting data into symbol failed"))
		free(encoded_data);
		return 4;
	}
	//printf("data fit\n");
	free(encoded_data);
	// 制作master metadata / set master metadata
	if(!isDefaultMode(enc)) {
		if(!encodeMasterMetadata(enc)) {
			J_REPORT_ERROR(("Encoding master symbol metadata failed"))
            return 1;
		}
	}
	//printf("encoded metadata\n");

    // 按顺序编码每个symbol：encode each symbol in turn
#ifdef MULTI_SYMBOL
    for(Int32 i=0; i<enc->symbol_number; i++) {
#else
        Int32 i = 0;
#endif
        // 获取所有数据的LDPC编码数据
        J_Data* ecc_encoded_data = encodeLDPC(enc->symbols[i].data, enc->symbols[i].wcwr);
        if(ecc_encoded_data == NULL) {
            J_REPORT_ERROR(("LDPC encoding for the data in symbol %d failed", i))
            return 1;
        }
        //printf("ldpc encoded\n");
        // 制作数据交错
        interleaveData(ecc_encoded_data);
        //printf("interleaved\n");
        // 制作数据矩阵
        Boolean cm_flag = createMatrix(enc, i, ecc_encoded_data);
        free(ecc_encoded_data);
        if(!cm_flag) {
			J_REPORT_ERROR(("Creating matrix for symbol %d failed", i))
			return 1;
		}
#ifdef MULTI_SYMBOL
    }
#endif
    /* 公告：基础操作已经做完，下面进行装饰 */
    
    // 根据已经编码的数据，搞一个J_Code对象
    J_Code* cp = getCodePara(enc);

    // 对整个编码进行掩膜操作 / mask all symbols in the code
    if(!cp) {
		return 1;
    }
    if(isDefaultMode(enc)) {	// default mode：使用缺省方案
		maskSymbols(enc, DEFAULT_MASKING_REFERENCE, 0, 0);  // 第7种掩膜
	}
	else {                      // 用机器评估方案
		Int32 mask_reference = maskCode(enc, cp);
		if(mask_reference < 0) {
			free(cp->row_height);
			free(cp->col_width);
			free(cp);
			return 1;
		}
#if TEST_MODE
		J_REPORT_INFO(("mask reference: %d", mask_reference))
#endif
        // 因为机器评估方案可能会改掉之前的掩膜种类，与已经编码进metadata的掩膜种类不一致，要刷新！否则可能无法解码
		if(mask_reference != DEFAULT_MASKING_REFERENCE) {
			// 刷新编码metadata PartII
			updateMasterMetadataPartII(enc, mask_reference);
			// 重新写入metadata PartII
			placeMasterMetadataPartII(enc);
		}
	}
	//printf("masked\n");	
    // 制作bitmap，生成的Bitmap将会存在Encode对象中 / create the code bitmap
    Boolean cb_flag = createBitmap(enc, cp);
    free(cp->row_height);
    free(cp->col_width);
    free(cp);
    if(!cb_flag) {
		J_REPORT_ERROR(("Creating the code bitmap failed"))
		return 1;
	}
	//printf("bitmap created\n");
    return 0;
}

