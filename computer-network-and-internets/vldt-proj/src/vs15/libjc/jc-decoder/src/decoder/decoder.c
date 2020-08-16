// decoder.c    09/03/2020
// J_Code
// brief: the decoder functions
// Han Li adapted from Fraunhofer SIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jc.h"
#include "jc_finder.h"
#include "../supports/ldpc.h"
#include "decoder.h"
#include "encoder.h"
#include "../dll.h"

// 9.3.2020 —— 删去了有关颜色扩充的内容（当颜色>64种时）

/**
 * @brief 将读取出来的色盘信息写入symbol对象
 * @param matrix 像素矩阵
 * @param symbol symbol对象
 * @param p_index 色盘index
 * @param colour_index 色盘中某颜色的index
 * @param x the x 该颜色module的x
 * @param y the y 该颜色module的y
 */
void writeColourPalette(Bitmap* matrix, J_Decoded_Symbol* symbol, Int32 p_index, Int32 colour_index, Int32 x, Int32 y) {
    Int32 colour_number = (Int32)pow(2, symbol->metadata.Nc + 1);
    Int32 mtx_bytes_per_pixel = matrix->bits_per_pixel / 8;
    Int32 mtx_bytes_per_row = matrix->width * mtx_bytes_per_pixel;
    
    Int32 palette_offset = colour_number * 3 * p_index;
    Int32 mtx_offset = y * mtx_bytes_per_row + x * mtx_bytes_per_pixel;
    symbol->palette[palette_offset + colour_index*3 + 0]    = matrix->pixel[mtx_offset + 0];
    symbol->palette[palette_offset + colour_index*3 + 1] = matrix->pixel[mtx_offset + 1];
    symbol->palette[palette_offset + colour_index*3 + 2] = matrix->pixel[mtx_offset + 2];
}

/**
 * @brief 拿到FP/AP中的存储色盘的module的坐标
 * @param p_index 色盘的 index
 * @param matrix_width 像素矩阵宽
 * @param matrix_height 像素矩阵高
 * @param p1 色盘第一module的坐标
 * @param p2 色盘第二module的坐标
 */
void getColourPalettePosInFP(Int32 p_index, Int32 matrix_width, Int32 matrix_height, Vector2D* p1, Vector2D* p2) {
    switch(p_index) {
        case 0:
            p1->x = DISTANCE_TO_BORDER - 1;
            p1->y = DISTANCE_TO_BORDER - 1;
            p2->x = p1->x + 1;
            p2->y = p1->y;
            break;
        case 1:
            p1->x = matrix_width - DISTANCE_TO_BORDER;
            p1->y = DISTANCE_TO_BORDER - 1;
            p2->x = p1->x - 1;
            p2->y = p1->y;
            break;
        case 2:
            p1->x = matrix_width - DISTANCE_TO_BORDER;
            p1->y = matrix_height - DISTANCE_TO_BORDER;
            p2->x = p1->x - 1;
            p2->y = p1->y;
            break;
        case 3:
            p1->x = DISTANCE_TO_BORDER - 1;
            p1->y = matrix_height - DISTANCE_TO_BORDER;
            p2->x = p1->x + 1;
            p2->y = p1->y;
            break;
    }
}

/**
 * @brief 读取symbol中的色盘
 * @param matrix 像素矩阵
 * @param symbol symbol配置文件
 * @param data_map 标志矩阵
 * @param module_count module解码到这儿了
 * @param x the x 开始位置的x
 * @param y the y 开始位置的y
 * @return SUCCESS | FATAL_ERROR
 */
Int32 readColourPaletteInMaster(Bitmap* matrix, J_Decoded_Symbol* symbol, BYTE* data_map, Int32* module_count, Int32* x, Int32* y) {
    //allocate buffer for palette
    Int32 colour_number = (Int32)pow(2, symbol->metadata.Nc + 1);
    free(symbol->palette);
    symbol->palette = (BYTE*)malloc(colour_number * sizeof(BYTE) * 3 * COLOR_PALETTE_NUMBER);
    if(symbol->palette == NULL) {
        J_REPORT_ERROR(("Memory allocation for master palette failed"))
        return FATAL_ERROR;
    }
    
    // 读取颜色
    Int32 colour_index;            //the colour index number in colour palette
    for(Int32 i=0; i<COLOR_PALETTE_NUMBER; i++) {
        Vector2D p1, p2;
        getColourPalettePosInFP(i, matrix->width, matrix->height, &p1, &p2);
        //colour 0
        colour_index = master_palette_placement_index[i][0] % colour_number; //for 4-colour and 8-colour symbols
        writeColourPalette(matrix, symbol, i, colour_index, p1.x, p1.y);
        //colour 1
        colour_index = master_palette_placement_index[i][1] % colour_number; //for 4-colour and 8-colour symbols
        writeColourPalette(matrix, symbol, i, colour_index, p2.x, p2.y);
    }
    
    // 从metadata中读取颜色
    Int32 colour_counter = 2;    //the colour counter
    while(colour_counter < MIN(colour_number, 64))
    {
        //色盘 0
        colour_index = master_palette_placement_index[0][colour_counter] % colour_number; //for 4-colour and 8-colour symbols
        writeColourPalette(matrix, symbol, 0, colour_index, *x, *y);
        //更新 data map
        data_map[(*y) * matrix->width + (*x)] = 1;
        //下一module
        (*module_count)++;
        getNextMetadataModuleInMaster(matrix->height, matrix->width, (*module_count), x, y);
        
        //色盘 1
        colour_index = master_palette_placement_index[1][colour_counter] % colour_number; //for 4-colour and 8-colour symbols
        writeColourPalette(matrix, symbol, 1, colour_index, *x, *y);
        //更新 data map
        data_map[(*y) * matrix->width + (*x)] = 1;
        //下一module
        (*module_count)++;
        getNextMetadataModuleInMaster(matrix->height, matrix->width, (*module_count), x, y);
        
        //色盘 2
        colour_index = master_palette_placement_index[2][colour_counter] % colour_number; //for 4-colour and 8-colour symbols
        writeColourPalette(matrix, symbol, 2, colour_index, *x, *y);
        //更新 data map
        data_map[(*y) * matrix->width + (*x)] = 1;
        //下一module
        (*module_count)++;
        getNextMetadataModuleInMaster(matrix->height, matrix->width, (*module_count), x, y);
        
        //色盘 3
        colour_index = master_palette_placement_index[3][colour_counter] % colour_number; //for 4-colour and 8-colour symbols
        writeColourPalette(matrix, symbol, 3, colour_index, *x, *y);
        //更新 data map
        data_map[(*y) * matrix->width + (*x)] = 1;
        //下一module
        (*module_count)++;
        getNextMetadataModuleInMaster(matrix->height, matrix->width, (*module_count), x, y);
        
        //进行下一颜色
        colour_counter++;
    }
    
    
    return SUCCESS;
}



/**
 * @brief 从像素矩阵中的［色彩盘区域（不是metadata区域）］，取找某个像素［距离最近的］［色彩盘］的编号
 * @param matrix 像素矩阵
 * @param x 某个像素的x坐标
 * @param y 某个像素的y坐标
 * @return 该像素［距离最近的］［色彩盘］的编号
 */
Int32 getNearestPalette(Bitmap* matrix, Int32 x, Int32 y) {
    // 设置像素矩阵中的色彩盘区域坐标 / set the palette coordinate [COLOR_PALETTE_NUMBER = 4 in jc.h]
    Int32 px[COLOR_PALETTE_NUMBER], py[COLOR_PALETTE_NUMBER];
    px[0] = DISTANCE_TO_BORDER - 1 + 3;                 // DISTANCE_TO_BORDER = 4 in jc.h
    py[0] = DISTANCE_TO_BORDER - 1;
    px[1] = matrix->width - DISTANCE_TO_BORDER - 3;
    py[1] = DISTANCE_TO_BORDER - 1;
    px[2] = matrix->width - DISTANCE_TO_BORDER - 3;
    py[2] = matrix->height- DISTANCE_TO_BORDER;
    px[3] = DISTANCE_TO_BORDER - 1 + 3;
    py[3] = matrix->height- DISTANCE_TO_BORDER;
    
    // 用两点间距离公式，计算距离最近的palette
    Float min = DIST(0, 0, matrix->width, matrix->height);
    Int32 p_index = 0;
    for (Int32 i = 0; i < COLOR_PALETTE_NUMBER; i++) {
        Float dist = DIST(x, y, px[i], py[i]);
        if (dist < min) {
            min = dist;
            p_index = i;
        }
    }
    return p_index;
}

/**
 * @brief 用“非得选一个不可！”的方式解码一个数据module
 * @param matrix 像素矩阵
 * @param palette 色彩盘
 * @param colour_number module色彩可能的种类个数
 * @param norm_palette （经过正常化的）色彩盘
 * @param pal_ths 色彩盘各色彩的RGB阈值
 * @param x 待解码module的横坐标
 * @param y 待解码module的纵坐标
 * @return 解码出的bit stream！
 */
BYTE decodeModuleHD(Bitmap* matrix, BYTE* palette, Int32 colour_number, Float* norm_palette, Float* pal_ths, Int32 x, Int32 y) {
    // 获取最近的色彩盘的编号，方便取色 / get the nearest palette
    Int32 p_index = getNearestPalette(matrix, x, y);
    
    // 读取像素的RGB值
    BYTE rgb[3];
    Int32 mtx_bytes_per_pixel = matrix->bits_per_pixel / 8; // 有4个通道，而因此bits_per_pixel = 4 * 8 = 32
    Int32 mtx_bytes_per_row = matrix->width * mtx_bytes_per_pixel;
    Int32 mtx_offset = y * mtx_bytes_per_row + x * mtx_bytes_per_pixel;
    rgb[0] = matrix->pixel[mtx_offset + 0];
    rgb[1] = matrix->pixel[mtx_offset + 1];
    rgb[2] = matrix->pixel[mtx_offset + 2]; // 只取RGB三个通道，不管A通道
    
    BYTE index1 = 0, index2 = 0;
    // 根据R、G、B阈值检查是否是黑点（000） / check black module
    if (rgb[0] < pal_ths[p_index * 3 + 0] && rgb[1] < pal_ths[p_index*3 + 1] && rgb[2] < pal_ths[p_index*3 + 2]) {
        index1 = 0;
        return index1;
    }
    if (palette) {
        Float min1 = 255 * 255 * 3, min2 = 255 * 255 * 3;
        for (Int32 i = 0; i < colour_number; i++) {
            // 对该色块的R、G、B正常化
            Float rgb_max = MAX(rgb[0], MAX(rgb[1], rgb[2]));   // 找到rgb内的最大值
            Float r = (Float)rgb[0] / rgb_max;
            Float g = (Float)rgb[1] / rgb_max;
            Float b = (Float)rgb[2] / rgb_max;
            // Float l = ((rgb[0] + rgb[1] + rgb[2]) / 3.0f) / 255.0f;
            
            // 对正常化后的色盘内，目前正在检视的颜色的R、G、B的提取 / normalize the colour values in colour palette
            Float pr = norm_palette[colour_number * 4 * p_index + i * 4 + 0];
            Float pg = norm_palette[colour_number * 4 * p_index + i * 4 + 1];
            Float pb = norm_palette[colour_number * 4 * p_index + i * 4 + 2];
            // Float pl = norm_palette[colour_number*4*p_index + i*4 + 3];
            
            // 对比提取出正在检视的色盘颜色与当前像素颜色，寻找最适合的那一个 / compare the normalized module colour with palette
            Float diff = (pr - r) * (pr - r) + (pg - g) * (pg - g) + (pb - b) * (pb - b); // + (pl - l) * (pl - l);
            // 用空间距离计算公式，找差异最小的那个颜色
            if (diff < min1) {
                // copy min1 to min2
                min2 = min1;
                index2 = index1;
                // update min1
                min1 = diff;
                index1 = (BYTE)i;
            }
            else if(diff < min2) {
                min2 = diff;
                index2 = (BYTE)i;
            }
        }
        
        // 如果找到的最接近的颜色编号为0或7
        if (index1 == 0 || index1 == 7) {
            Int32 rgb_sum = rgb[0] + rgb[1] + rgb[2];
            Int32 p0_sum = palette[colour_number*3*p_index + 0*3 + 0] + palette[colour_number*3*p_index + 0*3 + 1] + palette[colour_number*3*p_index + 0*3 + 2];
            Int32 p7_sum = palette[colour_number*3*p_index + 7*3 + 0] + palette[colour_number*3*p_index + 7*3 + 1] + palette[colour_number*3*p_index + 7*3 + 2];
            
            // 仲裁
            if (rgb_sum < ((p0_sum + p7_sum) / 2)) {
                index1 = 0;
            }
            else {
                index1 = 7;
            }
        }
        
    }
    else {    //if no palette is available, decode the module as black/white
        index1 = ((rgb[0] > 100 ? 1 : 0) + (rgb[1] > 100 ? 1 : 0) + (rgb[2] > 100 ? 1 : 0)) > 1 ? 1 : 0;
    }
    return index1;
}

/**
 * @brief 为master symbol的第一部分 (Nc) 解码[一个像素]
 * @param rgb RGB格式的像素信息列
 * @return 返回一个解码出的像素，一共3个bits（000,001,010,011...111）
 */
BYTE decodeModuleNc(BYTE* rgb) {
    // 注意：第一部分(Nc)的所有block只有8种颜色！！！！！该函数只会返回8种值！！！！！
    // 注意：第一部分(Nc)的所有block只有8种颜色！！！！！该函数只会返回8种值！！！！！
    // 注意：第一部分(Nc)的所有block只有8种颜色！！！！！该函数只会返回8种值！！！！！
    Int32 ths_black = 80;
    Double ths_std = 0.08;  // 标准差的阈值，RGB的标准差低于这个值认为是白色（除非RGB均接近于0）
    // R、G、B三者值［都］小于阈值，认为这块颜色是黑色
    if(rgb[0] < ths_black && rgb[1] < ths_black && rgb[2] < ths_black) {
        return 0;   //000
    }
    // ---检查颜色---
    Double ave, var;
    // binarizer.c : 获取一块像素的RGB的平均值及RGB的方差
    getRGBAveVar(rgb, &ave, &var);
    Double std = sqrt(var);    // RGB的标准差
    BYTE min, mid, max;
    Int32 index_min, index_mid, index_max;
    // binarizer.c : 分别获取RGB中最大、最小、居中的值
    getRGBMinMax(rgb, &min, &mid, &max, &index_min, &index_mid, &index_max);
    std /= (Double)max;    // normalize std？
    BYTE bits[3];   // 用来存该像素点代表的3位比特：xxx
    if(std > ths_std) { // 标准差偏差较大，因此不会是白色
        // 其实PartI的解码，是按照RGB的偏大偏小来决定位的
        // 现在的情况是：必同时有1个1和0（非黑和白），所以要先确定哪个是1哪个是0。
        // 所以在这种情况下，R、G、B中哪个偏大哪个就置1，哪个最小哪个就置0，中间值到底是1还是0透过计算得到
        bits[index_max] = 1;
        bits[index_min] = 0;
        // 下面计算原理：若mid颜色大于max颜色和min颜色的几何平均数，认为mid颜色决定的位是1。
        // （可以证明，这样的认为方法的误差最小）
        Double r1 = (Double)rgb[index_mid] / (Double)rgb[index_min];
        Double r2 = (Double)rgb[index_max] / (Double)rgb[index_mid];
        if (r1 > r2)
            bits[index_mid] = 1;
        else
            bits[index_mid] = 0;
    }
    else { // 标准差偏差比较小，低于阈值，应该是白色
        return 7; //111
    }
    return ((bits[0] << 2) + (bits[1] << 1) + bits[2]); // 按照bits[]的各位返回解码值
}

/**
 * @brief 拿到色盘中的pixel的每个通道的阈值，超过阈值的我们认为有这个颜色，没超过阈值的我们认为这个颜色没有
 / Get the pixel value thresholds for each channel of the colours in the palette
 * @param palette 色盘
 * @param colour_number 色盘颜色的种类个数
 * @param palette_ths 色盘各像素通道阈值 / the palette RGB value thresholds
 */
void getPaletteThreshold(BYTE* palette, Int32 colour_number, Float* palette_ths) {
    if(colour_number == 4) {
        // 如果颜色有4种，则每个颜色放了4个样本
        Int32 cpr0 = MAX(palette[0], palette[3]);
        Int32 cpr1 = MIN(palette[6], palette[9]);
        Int32 cpg0 = MAX(palette[1], palette[7]);
        Int32 cpg1 = MIN(palette[4], palette[10]);
        Int32 cpb0 = MAX(palette[8], palette[11]);
        Int32 cpb1 = MIN(palette[2], palette[5]);
        
        // 每个颜色的最大最小平均作为阈
        palette_ths[0] = (cpr0 + cpr1) / 2.0f;
        palette_ths[1] = (cpg0 + cpg1) / 2.0f;
        palette_ths[2] = (cpb0 + cpb1) / 2.0f;
    }
    else if(colour_number == 8) {
        // 如果颜色有8种，则每个颜色放了8个样本
        Int32 cpr0 = MAX(MAX(MAX(palette[0], palette[3]), palette[6]), palette[9]);
        Int32 cpr1 = MIN(MIN(MIN(palette[12], palette[15]), palette[18]), palette[21]);
        Int32 cpg0 = MAX(MAX(MAX(palette[1], palette[4]), palette[13]), palette[16]);
        Int32 cpg1 = MIN(MIN(MIN(palette[7], palette[10]), palette[19]), palette[22]);
        Int32 cpb0 = MAX(MAX(MAX(palette[2], palette[8]), palette[14]), palette[20]);
        Int32 cpb1 = MIN(MIN(MIN(palette[5], palette[11]), palette[17]), palette[23]);
        
        // 每个颜色的最大最小平均作为阈
        palette_ths[0] = (cpr0 + cpr1) / 2.0f;
        palette_ths[1] = (cpg0 + cpg1) / 2.0f;
        palette_ths[2] = (cpb0 + cpb1) / 2.0f;
    }
}

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
 * @brief 从一对色块中解码出Nc讯息：Decode the encoded bits of Nc from the module colour
 * @param module1_colour 第一个module的颜色
 * @param module2_colour 第二个module的颜色
 * @return 解码出来的Nc讯息 the decoded bits
 */
BYTE decodeNcModuleColour(BYTE module1_colour, BYTE module2_colour) {
    for (Int32 i = 0; i < 8; i++) {
        // nc_colour_encode_table 在 encode.h中。让我们康康到底是8种type的哪一种
        if (module1_colour == nc_color_encode_table[i][0] && module2_colour == nc_color_encode_table[i][1])
            return i;
    }
    return 8; //if no match, return an invalid value
}

/**
 * @brief 解码master symbol的metadata（第一部分）
 * @param matrix symbol矩阵
 * @param symbol decoded symbol应存放的位置
 * @param data_map the data module positions
 * @param module_count the 实时修改已经探测的module数量，供下一轮探测时使用
 * @param x 供实时修改当前探测到的module的x坐标，供探测时及下一轮探测使用
 * @param y 供实时修改当前探测到的module的y坐标，供探测时及下一轮探测使用
 * @return SUCCESS | FAILURE | DECODE_METADATA_FAILED
 */
Int32 decodeMasterMetadataPartI(Bitmap* matrix, J_Decoded_Symbol* symbol, BYTE* data_map, Int32* module_count, Int32* x, Int32* y) {
    // 暂存：解码后Nc的每个module的颜色值
    BYTE module_colour[MASTER_METADATA_PART1_MODULE_NUMBER];         // 4
    Int32 mtx_bytes_per_pixel = matrix->bits_per_pixel / 8;         // RGBA图像：每像素4字节
    Int32 mtx_bytes_per_row = matrix->width * mtx_bytes_per_pixel;
    Int32 mtx_offset;
    //    // 解码PART 1：对于PART 1 (Nc)中的每个像素进行处理（循环，用module_count记录已经处理过的像素）
    //    saveImage(matrix, "/Users/phantef/Desktop/test/all/sampled_pic_before_going_to_decode.png");
    while((*module_count) < MASTER_METADATA_PART1_MODULE_NUMBER) {
        // 从(x,y)处的module解码出Bit讯息
        mtx_offset = (*y) * mtx_bytes_per_row + (*x) * mtx_bytes_per_pixel;
        // 提取颜色
        BYTE rgb = decodeModuleNc(&matrix->pixel[mtx_offset]);
        if (rgb != 0 && rgb != 3 && rgb != 6) {
#if TEST_MODE
            J_REPORT_ERROR(("Invalid module colour in primary metadata part 1 found"))
#endif
            return DECODE_METADATA_FAILED;  // 颜色有错
        }
        // 保存颜色
        module_colour[*module_count] = rgb;
        // （保存访问标示）set data map
        data_map[(*y) * matrix->width + (*x)] = 1;
        // 准备访问下一个module，找到下一个module的位置，存入xy中
        (*module_count)++;
        getNextMetadataModuleInMaster(matrix->height, matrix->width, (*module_count), x, y);// 留意xy是指针
    }
    
    // 解码 Nc。包含Nc讯息的色块只有4个，我们是去确定这4*3 = 12个比特究竟代表哪种Nc的颜色模式。
    BYTE bits[2];
    bits[0] = decodeNcModuleColour(module_colour[0], module_colour[1]);    //the first 3 bits
    bits[1] = decodeNcModuleColour(module_colour[2], module_colour[3]);    //the last 3 bits
    if(bits[0] > 7 || bits[1] > 7) {
#if TEST_MODE
        J_REPORT_ERROR(("Invalid colour combination in primary metadata part 1 found"))
#endif
        return DECODE_METADATA_FAILED;
    }
    // 从刚刚的bits[]中（用2个byte的前3位 表达了 6个bit）对他们表达的那些bit（6个）做一个对应。
    // 没办法了，只能用一个BYTE表达一个bit
    BYTE part1[MASTER_METADATA_PART1_LENGTH] = {0};            // 6 encoded bits
    Int32 bit_count = 0;
    for(Int32 n = 0; n < 2; n++) {
        for(Int32 i = 0; i < 3; i++) {  // little endian
            BYTE bit = (bits[n] >> (2 - i)) & 0x01; // &0x01，只保留最低位
            part1[bit_count] = bit;
            bit_count++;
        }
    }
    
    // 为这6个bit做LDPC硬解码（ldpc.c）传入参数：被编码的数据、数据长度、1列的“1”的个数、1行的“1”的个数
    // 后期视
    if( !decodeLDPChd(part1, MASTER_METADATA_PART1_LENGTH, MASTER_METADATA_PART1_LENGTH > 36 ? 4 : 3, 0) ) {
        // #if TEST_MODE
        J_REPORT_ERROR(("LDPC decoding for master metadata part 1 failed"))
        // #endif
        return FAILURE; // LDPC无法通过试验
    }
    // 分析解码后的part_1。原来的6个bit经过解码之后将只剩下3个bit / parse part1
    // 这3个bit将表示8种色彩模式中的1种
    symbol->metadata.Nc = (part1[0] << 2) + (part1[1] << 1) + part1[2];
    
    /*
     
     * 后期可能加上的改动：：：：：：：
     * 针对一些不可能用到的颜色模式，可以用来当启始帧标志！！！！！
     * 针对一些不可能用到的颜色模式，可以用来当启始帧标志！！！！！
     * 针对一些不可能用到的颜色模式，可以用来当启始帧标志！！！！！
     * 针对一些不可能用到的颜色模式，可以用来当启始帧标志！！！！！
     
     */
    
    
    return SUCCESS;
}

/**
 * @brief 解码metadata的第二部分，详见技术文档
 * @param matrix 采样后的像素矩阵
 * @param symbol master symbol暂存区域
 * @param data_map 已经解码过的数据点的位置 / the data module positions
 * @param norm_palette 被正常化后的色盘
 * @param pal_ths the 色盘的RGB阈值 / palette RGB value thresholds
 * @param module_count 标志［下一个］module的位置 / the index number of the next module
 * @param x 这一次及下一次识别的module的坐标x / the x coordinate of the current and the next module
 * @param y 这一次及下一次识别的module的坐标y / the y coordinate of the current and the next module
 * @return SUCCESS | FAILURE | DECODE_METADATA_FAILED | FATAL_ERROR
 */
Int32 decodeMasterMetadataPartII(Bitmap* matrix, J_Decoded_Symbol* symbol, BYTE* data_map, Float* norm_palette, Float* pal_ths, Int32* module_count, Int32* x, Int32* y) {
    // Part II 的暂存区域（长度38）
    BYTE part2[MASTER_METADATA_PART2_LENGTH] = {0};            // 38 encoded bits
    Int32 part2_bit_count = 0;  // 已经进行的个数
    UInt32 V, E;                // side-version及error-correction参数
    UInt32 V_length = 10, E_length = 6;
    
    Int32 colour_number = (Int32)pow(2, symbol->metadata.Nc + 1);   // 换算颜色种类与Nc的关系
    // 每个module的比特数量 ＝ log2(颜色种类，即码本个数)
    Int32 bits_per_module = (Int32)(log(colour_number) / log(2));
    
    // 读取第二部分
    while (part2_bit_count < MASTER_METADATA_PART2_LENGTH) {
        // 将 (x,y) 处的比特都读取出来
        BYTE bits = decodeModuleHD(matrix, symbol->palette, colour_number, norm_palette, pal_ths, *x, *y);
        // 解码后的数据写入part 2，请注意写入的其实是色盘中色彩的种类（0～7）
        for (Int32 i = 0; i < bits_per_module; i++) {
            BYTE bit = (bits >> (bits_per_module - 1 - i)) & 0x01;  // 只取最后一个图像
            if(part2_bit_count < MASTER_METADATA_PART2_LENGTH) {
                part2[part2_bit_count] = bit;
                part2_bit_count++;
            }
            else {    // if part2 is full, stop
                break;
            }
        }
        // 设置已经解码纪录 / set data map
        data_map[(*y) * matrix->width + (*x)] = 1;
        // 查找下一个解码module / go to the next module
        (*module_count)++;
        getNextMetadataModuleInMaster(matrix->height, matrix->width, (*module_count), x, y);
    }
    
    // 为Part II进行LDPC硬纠错解码（和Nc部分的纠错方法相同）
    if( !decodeLDPChd(part2, MASTER_METADATA_PART2_LENGTH, MASTER_METADATA_PART2_LENGTH > 36 ? 4 : 3, 0) ) {
#if TEST_MODE
        J_REPORT_ERROR(("LDPC decoding for master metadata part 2 failed"))
#endif
        return DECODE_METADATA_FAILED;
    }
    
    // 检视第2部分 / parse part2
    
    // 读取side-version / read V
    // 检视水平（x）方向side-version / get horizontal side version
    V = 0;
    for(Int32 i = 0; i < V_length / 2; i++) {   // for i in 0...4
        // part2[]的前5位（可表示0～31这32个数字）表示水平side-version
        V += part2[i] << (V_length / 2 - 1 - i);
    }
    symbol->metadata.side_version.x = V + 1;    // +1是为了从1开始到32
    // 检视垂直（y）方向side-version / get vertical side version
    V = 0;
    for(Int32 i = 0; i < V_length / 2; i++) {
        // part2[]的后5位（可表示0～31这32个数字）表示垂直side-version
        V += part2[i + V_length / 2] << (V_length / 2 - 1 - i);
    }
    symbol->metadata.side_version.y = V + 1;
    
    // 读取error correction参数 / read E
    Int32 bit_index = V_length;
    // 读取 wc 参数 (the first half of E)
    E = 0;
    // 从刚结束识别V的地方开始
    for(Int32 i = bit_index; i < (bit_index + E_length / 2); i++) {
        E += part2[i] << (E_length / 2 - 1 - (i - bit_index));
    }
    symbol->metadata.ecl.x = E + 3;        // wc = E_part1 + 3
    // 读取 wr 参数 (the second half of E)
    E = 0;
    for(Int32 i = bit_index; i < (bit_index + E_length / 2); i++) {
        E += part2[i + E_length / 2] << (E_length / 2 - 1 - (i - bit_index));
    }
    symbol->metadata.ecl.y = E + 4;        // wr = E_part2 + 4
    
    // 读取mask引用类型（3bit，8种） / read MSK
    bit_index = V_length + E_length;
    symbol->metadata.mask_type = (part2[bit_index + 0] << 2) + (part2[bit_index + 1] << 1) + part2[bit_index + 2];
    
    symbol->metadata.docked_position = 0;
    
    // 检视这个取样后的matrix是否与metadata种表达的一致
    symbol->side_size.x = VERSION2SIZE(symbol->metadata.side_version.x);
    symbol->side_size.y = VERSION2SIZE(symbol->metadata.side_version.y);
    if(matrix->width != symbol->side_size.x || matrix->height != symbol->side_size.y) {
        J_REPORT_INFO(("Sampled matrix size is ambiguous. Trying force decode."))
		return FAILURE;
    }
    // 检视wc、wr是否符合逻辑
    Int32 wc = symbol->metadata.ecl.x;
    Int32 wr = symbol->metadata.ecl.y;
    if(wc >= wr) {
        J_REPORT_ERROR(("Incorrect error correction parameter in primary symbol metadata"))
        return DECODE_METADATA_FAILED;
    }
    return SUCCESS;
}

/**
 * @brief 尝试读取矩阵中的像素数据（生肉）
 * @param matrix 像素矩阵
 * @param symbol 存放解码的symbol的数据区域
 * @param data_map data module的访问记录
 * @param norm_palette 正常化之后的色盘
 * @param pal_ths 色盘各色彩RGB阈值
 * @return the decoded data | NULL if failed
 */
J_Data* readRawModuleData(Bitmap* matrix, J_Decoded_Symbol* symbol, BYTE* data_map, Float* norm_palette, Float* pal_ths) {
    // 根据Nc计算出颜色种类
    Int32 colour_number = (Int32)pow(2, symbol->metadata.Nc + 1);
    Int32 module_count = 0;
    J_Data* data = (J_Data*)malloc(sizeof(J_Data) + matrix->width * matrix->height * sizeof(Char));
    if(data == NULL) {
        J_REPORT_ERROR(("Memory allocation for raw module data failed"))
        return NULL;
    }
    
    
    // 注意：从左到右逐列扫描！！因为编码是按列进行的！按列进行的！按列进行的！
    for (Int32 j = 0; j < matrix->width; j++) {
        // 扫描列从上到下的每个module
        for (Int32 i = 0; i < matrix->height; i++) {
            // 解码没被访问过的数据点
            if (data_map[i * matrix->width + j] == 0) {
                // 解码坐标位于 (j,i) 的点
                BYTE bits = decodeModuleHD(matrix, symbol->palette, colour_number, norm_palette, pal_ths, j, i);
                // 将解码出的颜色种类（其实表达了bits）
                data -> data[module_count] = (Char)bits;
                module_count++;
#if TEST_MODE
                decoded_module_colour_index[i*matrix->width + j] = bits;
#endif
            }
            else {
#if TEST_MODE
                decoded_module_colour_index[i*matrix->width + j] = 255;
#endif
            }
        }
    }
    data->length = module_count; 
    return data;
}

/**
 * @brief 位操作：将一次解码声称的“1个BYTE包含多个bit”类型的数据，一位一位地搞成“1个BYTE只代表1个bit”的形式。所用BYTE位将会增加。输出的数据模块中，每个字节的合法表达只能是0000H或0001H。即对数据进行字节扩展 / Convert multi-bit-per-byte raw module data to one-bit-per-byte raw data
 * @param raw_module_data “1个BYTE包含多个bit”类型的module形数据
 * @param bits_per_module module型数据中，1个BYTE究竟包含多少个bit
 * @return the converted data | NULL if failed
 */
J_Data* rawModuleData2RawData(J_Data* raw_module_data, Int32 bits_per_module) {
    // 新建内存保存二进制的BYTE
    J_Data* raw_data = (J_Data *)malloc(sizeof(J_Data) + raw_module_data->length * bits_per_module * sizeof(Char));
    if(raw_data == NULL) {
        J_REPORT_ERROR(("Memory allocation for raw data failed"))
        return NULL;
    }
    // 进行转换。每个module从左望右，一个一个bit读取及写入
    for(Int32 i = 0; i < raw_module_data->length; i++) {
        for(Int32 j = 0; j < bits_per_module; j++) {
            raw_data->data[i * bits_per_module + j] = (raw_module_data->data[i] >> (bits_per_module - 1 - j)) & 0x01;
        }
    }
    raw_data->length = raw_module_data->length * bits_per_module;
    return raw_data;
}

/**
 * @brief 在data_map中标明fp和alignment pattern的位置
 * @param data_map the data module positions
 * @param width the width of the data map
 * @param height the height of the data map
 * @param type symble的类型 0: master, 1: slave
 */
void fillDataMap(BYTE* data_map, Int32 width, Int32 height, Int32 type) {
    Int32 side_ver_x_index = SIZE2VERSION(width) - 1;
    Int32 side_ver_y_index = SIZE2VERSION(height) - 1;
    Int32 number_of_ap_x = j_ap_num[side_ver_x_index];
    Int32 number_of_ap_y = j_ap_num[side_ver_y_index];
    for (Int32 i = 0; i < number_of_ap_y; i++) {
        for (Int32 j=0; j<number_of_ap_x; j++) {
            // 获取各ap中心坐标
            Int32 x_offset = j_ap_pos[side_ver_x_index][j] - 1;
            Int32 y_offset = j_ap_pos[side_ver_y_index][i] - 1;
            // 在data_map标注ap的位置，防止读取器读取到它们 / the cross
            data_map[y_offset         * width + x_offset]          =
            data_map[y_offset        * width + (x_offset - 1)] =
            data_map[y_offset        * width + (x_offset + 1)] =
            data_map[(y_offset - 1) * width + x_offset]       =
            data_map[(y_offset + 1) * width + x_offset]       = 1;
            
            // 对角线上的modules / the diagonal modules
            if (i == 0 && (j == 0 || j == number_of_ap_x - 1)) {
                // at finder pattern 0 and 1 positions （朝左倾斜）
                data_map[(y_offset - 1) * width + (x_offset - 1)] =
                data_map[(y_offset + 1) * width + (x_offset + 1)] = 1;
                if (type == 0) {    //master symbol才有
                    data_map[(y_offset - 2) * width + (x_offset - 2)] =
                    data_map[(y_offset - 2) * width + (x_offset - 1)] =
                    data_map[(y_offset - 2) * width +  x_offset]       =
                    data_map[(y_offset - 1) * width + (x_offset - 2)] =
                    data_map[ y_offset        * width + (x_offset - 2)] = 1;
                    
                    data_map[(y_offset + 2) * width + (x_offset + 2)] =
                    data_map[(y_offset + 2) * width + (x_offset + 1)] =
                    data_map[(y_offset + 2) * width +  x_offset]       =
                    data_map[(y_offset + 1) * width + (x_offset + 2)] =
                    data_map[ y_offset        * width + (x_offset + 2)] = 1;
                }
            }
            else if(i == number_of_ap_y - 1 && (j == 0 || j == number_of_ap_x - 1)) {
                // at finder pattern 2 and 3 positions （朝右倾斜）
                data_map[(y_offset - 1) * width + (x_offset + 1)] =
                data_map[(y_offset + 1) * width + (x_offset - 1)] = 1;
                if(type == 0) {     // master symbol才有
                    data_map[(y_offset - 2) * width + (x_offset + 2)] =
                    data_map[(y_offset - 2) * width + (x_offset + 1)] =
                    data_map[(y_offset - 2) * width +  x_offset]       =
                    data_map[(y_offset - 1) * width + (x_offset + 2)] =
                    data_map[ y_offset        * width + (x_offset + 2)] = 1;
                    
                    data_map[(y_offset + 2) * width + (x_offset - 2)] =
                    data_map[(y_offset + 2) * width + (x_offset - 1)] =
                    data_map[(y_offset + 2) * width +  x_offset]       =
                    data_map[(y_offset + 1) * width + (x_offset - 2)] =
                    data_map[ y_offset        * width + (x_offset - 2)] = 1;
                }
            }
            else {    // at other positions
                // 偶行偶列 或 奇行奇列 / even row, even column / odd row, odd column
                if( (i % 2 == 0 && j % 2 == 0) || (i % 2 == 1 && j % 2 == 1)) {
                    data_map[(y_offset - 1) * width + (x_offset - 1)] =
                    data_map[(y_offset + 1) * width + (x_offset + 1)] = 1;
                }
                // 奇行偶列 或 偶行奇列 / odd row, even column / even row, old column
                else {
                    data_map[(y_offset - 1) * width + (x_offset + 1)] =
                    data_map[(y_offset + 1) * width + (x_offset - 1)] = 1;
                }
            }
        }
    }
}

/**
 * @brief 为symbol加载默认的关于metadata的系统设置 / Load default metadata values and colour palettes for master symbol
 * @param matrix 像素矩阵 / the symbol matrix
 * @param symbol symbol存储区域 / the master symbol
 */
void loadDefaultMasterMetadata(Bitmap* matrix, J_Decoded_Symbol* symbol) {
#if TEST_MODE
    J_REPORT_INFO(("Loading default master metadata"))
#endif
    // 为symbol加载默认的关于metadata的系统设置
    symbol->metadata.default_mode = 1;
    symbol->metadata.Nc = DEFAULT_MODULE_COLOR_MODE;
    symbol->metadata.ecl.x = ecclevel2wcwr[DEFAULT_ECC_LEVEL][0];
    symbol->metadata.ecl.y = ecclevel2wcwr[DEFAULT_ECC_LEVEL][1];
    symbol->metadata.mask_type = DEFAULT_MASKING_REFERENCE;
    symbol->metadata.docked_position = 0;                            //no default value
    symbol->metadata.side_version.x = SIZE2VERSION(matrix->width);    //no default value
    symbol->metadata.side_version.y = SIZE2VERSION(matrix->height);    //no default value
}

/**
 * @brief 解码symbol中的数据，根据已经识别出的色盘、错误识别。解码后的信息放在symbol中。
 * @param matrix 像素矩阵
 * @param symbol 等待被解码的symbol，其中应包含必要信息
 * @param data_map 解码记录表
 * @param norm_palette 被正常化过的色盘
 * @param pal_ths 色盘中每种颜色的RGB阈值
 * @param type 码块种类。 0: master, 1: slave
 * @return SUCCESS | FAILURE | DECODE_METADATA_FAILED | FATAL_ERROR
 */
Int32 decodeSymbol(Bitmap* matrix, J_Decoded_Symbol* symbol, BYTE* data_map, Float* norm_palette, Float* pal_ths, Int32 type, BYTE expected_pack_no) {
    
    // 将finder pattern及alignment pattern全部设为忽略
    fillDataMap(data_map, matrix->width, matrix->height, type);
    
    // ！！！！读取数据（生肉）！！！！
    J_Data* raw_module_data = readRawModuleData(matrix, symbol, data_map, norm_palette, pal_ths);
    if(raw_module_data == NULL) {
        J_REPORT_ERROR(("Reading raw module data in symbol %d failed", symbol->index))
        free(data_map);
        return FATAL_ERROR;
    }
#if TEST_MODE
    FILE* fp = fopen("j_dec_module_data_han.bin", "wb");
    fwrite(raw_module_data->data, raw_module_data->length, 1, fp);
    fclose(fp);
#endif // TEST_MODE (9.3.2020以上并无错误)
    
    // 扔掉mask / demask
    demaskSymbol(raw_module_data, data_map, symbol->side_size, symbol->metadata.mask_type, (Int32)pow(2, symbol->metadata.Nc + 1));
    free(data_map);
    // ---------------至此，解码实际上已经完成---------------
#if TEST_MODE
    fp = fopen("j_demasked_module_data_han.bin", "wb");
    fwrite(raw_module_data->data, raw_module_data->length, 1, fp);
    fclose(fp);
#endif // TEST_MODE
    
    // 将解码后用多位的BYTE表达的一个个module的数据，改换为一个BYTE只包含1个二进制位的形式
    J_Data* raw_data = rawModuleData2RawData(raw_module_data, symbol->metadata.Nc + 1);
    free(raw_module_data);
    if(raw_data == NULL) {
        J_REPORT_ERROR(("Reading raw data in symbol %d failed", symbol->index))
        return FATAL_ERROR;
    }
    
    // 利用wc、wr参数，计算symbol的在编码前的bit个数（生猪型bit个数）(Pn) 及 被编码后变成实际bit个数（生肉型bit个数）(Pg)
    Int32 wc = symbol->metadata.ecl.x;
    Int32 wr = symbol->metadata.ecl.y;
    Int32 Pg = (raw_data->length / wr) * wr;    // max_gross_payload = floor(capacity / wr) * wr
    Int32 Pn = Pg * (wr - wc) / wr;                // code_rate = 1 - wc/wr = (wr - wc)/wr, max_net_payload = max_gross_payload * code_rate
    
    // 给生肉去交错 / deinterleave data
    raw_data->length = Pg;    //drop the padding bits
    deinterleaveData(raw_data); // interleave.c
	
	//printf(" DEBUG INFO: decodeLDPC started.");
	// 用硬选择模式解码 ldpc
    if(decodeLDPChd((BYTE*)raw_data->data, Pg, symbol->metadata.ecl.x, symbol->metadata.ecl.y) != Pn) {
        //J_REPORT_ERROR(("LDPC decoding for data in symbol %d failed", symbol->index))
        free(raw_data);
        return FAILURE;
    }
	//printf(" DEBUG INFO: decodeLDPC ended.");
    
    // ?
    // find the start flag of metadata
    //printf(" DEBUG INFO: metadata_offset find started.\n");
    Int32 metadata_offset = Pn - 1;
    while(raw_data->data[metadata_offset] == 0) {
        metadata_offset--;
    }
    //printf(" DEBUG INFO: metadata_offset find ended.\n");
    // skip the flag bit
    metadata_offset--;
    // set docked positions in host metadata
    //printf(" DEBUG INFO: set docked_position started.\n");
    symbol->metadata.docked_position = 0;
    for(Int32 i=0; i<4; i++) {
        if(type == 1) {    //if host is a slave symbol
            if(i == symbol->host_position) continue; //skip host position
        }
        symbol->metadata.docked_position += raw_data->data[metadata_offset--] << (3 - i);
    }
    //printf(" DEBUG INFO: set docked_position ended.\n");
    
    // 最后把解码后的数据拷贝到symbol中。 / copy the decoded data to symbol
    Int32 net_data_length = metadata_offset + 1;	// 
    
	//printf(" DEBUG INFO: copy symbol_data started.\n");
	symbol->data = (J_Data *)malloc(sizeof(J_Data) + net_data_length * sizeof(Char));
    if(symbol->data == NULL) {
        J_REPORT_ERROR(("Memory allocation for symbol data failed"))
        free(raw_data);
        return FATAL_ERROR;
    }
    symbol->data->length = net_data_length;
    memcpy(symbol->data->data, raw_data->data, net_data_length);
    //printf(" DEBUG INFO: copy symbol_data ended.\n");
    
    // 削除raw data暂存
    //printf(" DEBUG INFO: raw_data freeing started.\n");
    free(raw_data);
    //printf(" DEBUG INFO: raw_data freeing ended.\n");
    return SUCCESS;
}



// 对色盘各色值进行正常化
void normalizeColourPalette(J_Decoded_Symbol* symbol, Float* norm_palette, Int32 colour_number) {
    for (Int32 i = 0; i < (colour_number * COLOR_PALETTE_NUMBER); i++) {
        Float rgb_max = MAX(symbol->palette[i*3 + 0], MAX(symbol->palette[i*3 + 1], symbol->palette[i*3 + 2]));
        norm_palette[i*4 + 0] = (Float)symbol->palette[i*3 + 0] / rgb_max;
        norm_palette[i*4 + 1] = (Float)symbol->palette[i*3 + 1] / rgb_max;
        norm_palette[i*4 + 2] = (Float)symbol->palette[i*3 + 2] / rgb_max;
        norm_palette[i*4 + 3] = ((symbol->palette[i*3 + 0] + symbol->palette[i*3 + 1] + symbol->palette[i*3 + 2]) / 3.0f) / 255.0f; ;
    }
}

/**
 * @brief 解码一个（取样出来的）Master Symbol（总纲）
 * @param matrix sample过后的master symbol
 * @param symbol 返回解码出来的Symbol对象应存放的位置
 * @return SUCCESS | FAILURE | FATAL_ERROR
 */
Int32 decodeSampledMaster(Bitmap* matrix, J_Decoded_Symbol* symbol, int expected_pack_no) {
    if(matrix == NULL) {
        J_REPORT_ERROR(("Invalid master symbol matrix"))
        return FATAL_ERROR;
    }
    
    // 建立data map，其值一一对应Symbol上的每一个像素
    BYTE* data_map = (BYTE*)calloc(1, matrix->width * matrix->height * sizeof(BYTE));
    if(data_map == NULL) {
        J_REPORT_ERROR(("Memory allocation for data map in master failed"))
        return FATAL_ERROR;
    }
    
    // 解码metadata及获得（本图像的）基准palette
    // 根据官方文档，metadata的位置在(6,1)
    Int32 x = MASTER_METADATA_X;    // 6
    Int32 y = MASTER_METADATA_Y;    // 1
    Int32 module_count = 0;
    
    // ---解码metadata Part I (Nc：解码出module的colour mode的编号)---
    Int32 decode_partI_ret = decodeMasterMetadataPartI(matrix, symbol, data_map, &module_count, &x, &y);
    if (decode_partI_ret == FAILURE) {                  // LDPC 解码错误
        free(data_map);
		return FAILURE;
    }
    if (decode_partI_ret == DECODE_METADATA_FAILED) {   // 颜色模式识别错误
        // 刷新启始位置，重新识别
        x = MASTER_METADATA_X;
        y = MASTER_METADATA_Y;
        module_count = 0;
        // 清除刚识别出的 data_map
        memset(data_map, 0, matrix->width * matrix->height * sizeof(BYTE));
        // 为symbol加载默认metadata系统设置，使用默认设置直接进行读取
        loadDefaultMasterMetadata(matrix, symbol);
    }
    
    
    // ------ 下面尝试读取色盘 / read colour palettes ------
    if (readColourPaletteInMaster(matrix, symbol, data_map, &module_count, &x, &y) < 0) {
        J_REPORT_ERROR(("Reading colour palettes in master symbol failed"))
        free(data_map);
        return FAILURE;
    }
    
    // 对刚读出的colour palettes进行正常化 / normalize the RGB values in colour palettes
    Int32 colour_number = (Int32)pow(2, symbol->metadata.Nc + 1); // 2^Nc 种颜色
    // 每种色块都有4个被正常化的值 / each colour contains 4 normalized values, i.e. R, G, B and Luminance
    Float *norm_palette = (Float *)malloc(sizeof(Float) * colour_number * 4 * COLOR_PALETTE_NUMBER); // 整幅图共有4个色盘
    normalizeColourPalette(symbol, norm_palette, colour_number);
    
    // 拿到palette RGB的阈（颜色容差的最大值及最小值） / get the palette RGB thresholds
    Float pal_ths[3 * COLOR_PALETTE_NUMBER];
    for(Int32 i = 0; i < COLOR_PALETTE_NUMBER; i++) {
        getPaletteThreshold(symbol->palette + (colour_number * 3) * i, colour_number, &pal_ths[i * 3]);
    }
    
    // ----解码第二部分（码块大小等）----
    if(decode_partI_ret == SUCCESS) {
        if(decodeMasterMetadataPartII(matrix, symbol, data_map, norm_palette, pal_ths, &module_count, &x, &y) <= 0) {
            free(data_map);
			free(norm_palette);
			return FAILURE;
        }
    }
    
    // ----解码所有数据----
    // printf(" DEBUG INFO: decodeSymbol started. expected_pack_no is %d.\n", expected_pack_no);
    int ret = decodeSymbol(matrix, symbol, data_map, norm_palette, pal_ths, 0, expected_pack_no);
    //printf(" DEBUG INFO: decodeSymbol ended. freeing data_map started\n", expected_pack_no);
    // free(data_map);
    //printf(" DEBUG INFO: freeing data_map ended\n");
	free(norm_palette);
	return ret;
}



/**
 * @brief 读取比特流数据 / Read bit data
 * @param data 数据缓冲区
 * @param start 从这个偏移量开始阅读 / the start reading offset
 * @param length 要阅读的长度 / the length of the data to be read
 * @param value 阅读出来的数据 / the read data
 * @return 阅读出来的数据的长度
 */
Int32 readData(J_Data* data, Int32 start, Int32 length, Int32* value) {
    Int32 i;
    Int32 val = 0;
    for(i = start; i < (start + length) && i < data->length; i++) {
        val += data->data[i] << (length - 1 - (i - start));
    }
    *value = val;
    return (i - start);
}

/**
 * @brief 把解析出来的bit stream译码成需要的形式
 * @param bits 输入bit
 * @return 数据
 */
J_Data* decodeData(J_Data* bits) {
    BYTE* decoded_bytes = (BYTE *)malloc(bits->length * sizeof(BYTE));
    if(decoded_bytes == NULL) {
        J_REPORT_ERROR(("Memory allocation for decoded bytes failed"))
        return NULL;
    }
    
    j_encode_mode mode = Upper;   // 跟你们官方文档不一样阿？
    j_encode_mode pre_mode = None;
    Int32 index = 0;    // index of input bits
    Int32 count = 0;    // index of decoded bytes
    
    // 译码，直到index已经到达末尾
    while(index < bits->length) {
        // 读取 read the encoded value
        Boolean flag = 0;
        Int32 value = 0;
        Int32 n;
        if (mode != Byte) {     // 对于非byte读取模式
            n = readData(bits, index, character_size[mode], &value);
            if(n < character_size[mode])    // did not read enough bits
                break;
            //update index
            index += character_size[mode];
        }
        
        // 译码。请注意：每批数据先从Upper类型开始探测，如果读出的第一个character是模式变换字符（参考表12）,就切换到相应模式检测接下来的数据块。
        switch(mode) {
            case Upper:
                if(value <= 26) {
                    decoded_bytes[count++] = j_decoding_table_upper[value];
                    if(pre_mode != None)
                        mode = pre_mode;
                }
                else {
                    switch(value) {
                        case 27: // 标点模式
                            mode = Punct;
                            pre_mode = Upper;
                            break;
                        case 28: // 小写字母模式
                            mode = Lower;
                            pre_mode = None;
                            break;
                        case 29: // 数字模式
                            mode = Numeric;
                            pre_mode = None;
                            break;
                        case 30:
                            mode = Alphanumeric;
                            pre_mode = None;
                            break;
                        case 31: // 可能有多种可能，再读两个bit先
                            n = readData(bits, index, 2, &value);
                            if(n < 2) {    //did not read enough bits
                                flag = 1;
                                break;
                            }
                            // update index
                            index += 2;
                            switch(value) {
                                case 0:
                                    mode = Byte;
                                    pre_mode = Upper;
                                    break;
                                case 1:
                                    mode = Mixed;
                                    pre_mode = Upper;
                                    break;
                                case 2:
                                    mode = ECI;
                                    pre_mode = None;
                                    break;
                                case 3:
                                    flag = 1;        // ［end of message (EOM)］和官方文档有出入？？？
                                    break;
                            }
                            break;
                        default:
                        	J_REPORT_ERROR(("Invalid value decoded"))
                            free(decoded_bytes);
                            return NULL;
                    }
                }
                break;
            case Lower:     // 小写模式
                if(value <= 26) {
                    decoded_bytes[count++] = j_decoding_table_lower[value];
                    if(pre_mode != None)
                        mode = pre_mode;
                }
                else {
                    switch(value) {
                        case 27:
                            mode = Punct;
                            pre_mode = Lower;
                            break;
                        case 28:
                            mode = Upper;
                            pre_mode = Lower;
                            break;
                        case 29:
                            mode = Numeric;
                            pre_mode = None;
                            break;
                        case 30:
                            mode = Alphanumeric;
                            pre_mode = None;
                            break;
                        case 31:
                            //read 2 bits more
                            n = readData(bits, index, 2, &value);
                            if(n < 2) {    //did not read enough bits
                                
                                flag = 1;
                                break;
                            }
                            //update index
                            index += 2;
                            switch(value) {
                                case 0:
                                    mode = Byte;
                                    pre_mode = Lower;
                                    break;
                                case 1:
                                    mode = Mixed;
                                    pre_mode = Lower;
                                    break;
                                case 2:
                                    mode = Upper;
                                    pre_mode = None;
                                    break;
                                case 3:
                                    mode = FNC1;
                                    pre_mode = None;
                                    break;
                            }
                            break;
                        default:
                        	J_REPORT_ERROR(("Invalid value decoded"))
                            free(decoded_bytes);
                            return NULL;
                    }
                }
                break;
            case Numeric:
                if(value <= 12)
                {
                    decoded_bytes[count++] = j_decoding_table_numeric[value];
                    if(pre_mode != None)
                        mode = pre_mode;
                }
                else
                {
                    switch(value)
                    {
                        case 13:
                            mode = Punct;
                            pre_mode = Numeric;
                            break;
                        case 14:
                            mode = Upper;
                            pre_mode = None;
                            break;
                        case 15:
                            //read 2 bits more
                            n = readData(bits, index, 2, &value);
                            if(n < 2)    //did not read enough bits
                            {
                                flag = 1;
                                break;
                            }
                            //update index
                            index += 2;
                            switch(value)
                        {
                            case 0:
                                mode = Byte;
                                pre_mode = Numeric;
                                break;
                            case 1:
                                mode = Mixed;
                                pre_mode = Numeric;
                                break;
                            case 2:
                                mode = Upper;
                                pre_mode = Numeric;
                                break;
                            case 3:
                                mode = Lower;
                                pre_mode = None;
                                break;
                        }
                            break;
                        default:
                        	J_REPORT_ERROR(("Invalid value decoded"))
                            free(decoded_bytes);
                            return NULL;
                    }
                }
                break;
            case Punct:
                if(value >=0 && value <= 15)
                {
                    decoded_bytes[count++] = j_decoding_table_punct[value];
                    mode = pre_mode;
                }
                else
                {
                    J_REPORT_ERROR(("Invalid value decoded"))
                    free(decoded_bytes);
                    return NULL;
                }
                break;
            case Mixed:
                if(value >=0 && value <= 31)
                {
                    if(value == 19)
                    {
                        decoded_bytes[count++] = 10;
                        decoded_bytes[count++] = 13;
                    }
                    else if(value == 20)
                    {
                        decoded_bytes[count++] = 44;
                        decoded_bytes[count++] = 32;
                    }
                    else if(value == 21)
                    {
                        decoded_bytes[count++] = 46;
                        decoded_bytes[count++] = 32;
                    }
                    else if(value == 22)
                    {
                        decoded_bytes[count++] = 58;
                        decoded_bytes[count++] = 32;
                    }
                    else
                    {
                        decoded_bytes[count++] = j_decoding_table_mixed[value];
                    }
                    mode = pre_mode;
                }
                else
                {
                    J_REPORT_ERROR(("Invalid value decoded"))
                    free(decoded_bytes);
                    return NULL;
                }
                break;
            case Alphanumeric:
                if(value <= 62)
                {
                    decoded_bytes[count++] = j_decoding_table_alphanumeric[value];
                    if(pre_mode != None)
                        mode = pre_mode;
                }
                else if(value == 63)
                {
                    //read 2 bits more
                    n = readData(bits, index, 2, &value);
                    if(n < 2)    //did not read enough bits
                    {
                        flag = 1;
                        break;
                    }
                    //update index
                    index += 2;
                    switch(value)
                    {
                        case 0:
                            mode = Byte;
                            pre_mode = Alphanumeric;
                            break;
                        case 1:
                            mode = Mixed;
                            pre_mode = Alphanumeric;
                            break;
                        case 2:
                            mode = Punct;
                            pre_mode = Alphanumeric;
                            break;
                        case 3:
                            mode = Upper;
                            pre_mode = None;
                            break;
                    }
                }
                else
                {
                    J_REPORT_ERROR(("Invalid value decoded"))
                    free(decoded_bytes);
                    return NULL;
                }
                break;
            case Byte: {
                
                // 读取4个比特进入value / read 4 bits more
                n = readData(bits, index, 4, &value);
                if (n < 4) {        // 没有读取够足够的值 did not read enough bits
                    J_REPORT_ERROR(("Not enough bits to decode"))
                    free(decoded_bytes);
                    return NULL;
                }
                // update index
                index += 4;
                if(value == 0) {    // 后面包含的Byte大于15个
                    // 再读取13个比特进入value / read 13 bits more
                    n = readData(bits, index, 13, &value);
                    if(n < 13) {    //did not read enough bits
                        J_REPORT_ERROR(("Not enough bits to decode"))
                        free(decoded_bytes);
                        return NULL;
                    }
                    // 为了尽可能节省一切byte位，而且这13位将描述大于15的数值，所以编码时-15来编码，
                    // 以期能让这批数字能从0开始。读取时应该［加回16］恢复。
                    value += 15 + 1;    // the number of encoded bytes = value + 15
                    // update index
                    index += 13;
                }
                Int32 byte_length = value;
                // 读取接下来的一批bit，8位8位地读，读够value描述的长度为止 / read the next (byte_length * 8) bits
                for(Int32 i = 0; i < byte_length; i++) {
                    n = readData(bits, index, 8, &value);
                    if(n < 8) {    //did not read enough bits
                        J_REPORT_ERROR(("Not enough bits to decode"))
                        free(decoded_bytes);
                        return NULL;
                    }
                    // update index
                    index += 8;
                    // 保存读取出的二进制数位
                    decoded_bytes[count++] = (BYTE)value;
                }
                mode = pre_mode;    // 读取完毕，恢复pre_mode, which is zero
                break;
            }
            case ECI:
                //TODO: not implemented
                index += bits->length;
                break;
            case FNC1:
                //TODO: not implemented
                index += bits->length;
                break;
            case None:
                J_REPORT_ERROR(("Decoding mode is None."))
                index += bits->length;
                break;
        }
        if (flag) break;
    }
    
    // 搬运暂存中翻译后的bit到标准的输出结构体中 / copy decoded data
    J_Data* decoded_data = (J_Data *)malloc(sizeof(J_Data) + count * sizeof(BYTE));
    if(decoded_data == NULL) {
        free(decoded_bytes);
        J_REPORT_ERROR(("Memory allocation for decoded data failed"))
        return NULL;
    }
    decoded_data->length = count;
    memcpy(decoded_data->data, decoded_bytes, count);
    
    // 削除暂存
    free(decoded_bytes);
    return decoded_data;
}



// 拧为啥要用marix？？？？？？？？？？
// 现在声明：参数expected_pack_no表明期待的包数，若解出包数不符就直接停止解码并扔掉。
// 如果解码包数相符，就接受，并且扔掉包数字符，长度-1
Int32 decodeMaster(J_Found_Symbol* found_ms, J_Decoded_Symbol* decoded_master_symbol, int expected_pack_no) {
	// printf(" DEBUG INFO: decode-master started.\n");
    if (!found_ms) {
        return FAILURE;
    } else {
    	// printf(" DEBUG INFO: decode-master started.\n");
    	// HANTEST
    	// saveImage(found_ms->sampled_img, "found_symbol.png");
        // 尝试解码刚刚侦测及采样到的 matrix 的【采样后数据】，根据记录的master symbol的信息
        // (9.3.2020补充：您不handle返回的包么？？？)
        decoded_master_symbol->side_size = found_ms->side_size;
        decoded_master_symbol->index = found_ms->index;
        decoded_master_symbol->host_index = found_ms->host_index;
        decoded_master_symbol->module_size = found_ms->module_size;
		decoded_master_symbol->pattern_positions[0] = found_ms->fps[0].center;	// 20/3/2020
        decoded_master_symbol->pattern_positions[1] = found_ms->fps[1].center;
        decoded_master_symbol->pattern_positions[2] = found_ms->fps[2].center;
        decoded_master_symbol->pattern_positions[3] = found_ms->fps[3].center;
		Int32 decode_result = decodeSampledMaster(found_ms->sampled_img, decoded_master_symbol, expected_pack_no); // 解码结果
        if(decode_result == SUCCESS) {
            return SUCCESS;
        } else if(decode_result < 0) { // fatal error：不可饶恕的错误 occurred
			return FAILURE;
        }
        // 若只依靠 finder patterns 不可解码, 尝试介入 alignment patterns
// #ifdef ALIGNMENT_SYMBOL
        else {
#if TEST_MODE
            J_REPORT_INFO(("Trying to sample master symbol using alignment pattern..."))
#endif // TEST_MODE
            // 由side version计算Size
            decoded_master_symbol->side_size.x = VERSION2SIZE(decoded_master_symbol->metadata.side_version.x);
            decoded_master_symbol->side_size.y = VERSION2SIZE(decoded_master_symbol->metadata.side_version.y);
            // 根据Alignment Pattern进行采样
            Bitmap* matrix = sampleSymbolByAlignmentPattern(found_ms->bitmap, found_ms->ch, decoded_master_symbol, found_ms->fps);
            // HAN TEST
            if(matrix == NULL) {
            	//printf(" DEBUG INFO: decode-master end.\n");
#if TEST_MODE
                J_REPORT_ERROR(("Sampling master symbol using alignment pattern failed"))
#endif // TEST_MODE
                return FAILURE;
            }
            // 现在尝试解码
            decode_result = decodeSampledMaster(matrix, decoded_master_symbol, expected_pack_no);
            free(matrix);
            //printf(" DEBUG INFO: decode-master end.\n");
            if(decode_result == SUCCESS)
                return SUCCESS;
			else
                return FAILURE;
        }
    }
}




// 汇总及用户交互层



/**
 透过一个J_Found_Symbol解码
 
 @param found_ms J_Found_Symbol
 @param mode NORMAL_DECODE: only output completely decoded data when all symbols are correctly decoded | COMPATIBLE_DECODE: also output partly decoded data even if some symbols are not correctly decoded
 @param status 返回解码后标志位——0: not detectable, 1: not decodable, 2: partly decoded with COMPATIBLE_DECODE mode, 3: fully decoded, SKIP: skipped, FORCESTOP: force stop, LOST: lost pack detected.
 @param symbols 解码后的symbol（decoded_symbol类型数据）
 @return 该码块数据流
 */
J_Data* decodeJCodeEx_using_found_symbol(J_Found_Symbol* found_ms,
                                         Int32 mode,
                                         Int32* status,
                                         J_Decoded_Symbol* symbols,
										 int expected_pack_no) {
    if(status) *status = 0;
    if(!symbols) {
        J_REPORT_ERROR(("Invalid symbol buffer"))
        return NULL;
    }
    memset(symbols, 0, 1 * sizeof(J_Decoded_Symbol));
    
    Int32 total = 0;    // 已解出码的element数（应为1）
    Boolean res = 1;    // 1：目前并无错误需要纠正；0：目前有错误需要纠正
    if (!found_ms) {
        return FAILURE;
    }
    // ---探测及解析Master区域---
	// Int32 decodeStatus = ;
	// clean_foundSymbol(found_ms);
	// printf(" DEBUG_INFO: found_symbol got.\n");
    if (decodeMaster(found_ms, symbols, expected_pack_no) == SUCCESS) {
        total++;
    } 
    // printf(" DEBUG_INFO: master found and decoded.\n");
    // deleted slaves
    
    // ---检查解码的结果---
    // 1. 没解出码，或要求精准解码的时候有错误探测到(res == 0)
    if(total == 0 || (mode == NORMAL_DECODE && res == 0 )) {
        if (symbols[0].module_size > 0 && status)
            *status = 1; // 解码错误
        // --削除暂存--
        // 削除整幅图用于存储palette及data的数组（可能没用完,比如解码到一半发现解不出来，所以下面i = total但是仍然需要free是有可能的）
        free(symbols[0].palette);
        free(symbols[0].data);
        return NULL;
    }
    // 2. 不要求精准解码的时候有错误探测到(res == 0)
    if(mode == COMPATIBLE_DECODE && res == 0) {
        if(status) *status = 2;
        res = 1;
    } // ---检查解码的结果结束---
    
    // ---连接解码结果---
    // 计算所有symbol的data的总长度
    Int32 total_data_length = symbols[0].data->length;
    // 分配一个存放解码结果的区域（一个可变结构体）
	J_Data* decoded_bits = symbols[0].data;
	
    
    // ---根据比特流译码---
    J_Data* decoded_data = decodeData(decoded_bits);
    if(decoded_data == NULL) {
        J_REPORT_ERROR(("Decoding data failed"))
        free(decoded_bits);
        if(status) *status = 1;
        res = 0;
    }
    
    // You could only detect first byte here!
    int* decoded_frame_no = (int *)decoded_data->data;
	if (decoded_data && expected_pack_no > *decoded_frame_no) {
    	J_REPORT_INFO(("Detected symbol %d while symbol %d is needed. Skip.", *decoded_frame_no, expected_pack_no))
    	if (status)
            *status = SKIP; // 跳过解码
        // ---削除暂存---
		// 削除整幅图用于存储palette及data的数据
		free(symbols[0].palette);
		free(decoded_data);
		free(decoded_bits);
		return NULL;
	} else if (decoded_data && expected_pack_no < *decoded_frame_no){
		J_REPORT_ERROR(("Packet loss of symbol %d! Detected symbol %d. Ignoring.", expected_pack_no, *decoded_frame_no));
		if (status)
            *status = LOST; // 跳过解码
	}
    
    // ---削除暂存---
    // 削除整幅图用于存储palette及data的数组
    free(symbols[0].palette);
    // 削除比特流，因为只需输出decoded_data
    free(decoded_bits);
#if TEST_MODE
    free(test_mode_bitmap);
#endif // TEST_MODE
    if(res == 0) return NULL;    // 如果有错误发生
    if(status) {
        if(*status != LOST && *status != 2)        // 2: PARTLY_DECODED
            *status = 3;        // 3: FULLY_DECODED
    }
    return decoded_data;
}



/**
 图片文件解码（经已弃用）
 
 @param src 图片文件位置
 @param mode NORMAL_DECODE: only output completely decoded data when all symbols are correctly decoded | COMPATIBLE_DECODE: also output partly decoded data even if some symbols are not correctly decoded
 @param status 返回解码后标志位——0: not detectable, 1: not decodable, 2: partly decoded with COMPATIBLE_DECODE mode, 3: fully decoded
 @param symbols 解码后的symbol（decoded_symbol类型数据）
 @return 该码块数据流
 */
J_Data* decodeJCodeEx(const char src[], Int32 mode, Int32* status, J_Decoded_Symbol* symbols, BYTE expected_pack_no) {
    if(status) *status = 0;
    if(!symbols) {
        J_REPORT_ERROR(("Invalid symbol buffer"))
        return NULL;
    }
    // 8/3/2020 及 9/3/2020改进
    J_Found_Symbol* found_ms = importImage(src);
    J_Data* ret = decodeJCodeEx_using_found_symbol(found_ms, mode, status, symbols, expected_pack_no);
    clean_foundSymbol(found_ms);
    return ret;
}



/**
 直接透过【图片文件】解码（封装）
 
 @param src 原图所在的（文件）位置
 @param mode NORMAL_DECODE: only output completely decoded data when all symbols are correctly decoded | COMPATIBLE_DECODE: also output partly decoded data even if some symbols are not correctly decoded
 @param status 解码标志位 (0: not detectable, 1: not decodable, 2: partly decoded with COMPATIBLE_DECODE mode, 3: fully decoded, 255: skip code)
 @return 该码块数据流 | NULL if failed
 */
J_Data* decodeJCode(const char src[], Int32 mode, Int32* status, BYTE expected_pack_no) {
    J_Decoded_Symbol symbols[MAX_SYMBOL_NUMBER];
    return decodeJCodeEx(src, mode, status, symbols, expected_pack_no);
}
