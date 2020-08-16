// detector.c    03/03/2020
// J_Code 
// 请注意：该文件内容很大程度上源自源码，因为探测码块方面不属于本课程内容！
// Please be informed that components in this file were largely
//  subjected to that of the libjabcode detector source codes, for
//  the reason that symbol detecting procedures ARE NOT what this
//  course's aims.
// brief: J_Code detector

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jc.h"
#include "detector.h"
#include "sample.h"
#include "transform.h"
#include "detector.h"
#include "encoder.h"


/**
 * @brief Check the proportion of layer sizes in finder pattern
 * @param state_count the layer sizes in pixel
 * @param module_size the module size
 * @return SUCCESS | FAILURE
 */
Boolean checkPatternCross(Int32* state_count, Float* module_size)
{
    Int32 layer_number = 3;
    Int32 inside_layer_size = 0;
    for(Int32 i=1; i<layer_number+1; i++)
    {
        if(state_count[i] == 0)
            return FAILURE;
        inside_layer_size += state_count[i];
    }
    
    Float layer_size = (Float)inside_layer_size / 3.0f;
    *module_size = layer_size;
    Float layer_tolerance = layer_size / 2.0f;
    
    Boolean size_condition;
    //layer size proportion must be n-1-1-1-m where n>1, m>1
    size_condition = fabs(layer_size - (Float)state_count[1]) < layer_tolerance &&
    fabs(layer_size - (Float)state_count[2]) < layer_tolerance &&
    fabs(layer_size - (Float)state_count[3]) < layer_tolerance &&
    (Float)state_count[0] > 0.5 * layer_tolerance && //the two outside layers can be larger than layer_size
    (Float)state_count[4] > 0.5 * layer_tolerance &&
    abs(state_count[1] - state_count[3]) < layer_tolerance; //layer 1 and layer 3 shall be of the same size
    
    return size_condition;
}

/**
 * @brief Check if the input module sizes are the same
 * @param size_r the first module size
 * @param size_g the second module size
 * @param size_b the third module size
 * @return SUCCESS | FAILURE
 */
Boolean checkModuleSize3(Float size_r, Float size_g, Float size_b)
{
    Float mean= (size_r + size_g + size_b) / 3.0f;
    Float tolerance = mean / 2.5f;
    
    Boolean condition = fabs(mean - size_r) < tolerance &&
    fabs(mean - size_g) < tolerance &&
    fabs(mean - size_b) < tolerance;
    return condition;
}

/**
 * @brief Check if the input module sizes are the same
 * @param size1 the first module size
 * @param size2 the second module size
 
 * @return SUCCESS | FAILURE
 */
Boolean checkModuleSize2(Float size1, Float size2) {
    Float mean= (size1 + size2) / 2.0f;
    Float tolerance = mean / 2.5f;
    
    Boolean condition = fabs(mean - size1) < tolerance &&
    fabs(mean - size2) < tolerance;
    
    return condition;
}

/**
 * @brief Find a candidate scanline of finder pattern
 * @param ch the image channel
 * @param row the row to be scanned
 * @param col the column to be scanned
 * @param start the start position
 * @param end the end position
 * @param center the center of the candidate scanline
 * @param module_size the module size
 * @param skip the number of pixels to be skipped in the next scan
 * @return SUCCESS | FAILURE
 */
Boolean seekPattern(Bitmap* ch, Int32 row, Int32 col, Int32* start, Int32* end, Float* center, Float* module_size, Int32* skip) {
    Int32 state_number = 5;
    Int32 cur_state = 0;
    Int32 state_count[5] = {0};
    
    Int32 min = *start;
    Int32 max = *end;
    for(Int32 p=min; p<max; p++)
    {
        //first pixel in a scanline
        if(p == min)
        {
            state_count[cur_state]++;
            *start = p;
        }
        else
        {
            //previous pixel and current pixel
            BYTE prev;
            BYTE curr;
            if(row >= 0)        //horizontal scan
            {
                prev = ch->pixel[row*ch->width + (p-1)];
                curr = ch->pixel[row*ch->width + p];
            }
            else if(col >= 0)    //vertical scan
            {
                prev = ch->pixel[(p-1)*ch->width + col];
                curr = ch->pixel[p*ch->width + col];
            }
            else
                return FAILURE;
            //the pixel has the same color as the preceding pixel
            if(curr == prev)
            {
                state_count[cur_state]++;
            }
            //the pixel has different color from the preceding pixel
            if(curr != prev || p == max-1)
            {
                //change state
                if(cur_state < state_number-1)
                {
                    //check if the current state is valid
                    if(state_count[cur_state] < 3)
                    {
                        if(cur_state == 0)
                        {
                            state_count[cur_state]=1;
                            *start = p;
                        }
                        else
                        {
                            //combine the current state to the previous one and continue the previous state
                            state_count[cur_state-1] += state_count[cur_state];
                            state_count[cur_state] = 0;
                            cur_state--;
                            state_count[cur_state]++;
                        }
                    }
                    else
                    {
                        //enter the next state
                        cur_state++;
                        state_count[cur_state]++;
                    }
                }
                //find a candidate
                else
                {
                    if(state_count[cur_state] < 3)
                    {
                        //combine the current state to the previous one and continue the previous state
                        state_count[cur_state-1] += state_count[cur_state];
                        state_count[cur_state] = 0;
                        cur_state--;
                        state_count[cur_state]++;
                        continue;
                    }
                    //check if it is a valid finder pattern
                    if(checkPatternCross(state_count, module_size))
                    {
                        *end = p+1;
                        if(skip)  *skip = state_count[0];
                        Int32 end_pos;
                        if(p == (max - 1) && curr == prev) end_pos = p + 1;
                        else end_pos = p;
                        *center = (Float)(end_pos - state_count[4] - state_count[3]) - (Float)state_count[2] / 2.0f;
                        return SUCCESS;
                    }
                    else //check failed, update state_count
                    {
                        *start += state_count[0];
                        for(Int32 k=0; k<state_number-1; k++)
                        {
                            state_count[k] = state_count[k+1];
                        }
                        state_count[state_number-1] = 1;
                        cur_state = state_number-1;
                    }
                }
            }
        }
    }
    *end = max;
    return FAILURE;
}

/**
 * @brief Find a candidate horizontal scanline of finder pattern
 * @param row the bitmap row
 * @param startx the start position
 * @param endx the end position
 * @param centerx the center of the candidate scanline
 * @param module_size the module size
 * @param skip the number of pixels to be skipped in the next scan
 * @return SUCCESS | FAILURE
 */
Boolean seekPatternHorizontal(BYTE* row, Int32* startx, Int32* endx, Float* centerx, Float* module_size, Int32* skip)
{
    Int32 state_number = 5;
    Int32 cur_state = 0;
    Int32 state_count[5] = {0};
    
    Int32 min = *startx;
    Int32 max = *endx;
    for(Int32 j=min; j<max; j++)
    {
        //first pixel in a scanline
        if(j == min)
        {
            state_count[cur_state]++;
            *startx = j;
        }
        else
        {
            //the pixel has the same color as the preceding pixel
            if(row[j] == row[j-1])
            {
                state_count[cur_state]++;
            }
            //the pixel has different color from the preceding pixel
            if(row[j] != row[j-1] || j == max-1)
            {
                //change state
                if(cur_state < state_number-1)
                {
                    //check if the current state is valid
                    if(state_count[cur_state] < 3)
                    {
                        if(cur_state == 0)
                        {
                            state_count[cur_state]=1;
                            *startx = j;
                        }
                        else
                        {
                            //combine the current state to the previous one and continue the previous state
                            state_count[cur_state-1] += state_count[cur_state];
                            state_count[cur_state] = 0;
                            cur_state--;
                            state_count[cur_state]++;
                        }
                    }
                    else
                    {
                        //enter the next state
                        cur_state++;
                        state_count[cur_state]++;
                    }
                }
                //find a candidate
                else
                {
                    if(state_count[cur_state] < 3)
                    {
                        //combine the current state to the previous one and continue the previous state
                        state_count[cur_state-1] += state_count[cur_state];
                        state_count[cur_state] = 0;
                        cur_state--;
                        state_count[cur_state]++;
                        continue;
                    }
                    //check if it is a valid finder pattern
                    if(checkPatternCross(state_count, module_size))
                    {
                        *endx = j+1;
                        if(skip)  *skip = state_count[0];
                        Int32 end;
                        if(j == (max - 1) && row[j] == row[j-1]) end = j + 1;
                        else end = j;
                        *centerx = (Float)(end - state_count[4] - state_count[3]) - (Float)state_count[2] / 2.0f;
                        return SUCCESS;
                    }
                    else //check failed, update state_count
                    {
                        *startx += state_count[0];
                        for(Int32 k=0; k<state_number-1; k++)
                        {
                            state_count[k] = state_count[k+1];
                        }
                        state_count[state_number-1] = 1;
                        cur_state = state_number-1;
                    }
                }
            }
        }
    }
    *endx = max;
    return FAILURE;
}

/**
 * @brief Crosscheck the finder pattern candidate in diagonal direction
 * @param image the image bitmap
 * @param type the finder pattern type
 * @param module_size_max the maximal allowed module size
 * @param centerx the x coordinate of the finder pattern center
 * @param centery the y coordinate of the finder pattern center
 * @param module_size the module size in diagonal direction
 * @param dir the finder pattern direction
 * @param both_dir scan both diagonal scanlines
 * @return the number of confirmed diagonal scanlines
 */
Int32 crossCheckPatternDiagonal(Bitmap* image, Int32 type, Float module_size_max, Float* centerx, Float* centery, Float* module_size, Int32* dir, Boolean both_dir)
{
    Int32 state_number = 5;
    Int32 state_middle = (state_number - 1) / 2;
    
    Int32 offset_x, offset_y;
    Boolean fix_dir = 0;
    //if the direction is given, ONLY check the given direction
    if((*dir) != 0)
    {
        if((*dir) > 0)
        {
            offset_x = -1;
            offset_y = -1;
            *dir = 1;
        }
        else
        {
            offset_x = 1;
            offset_y = -1;
            *dir = -1;
        }
        fix_dir = 1;
    }
    else
    {
        //for fp0 (and fp1 in 4 color mode), first check the diagonal at +45 degree
        if(type == FP0 || type == FP1)
        {
            offset_x = -1;
            offset_y = -1;
            *dir = 1;
        }
        //for fp2, fp3, (and fp1 in 2 color mode) first check the diagonal at -45 degree
        else
        {
            offset_x = 1;
            offset_y = -1;
            *dir = -1;
        }
    }
    
    Int32 confirmed = 0;
    Boolean flag = 0;
    Int32 try_count = 0;
    Float tmp_module_size = 0.0f;
    do
    {
        flag = 0;
        try_count++;
        
        Int32 i = 0, j, state_index;
        Int32 state_count[5] = {0};
        Int32 startx = (Int32)(*centerx);
        Int32 starty = (Int32)(*centery);
        
        state_count[state_middle]++;
        for(j=1, state_index=0; (starty+j*offset_y)>=0 && (starty+j*offset_y)<image->height && (startx+j*offset_x)>=0 && (startx+j*offset_x)<image->width && state_index<=state_middle; j++)
        {
            if( image->pixel[(starty + j*offset_y)*image->width + (startx + j*offset_x)] == image->pixel[(starty + (j-1)*offset_y)*image->width + (startx + (j-1)*offset_x)] )
            {
                state_count[state_middle - state_index]++;
            }
            else
            {
                if(state_index >0 && state_count[state_middle - state_index] < 3)
                {
                    state_count[state_middle - (state_index-1)] += state_count[state_middle - state_index];
                    state_count[state_middle - state_index] = 0;
                    state_index--;
                    state_count[state_middle - state_index]++;
                }
                else
                {
                    state_index++;
                    if(state_index > state_middle) break;
                    else state_count[state_middle - state_index]++;
                }
            }
        }
        if(state_index < state_middle)
        {
            if(try_count == 1)
            {
                flag = 1;
                offset_x = -offset_x;
                *dir = -(*dir);
            }
            else
                return confirmed;
        }
        
        if(!flag)
        {
            for(i=1, state_index=0; (starty-i*offset_y)>=0 && (starty-i*offset_y)<image->height && (startx-i*offset_x)>=0 && (startx-i*offset_x)<image->width && state_index<=state_middle; i++)
            {
                if( image->pixel[(starty - i*offset_y)*image->width + (startx - i*offset_x)] == image->pixel[(starty - (i-1)*offset_y)*image->width + (startx - (i-1)*offset_x)] )
                {
                    state_count[state_middle + state_index]++;
                }
                else
                {
                    if(state_index >0 && state_count[state_middle + state_index] < 3)
                    {
                        state_count[state_middle + (state_index-1)] += state_count[state_middle + state_index];
                        state_count[state_middle + state_index] = 0;
                        state_index--;
                        state_count[state_middle + state_index]++;
                    }
                    else
                    {
                        state_index++;
                        if(state_index > state_middle) break;
                        else state_count[state_middle + state_index]++;
                    }
                }
            }
            if(state_index < state_middle)
            {
                if(try_count == 1)
                {
                    flag = 1;
                    offset_x = -offset_x;
                    *dir = -(*dir);
                }
                else
                    return confirmed;
            }
        }
        
        if(!flag)
        {
            //check module size, if it is too big, assume it is a false positive
            Boolean ret = checkPatternCross(state_count, module_size);
            if(ret && ((*module_size) <= module_size_max))
            {
                if(tmp_module_size > 0)
                    *module_size = (*module_size + tmp_module_size) / 2.0f;
                else
                    tmp_module_size = *module_size;
                
                //calculate the center x
                *centerx = (Float)(startx+i - state_count[4] - state_count[3]) - (Float)state_count[2] / 2.0f;
                //calculate the center y
                *centery = (Float)(starty+i - state_count[4] - state_count[3]) - (Float)state_count[2] / 2.0f;
                confirmed++;
                if( !both_dir || try_count == 2 || fix_dir)
                {
                    if(confirmed == 2)
                        *dir = 2;
                    return confirmed;
                }
            }
            else
            {
                offset_x = -offset_x;
                *dir = -(*dir);
            }
        }
    }while(try_count < 2 && !fix_dir);
    
    return confirmed;
}

/**
 * @brief Crosscheck the finder pattern candidate in vertical direction
 * @param image the image bitmap
 * @param module_size_max the maximal allowed module size
 * @param centerx the x coordinate of the finder pattern center
 * @param centery the y coordinate of the finder pattern center
 * @param module_size the module size in vertical direction
 * @return SUCCESS | FAILURE
 */
Boolean crossCheckPatternVertical(Bitmap* image, Int32 module_size_max, Float centerx, Float* centery, Float* module_size)
{
    Int32 state_number = 5;
    Int32 state_middle = (state_number - 1) / 2;
    Int32 state_count[5] = {0};
    
    Int32 centerx_int = (Int32)centerx;
    Int32 centery_int = (Int32)(*centery);
    Int32 i, state_index;
    
    state_count[1]++;
    for(i=1, state_index=0; i<=centery_int && state_index<=state_middle; i++)
    {
        if( image->pixel[(centery_int-i)*image->width + centerx_int] == image->pixel[(centery_int-(i-1))*image->width + centerx_int] )
        {
            state_count[state_middle - state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[state_middle - state_index] < 3)
            {
                state_count[state_middle - (state_index-1)] += state_count[state_middle - state_index];
                state_count[state_middle - state_index] = 0;
                state_index--;
                state_count[state_middle - state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > state_middle) break;
                else state_count[state_middle - state_index]++;
            }
        }
    }
    if(state_index < state_middle)
        return FAILURE;
    
    for(i=1, state_index=0; (centery_int+i)<image->height && state_index<=state_middle; i++)
    {
        if( image->pixel[(centery_int+i)*image->width + centerx_int] == image->pixel[(centery_int+(i-1))*image->width + centerx_int] )
        {
            state_count[state_middle + state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[state_middle + state_index] < 3)
            {
                state_count[state_middle + (state_index-1)] += state_count[state_middle + state_index];
                state_count[state_middle + state_index] = 0;
                state_index--;
                state_count[state_middle + state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > state_middle) break;
                else state_count[state_middle + state_index]++;
            }
        }
    }
    if(state_index < state_middle)
        return FAILURE;
    
    //check module size, if it is too big, assume it is a false positive
    Boolean ret = checkPatternCross(state_count, module_size);
    if(ret && ((*module_size) <= module_size_max))
    {
        //calculate the center y
        *centery = (Float)(centery_int + i - state_count[4] - state_count[3]) - (Float)state_count[2] / 2.0f;
        return SUCCESS;
    }
    return FAILURE;
}

/**
 * @brief Crosscheck the finder pattern candidate in horizontal direction
 * @param image the image bitmap
 * @param module_size_max the maximal allowed module size
 * @param centerx the x coordinate of the finder pattern center
 * @param centery the y coordinate of the finder pattern center
 * @param module_size the module size in horizontal direction
 * @return SUCCESS | FAILURE
 */
Boolean crossCheckPatternHorizontal(Bitmap* image, Float module_size_max, Float* centerx, Float centery, Float* module_size)
{
    Int32 state_number = 5;
    Int32 state_middle = (state_number - 1) / 2;
    Int32 state_count[5] = {0};
    
    Int32 startx = (Int32)(*centerx);
    Int32 offset = (Int32)centery * image->width;
    Int32 i, state_index;
    
    state_count[state_middle]++;
    for(i=1, state_index=0; i<=startx && state_index<=state_middle; i++)
    {
        if( image->pixel[offset + (startx - i)] == image->pixel[offset + (startx - (i-1))] )
        {
            state_count[state_middle - state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[state_middle - state_index] < 3)
            {
                state_count[state_middle - (state_index-1)] += state_count[state_middle - state_index];
                state_count[state_middle - state_index] = 0;
                state_index--;
                state_count[state_middle - state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > state_middle) break;
                else state_count[state_middle - state_index]++;
            }
        }
    }
    if(state_index < state_middle)
        return FAILURE;
    
    for(i=1, state_index=0; (startx+i)<image->width && state_index<=state_middle; i++)
    {
        if( image->pixel[offset + (startx + i)] == image->pixel[offset + (startx + (i-1))] )
        {
            state_count[state_middle + state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[state_middle + state_index] < 3)
            {
                state_count[state_middle + (state_index-1)] += state_count[state_middle + state_index];
                state_count[state_middle + state_index] = 0;
                state_index--;
                state_count[state_middle + state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > state_middle) break;
                else state_count[state_middle + state_index]++;
            }
        }
    }
    if(state_index < state_middle)
        return FAILURE;
    
    //check module size, if it is too big, assume it is a false positive
    Boolean ret = checkPatternCross(state_count, module_size);
    if(ret && ((*module_size) <= module_size_max))
    {
        //calculate the center x
        *centerx = (Float)(startx+i - state_count[4] - state_count[3]) - (Float)state_count[2] / 2.0f;
        return SUCCESS;
    }
    return FAILURE;
}

/**
 * @brief Crosscheck the finder pattern color
 * @param image the image bitmap
 * @param color the expected module color
 * @param module_size the module size
 * @param module_number the number of modules that should be checked
 * @param centerx the x coordinate of the finder pattern center
 * @param centery the y coordinate of the finder pattern center
 * @param dir the check direction
 * @return SUCCESS | FAILURE
 */
Boolean crossCheckColor(Bitmap* image, Int32 color, Int32 module_size, Int32 module_number, Int32 centerx, Int32 centery, Int32 dir)
{
    Int32 tolerance = 3;
    //horizontal
    if(dir == 0)
    {
        Int32 length = module_size * (module_number - 1); //module number minus one for better tolerance
        Int32 startx = (centerx - length/2) < 0 ? 0 : (centerx - length/2);
        Int32 unmatch = 0;
        for(Int32 j=startx; j<(startx+length) && j<image->width; j++)
        {
            if(image->pixel[centery * image->width + j] != color) unmatch++;
            else
            {
                if(unmatch <= tolerance) unmatch = 0;
            }
            if(unmatch > tolerance)    return FAILURE;
        }
        return SUCCESS;
    }
    //vertical
    else if(dir == 1)
    {
        Int32 length = module_size * (module_number - 1);
        Int32 starty = (centery - length/2) < 0 ? 0 : (centery - length/2);
        Int32 unmatch = 0;
        for(Int32 i=starty; i<(starty+length) && i<image->height; i++)
        {
            if(image->pixel[image->width * i + centerx] != color) unmatch++;
            else
            {
                if(unmatch <= tolerance) unmatch = 0;
            }
            if(unmatch > tolerance)    return FAILURE;
        }
        return SUCCESS;
    }
    //diagonal
    else if(dir == 2)
    {
        Int32 offset = module_size * (module_number / (2.0f * 1.41421f)); // e.g. for FP, module_size * (5/(2*sqrt(2));
        Int32 length = offset * 2;
        
        //one direction
        Int32 unmatch = 0;
        Int32 startx = (centerx - offset) < 0 ? 0 : (centerx - offset);
        Int32 starty = (centery - offset) < 0 ? 0 : (centery - offset);
        for(Int32 i=0; i<length && (starty+i)<image->height; i++)
        {
            if(image->pixel[image->width * (starty+i) + (startx+i)] != color) unmatch++;
            else
            {
                if(unmatch <= tolerance) unmatch = 0;
            }
            if(unmatch > tolerance) break;
        }
        if(unmatch < tolerance) return SUCCESS;
        
        //the other direction
        unmatch = 0;
        startx = (centerx - offset) < 0 ? 0 : (centerx - offset);
        starty = (centery + offset) > (image->height - 1) ? (image->height - 1) : (centery + offset);
        for(Int32 i=0; i<length && (starty-i)>=0; i++)
        {
            if(image->pixel[image->width * (starty-i) + (startx+i)] != color) unmatch++;
            else
            {
                if(unmatch <= tolerance) unmatch = 0;
            }
            if(unmatch > tolerance) return FAILURE;
        }
        return SUCCESS;
    }
    else
    {
        J_REPORT_ERROR(("Invalid direction"))
        return FAILURE;
    }
}

/**
 * @brief Crosscheck the finder pattern candidate in one channel
 * @param ch the binarized color channel
 * @param type the finder pattern type
 * @param h_v the direction of the candidate scanline, 0:horizontal 1:vertical
 * @param module_size_max the maximal allowed module size
 * @param module_size the module size in all directions
 * @param centerx the x coordinate of the finder pattern center
 * @param centery the y coordinate of the finder pattern center
 * @param dir the finder pattern direction
 * @param dcc the diagonal crosscheck result
 * @return SUCCESS | FAILURE
 */
Boolean crossCheckPatternCh(Bitmap* ch, Int32 type, Int32 h_v, Float module_size_max, Float* module_size, Float* centerx, Float* centery, Int32* dir, Int32* dcc)
{
    Float module_size_v = 0.0f;
    Float module_size_h = 0.0f;
    Float module_size_d = 0.0f;
    
    if(h_v == 0)
    {
        Boolean vcc = FAILURE;
        if(crossCheckPatternVertical(ch, module_size_max, *centerx, centery, &module_size_v))
        {
            vcc = SUCCESS;
            if(!crossCheckPatternHorizontal(ch, module_size_max, centerx, *centery, &module_size_h))
                return FAILURE;
        }
        *dcc = crossCheckPatternDiagonal(ch, type, module_size_max, centerx, centery, &module_size_d, dir, !vcc);
        if(vcc && *dcc > 0)
        {
            *module_size = (module_size_v + module_size_h + module_size_d) / 3.0f;
            return SUCCESS;
        }
        else if(*dcc == 2)
        {
            if(!crossCheckPatternHorizontal(ch, module_size_max, centerx, *centery, &module_size_h))
                return FAILURE;
            *module_size = (module_size_h + module_size_d * 2.0f) / 3.0f;
            return SUCCESS;
        }
    }
    else
    {
        Boolean hcc = FAILURE;
        if(crossCheckPatternHorizontal(ch, module_size_max, centerx, *centery, &module_size_h))
        {
            hcc = SUCCESS;
            if(!crossCheckPatternVertical(ch, module_size_max, *centerx, centery, &module_size_v))
                return FAILURE;
        }
        *dcc = crossCheckPatternDiagonal(ch, type, module_size_max, centerx, centery, &module_size_d, dir, !hcc);
        if(hcc && *dcc > 0)
        {
            *module_size = (module_size_v + module_size_h + module_size_d) / 3.0f;
            return SUCCESS;
        }
        else if(*dcc == 2)
        {
            if(!crossCheckPatternVertical(ch, module_size_max, *centerx, centery, &module_size_v))
                return FAILURE;
            *module_size = (module_size_v + module_size_d * 2.0f) / 3.0f;
            return SUCCESS;
        }
    }
    return FAILURE;
}

/**
 * @brief Crosscheck the finder pattern candidate
 * @param ch the binarized color channels of the image
 * @param fp the finder pattern
 * @param h_v the direction of the candidate scanline, 0:horizontal 1:vertical
 * @return SUCCESS | FAILURE
 */
Boolean crossCheckPattern(Bitmap* ch[], J_Finder_Pattern* fp, Int32 h_v)
{
    Float module_size_max = fp->module_size * 2.0f;
    
    //check g channel
    Float module_size_g;
    Float centerx_g = fp->center.x;
    Float centery_g = fp->center.y;
    Int32 dir_g = 0;
    Int32 dcc_g = 0;
    if(!crossCheckPatternCh(ch[1], fp->type, h_v, module_size_max, &module_size_g, &centerx_g, &centery_g, &dir_g, &dcc_g))
        return FAILURE;
    
    //Finder Pattern FP1 and FP2
    if(fp->type == FP1 || fp->type == FP2)
    {
        //check r channel
        Float module_size_r;
        Float centerx_r = fp->center.x;
        Float centery_r = fp->center.y;
        Int32 dir_r = 0;
        Int32 dcc_r = 0;
        if(!crossCheckPatternCh(ch[0], fp->type, h_v, module_size_max, &module_size_r, &centerx_r, &centery_r, &dir_r, &dcc_r))
            return FAILURE;
        
        //module size must be consistent
        if(!checkModuleSize2(module_size_r, module_size_g))
            return FAILURE;
        fp->module_size = (module_size_r + module_size_g) / 2.0f;
        fp->center.x = (centerx_r + centerx_g) / 2.0f;
        fp->center.y = (centery_r + centery_g) / 2.0f;
        
        //check b channel
        Int32 core_color_in_blue_channel = J_Default_Palette[FP2_CORE_COLOR * 3 + 2];
        if(!crossCheckColor(ch[2], core_color_in_blue_channel, (Int32)fp->module_size, 5, (Int32)fp->center.x, (Int32)fp->center.y, 0))
            return FAILURE;
        if(!crossCheckColor(ch[2], core_color_in_blue_channel, (Int32)fp->module_size, 5, (Int32)fp->center.x, (Int32)fp->center.y, 1))
            return FAILURE;
        if(!crossCheckColor(ch[2], core_color_in_blue_channel, (Int32)fp->module_size, 5, (Int32)fp->center.x, (Int32)fp->center.y, 2))
            return FAILURE;
        
        if(dcc_r == 2 || dcc_g == 2)
            fp->direction = 2;
        else
            fp->direction = (dir_r + dir_g) > 0 ? 1 : -1;
    }
    
    //Finder Pattern FP0 and FP3
    if(fp->type == FP0 || fp->type == FP3)
    {
        //check b channel
        Float module_size_b;
        Float centerx_b = fp->center.x;
        Float centery_b = fp->center.y;
        Int32 dir_b = 0;
        Int32 dcc_b = 0;
        if(!crossCheckPatternCh(ch[2], fp->type, h_v, module_size_max, &module_size_b, &centerx_b, &centery_b, &dir_b, &dcc_b))
            return FAILURE;
        
        //module size must be consistent
        if(!checkModuleSize2(module_size_g, module_size_b))
            return FAILURE;
        fp->module_size = (module_size_g + module_size_b) / 2.0f;
        fp->center.x = (centerx_g + centerx_b) / 2.0f;
        fp->center.y = (centery_g + centery_b) / 2.0f;
        
        //check r channel
        Int32 core_color_in_red_channel = J_Default_Palette[FP3_CORE_COLOR * 3 + 0];
        if(!crossCheckColor(ch[0], core_color_in_red_channel, (Int32)fp->module_size, 5, (Int32)fp->center.x, (Int32)fp->center.y, 0))
            return FAILURE;
        if(!crossCheckColor(ch[0], core_color_in_red_channel, (Int32)fp->module_size, 5, (Int32)fp->center.x, (Int32)fp->center.y, 1))
            return FAILURE;
        if(!crossCheckColor(ch[0], core_color_in_red_channel, (Int32)fp->module_size, 5, (Int32)fp->center.x, (Int32)fp->center.y, 2))
            return FAILURE;
        
        if(dcc_g == 2 || dcc_b == 2)
            fp->direction = 2;
        else
            fp->direction = (dir_g + dir_b) > 0 ? 1 : -1;
    }
    
    return SUCCESS;
}

/**
 * @brief Save a found alignment pattern into the alignment pattern list
 * @param ap the alignment pattern
 * @param aps the alignment pattern list
 * @param counter the number of alignment patterns in the list
 * @return  -1 if added as a new alignment pattern | the alignment pattern index if combined with an existing pattern
 */
Int32 saveAlignmentPattern(J_Alignment_Pattern* ap, J_Alignment_Pattern* aps, Int32* counter)
{
    //combine the alignment patterns at the same position with the same size
    for(Int32 i=0; i<(*counter); i++)
    {
        if(aps[i].found_count > 0)
        {
            if( fabs(ap->center.x - aps[i].center.x) <= ap->module_size && fabs(ap->center.y - aps[i].center.y) <= ap->module_size &&
               (fabs(ap->module_size - aps[i].module_size) <= aps[i].module_size || fabs(ap->module_size - aps[i].module_size) <= 1.0) &&
               ap->type == aps[i].type )
            {
                aps[i].center.x = ((Float)aps[i].found_count * aps[i].center.x + ap->center.x) / (Float)(aps[i].found_count + 1);
                aps[i].center.y = ((Float)aps[i].found_count * aps[i].center.y + ap->center.y) / (Float)(aps[i].found_count + 1);
                aps[i].module_size = ((Float)aps[i].found_count * aps[i].module_size + ap->module_size) / (Float)(aps[i].found_count + 1);
                aps[i].found_count++;
                return i;
            }
        }
    }
    //add a new alignment pattern
    aps[*counter] = *ap;
    (*counter)++;
    return -1;
}

/**
 * @brief Save a found finder pattern into the finder pattern list
 * @param fp the finder pattern
 * @param fps the finder pattern list
 * @param counter the number of finder patterns in the list
 * @param fp_type_count the number of finder pattern types in the list
 */
void saveFinderPattern(J_Finder_Pattern* fp, J_Finder_Pattern* fps, Int32* counter, Int32* fp_type_count) {
    //combine the finder patterns at the same position with the same size
    for(Int32 i=0; i<(*counter); i++)
    {
        if(fps[i].found_count > 0)
        {
            if( fabs(fp->center.x - fps[i].center.x) <= fp->module_size && fabs(fp->center.y - fps[i].center.y) <= fp->module_size &&
               (fabs(fp->module_size - fps[i].module_size) <= fps[i].module_size || fabs(fp->module_size - fps[i].module_size) <= 1.0) &&
               fp->type == fps[i].type)
            {
                fps[i].center.x = ((Float)fps[i].found_count * fps[i].center.x + fp->center.x) / (Float)(fps[i].found_count + 1);
                fps[i].center.y = ((Float)fps[i].found_count * fps[i].center.y + fp->center.y) / (Float)(fps[i].found_count + 1);
                fps[i].module_size = ((Float)fps[i].found_count * fps[i].module_size + fp->module_size) / (Float)(fps[i].found_count + 1);
                fps[i].found_count++;
                fps[i].direction += fp->direction;
                return;
            }
        }
    }
    //add a new finder pattern
    fps[*counter] = *fp;
    (*counter)++;
    fp_type_count[fp->type]++;
}

#if TEST_MODE
void drawFoundFinderPatterns(jab_finder_pattern* fps, jab_int32 number, jab_int32 color)
{
    jab_int32 bytes_per_pixel = test_mode_bitmap->bits_per_pixel / 8;
    jab_int32 bytes_per_row = test_mode_bitmap->width * bytes_per_pixel;
    for(jab_int32 k = 0; k<number; k++)
    {
        if(fps[k].found_count > 0)
        {
            jab_int32 centerx = (jab_int32)(fps[k].center.x + 0.5f);
            jab_int32 centery = (jab_int32)(fps[k].center.y + 0.5f);
            
            jab_int32 starty = (jab_int32)(fps[k].center.y - fps[k].module_size/2 + 0.5f);
            jab_int32 endy = (jab_int32)(fps[k].center.y + fps[k].module_size/2 + 1 + 0.5f);
            for(jab_int32 i=starty; i<endy; i++)
            {
                if(i*bytes_per_row + centerx*bytes_per_pixel + 2 < test_mode_bitmap->width * test_mode_bitmap->height * bytes_per_pixel)
                {
                    test_mode_bitmap->pixel[i*bytes_per_row + centerx*bytes_per_pixel]        = (color >> 16) & 0xff;
                    test_mode_bitmap->pixel[i*bytes_per_row + centerx*bytes_per_pixel + 1] = (color >>  8) & 0xff;;
                    test_mode_bitmap->pixel[i*bytes_per_row + centerx*bytes_per_pixel + 2] = color & 0xff;
                }
                else
                {
                    J_REPORT_ERROR(("Drawing finder pattern %d out of image", k))
                    break;
                }
            }
            
            jab_int32 startx = (jab_int32)(fps[k].center.x - fps[k].module_size/2 + 0.5f);
            jab_int32 endx = (jab_int32)(fps[k].center.x + fps[k].module_size/2 + 1 + 0.5f);
            for(jab_int32 i=startx; i<endx; i++)
            {
                if(centery*bytes_per_row + i*bytes_per_pixel + 2 < test_mode_bitmap->width * test_mode_bitmap->height * bytes_per_pixel)
                {
                    test_mode_bitmap->pixel[centery*bytes_per_row + i*bytes_per_pixel]        = (color >> 16) & 0xff;
                    test_mode_bitmap->pixel[centery*bytes_per_row + i*bytes_per_pixel + 1] = (color >>  8) & 0xff;;
                    test_mode_bitmap->pixel[centery*bytes_per_row + i*bytes_per_pixel + 2] = color & 0xff;
                }
                else
                {
                    J_REPORT_ERROR(("Drawing finder pattern %d out of image", k))
                    break;
                }
            }
        }
    }
}
#endif

/**
 * @brief Remove the finder patterns with greatly different module size
 * @param fps the finder pattern list
 * @param fp_count the number of finder patterns in the list
 * @param mean the average module size
 * @param threshold the tolerance threshold
 */
void removeBadPatterns(J_Finder_Pattern* fps, Int32 fp_count, Float mean, Float threshold)
{
    Int32 remove_count = 0;
    Int32 *backup = (Int32 *)malloc(sizeof(Int32) * fp_count);
    for(Int32 i=0; i<fp_count; i++)
    {
        if( fps[i].found_count < 2 || fabs(fps[i].module_size - mean) > threshold )
        {
            backup[i] = fps[i].found_count;
            fps[i].found_count = 0;
            remove_count++;
        }
    }
    //in case all finder patterns were removed, recover the one whose module size differs from the mean minimally
    if(remove_count == fp_count)
    {
        Float min = (threshold + mean)* 100;
        Int32 min_index = 0;
        for(Int32 i=0; i<fp_count; i++)
        {
            Float diff = (Float)fabs(fps[i].module_size - mean);
            if(diff < min)
            {
                min = diff;
                min_index = i;
            }
        }
        fps[min_index].found_count = backup[min_index];
        remove_count--;
    }
	free(backup);
}

/**
 * @brief Find the finder pattern with most detected times
 * @param fps the finder pattern list
 * @param fp_count the number of finder patterns in the list
 */
J_Finder_Pattern getBestPattern(J_Finder_Pattern* fps, Int32 fp_count)
{
    Int32 counter = 0;
    Float total_module_size = 0;
    for(Int32 i=0; i<fp_count; i++)
    {
        if(fps[i].found_count > 0)
        {
            counter++;
            total_module_size += fps[i].module_size;
        }
    }
    Float mean = total_module_size / (Float)counter;
    
    Int32 max_found_count = 0;
    Float min_diff = 100;
    Int32 max_found_count_index = 0;
    for(Int32 i=0; i<fp_count; i++)
    {
        if(fps[i].found_count > 0)
        {
            if(fps[i].found_count > max_found_count)
            {
                max_found_count = fps[i].found_count;
                max_found_count_index = i;
                min_diff = (Float)fabs(fps[i].module_size - mean);
            }
            else if(fps[i].found_count == max_found_count)
            {
                if( fabs(fps[i].module_size - mean) < min_diff )
                {
                    max_found_count_index = i;
                    min_diff = (Float)fabs(fps[i].module_size - mean);
                }
            }
        }
    }
    J_Finder_Pattern fp = fps[max_found_count_index];
    fps[max_found_count_index].found_count = 0;
    return fp;
}

/**
 * @brief Select the best finder patterns out of the list
 * @param fps the finder pattern list
 * @param fp_count the number of finder patterns in the list
 * @param fp_type_count the number of each finder pattern type
 * @return the number of missing finder pattern types
 */
Int32 selectBestPatterns(J_Finder_Pattern* fps, Int32 fp_count, Int32* fp_type_count)
{
    //check if more than one finder pattern type not found
    Int32 missing_fp_type_count = 0;
    for(Int32 i=FP0; i<=FP3; i++)
    {
        if(fp_type_count[i] == 0)
            missing_fp_type_count++;
    }
    if(missing_fp_type_count > 1)
        return missing_fp_type_count;
    
    //classify finder patterns into four types
	J_Finder_Pattern *fps0 = new J_Finder_Pattern[fp_type_count[FP0]];
	J_Finder_Pattern *fps1 = new J_Finder_Pattern[fp_type_count[FP1]];
	J_Finder_Pattern *fps2 = new J_Finder_Pattern[fp_type_count[FP2]]; 
	J_Finder_Pattern *fps3 = new J_Finder_Pattern[fp_type_count[FP3]];
    Int32 counter0 = 0, counter1 = 0, counter2 = 0, counter3 = 0;
    
    for(Int32 i=0; i<fp_count; i++)
    {
        switch (fps[i].type)
        {
            case FP0:
                fps0[counter0++] = fps[i];
                break;
            case FP1:
                fps1[counter1++] = fps[i];
                break;
            case FP2:
                fps2[counter2++] = fps[i];
                break;
            case FP3:
                fps3[counter3++] = fps[i];
                break;
        }
    }
    
    //set fp0
    if(counter0 > 1)
        fps[0] = getBestPattern(fps0, counter0);
    else if(counter0 == 1)
        fps[0] = fps0[0];
    else
        memset(&fps[0], 0, sizeof(J_Finder_Pattern));
    //set fp1
    if(counter1 > 1)
        fps[1] = getBestPattern(fps1, counter1);
    else if(counter1 == 1)
        fps[1] = fps1[0];
    else
        memset(&fps[1], 0, sizeof(J_Finder_Pattern));
    //set fp2
    if(counter2 > 1)
        fps[2] = getBestPattern(fps2, counter2);
    else if(counter2 == 1)
        fps[2] = fps2[0];
    else
        memset(&fps[2], 0, sizeof(J_Finder_Pattern));
    //set fp3
    if(counter3 > 1)
        fps[3] = getBestPattern(fps3, counter3);
    else if(counter3 == 1)
        fps[3] = fps3[0];
    else
        memset(&fps[3], 0, sizeof(J_Finder_Pattern));
    
    //check finder patterns' direction
    //if the direction of fp0 and fp1 not consistent, abandon the one with smaller found_count
    /*    if(fps[0].found_count > 0 && fps[1].found_count > 0 && fps[0].found_count != fps[1].found_count && fps[0].direction != fps[1].direction)
     {
     if(fps[0].found_count < fps[1].found_count)
     fps[0].found_count = 0;
     else
     fps[1].found_count = 0;
     }
     //if the direction of fp2 and fp3 not consistent, abandon the one with smaller found_count
     if(fps[2].found_count > 0 && fps[3].found_count > 0 && fps[2].found_count != fps[3].found_count && fps[2].direction != fps[3].direction)
     {
     if(fps[2].found_count < fps[3].found_count)
     fps[2].found_count = 0;
     else
     fps[3].found_count = 0;
     }
     */
    //check how many finder patterns are not found
    Int32 missing_fp_count = 0;
    for(Int32 i=0; i<4; i++)
    {
        if(fps[i].found_count == 0)
            missing_fp_count++;
    }

	delete[] fps0;
	delete[] fps1;
	delete[] fps2;
	delete[] fps3;

    return missing_fp_count;
}

/**
 * @brief Scan the image vertically
 * @param ch the binarized color channels of the image
 * @param min_module_size the minimal module size
 * @param fps the found finder patterns
 * @param fp_type_count the number of found finder patterns for each type
 * @param total_finder_patterns the number of totally found finder patterns
 */
void scanPatternVertical(Bitmap* ch[], Int32 min_module_size, J_Finder_Pattern* fps, Int32* fp_type_count, Int32* total_finder_patterns)
{
    Boolean done = 0;
    
    for(Int32 j=0; j<ch[0]->width && done == 0; j+=min_module_size)
    {
        Int32 starty = 0;
        Int32 endy = ch[0]->height;
        Int32 skip = 0;
        
        do
        {
            Int32 type_r = 0, type_g, type_b = 0;
            Float centery_r, centery_g, centery_b;
            Float module_size_r, module_size_g, module_size_b;
            Boolean finder_pattern1_found = FAILURE;
            Boolean finder_pattern2_found = FAILURE;
            
            starty += skip;
            endy = ch[0]->height;
            //green channel
            if(seekPattern(ch[1], -1, j, &starty, &endy, &centery_g, &module_size_g, &skip))
            {
                type_g = ch[1]->pixel[(Int32)(centery_g)*ch[0]->width + j] > 0 ? 255 : 0;
                
                centery_r = centery_g;
                centery_b = centery_g;
                //check blue channel for Finder Pattern UL and LL
                if(crossCheckPatternVertical(ch[2], module_size_g*2, (Float)j, &centery_b, &module_size_b))
                {
                    type_b = ch[2]->pixel[(Int32)(centery_b)*ch[2]->width + j] > 0 ? 255 : 0;
                    //check red channel
                    module_size_r = module_size_g;
                    Int32 core_color_in_red_channel = J_Default_Palette[FP3_CORE_COLOR * 3 + 0];
                    if(crossCheckColor(ch[0], core_color_in_red_channel, module_size_r, 5, j, (Int32)centery_r, 1))
                    {
                        type_r = 0;
                        finder_pattern1_found = SUCCESS;
                    }
                }
                //check red channel for Finder Pattern UR and LR
                else if(crossCheckPatternVertical(ch[0], module_size_g*2, (Float)j, &centery_r, &module_size_r))
                {
                    type_r = ch[0]->pixel[(Int32)(centery_r)*ch[0]->width + j] > 0 ? 255 : 0;
                    //check blue channel
                    module_size_b = module_size_g;
                    Int32 core_color_in_blue_channel = J_Default_Palette[FP2_CORE_COLOR * 3 + 2];
                    if(crossCheckColor(ch[2], core_color_in_blue_channel, module_size_b, 5, j, (Int32)centery_b, 1))
                    {
                        type_b = 0;
                        finder_pattern2_found = SUCCESS;
                    }
                }
                //finder pattern candidate found
                if(finder_pattern1_found || finder_pattern2_found)
                {
                    J_Finder_Pattern fp;
                    fp.center.x = (Float)j;
                    fp.found_count = 1;
                    if(finder_pattern1_found)
                    {
                        if(!checkModuleSize2(module_size_g, module_size_b)) continue;
                        fp.center.y = (centery_g + centery_b) / 2.0f;
                        fp.module_size = (module_size_g + module_size_b) / 2.0f;
                        if( type_r == J_Default_Palette[FP0_CORE_COLOR * 3] &&
                           type_g == J_Default_Palette[FP0_CORE_COLOR * 3 + 1] &&
                           type_b == J_Default_Palette[FP0_CORE_COLOR * 3 + 2])
                        {
                            fp.type = FP0;    //candidate for fp0
                        }
                        else if(type_r == J_Default_Palette[FP3_CORE_COLOR * 3] &&
                                type_g == J_Default_Palette[FP3_CORE_COLOR * 3 + 1] &&
                                type_b == J_Default_Palette[FP3_CORE_COLOR * 3 + 2])
                        {
                            fp.type = FP3;    //candidate for fp3
                        }
                        else
                        {
                            continue;        //invalid type
                        }
                    }
                    else if(finder_pattern2_found)
                    {
                        if(!checkModuleSize2(module_size_r, module_size_g)) continue;
                        fp.center.y = (centery_r + centery_g) / 2.0f;
                        fp.module_size = (module_size_r + module_size_g) / 2.0f;
                        if(type_r == J_Default_Palette[FP1_CORE_COLOR * 3] &&
                           type_g == J_Default_Palette[FP1_CORE_COLOR * 3 + 1] &&
                           type_b == J_Default_Palette[FP1_CORE_COLOR * 3 + 2])
                        {
                            fp.type = FP1;    //candidate for fp1
                        }
                        else if(type_r == J_Default_Palette[FP2_CORE_COLOR * 3] &&
                                type_g == J_Default_Palette[FP2_CORE_COLOR * 3 + 1] &&
                                type_b == J_Default_Palette[FP2_CORE_COLOR * 3 + 2])
                        {
                            fp.type = FP2;    //candidate for fp2
                        }
                        else
                        {
                            continue;        //invalid type
                        }
                    }
                    //cross check
                    if( crossCheckPattern(ch, &fp, 1) )
                    {
                        saveFinderPattern(&fp, fps, total_finder_patterns, fp_type_count);
                        if(*total_finder_patterns >= (MAX_FINDER_PATTERNS - 1) )
                        {
                            done = 1;
                            break;
                        }
                    }
                }
            }
        }while(starty < ch[0]->height && endy < ch[0]->height);
    }
}

/**
 * @brief Search for the missing finder pattern at the estimated position
 * @param bitmap the image bitmap
 * @param fps the finder patterns
 * @param miss_fp_index the index of the missing finder pattern
 */
void seekMissingFinderPattern(Bitmap* bitmap, J_Finder_Pattern* fps, Int32 miss_fp_index)
{
    //calculate the average module size
    Float ave_module_size = 0;
    for(Int32 i=0; i<4; i++)
    {
        if(fps[i].found_count > 1)
        {
            ave_module_size += fps[i].module_size;
        }
    }
    ave_module_size /= 3;
    //determine the search area
    Int32 radius = ave_module_size * 5;    //search radius
    Int32 start_x = (fps[miss_fp_index].center.x - radius) >= 0 ? (fps[miss_fp_index].center.x - radius) : 0;
    Int32 start_y = (fps[miss_fp_index].center.y - radius) >= 0 ? (fps[miss_fp_index].center.y - radius) : 0;
    Int32 end_x      = (fps[miss_fp_index].center.x + radius) <= (bitmap->width - 1) ? (fps[miss_fp_index].center.x + radius) : (bitmap->width - 1);
    Int32 end_y   = (fps[miss_fp_index].center.y + radius) <= (bitmap->height- 1) ? (fps[miss_fp_index].center.y + radius) : (bitmap->height- 1);
    Int32 area_width = end_x - start_x;
    Int32 area_height= end_y - start_y;
    
    Bitmap* rgb[3];
    for(Int32 i=0; i<3; i++)
    {
        rgb[i] = (Bitmap*)calloc(1, sizeof(Bitmap) + area_height*area_width*sizeof(BYTE));
        if(rgb[i] == NULL)
        {
            J_REPORT_ERROR(("Memory allocation for binary bitmap failed, the missing finder pattern can not be found."));
            return;
        }
        rgb[i]->width = area_width;
        rgb[i]->height= area_height;
        rgb[i]->bits_per_channel = 8;
        rgb[i]->bits_per_pixel = 8;
        rgb[i]->channel_count = 1;
    }
    
    //calculate average pixel value
    Int32 bytes_per_pixel = bitmap->bits_per_pixel / 8;
    Int32 bytes_per_row = bitmap->width * bytes_per_pixel;
    Float pixel_sum[3] = {0, 0, 0};
    Float pixel_ave[3];
    for(Int32 i=start_y; i<end_y; i++)
    {
        for(Int32 j=start_x; j<end_x; j++)
        {
            Int32 offset = i * bytes_per_row + j * bytes_per_pixel;
            pixel_sum[0] += bitmap->pixel[offset + 0];
            pixel_sum[1] += bitmap->pixel[offset + 1];
            pixel_sum[2] += bitmap->pixel[offset + 2];
        }
    }
    pixel_ave[0] = pixel_sum[0] / (area_width * area_height);
    pixel_ave[1] = pixel_sum[1] / (area_width * area_height);
    pixel_ave[2] = pixel_sum[2] / (area_width * area_height);
    
    //quantize the pixels inside the search area to black, cyan and yellow
    for(Int32 i=start_y, y=0; i<end_y; i++, y++)
    {
        for(Int32 j=start_x, x=0; j<end_x; j++, x++)
        {
            Int32 offset = i * bytes_per_row + j * bytes_per_pixel;
            //check black pixel
            if(bitmap->pixel[offset + 0] < pixel_ave[0] && bitmap->pixel[offset + 1] < pixel_ave[1] && bitmap->pixel[offset + 2] < pixel_ave[2])
            {
                rgb[0]->pixel[y*area_width + x] = 0;
                rgb[1]->pixel[y*area_width + x] = 0;
                rgb[2]->pixel[y*area_width + x] = 0;
            }
            else if(bitmap->pixel[offset + 0] < bitmap->pixel[offset + 2])    //R < B
            {
                rgb[0]->pixel[y*area_width + x] = 0;
                rgb[1]->pixel[y*area_width + x] = 255;
                rgb[2]->pixel[y*area_width + x] = 255;
            }
            else                                                            //R > B
            {
                rgb[0]->pixel[y*area_width + x] = 255;
                rgb[1]->pixel[y*area_width + x] = 255;
                rgb[2]->pixel[y*area_width + x] = 0;
            }
        }
    }
    
    //set the core type of the expected finder pattern
    Int32 exp_type_r=0, exp_type_g=0, exp_type_b=0;
    switch(miss_fp_index)
    {
        case 0:
        case 1:
            exp_type_r = 0;
            exp_type_g = 0;
            exp_type_b = 0;
            break;
        case 2:
            exp_type_r = 255;
            exp_type_g = 255;
            exp_type_b = 0;
            break;
        case 3:
            exp_type_r = 0;
            exp_type_g = 255;
            exp_type_b = 255;
            break;
    }
    //search for the missing finder pattern
    J_Finder_Pattern* fps_miss = (J_Finder_Pattern*)calloc(MAX_FINDER_PATTERNS, sizeof(J_Finder_Pattern));
    if(fps_miss == NULL)
    {
        J_REPORT_ERROR(("Memory allocation for finder patterns failed, the missing finder pattern can not be found."))
        return;
    }
    Int32 total_finder_patterns = 0;
    Boolean done = 0;
    Int32 fp_type_count[4] = {0};
    
    for(Int32 i=0; i<area_height && done == 0; i++)
    {
        //get row
        BYTE* row_r = rgb[0]->pixel + i*rgb[0]->width;
        BYTE* row_g = rgb[1]->pixel + i*rgb[1]->width;
        BYTE* row_b = rgb[2]->pixel + i*rgb[2]->width;
        
        Int32 startx = 0;
        Int32 endx = rgb[0]->width;
        Int32 skip = 0;
        
        do
        {
            J_Finder_Pattern fp;
            
            Int32 type_r, type_g, type_b;
            Float centerx_r, centerx_g, centerx_b;
            Float module_size_r, module_size_g, module_size_b;
            Boolean finder_pattern_found = FAILURE;
            
            startx += skip;
            endx = rgb[0]->width;
            //green channel
            if(seekPatternHorizontal(row_g, &startx, &endx, &centerx_g, &module_size_g, &skip))
            {
                type_g = row_g[(Int32)(centerx_g)] > 0 ? 255 : 0;
                if(type_g != exp_type_g) continue;
                
                centerx_r = centerx_g;
                centerx_b = centerx_g;
                switch(miss_fp_index)
                {
                    case 0:
                    case 3:
                        //check blue channel for Finder Pattern UL and LL
                        if(crossCheckPatternHorizontal(rgb[2], module_size_g*2, &centerx_b, (Float)i, &module_size_b))
                        {
                            type_b = row_b[(Int32)(centerx_b)] > 0 ? 255 : 0;
                            if(type_b != exp_type_b) continue;
                            //check red channel
                            module_size_r = module_size_g;
                            Int32 core_color_in_red_channel = J_Default_Palette[FP3_CORE_COLOR * 3 + 0];
                            if(crossCheckColor(rgb[0], core_color_in_red_channel, module_size_r, 5, (Int32)centerx_r, i, 0))
                            {
                                type_r = 0;
                                finder_pattern_found = SUCCESS;
                            }
                        }
                        if(finder_pattern_found)
                        {
                            if(!checkModuleSize2(module_size_g, module_size_b)) continue;
                            fp.center.x = (centerx_g + centerx_b) / 2.0f;
                            fp.module_size = (module_size_g + module_size_b) / 2.0f;
                        }
                        break;
                    case 1:
                    case 2:
                        //check red channel for Finder Pattern UR and LR
                        if(crossCheckPatternHorizontal(rgb[0], module_size_g*2, &centerx_r, (Float)i, &module_size_r))
                        {
                            type_r = row_r[(Int32)(centerx_r)] > 0 ? 255 : 0;
                            if(type_r != exp_type_r) continue;
                            //check blue channel
                            module_size_b = module_size_g;
                            Int32 core_color_in_blue_channel = J_Default_Palette[FP2_CORE_COLOR * 3 + 2];
                            if(crossCheckColor(rgb[2], core_color_in_blue_channel, module_size_b, 5, (Int32)centerx_b, i, 0))
                            {
                                type_b = 0;
                                finder_pattern_found = SUCCESS;
                            }
                        }
                        if(finder_pattern_found)
                        {
                            if(!checkModuleSize2(module_size_r, module_size_g)) continue;
                            fp.center.x = (centerx_r + centerx_g) / 2.0f;
                            fp.module_size = (module_size_r + module_size_g) / 2.0f;
                        }
                        break;
                }
                //finder pattern candidate found
                if(finder_pattern_found)
                {
                    fp.center.y = (Float)i;
                    fp.found_count = 1;
                    fp.type = miss_fp_index;
                    //cross check
                    if( crossCheckPattern(rgb, &fp, 0) )
                    {
                        //combine the finder patterns at the same position with the same size
                        saveFinderPattern(&fp, fps_miss, &total_finder_patterns, fp_type_count);
                        if(total_finder_patterns >= (MAX_FINDER_PATTERNS - 1) )
                        {
                            done = 1;
                            break;
                        }
                    }
                }
            }
        }while(startx < rgb[0]->width && endx < rgb[0]->width);
    }
    //get the best found
    Int32 max_found = 0;
    Int32 max_index = 0;
    for(Int32 i=0; i<total_finder_patterns; i++)
    {
        if(fps_miss[i].found_count > max_found)
        {
            max_found = fps_miss[i].found_count;
            max_index = i;
        }
    }
    fps[miss_fp_index] = fps_miss[max_index];
    //recover the coordinates in bitmap
    fps[miss_fp_index].center.x += start_x;
    fps[miss_fp_index].center.y += start_y;
}

/**
 * @brief 在（整幅）图像中，透过一定的数值比例搜寻Master Symbol
 * @param bitmap 位图
 * @param ch 二值化R、G、B通道数组
 * @param mode 搜寻模式：QUICK_DETECT = 0, NORMAL_DETECT = 1, INTENSIVE_DETECT = 2
 * @return the finder pattern list | NULL
 */
J_Finder_Pattern* findMasterSymbol(Bitmap* bitmap, Bitmap* ch[], J_Detect_Mode mode) {
    // suppose the code size is minimally 1/4 image size
    // 现认为：最小的像素大小应设为1/4的图像大小
    Int32 min_module_size = ch[0]->height / (2 * MAX_SYMBOL_ROWS * MAX_MODULES);
    if (min_module_size < 1 || mode == INTENSIVE_DETECT) min_module_size = 1;
    
    // 存放最多可达500个FINDER_PATTERN数据的数组
    J_Finder_Pattern* fps = (J_Finder_Pattern *)calloc(MAX_FINDER_PATTERNS, sizeof(J_Finder_Pattern));
    if(fps == NULL) {
        J_REPORT_ERROR(("Memory allocation for finder patterns failed"))
        return NULL;
    }
    Int32 total_finder_patterns = 0;    // 总共找到的finder pattern数量
    Boolean done = 0;                   // 完成FLAG
    Int32 fp_type_count[4] = {0};       // 4中finder pattern分别共找到的数量
    
    // ---开始扫描---
    // 纵向扫描，按照最小module_size为每行的宽度逐行进行
    for(Int32 i = 0; i < ch[0]->height && done == 0; i += min_module_size) {
        // 该行的R、G、B三原色信息
        BYTE* row_r = ch[0]->pixel + i * ch[0]->width;
        BYTE* row_g = ch[1]->pixel + i * ch[1]->width;
        BYTE* row_b = ch[2]->pixel + i * ch[2]->width;
        // 扫描的参数设置
        Int32 startx = 0;
        Int32 endx = ch[0]->width;
        Int32 skip = 0;
        // --该do-while循环扫描这条线上的所有FP的中心及储存，直到这条线被扫描完--
        do {
            Int32 type_r = 0, type_g, type_b = 0;
            Float centerx_r, centerx_g, centerx_b;
            Float module_size_r, module_size_g, module_size_b;
            Boolean finder_pattern1_found = FAILURE;
            Boolean finder_pattern2_found = FAILURE;
            
            startx += skip;
            endx = ch[0]->width;
            // 尝试在水平方向上查询某条线的疑似的第一个FP（Finder Pattern UL），
            // 如果一次查询失败就设置skip值*在下一回*继续查找，如果本行查询结束则跳到下一条线，
            // 如果在green channel上查询成功则继续在蓝、红channel上进行查找。
            if (seekPatternHorizontal(row_g, &startx, &endx, &centerx_g, &module_size_g, &skip)) {
                // 经过seek，centrex_g被设置为当前找到的pattern的中心
                // 二值化图片还是存在一些灰色的象素点的，要归一化
                type_g = row_g[(Int32)(centerx_g)] > 0 ? 255 : 0;
                
                // R、B的centre设为跟FP_UL的一致，分别去找LL及UR
                centerx_r = centerx_g;
                centerx_b = centerx_g;
                // check blue channel for Finder Pattern UL and LL
                if (crossCheckPatternHorizontal(ch[2], module_size_g * 2,   // Maximum module size
                                                &centerx_b, (Float)i, &module_size_b)) {
                    type_b = row_b[(Int32)(centerx_b)] > 0 ? 255 : 0;
                    // check red channel 是否 correspond
                    module_size_r = module_size_g;
                    Int32 core_color_in_red_channel = J_Default_Palette[FP3_CORE_COLOR * 3 + 0];
                    if(crossCheckColor(ch[0], core_color_in_red_channel, module_size_r, 5, (Int32)centerx_r, i, 0)) {
                        type_r = 0;
                        finder_pattern1_found = SUCCESS;
                    }
                }
                // check red channel for Finder Pattern UR and LR
                else if (crossCheckPatternHorizontal(ch[0], module_size_g*2, &centerx_r,
                                                     (Float)i, &module_size_r)) {
                    type_r = row_r[(Int32)(centerx_r)] > 0 ? 255 : 0;
                    // check blue channel
                    module_size_b = module_size_g;
                    Int32 core_color_in_blue_channel = J_Default_Palette[FP2_CORE_COLOR * 3 + 2];
                    if(crossCheckColor(ch[2], core_color_in_blue_channel, module_size_b, 5, (Int32)centerx_b, i, 0)) {
                        type_b = 0;
                        finder_pattern2_found = SUCCESS;
                    }
                }
                // finder pattern candidate found
                if(finder_pattern1_found || finder_pattern2_found) {
                    J_Finder_Pattern fp;
                    fp.center.y = (Float)i;
                    fp.found_count = 1;
                    if(finder_pattern1_found) {
                        if(!checkModuleSize2(module_size_g, module_size_b)) continue;
                        fp.center.x = (centerx_g + centerx_b) / 2.0f;
                        fp.module_size = (module_size_g + module_size_b) / 2.0f;
                        if( type_r == J_Default_Palette[FP0_CORE_COLOR * 3] &&
                           type_g == J_Default_Palette[FP0_CORE_COLOR * 3 + 1] &&
                           type_b == J_Default_Palette[FP0_CORE_COLOR * 3 + 2]) {
                            fp.type = FP0;    // candidate for fp0
                        }
                        else if(type_r == J_Default_Palette[FP3_CORE_COLOR * 3] &&
                                type_g == J_Default_Palette[FP3_CORE_COLOR * 3 + 1] &&
                                type_b == J_Default_Palette[FP3_CORE_COLOR * 3 + 2]) {
                            fp.type = FP3;    // candidate for fp3
                        }
                        else {
                            continue;        //invalid type
                        }
                    }
                    else if(finder_pattern2_found) {
                        if(!checkModuleSize2(module_size_r, module_size_g)) continue;
                        fp.center.x = (centerx_r + centerx_g) / 2.0f;
                        fp.module_size = (module_size_r + module_size_g) / 2.0f;
                        if(type_r == J_Default_Palette[FP1_CORE_COLOR * 3] &&
                           type_g == J_Default_Palette[FP1_CORE_COLOR * 3 + 1] &&
                           type_b == J_Default_Palette[FP1_CORE_COLOR * 3 + 2]) {
                            fp.type = FP1;    //candidate for fp1
                        }
                        else if(type_r == J_Default_Palette[FP2_CORE_COLOR * 3] &&
                                type_g == J_Default_Palette[FP2_CORE_COLOR * 3 + 1] &&
                                type_b == J_Default_Palette[FP2_CORE_COLOR * 3 + 2]) {
                            fp.type = FP2;    //candidate for fp2
                        }
                        else {
                            continue;        //invalid type
                        }
                    }
                    // cross check
                    if( crossCheckPattern(ch, &fp, 0) ) {
                        saveFinderPattern(&fp, fps, &total_finder_patterns, fp_type_count);
                        if(total_finder_patterns >= (MAX_FINDER_PATTERNS - 1) ) {
                            done = 1;
                            break;
                        }
                    }
                }
            }
        } while(startx < ch[0]->width && endx < ch[0]->width);
    }
    
    //if only FP0 and FP1 are found or only FP2 and FP3 are found, do vertical-scan
    if( (fp_type_count[0] != 0 && fp_type_count[1] !=0 && fp_type_count[2] == 0 && fp_type_count[3] == 0) ||
       (fp_type_count[0] == 0 && fp_type_count[1] ==0 && fp_type_count[2] != 0 && fp_type_count[3] != 0) ) {
        scanPatternVertical(ch, min_module_size, fps, fp_type_count, &total_finder_patterns);
        // set dir to 2?
    }
    
    
#if TEST_MODE
    //output all found finder patterns
    JAB_REPORT_INFO(("Total found: %d", total_finder_patterns))
    for(jab_int32 i=0; i<total_finder_patterns; i++) {
        JAB_REPORT_INFO(("x:%6.1f\ty:%6.1f\tsize:%.2f\tcnt:%d\ttype:%d\tdir:%d", fps[i].center.x, fps[i].center.y, fps[i].module_size, fps[i].found_count, fps[i].type, fps[i].direction))
    }
    drawFoundFinderPatterns(fps, total_finder_patterns, 0x00ff00);
    saveImage(test_mode_bitmap, "jab_detector_result_fp.png");
#endif
    
    //if less than 3 finder patterns are found, detection fails
    if(total_finder_patterns < 3) {
        J_REPORT_INFO(("Too few FP found in this frame. Skip."))
        free(fps);
        return NULL;
    }
    
    //set finder patterns' direction
    for(Int32 i=0; i<total_finder_patterns; i++) {
        fps[i].direction = fps[i].direction >=0 ? 1 : -1;
    }
    //select best patterns
    Int32 missing_fp_count = selectBestPatterns(fps, total_finder_patterns, fp_type_count);
    
    //if more than one finder patterns are missing, detection fails
    if(missing_fp_count > 1) {
        J_REPORT_INFO(("Missing too many FPs in this frame! Skip."))
        free(fps);
        return NULL;
    }

    //if only one finder pattern is missing, 尝试去推测missing的那一个
    if(missing_fp_count == 1) {
        // estimate the missing finder pattern
        Int32 miss_fp = 0;
        if(fps[0].found_count == 0) {
            miss_fp = 0;
            Float ave_size_fp23 = (fps[2].module_size + fps[3].module_size) / 2.0f;
            Float ave_size_fp13 = (fps[1].module_size + fps[3].module_size) / 2.0f;
            fps[0].center.x = (fps[3].center.x - fps[2].center.x) / ave_size_fp23 * ave_size_fp13 + fps[1].center.x;
            fps[0].center.y = (fps[3].center.y - fps[2].center.y) / ave_size_fp23 * ave_size_fp13 + fps[1].center.y;
            fps[0].type = FP0;
            fps[0].found_count = 1;
            fps[0].direction = -fps[1].direction;
            fps[0].module_size = (fps[1].module_size + fps[2].module_size + fps[3].module_size) / 3.0f;
        }
        else if(fps[1].found_count == 0) {
            miss_fp = 1;
            Float ave_size_fp23 = (fps[2].module_size + fps[3].module_size) / 2.0f;
            Float ave_size_fp02 = (fps[0].module_size + fps[2].module_size) / 2.0f;
            fps[1].center.x = (fps[2].center.x - fps[3].center.x) / ave_size_fp23 * ave_size_fp02 + fps[0].center.x;
            fps[1].center.y = (fps[2].center.y - fps[3].center.y) / ave_size_fp23 * ave_size_fp02 + fps[0].center.y;
            fps[1].type = FP1;
            fps[1].found_count = 1;
            fps[1].direction = -fps[0].direction;
            fps[1].module_size = (fps[0].module_size + fps[2].module_size + fps[3].module_size) / 3.0f;
        }
        else if(fps[2].found_count == 0) {
            miss_fp = 2;
            Float ave_size_fp01 = (fps[0].module_size + fps[1].module_size) / 2.0f;
            Float ave_size_fp13 = (fps[1].module_size + fps[3].module_size) / 2.0f;
            fps[2].center.x = (fps[1].center.x - fps[0].center.x) / ave_size_fp01 * ave_size_fp13 + fps[3].center.x;
            fps[2].center.y = (fps[1].center.y - fps[0].center.y) / ave_size_fp01 * ave_size_fp13 + fps[3].center.y;
            fps[2].type = FP2;
            fps[2].found_count = 1;
            fps[2].direction = fps[3].direction;
            fps[2].module_size = (fps[0].module_size + fps[1].module_size + fps[3].module_size) / 3.0f;
        }
        else if(fps[3].found_count == 0) {
            miss_fp = 3;
            Float ave_size_fp01 = (fps[0].module_size + fps[1].module_size) / 2.0f;
            Float ave_size_fp02 = (fps[0].module_size + fps[2].module_size) / 2.0f;
            fps[3].center.x = (fps[0].center.x - fps[1].center.x) / ave_size_fp01 * ave_size_fp02 + fps[2].center.x;
            fps[3].center.y = (fps[0].center.y - fps[1].center.y) / ave_size_fp01 * ave_size_fp02 + fps[2].center.y;
            fps[3].type = FP3;
            fps[3].found_count = 1;
            fps[3].direction = fps[2].direction;
            fps[3].module_size = (fps[0].module_size + fps[1].module_size + fps[2].module_size) / 3.0f;
        }
        // check the position of the missed finder pattern
        if(fps[miss_fp].center.x < 0 || fps[miss_fp].center.x > ch[0]->width - 1 ||
           fps[miss_fp].center.y < 0 || fps[miss_fp].center.y > ch[0]->height - 1) {
            J_REPORT_ERROR(("Finder pattern %d out of image", miss_fp))
            free(fps);
            return NULL;
        }
        
        //try to find the missing finder pattern by a local search at the estimated position
#if TEST_MODE
        JAB_REPORT_INFO(("Trying to confirm the missing finder pattern by a local search..."))
#endif
        seekMissingFinderPattern(bitmap, fps, miss_fp);
    }
#if TEST_MODE
    //output the final selected 4 patterns
    JAB_REPORT_INFO(("Final patterns:"))
    for(jab_int32 i=0; i<4; i++) {
        JAB_REPORT_INFO(("x:%6.1f\ty:%6.1f\tsize:%.2f\tcnt:%d\ttype:%d\tdir:%d", fps[i].center.x, fps[i].center.y, fps[i].module_size, fps[i].found_count, fps[i].type, fps[i].direction))
    }
    drawFoundFinderPatterns(fps, 4, 0xff0000);
    saveImage(test_mode_bitmap, "jab_detector_result_fp.png");
#endif
    return fps;
}

/**
 * @brief Crosscheck the alignment pattern candidate in diagonal direction
 * @param image the image bitmap
 * @param ap_type the alignment pattern type
 * @param module_size_max the maximal allowed module size
 * @param center the alignment pattern center
 * @param dir the alignment pattern direction
 * @return the y coordinate of the diagonal scanline center | -1 if failed
 */
Float crossCheckPatternDiagonalAP(Bitmap* image, Int32 ap_type, Int32 module_size_max, J_POINT center, Int32* dir)
{
    Int32 offset_x, offset_y;
    Boolean fix_dir = 0;
    //if the direction is given, ONLY check the given direction
    if((*dir) != 0)
    {
        if((*dir) > 0)
        {
            offset_x = -1;
            offset_y = -1;
            *dir = 1;
        }
        else
        {
            offset_x = 1;
            offset_y = -1;
            *dir = -1;
        }
        fix_dir = 1;
    }
    else
    {
        //for ap0 and ap1, first check the diagonal at +45 degree
        if(ap_type == AP0 || ap_type == AP1)
        {
            offset_x = -1;
            offset_y = -1;
            *dir = 1;
        }
        //for ap2 and ap3, first check the diagonal at -45 degree
        else
        {
            offset_x = 1;
            offset_y = -1;
            *dir = -1;
        }
    }
    
    Boolean flag = 0;
    Int32 try_count = 0;
    do
    {
        flag = 0;
        try_count++;
        
        Int32 i, state_index;
        Int32 state_count[3] = {0};
        Int32 startx = (Int32)center.x;
        Int32 starty = (Int32)center.y;
        
        state_count[1]++;
        for(i=1, state_index=0; i<=starty && i<=startx && state_index<=1; i++)
        {
            if( image->pixel[(starty + i*offset_y)*image->width + (startx + i*offset_x)] == image->pixel[(starty + (i-1)*offset_y)*image->width + (startx + (i-1)*offset_x)] )
            {
                state_count[1 - state_index]++;
            }
            else
            {
                if(state_index >0 && state_count[1 - state_index] < 3)
                {
                    state_count[1 - (state_index-1)] += state_count[1 - state_index];
                    state_count[1 - state_index] = 0;
                    state_index--;
                    state_count[1 - state_index]++;
                }
                else
                {
                    state_index++;
                    if(state_index > 1) break;
                    else state_count[1 - state_index]++;
                }
            }
        }
        if(state_index < 1)
        {
            if(try_count == 1)
            {
                flag = 1;
                offset_x = -offset_x;
                *dir = -(*dir);
            }
            else
                return -1;
        }
        
        if(!flag)
        {
            for(i=1, state_index=0; (starty+i)<image->height && (startx+i)<image->width && state_index<=1; i++)
            {
                if( image->pixel[(starty - i*offset_y)*image->width + (startx - i*offset_x)] == image->pixel[(starty - (i-1)*offset_y)*image->width + (startx - (i-1)*offset_x)] )
                {
                    state_count[1 + state_index]++;
                }
                else
                {
                    if(state_index >0 && state_count[1 + state_index] < 3)
                    {
                        state_count[1 + (state_index-1)] += state_count[1 + state_index];
                        state_count[1 + state_index] = 0;
                        state_index--;
                        state_count[1 + state_index]++;
                    }
                    else
                    {
                        state_index++;
                        if(state_index > 1) break;
                        else state_count[1 + state_index]++;
                    }
                }
            }
            if(state_index < 1)
            {
                if(try_count == 1)
                {
                    flag = 1;
                    offset_x = -offset_x;
                    *dir = -(*dir);
                }
                else
                    return -1;
            }
        }
        
        if(!flag)
        {
            //check module size, if it is too big, assume it is a false positive
            if(state_count[1] < module_size_max && state_count[0] > 0.5 * state_count[1] && state_count[2] > 0.5 * state_count[1])
            {
                Float new_centery = (Float)(starty + i - state_count[2]) - (Float)state_count[1] / 2.0f;
                return new_centery;
            }
            else
            {
                flag = 1;
                offset_x = - offset_x;
                *dir = -(*dir);
            }
        }
    }
    while(flag && try_count < 2 && !fix_dir);
    
    return -1;
}

/**
 * @brief Crosscheck the alignment pattern candidate in vertical direction
 * @param image the image bitmap
 * @param center the alignment pattern center
 * @param module_size_max the maximal allowed module size
 * @param module_size the module size in vertical direction
 * @return the y coordinate of the vertical scanline center | -1 if failed
 */
Float crossCheckPatternVerticalAP(Bitmap* image, J_POINT center, Int32 module_size_max, Float* module_size)
{
    Int32 state_count[3] = {0};
    Int32 centerx = (Int32)center.x;
    Int32 centery = (Int32)center.y;
    Int32 i, state_index;
    
    state_count[1]++;
    for(i=1, state_index=0; i<=centery && state_index<=1; i++)
    {
        if( image->pixel[(centery-i)*image->width + centerx] == image->pixel[(centery-(i-1))*image->width + centerx] )
        {
            state_count[1 - state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[1 - state_index] < 3)
            {
                state_count[1 - (state_index-1)] += state_count[1 - state_index];
                state_count[1 - state_index] = 0;
                state_index--;
                state_count[1 - state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > 1) break;
                else state_count[1 - state_index]++;
            }
        }
    }
    if(state_index < 1)
        return -1;
    
    for(i=1, state_index=0; (centery+i)<image->height && state_index<=1; i++)
    {
        if( image->pixel[(centery+i)*image->width + centerx] == image->pixel[(centery+(i-1))*image->width + centerx] )
        {
            state_count[1 + state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[1 + state_index] < 3)
            {
                state_count[1 + (state_index-1)] += state_count[1 + state_index];
                state_count[1 + state_index] = 0;
                state_index--;
                state_count[1 + state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > 1) break;
                else state_count[1 + state_index]++;
            }
        }
    }
    if(state_index < 1)
        return -1;
    
    //check module size, if it is too big, assume it is a false positive
    if(state_count[1] < module_size_max && state_count[0] > 0.5 * state_count[1] && state_count[2] > 0.5 * state_count[1])
    {
        *module_size = state_count[1];
        Float new_centery = (Float)(centery + i - state_count[2]) - (Float)state_count[1] / 2.0f;
        return new_centery;
    }
    return -1;
}

/**
 * @brief Crosscheck the alignment pattern candidate in horizontal direction
 * @param row the bitmap row
 * @param channel the color channel
 * @param startx the start position
 * @param endx the end position
 * @param centerx the center of the candidate scanline
 * @param ap_type the alignment pattern type
 * @param module_size_max the maximal allowed module size
 * @param module_size the module size in horizontal direction
 * @return the x coordinate of the horizontal scanline center | -1 if failed
 */
Float crossCheckPatternHorizontalAP(BYTE* row, Int32 channel, Int32 startx, Int32 endx, Int32 centerx, Int32 ap_type, Float module_size_max, Float* module_size)
{
    Int32 core_color = -1;
    switch(ap_type)
    {
        case AP0:
            core_color = J_Default_Palette[AP0_CORE_COLOR * 3 + channel];
            break;
        case AP1:
            core_color = J_Default_Palette[AP1_CORE_COLOR * 3 + channel];
            break;
        case AP2:
            core_color = J_Default_Palette[AP2_CORE_COLOR * 3 + channel];
            break;
        case AP3:
            core_color = J_Default_Palette[AP3_CORE_COLOR * 3 + channel];
            break;
        case APX:
            core_color = J_Default_Palette[APX_CORE_COLOR * 3 + channel];
            break;
    }
    if(row[centerx] != core_color)
        return -1;
    
    Int32 state_count[3] = {0};
    Int32 i, state_index;
    
    state_count[1]++;
    for(i=1, state_index=0; (centerx-i)>=startx && state_index<=1; i++)
    {
        if( row[centerx - i] == row[centerx - (i-1)] )
        {
            state_count[1 - state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[1 - state_index] < 3)
            {
                state_count[1 - (state_index-1)] += state_count[1 - state_index];
                state_count[1 - state_index] = 0;
                state_index--;
                state_count[1 - state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > 1) break;
                else state_count[1 - state_index]++;
            }
        }
    }
    if(state_index < 1)
        return -1;
    
    for(i=1, state_index=0; (centerx+i)<=endx && state_index<=1; i++)
    {
        if( row[centerx + i] == row[centerx + (i-1)] )
        {
            state_count[1 + state_index]++;
        }
        else
        {
            if(state_index > 0 && state_count[1 + state_index] < 3)
            {
                state_count[1 + (state_index-1)] += state_count[1 + state_index];
                state_count[1 + state_index] = 0;
                state_index--;
                state_count[1 + state_index]++;
            }
            else
            {
                state_index++;
                if(state_index > 1) break;
                else state_count[1 + state_index]++;
            }
        }
    }
    if(state_index < 1)
        return -1;
    
    //check module size, if it is too big, assume it is a false positive
    if(state_count[1] < module_size_max && state_count[0] > 0.5 * state_count[1] && state_count[2] > 0.5 * state_count[1])
    {
        *module_size = state_count[1];
        Float new_centerx = (Float)(centerx + i - state_count[2]) - (Float)state_count[1] / 2.0f;
        return new_centerx;
    }
    return -1;
}

/**
 * @brief Crosscheck the alignment pattern
 * @param ch the binarized color channels of the image
 * @param y the y coordinate of the horizontal scanline
 * @param minx the minimal coordinate of the horizontal scanline
 * @param maxx the maximal coordinate of the horizontal scanline
 * @param cur_x the start position of the horizontal scanline
 * @param ap_type the alignment pattern type
 * @param max_module_size the maximal allowed module size
 * @param centerx the x coordinate of the alignment pattern center
 * @param centery the y coordinate of the alignment pattern center
 * @param module_size the module size
 * @param dir the alignment pattern direction
 * @return SUCCESS | FAILURE
 */
Boolean crossCheckPatternAP(Bitmap* ch[], Int32 y, Int32 minx, Int32 maxx, Int32 cur_x, Int32 ap_type, Float max_module_size, Float* centerx, Float* centery, Float* module_size, Int32* dir)
{
    //get row
    BYTE* row_r = ch[0]->pixel + y*ch[0]->width;
    BYTE* row_b = ch[2]->pixel + y*ch[2]->width;
    
    Float l_centerx[3] = {0.0f};
    Float l_centery[3] = {0.0f};
    Float l_module_size_h[3] = {0.0f};
    Float l_module_size_v[3] = {0.0f};
    
    //check r channel horizontally
    l_centerx[0] = crossCheckPatternHorizontalAP(row_r, 0, minx, maxx, cur_x, ap_type, max_module_size, &l_module_size_h[0]);
    if(l_centerx[0] < 0) return FAILURE;
    //check b channel horizontally
    l_centerx[2] = crossCheckPatternHorizontalAP(row_b, 2, minx, maxx, (Int32)l_centerx[0], ap_type, max_module_size, &l_module_size_h[2]);
    if(l_centerx[2] < 0) return FAILURE;
    //calculate the center and the module size
    J_POINT center;
    center.x = (l_centerx[0] + l_centerx[2]) / 2.0f;
    center.y = y;
    (*module_size) = (l_module_size_h[0] + l_module_size_h[2]) / 2.0f;
    //check g channel horizontally
    Int32 core_color_in_green_channel = -1;
    switch(ap_type)
    {
        case AP0:
            core_color_in_green_channel = J_Default_Palette[AP0_CORE_COLOR * 3 + 1];
            break;
        case AP1:
            core_color_in_green_channel = J_Default_Palette[AP1_CORE_COLOR * 3 + 1];
            break;
        case AP2:
            core_color_in_green_channel = J_Default_Palette[AP2_CORE_COLOR * 3 + 1];
            break;
        case AP3:
            core_color_in_green_channel = J_Default_Palette[AP3_CORE_COLOR * 3 + 1];
            break;
        case APX:
            core_color_in_green_channel = J_Default_Palette[APX_CORE_COLOR * 3 + 1];
            break;
    }
    if(!crossCheckColor(ch[1], core_color_in_green_channel, (Int32)(*module_size), 3, (Int32)center.x, (Int32)center.y, 0))
        return FAILURE;
    
    //check r channel vertically
    l_centery[0] = crossCheckPatternVerticalAP(ch[0], center, max_module_size, &l_module_size_v[0]);
    if(l_centery[0] < 0) return FAILURE;
    //again horizontally
    row_r = ch[0]->pixel + ((Int32)l_centery[0])*ch[0]->width;
    l_centerx[0] = crossCheckPatternHorizontalAP(row_r, 0, minx, maxx, center.x, ap_type, max_module_size, &l_module_size_h[0]);
    if(l_centerx[0] < 0) return FAILURE;
    
    //check b channel vertically
    l_centery[2] = crossCheckPatternVerticalAP(ch[2], center, max_module_size, &l_module_size_v[2]);
    if(l_centery[2] < 0) return FAILURE;
    //again horizontally
    row_b = ch[2]->pixel + ((Int32)l_centery[2])*ch[2]->width;
    l_centerx[2] = crossCheckPatternHorizontalAP(row_b, 2, minx, maxx, center.x, ap_type, max_module_size, &l_module_size_h[2]);
    if(l_centerx[2] < 0) return FAILURE;
    
    //update the center and the module size
    (*module_size) = (l_module_size_h[0] + l_module_size_h[2] + l_module_size_v[0] + l_module_size_v[2]) / 4.0f;
    (*centerx) = (l_centerx[0] + l_centerx[2]) / 2.0f;
    (*centery) = (l_centery[0] + l_centery[2]) / 2.0f;
    center.x = (*centerx);
    center.y = (*centery);
    
    //check g channel vertically
    if(!crossCheckColor(ch[1], core_color_in_green_channel, (Int32)(*module_size), 3, (Int32)center.x, (Int32)center.y, 1))
        return FAILURE;
    
    //diagonal check
    Int32 l_dir[3] = {0};
    if(crossCheckPatternDiagonalAP(ch[0], ap_type, (*module_size)*2, center, &l_dir[0]) < 0) return FAILURE;
    if(crossCheckPatternDiagonalAP(ch[2], ap_type, (*module_size)*2, center, &l_dir[2]) < 0) return FAILURE;
    if(!crossCheckColor(ch[1], core_color_in_green_channel, (Int32)(*module_size), 3, (Int32)center.x, (Int32)center.y, 2))
        return FAILURE;
    (*dir) = (l_dir[0] + l_dir[2]) > 0 ? 1 : -1;
    
    return SUCCESS;
}

/**
 * @brief Find alignment pattern around a given position
 * @param ch the binarized color channels of the image
 * @param x the x coordinate of the given position
 * @param y the y coordinate of the given position
 * @param module_size the module size
 * @param ap_type the alignment pattern type
 * @return the found alignment pattern
 */
J_Alignment_Pattern findAlignmentPattern(Bitmap* ch[], Float x, Float y, Float module_size, Int32 ap_type)
{
    J_Alignment_Pattern ap;
    ap.type = -1;
    ap.found_count = 0;
    
    //determine the core color of r channel
    Int32 core_color_r = -1;
    switch(ap_type)
    {
        case AP0:
            core_color_r = J_Default_Palette[AP0_CORE_COLOR * 3];
            break;
        case AP1:
            core_color_r = J_Default_Palette[AP1_CORE_COLOR * 3];
            break;
        case AP2:
            core_color_r = J_Default_Palette[AP2_CORE_COLOR * 3];
            break;
        case AP3:
            core_color_r = J_Default_Palette[AP3_CORE_COLOR * 3];
            break;
        case APX:
            core_color_r = J_Default_Palette[APX_CORE_COLOR * 3];
            break;
    }
    
    //define search range
    Int32 radius = (Int32)(4 * module_size);
    Int32 radius_max = 4 * radius;
    
    for(; radius<radius_max; radius<<=1)
    {
        J_Alignment_Pattern* aps = (J_Alignment_Pattern*)calloc(MAX_FINDER_PATTERNS, sizeof(J_Alignment_Pattern));
        if(aps == NULL)
        {
            J_REPORT_ERROR(("Memory allocation for alignment patterns failed"))
            return ap;
        }
        
        Int32 startx = (Int32)MAX(0, x - radius);
        Int32 starty = (Int32)MAX(0, y - radius);
        Int32 endx = (Int32)MIN(ch[0]->width - 1, x + radius);
        Int32 endy = (Int32)MIN(ch[0]->height - 1, y + radius);
        if(endx - startx < 3 * module_size || endy - starty < 3 * module_size) continue;
        
        Int32 counter = 0;
        for(Int32 k=starty; k<endy; k++)
        {
            //search from middle outwards
            Int32 kk = k - starty;
            Int32 i = y + ((kk & 0x01) == 0 ? (kk + 1) / 2 : -((kk + 1) / 2));
            if(i < starty)
                continue;
            else if(i > endy)
                continue;
            
            //get r channel row
            BYTE* row_r = ch[0]->pixel + i*ch[0]->width;
            
            Float ap_module_size = 0.0, centerx = 0.0, centery = 0.0;
            Int32 ap_dir = 0;
            
            Boolean ap_found = 0;
            Int32 dir = -1;
            Int32 left_tmpx = x;
            Int32 right_tmpx = x;
            while((left_tmpx > startx || right_tmpx < endx) && !ap_found)
            {
                if(dir < 0)    //go to left
                {
                    while(row_r[left_tmpx] != core_color_r && left_tmpx > startx)
                    {
                        left_tmpx--;
                    }
                    if(left_tmpx <= startx)
                    {
                        dir = -dir;
                        continue;
                    }
                    ap_found = crossCheckPatternAP(ch, i, startx, endx, left_tmpx, ap_type, module_size*2, &centerx, &centery, &ap_module_size, &ap_dir);
                    while(row_r[left_tmpx] == core_color_r && left_tmpx > startx)
                    {
                        left_tmpx--;
                    }
                    dir = -dir;
                }
                else //go to right
                {
                    while(row_r[right_tmpx] == core_color_r && right_tmpx < endx)
                    {
                        right_tmpx++;
                    }
                    while(row_r[right_tmpx] != core_color_r && right_tmpx < endx)
                    {
                        right_tmpx++;
                    }
                    if(right_tmpx >= endx)
                    {
                        dir = -dir;
                        continue;
                    }
                    ap_found = crossCheckPatternAP(ch, i, startx, endx, right_tmpx, ap_type, module_size*2, &centerx, &centery, &ap_module_size, &ap_dir);
                    while(row_r[right_tmpx] == core_color_r && right_tmpx < endx)
                    {
                        right_tmpx++;
                    }
                    dir = -dir;
                }
            }
            
            if(!ap_found) continue;
            
            ap.center.x = centerx;
            ap.center.y = centery;
            ap.module_size = ap_module_size;
            ap.direction = ap_dir;
            ap.type = ap_type;
            ap.found_count = 1;
            
            Int32 index = saveAlignmentPattern(&ap, aps, &counter);
            if(index >= 0) //if found twice, done!
            {
                ap = aps[index];
                free(aps);
                return ap;
            }
        }
        free(aps);
    }
    ap.type = -1;
    ap.found_count = 0;
    return ap;
}

/**
 * @brief Find a docked slave symbol
 * @param bitmap the image bitmap
 * @param ch the binarized color channels of the image
 * @param host_symbol the host symbol
 * @param slave_symbol the slave symbol
 * @param docked_position the docked position
 * @return SUCCESS | FAILURE
 */
Boolean findSlaveSymbol(Bitmap* bitmap, Bitmap* ch[], J_Decoded_Symbol* host_symbol, J_Decoded_Symbol* slave_symbol, Int32 docked_position)
{
    J_Alignment_Pattern* aps = (J_Alignment_Pattern*)calloc(4, sizeof(J_Alignment_Pattern));
    if(aps == NULL)
    {
        J_REPORT_ERROR(("Memory allocation for alignment patterns failed"))
        return FAILURE;
    }
    
    //docked horizontally
    Float distx01 = host_symbol->pattern_positions[1].x - host_symbol->pattern_positions[0].x;
    Float disty01 = host_symbol->pattern_positions[1].y - host_symbol->pattern_positions[0].y;
    Float distx32 = host_symbol->pattern_positions[2].x - host_symbol->pattern_positions[3].x;
    Float disty32 = host_symbol->pattern_positions[2].y - host_symbol->pattern_positions[3].y;
    //docked vertically
    Float distx03 = host_symbol->pattern_positions[3].x - host_symbol->pattern_positions[0].x;
    Float disty03 = host_symbol->pattern_positions[3].y - host_symbol->pattern_positions[0].y;
    Float distx12 = host_symbol->pattern_positions[2].x - host_symbol->pattern_positions[1].x;
    Float disty12 = host_symbol->pattern_positions[2].y - host_symbol->pattern_positions[1].y;
    
    Float alpha1 = 0.0f, alpha2 = 0.0f;
    Int32 sign = 1;
    Int32 ap1 = 0, ap2 = 0, ap3 = 0, ap4 = 0, hp1 = 0, hp2 = 0;
    switch(docked_position)
    {
        case 3:
            /*
             fp[0] ... fp[1] .. ap[0] ... ap[1]
             .         .         .         .
             .  master .         .  slave  .
             .         .         .         .
             fp[3] ... fp[2] .. ap[3] ... ap[2]
             */
            alpha1 = atan2(disty01, distx01);
            alpha2 = atan2(disty32, distx32);
            sign = 1;
            ap1 = AP0;    //ap[0]
            ap2 = AP3;    //ap[3]
            ap3 = AP1;    //ap[1]
            ap4 = AP2;    //ap[2]
            hp1 = FP1;    //fp[1] or ap[1] of the host
            hp2 = FP2;    //fp[2] or ap[2] of the host
            slave_symbol->host_position = 2;
            break;
        case 2:
            /*
             ap[0] ... ap[1] .. fp[0] ... fp[1]
             .         .        .         .
             .  slave  .        .  master .
             .         .        .         .
             ap[3] ... ap[2] .. fp[3] ... fp[2]
             */
            alpha1 = atan2(disty32, distx32);
            alpha2 = atan2(disty01, distx01);
            sign = -1;
            ap1 = AP2;    //ap[2]
            ap2 = AP1;    //ap[1]
            ap3 = AP3;    //ap[3]
            ap4 = AP0;    //ap[0]
            hp1 = FP3;    //fp[3] or ap[3] of the host
            hp2 = FP0;    //fp[0] or ap[0] of the host
            slave_symbol->host_position = 3;
            break;
        case 1:
            /*
             fp[0] ... fp[1]
             .         .
             .  master .
             .         .
             fp[3] ... fp[2]
             .            .
             .         .
             ap[0] ... ap[1]
             .         .
             .  slave  .
             .         .
             ap[3] ... ap[2]
             */
            alpha1 = atan2(disty12, distx12);
            alpha2 = atan2(disty03, distx03);
            sign = 1;
            ap1 = AP1;    //ap[1]
            ap2 = AP0;    //ap[0]
            ap3 = AP2;    //ap[2]
            ap4 = AP3;    //ap[3]
            hp1 = FP2;    //fp[2] or ap[2] of the host
            hp2 = FP3;    //fp[3] or ap[3] of the host
            slave_symbol->host_position = 0;
            break;
        case 0:
            /*
             ap[0] ... ap[1]
             .         .
             .  slave  .
             .         .
             ap[3] ... ap[2]
             .            .
             .         .
             fp[0] ... fp[1]
             .         .
             .  master .
             .         .
             fp[3] ... fp[2]
             */
            alpha1 = atan2(disty03, distx03);
            alpha2 = atan2(disty12, distx12);
            sign = -1;
            ap1 = AP3;    //ap[3]
            ap2 = AP2;    //ap[2]
            ap3 = AP0;    //ap[0]
            ap4 = AP1;    //ap[1]
            hp1 = FP0;    //fp[0] or ap[0] of the host
            hp2 = FP1;    //fp[1] or ap[1] of the host
            slave_symbol->host_position = 1;
            break;
    }
    
    //calculate the coordinate of ap1
    aps[ap1].center.x = host_symbol->pattern_positions[hp1].x + sign * 7 * host_symbol->module_size * cos(alpha1);
    aps[ap1].center.y = host_symbol->pattern_positions[hp1].y + sign * 7 * host_symbol->module_size * sin(alpha1);
    //find the alignment pattern around ap1
    aps[ap1] = findAlignmentPattern(ch, aps[ap1].center.x, aps[ap1].center.y, host_symbol->module_size, ap1);
    if(aps[ap1].found_count == 0)
    {
        free(aps);
        J_REPORT_ERROR(("The first alignment pattern in slave symbol %d not found", slave_symbol->index))
        return FAILURE;
    }
    //calculate the coordinate of ap2
    aps[ap2].center.x = host_symbol->pattern_positions[hp2].x + sign * 7 * host_symbol->module_size * cos(alpha2);
    aps[ap2].center.y = host_symbol->pattern_positions[hp2].y + sign * 7 * host_symbol->module_size * sin(alpha2);
    //find alignment pattern around ap2
    aps[ap2] = findAlignmentPattern(ch, aps[ap2].center.x, aps[ap2].center.y, host_symbol->module_size, ap2);
    if(aps[ap2].found_count == 0)
    {
        free(aps);
        J_REPORT_ERROR(("The second alignment pattern in slave symbol %d not found", slave_symbol->index))
        return FAILURE;
    }
    
    //get slave symbol side size from its metadata
    slave_symbol->side_size.x = VERSION2SIZE(slave_symbol->metadata.side_version.x);
    slave_symbol->side_size.y = VERSION2SIZE(slave_symbol->metadata.side_version.y);
    
    //estimate the module size in the slave symbol
    if(docked_position == 3 || docked_position == 2)
    {
        slave_symbol->module_size = DIST(aps[ap1].center.x, aps[ap1].center.y, aps[ap2].center.x, aps[ap2].center.y) / (slave_symbol->side_size.y - 7);
    }
    if(docked_position == 1 || docked_position == 0)
    {
        slave_symbol->module_size = DIST(aps[ap1].center.x, aps[ap1].center.y, aps[ap2].center.x, aps[ap2].center.y) / (slave_symbol->side_size.x - 7);
    }
    
    //calculate the coordinate of ap3
    aps[ap3].center.x = aps[ap1].center.x + sign * (slave_symbol->side_size.x - 7) * slave_symbol->module_size * cos(alpha1);
    aps[ap3].center.y = aps[ap1].center.y + sign * (slave_symbol->side_size.y - 7) * slave_symbol->module_size * sin(alpha1);
    //find alignment pattern around ap3
    aps[ap3] = findAlignmentPattern(ch, aps[ap3].center.x, aps[ap3].center.y, slave_symbol->module_size, ap3);
    //calculate the coordinate of ap4
    aps[ap4].center.x = aps[ap2].center.x + sign * (slave_symbol->side_size.x - 7) * slave_symbol->module_size * cos(alpha2);
    aps[ap4].center.y = aps[ap2].center.y + sign * (slave_symbol->side_size.y - 7) * slave_symbol->module_size * sin(alpha2);
    //find alignment pattern around ap4
    aps[ap4] = findAlignmentPattern(ch, aps[ap4].center.x, aps[ap4].center.y, slave_symbol->module_size, ap4);
    
    //if neither ap3 nor ap4 is found, failed
    if(aps[ap3].found_count == 0 && aps[ap4].found_count == 0)
    {
        free(aps);
        return FAILURE;
    }
    //if only 3 aps are found, try anyway by estimating the coordinate of the fourth one
    if(aps[ap3].found_count == 0)
    {
        Float ave_size_ap24 = (aps[ap2].module_size + aps[ap4].module_size) / 2.0f;
        Float ave_size_ap14 = (aps[ap1].module_size + aps[ap4].module_size) / 2.0f;
        aps[ap3].center.x = (aps[ap4].center.x - aps[ap2].center.x) / ave_size_ap24 * ave_size_ap14 + aps[ap1].center.x;
        aps[ap3].center.y = (aps[ap4].center.y - aps[ap2].center.y) / ave_size_ap24 * ave_size_ap14 + aps[ap1].center.y;
        aps[ap3].type = ap3;
        aps[ap3].found_count = 1;
        aps[ap3].module_size = (aps[ap1].module_size + aps[ap2].module_size + aps[ap4].module_size) / 3.0f;
        if(aps[ap3].center.x > bitmap->width - 1 || aps[ap3].center.y > bitmap->height - 1)
        {
            J_REPORT_ERROR(("Alignment pattern %d out of image", ap3))
            free(aps);
            return FAILURE;
        }
    }
    if(aps[ap4].found_count == 0)
    {
        Float ave_size_ap13 = (aps[ap1].module_size + aps[ap3].module_size) / 2.0f;
        Float ave_size_ap23 = (aps[ap2].module_size + aps[ap3].module_size) / 2.0f;
        aps[ap4].center.x = (aps[ap3].center.x - aps[ap1].center.x) / ave_size_ap13 * ave_size_ap23 + aps[ap2].center.x;
        aps[ap4].center.y = (aps[ap3].center.y - aps[ap1].center.y) / ave_size_ap13 * ave_size_ap23 + aps[ap2].center.y;
        aps[ap4].type = ap4;
        aps[ap4].found_count = 1;
        aps[ap4].module_size = (aps[ap1].module_size + aps[ap1].module_size + aps[ap3].module_size) / 3.0f;
        if(aps[ap4].center.x > bitmap->width - 1 || aps[ap4].center.y > bitmap->height - 1)
        {
            J_REPORT_ERROR(("Alignment pattern %d out of image", ap4))
            free(aps);
            return FAILURE;
        }
    }
    
    //save the coordinates of aps into the slave symbol
    slave_symbol->pattern_positions[ap1] = aps[ap1].center;
    slave_symbol->pattern_positions[ap2] = aps[ap2].center;
    slave_symbol->pattern_positions[ap3] = aps[ap3].center;
    slave_symbol->pattern_positions[ap4] = aps[ap4].center;
    slave_symbol->module_size = (aps[ap1].module_size + aps[ap2].module_size + aps[ap3].module_size + aps[ap4].module_size) / 4.0f;
    
#if TEST_MODE
    JAB_REPORT_INFO(("Found alignment patterns in slave symbol %d:", slave_symbol->index))
    for(jab_int32 i=0; i<4; i++)
    {
        JAB_REPORT_INFO(("x: %6.1f\ty: %6.1f\tcount: %d\ttype: %d\tsize: %.2f", aps[i].center.x, aps[i].center.y, aps[i].found_count, aps[i].type, aps[i].module_size))
    }
    drawFoundFinderPatterns((jab_finder_pattern*)aps, 4, 0xff0000);
    saveImage(test_mode_bitmap, "jab_detector_result_slave.png");
#endif
    
    free(aps);
    return SUCCESS;
}

/**
 * @brief Get the nearest valid side size to a given size
 * @param size the input size
 * @param flag the flag indicating the magnitude of error
 * @return the nearest valid side size | -1: invalid side size
 */
Int32 getSideSize(Int32 size, Int32* flag)
{
    *flag = 1;
    switch (size & 0x03) //mod 4
    {
        case 0:
            size++;
            break;
        case 2:
            size--;
            break;
        case 3:
            size += 2;    //error is bigger than 1, guess the next version and try anyway
            *flag = 0;
            break;
    }
    if(size < 21)
    {
        size = -1;
        *flag = -1;
    }
    else if(size > 145)
    {
        size = -1;
        *flag = -1;
    }
    return size;
}

/**
 * @brief Choose the side size according to the detection reliability
 * @param size1 the first side size
 * @param flag1 the detection flag of the first size
 * @param size2 the second side size
 * @param flag2 the detection flag of the second size
 * @return the chosen side size
 */
Int32 chooseSideSize(Int32 size1, Int32 flag1, Int32 size2, Int32 flag2)
{
    if(flag1 == -1 && flag2 == -1)
    {
        return -1;
    }
    else if(flag1 == flag2)
        return MAX(size1, size2);
    else
    {
        if(flag1 > flag2)
            return size1;
        else
            return size2;
    }
}

/**
 * @brief Calculate the number of modules between two finder/alignment patterns
 * @param fp1 the first finder/alignment pattern
 * @param fp2 the second finder/alignment pattern
 * @return the number of modules
 */
Int32 calculateModuleNumber(J_Finder_Pattern fp1, J_Finder_Pattern fp2)
{
    Float dist = DIST(fp1.center.x, fp1.center.y, fp2.center.x, fp2.center.y);
    //the angle between the scanline and the connection between finder/alignment patterns
    Float cos_theta = MAX(fabs(fp2.center.x - fp1.center.x), fabs(fp2.center.y - fp1.center.y)) / dist;
    Float mean = (fp1.module_size + fp2.module_size)*cos_theta / 2.0f;
    Int32 number = (Int32)(dist / mean + 0.5f);
    return number;
}

/**
 * @brief Calculate the side sizes of master symbol
 * @param fps the finder patterns
 * @return the horizontal and vertical side sizes
 */
Vector2D calculateSideSize(J_Finder_Pattern* fps)
{
    /* finder pattern type layout
     0    1
     3    2
     */
    Vector2D side_size = {-1, -1};
    Int32 flag1, flag2;
    
    //calculate the horizontal side size
    Int32 size_x_top = calculateModuleNumber(fps[0], fps[1]) + 7;
    size_x_top = getSideSize(size_x_top, &flag1);
    Int32 size_x_bottom = calculateModuleNumber(fps[3], fps[2]) + 7;
    size_x_bottom = getSideSize(size_x_bottom, &flag2);
    
    side_size.x = chooseSideSize(size_x_top, flag1, size_x_bottom, flag2);
    
    //calculate the vertical side size
    Int32 size_y_left = calculateModuleNumber(fps[0], fps[3]) + 7;
    size_y_left = getSideSize(size_y_left, &flag1);
    Int32 size_y_right = calculateModuleNumber(fps[1], fps[2]) + 7;
    size_y_right = getSideSize(size_y_right, &flag2);
    
    side_size.y = chooseSideSize(size_y_left, flag1, size_y_right, flag2);
    
    return side_size;
}

/**
 * @brief Get the nearest valid position of the first alignment pattern
 * @param pos the input position
 * @return the nearest valid position | -1: invalid position
 */
Int32 getFirstAPPos(Int32 pos)
{
    switch (pos % 3)
    {
        case 0:
            pos--;
            break;
        case 1:
            pos++;
            break;
    }
    if(pos < 14 || pos > 26)
        pos = -1;
    return pos;
}

/**
 * @brief Detect the first alignment pattern between two finder patterns
 * @param ch the binarized color channels of the image
 * @param side_version the side version
 * @param fp1 the first finder pattern
 * @param fp2 the second finder pattern
 * @return the position of the found alignment pattern | FAILURE: if not found
 */
Int32 detectFirstAP(Bitmap* ch[], Int32 side_version, J_Finder_Pattern fp1, J_Finder_Pattern fp2)
{
    J_Alignment_Pattern ap;
    //direction: from FP1 to FP2
    Float distx = fp2.center.x - fp1.center.x;
    Float disty = fp2.center.y - fp1.center.y;
    Float alpha = atan2(disty, distx);
    
    Int32 next_version = side_version;
    Int32 dir = 1;
    Int32 up = 0, down = 0;
    do
    {
        //distance to FP1
        Float distance = fp1.module_size * (j_ap_pos[next_version-1][1] - j_ap_pos[next_version-1][0]);
        //estimate the coordinate of the first AP
        ap.center.x = fp1.center.x + distance * cos(alpha);
        ap.center.y = fp1.center.y + distance * sin(alpha);
        ap.module_size = fp1.module_size;
        //detect AP
        ap.found_count = 0;
        ap = findAlignmentPattern(ch, ap.center.x, ap.center.y, ap.module_size, APX);
        if(ap.found_count > 0)
        {
            Int32 pos = getFirstAPPos(4 + calculateModuleNumber(fp1, ap));
            if(pos > 0)
                return pos;
        }
        
        //try next version
        dir = -dir;
        if(dir == -1)
        {
            up++;
            next_version = up * dir + side_version;
            if(next_version < 6 || next_version > 32)
            {
                dir = -dir;
                up--;
                down++;
                next_version = down * dir + side_version;
            }
        }
        else
        {
            down++;
            next_version = down * dir + side_version;
            if(next_version < 6 || next_version > 32)
            {
                dir = -dir;
                down--;
                up++;
                next_version = up * dir + side_version;
            }
        }
    }while((up+down) < 5);
    
    return FAILURE;
    
}

/**
 * @brief Confirm the side version by alignment pattern's positions
 * @param side_version the side version
 * @return the confirmed side version | FAILURE: if can not be confirmed
 */
Int32 confirmSideVersion(Int32 side_version, Int32 first_ap_pos)
{
    if(first_ap_pos <= 0)
    {
#if TEST_MODE
        J_REPORT_ERROR(("Invalid position of the first AP."))
#endif
        return FAILURE;
    }
    
    Int32 v = side_version;
    Int32 k = 1, sign = -1;
    Boolean flag = 0;
    do
    {
        if(first_ap_pos == j_ap_pos[v-1][1])
        {
            flag = 1;
            break;
        }
        v = side_version + sign*k;
        if(sign > 0) k++;
        sign = -sign;
    }while(v>=6 && v<=32);
    
    if(flag) return v;
    else     return FAILURE;
}

/**
 * @brief Confirm the symbol size by alignment patterns
 * @param ch the binarized color channels of the image
 * @param fps the finder patterns
 * @param symbol the symbol
 * @return SUCCESS | FAILURE
 */
Int32 confirmSymbolSize(Bitmap* ch[], J_Finder_Pattern* fps, J_Decoded_Symbol* symbol) {
    Int32 first_ap_pos;
    
    //side version x: scan the line between FP0 and FP1
    first_ap_pos = detectFirstAP(ch, symbol->metadata.side_version.x, fps[0], fps[1]);
#if TEST_MODE
    JAB_REPORT_INFO(("The position of the first AP between FP0 and FP1 is %d", first_ap_pos))
#endif // TEST_MODE
    Int32 side_version_x = confirmSideVersion(symbol->metadata.side_version.x, first_ap_pos);
    if(side_version_x == 0) //if failed, try the line between FP3 and FP2
    {
        first_ap_pos = detectFirstAP(ch, symbol->metadata.side_version.x, fps[3], fps[2]);
#if TEST_MODE
        JAB_REPORT_INFO(("The position of the first AP between FP3 and FP2 is %d", first_ap_pos))
#endif // TEST_MODE
        side_version_x = confirmSideVersion(symbol->metadata.side_version.x, first_ap_pos);
        if(side_version_x == 0)
        {
#if TEST_MODE
            J_REPORT_ERROR(("Confirming side version x failed."))
#endif
            return FAILURE;
        }
    }
    symbol->metadata.side_version.x = side_version_x;
    symbol->side_size.x = VERSION2SIZE(side_version_x);
    
    //side version y: scan the line between FP0 and FP3
    first_ap_pos = detectFirstAP(ch, symbol->metadata.side_version.y, fps[0], fps[3]);
#if TEST_MODE
    JAB_REPORT_INFO(("The position of the first AP between FP0 and FP3 is %d", first_ap_pos))
#endif // TEST_MODE
    Int32 side_version_y = confirmSideVersion(symbol->metadata.side_version.y, first_ap_pos);
    if(side_version_y == 0) //if failed, try the line between FP1 and FP2
    {
        first_ap_pos = detectFirstAP(ch, symbol->metadata.side_version.y, fps[1], fps[2]);
#if TEST_MODE
        JAB_REPORT_INFO(("The position of the first AP between FP1 and FP2 is %d", first_ap_pos))
#endif // TEST_MODE
        side_version_y = confirmSideVersion(symbol->metadata.side_version.y, first_ap_pos);
        if(side_version_y == 0)
        {
#if TEST_MODE
            J_REPORT_ERROR(("Confirming side version y failed."))
#endif
            return FAILURE;
        }
    }
    symbol->metadata.side_version.y = side_version_y;
    symbol->side_size.y = VERSION2SIZE(side_version_y);
    
    return SUCCESS;
}



/**
 探测Master Symbol及采样
 
 @param bitmap 图像的Bitmap
 @param ch 图像的3通道
 @return 返回的处理后Symbol信息
 */
J_Found_Symbol* detectMaster( Bitmap* bitmap, Bitmap* ch[]) {
    // 用来存master symbol的4个finder pattern（如有）
    J_Finder_Pattern* fps;
    fps = findMasterSymbol(bitmap, ch, INTENSIVE_DETECT);
    if(fps == NULL) {
		//J_REPORT_INFO(("Can't find symbol in this frame. Skip."))
		// 改进于20/3/2020
		free(bitmap);
		for (int i = 0; i < 3; i++) free(ch[i]);
        return NULL;
    }
    // 计算Master Symbol的每条边(Side)的Size
    Vector2D side_size = calculateSideSize(fps);
    if(side_size.x == -1 || side_size.y == -1) {
        J_REPORT_ERROR(("Calculating side size failed"))
        free(fps);
		// 改进于20/3/2020
		free(bitmap);
		for (int i = 0; i < 3; i++) free(ch[i]);
        return NULL;
    }
    // 计算透视变形矩阵(calculate perspective transform matrix)
    J_Perspective_Transform* pt = getPerspectiveTransform(fps[0].center, fps[1].center, fps[2].center,fps[3].center, side_size);
    if(pt == NULL) { // 如果计算错误
        free(fps);
		// 改进于20/3/2020
		free(bitmap);
		for (int i = 0; i < 3; i++) free(ch[i]);
        return NULL;
    }
    
    // 对bitmap的变形矩阵pt进行[第一次采样（sample.c）]
    Bitmap* matrix = sampleSymbol(bitmap, pt, side_size);
    free(pt);
    // 若采样失败
    if(matrix == NULL) {
        J_REPORT_ERROR(("Sampling master symbol failed"))
        free(fps);
		// 改进于20/3/2020
		free(bitmap);
		for (int i = 0; i < 3; i++) free(ch[i]);
        return NULL;
    }
    
    J_Found_Symbol* master_symbol = (J_Found_Symbol*)malloc(sizeof(J_Found_Symbol));
    if (!master_symbol) {
        J_REPORT_ERROR(("Memory of master symbol found data allocation failed"))
        free(fps);
		// 改进于20/3/2020
		free(bitmap);
		for (int i = 0; i < 3; i++) free(ch[i]);
        return NULL;
    }
    
    // 保存对master symbol的侦测数据
    master_symbol->index = 0;
    master_symbol->host_index = 0;
    master_symbol->side_size = side_size;
    // 采取finder pattern平均的module size作为整幅图的module size
    master_symbol->module_size = (fps[0].module_size + fps[1].module_size + fps[2].module_size + fps[3].module_size) / 4.0f;
    master_symbol->fps = fps;
    master_symbol->sampled_img = matrix;
    
    // 改进于19/3/2020 
    master_symbol->bitmap = bitmap;
    master_symbol->ch[0] = ch[0];
    master_symbol->ch[1] = ch[1];
    master_symbol->ch[2] = ch[2];
    
    return master_symbol;
}


