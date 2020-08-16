// sample.c    03/03/2020
// J_Code
// brief: J_Code symbol sampling (C)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jc.h"
#include "../dll.h"		// 19/3/2020
#include "detector.h"
#include "sample.h"     // transform then sample
#include "encoder.h"

#define SAMPLE_AREA_WIDTH	(CROSS_AREA_WIDTH / 2 - 2) //width of the columns where the metadata and palette in slave symbol are located
#define SAMPLE_AREA_HEIGHT	20	//height of the metadata rows including the first row, though it does not contain metadata

/**
 * @brief 对一个（已经进行透视变换的）symbol进行采样
 * @param bitmap 原图（已经过色彩增强）的bitmap
 * @param pt 透视变换矩阵（the transformation matrix）
 * @param side_size 用module数量描绘的symbol长宽（the symbol size in module）
 * @return 返回进行采样后的位图（矩阵格式的bitmap）
 */
Bitmap* sampleSymbol(Bitmap* bitmap, J_Perspective_Transform* pt, Vector2D side_size)
{
    // 每个像素 用 4个Byte的数据（4个channel） 来记载
	Int32 mtx_bytes_per_pixel = bitmap->bits_per_pixel / 8; // 32 / 8 = 4
    // 每行所用Byte数
	Int32 mtx_bytes_per_row = side_size.x * mtx_bytes_per_pixel;
    // 分配一块空间，存放采样后的位图
	Bitmap* matrix = (Bitmap*)malloc(sizeof(Bitmap) + side_size.x * side_size.y * mtx_bytes_per_pixel * sizeof(BYTE));
	if(matrix == NULL) {
		J_REPORT_ERROR(("Memory allocation for symbol bitmap matrix failed"))
		return NULL;
	}
	matrix->channel_count = bitmap->channel_count;
	matrix->bits_per_channel = bitmap->bits_per_channel;
	matrix->bits_per_pixel = matrix->bits_per_channel * matrix->channel_count;  // 8 bits * 4个channel = 32
	matrix->width = side_size.x;    // ?
	matrix->height= side_size.y;
    
    // 常量定义：Bitmapzh中的每Pixel所占的Byte数
	Int32 bmp_bytes_per_pixel = bitmap->bits_per_pixel / 8;
    // 常量定义：Bitmapzh中的每行所占据的Byte数，方便算偏移量
	Int32 bmp_bytes_per_row = bitmap->width * bmp_bytes_per_pixel;
    // 相当于symbol宽的个数的一组点，准备从上往下扫描用
	// J_POINT points[side_size.x];	移植至vs：丢弃了动态数组
	J_POINT *points = (J_POINT *)malloc(sizeof(J_POINT) * side_size.x);
    for(Int32 i = 0; i < side_size.y; i++) {        // 逐行扫描
		// 将采样位定义至每个“像素”的中心位置
        for(Int32 j = 0; j < side_size.x; j++) {
			points[j].x = (Float)j + 0.5f;
			points[j].y = (Float)i + 0.5f;
		}
        // transform.c中定义的：透过透视矩阵计算：一组点分别从采样图上映射到原图上的对应位置
		warpPoints(pt, points, side_size.x);
        // 每个采样位
		for(Int32 j = 0; j < side_size.x; j++) {
            // 定位到的该采样点位于原图上的位置x
			Int32 mapped_x = (Int32)points[j].x;
            // 定位到的该采样点位于原图上的位置y
			Int32 mapped_y = (Int32)points[j].y;
			if (mapped_x < 0 || mapped_x > bitmap->width - 1) {
                // 前两个：只是因为取整原因，不小心变形到图外去
				if (mapped_x == -1) mapped_x = 0;
				else if (mapped_x ==  bitmap->width) mapped_x = bitmap->width - 1;
                else {
                	J_REPORT_ERROR(("Perspective transform wrapping points on X direction failed."))
                    free(matrix);   // Han Add
					free(points);
                    return NULL;  // 计算偏颇过大
                }
			}
			if(mapped_y < 0 || mapped_y > bitmap->height-1) {
                // 前两个：只是因为取整原因，不小心变形到图外去
				if(mapped_y == -1) mapped_y = 0;
				else if(mapped_y ==  bitmap->height) mapped_y = bitmap->height - 1;
                else {
                	J_REPORT_ERROR(("Perspective transform wrapping points on Y direction failed."))
                    free(matrix);
					free(points);
                    return NULL;  // 计算偏颇过大
                }
			}
            // ---开始采样---
            // 对每个channel分别采样！
			for(Int32 c = 0; c < matrix->channel_count; c++) {
                // 根据以该点像素为中心、方圆3x3正方形大小的范围内的临近像素的色彩平均值，作为采样值
				Float sum = 0;
				for(Int32 dx = -1; dx <= 1; dx++) {
					for(Int32 dy = -1; dy <= 1; dy++) {
						Int32 px = mapped_x + dx;
						Int32 py = mapped_y + dy;
                        // 如果该取样像素正正好好就在原图的4个corner：（=_=哪有这么好的事），缩小取样范围即可，不用移动取样范围，节省脑子
						if(px < 0 || px > bitmap->width - 1)  px = mapped_x;
						if(py < 0 || py > bitmap->height - 1) py = mapped_y;
                        // +c是因为现在是按channel分开采样的
						sum += bitmap->pixel[py * bmp_bytes_per_row + px * bmp_bytes_per_pixel + c];
					}
				}
                // 共取样了9个点呢
				BYTE ave = (BYTE)(sum / 9.0f + 0.5f);
                // 将采样数据填入采样图中的对应位置
				matrix->pixel[i*mtx_bytes_per_row + j*mtx_bytes_per_pixel + c] = ave;
				// 错误示范：直接采样而不采取区域取平均的方式，容易在偶然出现的像素数据错误时出错（虽然这是不太可能的，因为我已经对图像预处理过了！！！！！）matrix->pixel[i*mtx_bytes_per_row + j*mtx_bytes_per_pixel + c] = bitmap->pixel[mapped_y*bmp_bytes_per_row + mapped_x*bmp_bytes_per_pixel + c];
#if TEST_MODE
				test_mode_bitmap->pixel[mapped_y*bmp_bytes_per_row + mapped_x*bmp_bytes_per_pixel + c] = test_mode_color;
				if(c == 3 && test_mode_color == 0)
                    test_mode_bitmap->pixel[mapped_y*bmp_bytes_per_row + mapped_x*bmp_bytes_per_pixel + c] = 255;
#endif
			}
		}
    }
    // 取样完毕
	free(points);
	return matrix;
}



/**
 * @brief 用alignment pattern来取样一个symbol
 * @param bitmap 原图Bitmap
 * @param ch 图像的三通道Bitmap
 * @param symbol 放symbol的相关信息
 * @param fps the finder patterns
 * @return the sampled symbol matrix | NULL if failed
*/
Bitmap* sampleSymbolByAlignmentPattern(Bitmap* bitmap, Bitmap* ch[], J_Decoded_Symbol* symbol, J_Finder_Pattern* fps) {
	// 如果不可能含有alignment pattern，就停止
    if(symbol->metadata.side_version.x < 6 && symbol->metadata.side_version.y < 6) {
		J_REPORT_INFO(("No AP in this frame is available. Skip."))
		return NULL;
	}

	// 一般要先确认长宽 / For default mode, first confirm the symbol side size
	if(symbol->metadata.default_mode) {
        // confirmSymbolSize是detector库函数
        if(!confirmSymbolSize(ch, fps, symbol)) {
            J_REPORT_INFO(("The symbol size can not be recognized."))
            return NULL;
        }
#if TEST_MODE
        J_REPORT_INFO(("Side sizes confirmed by APs: %d %d", symbol->side_size.x, symbol->side_size.y))
#endif
    }

    Int32 side_ver_x_index = symbol->metadata.side_version.x - 1;
	Int32 side_ver_y_index = symbol->metadata.side_version.y - 1;
	Int32 number_of_ap_x = j_ap_num[side_ver_x_index];
    Int32 number_of_ap_y = j_ap_num[side_ver_y_index];

    // 用来存储所有可能的ap的缓冲区
	J_Alignment_Pattern* aps = (J_Alignment_Pattern *)malloc(number_of_ap_x * number_of_ap_y *sizeof(J_Alignment_Pattern));
	if(aps == NULL) {
		J_REPORT_ERROR(("Memory allocation for alignment patterns failed"))
		return NULL;
	}
    // 探测所有AP
	for(Int32 i = 0; i < number_of_ap_y; i++) {
		for(Int32 j = 0; j < number_of_ap_x; j++) {
			Int32 index = i * number_of_ap_x + j;
			if (i == 0 && j == 0)
				aps[index] = fps[0];
			else if (i == 0 && j == number_of_ap_x - 1)
				aps[index] = fps[1];
			else if (i == number_of_ap_y - 1 && j == number_of_ap_x - 1)
				aps[index] = fps[2];
			else if (i == number_of_ap_y - 1 && j == 0)
				aps[index] = fps[3];
			else {
				if(i == 0) {
					// direction: from aps[0][j-1] to fps[1]
					Float distx = fps[1].center.x - aps[j-1].center.x;
					Float disty = fps[1].center.y - aps[j-1].center.y;
					Float alpha = atan2(disty, distx);
					// distance:  aps[0][j-1].module_size * module_number_between_APs
					Float distance = aps[j-1].module_size * (j_ap_pos[side_ver_x_index][j] - j_ap_pos[side_ver_x_index][j-1]);
					// calculate the coordinate of ap[index]
					aps[index].center.x = aps[j-1].center.x + distance * cos(alpha);
					aps[index].center.y = aps[j-1].center.y + distance * sin(alpha);
					aps[index].module_size = aps[j-1].module_size;
				}
				else if(j == 0) {
					//direction: from aps[i-1][0] to fps[3]
					Float distx = fps[3].center.x - aps[(i-1) * number_of_ap_x].center.x;
					Float disty = fps[3].center.y - aps[(i-1) * number_of_ap_x].center.y;
					Float alpha = atan2(disty, distx);
					//distance:  aps[i-1][0].module_size * module_number_between_APs
					Float distance = aps[(i-1) * number_of_ap_x].module_size * (j_ap_pos[side_ver_y_index][i] - j_ap_pos[side_ver_y_index][i-1]);
					//calculate the coordinate of ap[index]
					aps[index].center.x = aps[(i-1) * number_of_ap_x].center.x + distance * cos(alpha);
					aps[index].center.y = aps[(i-1) * number_of_ap_x].center.y + distance * sin(alpha);
					aps[index].module_size = aps[(i-1) * number_of_ap_x].module_size;
				}
				else {
					//estimate the coordinate of ap[index] from aps[i-1][j-1], aps[i-1][j] and aps[i][j-1]
					Int32 index_ap0 = (i-1) * number_of_ap_x + (j-1);	//ap at upper-left
					Int32 index_ap1 = (i-1) * number_of_ap_x + j;		//ap at upper-right
					Int32 index_ap3 = i * number_of_ap_x + (j-1);		//ap at left
					Float ave_size_ap01 = (aps[index_ap0].module_size + aps[index_ap1].module_size) / 2.0f;
					Float ave_size_ap13 = (aps[index_ap1].module_size + aps[index_ap3].module_size) / 2.0f;
					aps[index].center.x = (aps[index_ap1].center.x - aps[index_ap0].center.x) / ave_size_ap01 * ave_size_ap13 + aps[index_ap3].center.x;
					aps[index].center.y = (aps[index_ap1].center.y - aps[index_ap0].center.y) / ave_size_ap01 * ave_size_ap13 + aps[index_ap3].center.y;
					aps[index].module_size = ave_size_ap13;
				}
				//find aps[index]
				aps[index].found_count = 0;
				J_Alignment_Pattern tmp = aps[index];
				aps[index] = findAlignmentPattern(ch, aps[index].center.x, aps[index].center.y, aps[index].module_size, APX);
				if(aps[index].found_count == 0) {
					aps[index] = tmp;	//recover the estimated one
#if TEST_MODE
					J_REPORT_ERROR(("The alignment pattern (index: %d) at (X: %f, Y: %f) not found", index, aps[index].center.x, aps[index].center.y))
#endif
				}
			}
		}
	}

#if TEST_MODE
	drawFoundFinderPatterns((jab_finder_pattern*)aps, number_of_ap_x * number_of_ap_y, 0x0000ff);
#endif

	// 对每个块去确定最小的采样方块 / determine the minimal sampling rectangle for each block
	Int32 block_number = (number_of_ap_x-1) * (number_of_ap_y-1);
	Vector2D *rect = (Vector2D *)malloc(sizeof(Vector2D) * block_number * 2);
	Int32 rect_index = 0;
	for (Int32 i = 0; i < number_of_ap_y-1; i++) {
		for (Int32 j = 0; j < number_of_ap_x-1; j++) {
			Vector2D tl = {0,0};
			Vector2D br = {0,0};
			Boolean flag = 1;

			for(Int32 delta = 0; delta <= (number_of_ap_x-2 + number_of_ap_y-2) && flag; delta++) {
				for(Int32 dy = 0; dy <= MIN(delta, number_of_ap_y-2) && flag; dy++) {
					Int32 dx = MIN(delta - dy, number_of_ap_x-2);
					for(Int32 dy1 = 0; dy1 <= dy && flag; dy1++) {
						Int32 dy2 = dy - dy1;
						for(Int32 dx1 = 0; dx1 <= dx && flag; dx1++) {
							Int32 dx2 = dx - dx1;

							tl.x = MAX(j - dx1, 0);
							tl.y = MAX(i - dy1, 0);
							br.x = MIN(j + 1 + dx2, number_of_ap_x - 1);
							br.y = MIN(i + 1 + dy2, number_of_ap_y - 1);
							if(aps[tl.y*number_of_ap_x + tl.x].found_count > 0 &&
							   aps[tl.y*number_of_ap_x + br.x].found_count > 0 &&
							   aps[br.y*number_of_ap_x + tl.x].found_count > 0 &&
							   aps[br.y*number_of_ap_x + br.x].found_count > 0) {
								flag = 0;
							}
						}
					}
				}
			}
			// save the minimal rectangle if there is no duplicate
			Boolean dup_flag = 0;
			for(Int32 k=0; k<rect_index; k+=2) {
				if(rect[k].x == tl.x && rect[k].y == tl.y && rect[k+1].x == br.x && rect[k+1].y == br.y) {
					dup_flag = 1;
					break;
				}
			}
			if(!dup_flag) {
				rect[rect_index] = tl;
				rect[rect_index+1] = br;
				rect_index += 2;
			}
		}
	}
	//sort the rectangles in descending order according to the size
	for(Int32 i=0; i<rect_index-2; i+=2) {
		for(Int32 j=0; j<rect_index-2-i; j+=2) {
			Int32 size0 = (rect[j+1].x - rect[j].x) * (rect[j+1].y - rect[j].y);
			Int32 size1 = (rect[j+2+1].x - rect[j+2].x) * (rect[j+2+1].y - rect[j+2].y);
			if(size1 > size0) {
				Vector2D tmp;
				//swap top-left
				tmp = rect[j];
				rect[j] = rect[j+2];
				rect[j+2] = tmp;
				//swap bottom-right
				tmp = rect[j+1];
				rect[j+1] = rect[j+2+1];
				rect[j+2+1] = tmp;
			}
		}
	}

	//allocate the buffer for the sampled matrix of the symbol
    Int32 width = symbol->side_size.x;
	Int32 height= symbol->side_size.y;
	Int32 mtx_bytes_per_pixel = bitmap->bits_per_pixel / 8;
	Int32 mtx_bytes_per_row = width * mtx_bytes_per_pixel;
	Bitmap* matrix = (Bitmap*)malloc(sizeof(Bitmap) + width*height*mtx_bytes_per_pixel*sizeof(BYTE));
	if(matrix == NULL) {
        free(aps);
		free(rect);
		J_REPORT_ERROR(("Memory allocation for symbol bitmap matrix failed"))
		return NULL;
	}
	matrix->channel_count = bitmap->channel_count;
	matrix->bits_per_channel = bitmap->bits_per_channel;
	matrix->bits_per_pixel = matrix->bits_per_channel * matrix->channel_count;
	matrix->width = width;
	matrix->height= height;

	for(Int32 i=0; i<rect_index; i+=2) {
		Vector2D blk_size;
		J_POINT p0, p1, p2, p3;

		//middle blocks
		blk_size.x = j_ap_pos[side_ver_x_index][rect[i+1].x] - j_ap_pos[side_ver_x_index][rect[i].x] + 1;
		blk_size.y = j_ap_pos[side_ver_y_index][rect[i+1].y] - j_ap_pos[side_ver_y_index][rect[i].y] + 1;
		p0.x = 0.5f;
		p0.y = 0.5f;
		p1.x = (Float)blk_size.x - 0.5f;
		p1.y = 0.5f;
		p2.x = (Float)blk_size.x - 0.5f;
		p2.y = (Float)blk_size.y - 0.5f;
		p3.x = 0.5f;
		p3.y = (Float)blk_size.y - 0.5f;
		//blocks on the top border row
		if(rect[i].y == 0) {
			blk_size.y += (DISTANCE_TO_BORDER - 1);
			p0.y = 3.5f;
			p1.y = 3.5f;
			p2.y = (Float)blk_size.y - 0.5f;
			p3.y = (Float)blk_size.y - 0.5f;
		}
		//blocks on the bottom border row
		if(rect[i+1].y == (number_of_ap_y-1)) {
			blk_size.y += (DISTANCE_TO_BORDER - 1);
			p2.y = (Float)blk_size.y - 3.5f;
			p3.y = (Float)blk_size.y - 3.5f;
		}
		//blocks on the left border column
		if(rect[i].x == 0) {
			blk_size.x += (DISTANCE_TO_BORDER - 1);
			p0.x = 3.5f;
			p1.x = (Float)blk_size.x - 0.5f;
			p2.x = (Float)blk_size.x - 0.5f;
			p3.x = 3.5f;
		}
		//blocks on the right border column
		if(rect[i+1].x == (number_of_ap_x-1)) {
			blk_size.x += (DISTANCE_TO_BORDER - 1);
			p1.x = (Float)blk_size.x - 3.5f;
			p2.x = (Float)blk_size.x - 3.5f;
		}
		//calculate perspective transform matrix for the current block
		J_Perspective_Transform* pt = perspectiveTransform(
					p0.x, p0.y,
					p1.x, p1.y,
					p2.x, p2.y,
					p3.x, p3.y,
					aps[rect[i+0].y*number_of_ap_x + rect[i+0].x].center.x, aps[rect[i+0].y*number_of_ap_x + rect[i+0].x].center.y,
					aps[rect[i+0].y*number_of_ap_x + rect[i+1].x].center.x, aps[rect[i+0].y*number_of_ap_x + rect[i+1].x].center.y,
					aps[rect[i+1].y*number_of_ap_x + rect[i+1].x].center.x, aps[rect[i+1].y*number_of_ap_x + rect[i+1].x].center.y,
					aps[rect[i+1].y*number_of_ap_x + rect[i+0].x].center.x, aps[rect[i+1].y*number_of_ap_x + rect[i+0].x].center.y);
		if(pt == NULL) {
			J_REPORT_ERROR(("Get perspective transform failed"))
			free(aps);
			free(matrix);
			free(rect);
			return NULL;
		}
		//sample the current block
#if TEST_MODE
		test_mode_color = 0;
#endif
		Bitmap* block = sampleSymbol(bitmap, pt, blk_size);
		free(pt);
		if(block == NULL) {
			J_REPORT_ERROR(("Sampling block failed"))
			free(aps);
			free(matrix);
			free(rect);
			return NULL;
		}
		// save the sampled block in the matrix
		Int32 start_x = j_ap_pos[side_ver_x_index][rect[i].x] - 1;
		Int32 start_y = j_ap_pos[side_ver_y_index][rect[i].y] - 1;
		if(rect[i].x == 0)
			start_x = 0;
		if(rect[i].y == 0)
			start_y = 0;
		Int32 blk_bytes_per_pixel = block->bits_per_pixel / 8;
		Int32 blk_bytes_per_row = blk_size.x * mtx_bytes_per_pixel;
		for(Int32 y=0, mtx_y=start_y; y<blk_size.y && mtx_y<height; y++, mtx_y++) {
			for(Int32 x=0, mtx_x=start_x; x<blk_size.x && mtx_x<width; x++, mtx_x++) {
				Int32 mtx_offset = mtx_y * mtx_bytes_per_row + mtx_x * mtx_bytes_per_pixel;
				Int32 blk_offset = y * blk_bytes_per_row + x * blk_bytes_per_pixel;
				matrix->pixel[mtx_offset] 	  = block->pixel[blk_offset];
				matrix->pixel[mtx_offset + 1] = block->pixel[blk_offset + 1];
				matrix->pixel[mtx_offset + 2] = block->pixel[blk_offset + 2];
				matrix->pixel[mtx_offset + 3] = block->pixel[blk_offset + 3];
			}
		}
		free(block);
	}
#if TEST_MODE
    saveImage(test_mode_bitmap, "jab_sample_pos_ap.png");
#endif

    // 削除暂存
	free(aps);
	free(rect);
    // 返回矩阵
	return matrix;
}
