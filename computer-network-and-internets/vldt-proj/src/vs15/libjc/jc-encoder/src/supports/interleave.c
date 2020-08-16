/**
 * @file interleave.c	数据交错操作，从libjabcode引入
 * @brief 数据交错操作
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jc.h"
#include "encoder.h"
#include "pseudo_random.h"

#define INTERLEAVE_SEED 226759

/**
 * @brief In-place interleaving
 * @param data the input data to be interleaved
*/
void interleaveData(J_Data* data)
{
    setSeed(INTERLEAVE_SEED);
    for (Int32 i=0; i<data->length; i++)
    {
        Int32 pos = (Int32)( (Float)lcg64_temper() / (Float)UINT32_MAX * (data->length - i) );
        Char  tmp = data->data[data->length - 1 -i];
        data->data[data->length - 1 - i] = data->data[pos];
        data->data[pos] = tmp;
    }
}

/**
 * @brief 把数据排列恢复到正确的位置（去交错） / In-place deinterleaving
 * @param data 需要去交错的数据阵列
*/
void deinterleaveData(J_Data* data) {
    Int32 * index = (Int32 *)malloc(data->length * sizeof(Int32));
    //Int32 index[data->length * sizeof(Int32)];
    for(Int32 i = 0; i < data->length; i++) {
		index[i] = i;
    }
    // 生成 interleave index
    setSeed(INTERLEAVE_SEED);   // 规定 INTERLEAVE_SEED = 226759
    for(Int32 i = 0; i < data->length; i++) {
		Int32 pos = (Int32)( (Float)lcg64_temper() / (Float)UINT32_MAX * (data->length - i) );
		Int32 tmp = index[data->length - 1 - i];
		index[data->length - 1 -i] = index[pos];
		index[pos] = tmp;
    }
    // 削除交错 / deinterleave data
    Char* tmp_data = (Char *)malloc(data->length * sizeof(Char));
    if(tmp_data == NULL) {
		J_REPORT_ERROR(("Memory allocation for temporary buffer in deinterleaver failed"))
			free(index);
			return;
    }
    // 拷贝一份data，方便恢复在正确位置上的数据
	memcpy(tmp_data, data->data, data->length * sizeof(Char));
	for(Int32 i = 0; i < data->length; i++) {
        data->data[index[i]] = tmp_data[i];
    }
    free(tmp_data);
    free(index);
}
