// transform.c    03/03/2020
// J_Code
// brief: J_Code image transform
// 请注意：透视变换不属于本工程主要内容，故直接参考标准源码。
// original work --

/**
 * libjabcode - JABCode Encoding/Decoding Library
 *
 * Copyright 2016 by Fraunhofer SIT. All rights reserved.
 * See LICENSE file for full terms of use and distribution.
 *
 * Contact: Huajian Liu <liu@sit.fraunhofer.de>
 *          Waldemar Berchtold <waldemar.berchtold@sit.fraunhofer.de>
 *
 * @file transform.c
 * @brief Symbol transformer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jc.h>
#include "transform.h"

/**
 * @brief Calculate transformation matrix of square to quadrilateral
 * @param x0 the x coordinate of the 1st destination point
 * @param y0 the y coordinate of the 1st destination point
 * @param x1 the x coordinate of the 2nd destination point
 * @param y1 the y coordinate of the 2nd destination point
 * @param x2 the x coordinate of the 3rd destination point
 * @param y2 the y coordinate of the 3rd destination point
 * @param x3 the x coordinate of the 4th destination point
 * @param y3 the y coordinate of the 4th destination point
 * @return the transformation matrix
*/
J_Perspective_Transform* square2Quad( Float x0, Float y0,
										Float x1, Float y1,
										Float x2, Float y2,
										Float x3, Float y3)
{
	J_Perspective_Transform* pt = (J_Perspective_Transform*)malloc(sizeof(J_Perspective_Transform));
	if(pt == NULL)
	{
		J_REPORT_ERROR(("Memory allocation for perspective transform failed"))
		return NULL;
	}
	Float dx3 = x0 - x1 + x2 - x3;
	Float dy3 = y0 - y1 + y2 - y3;
	if (dx3 == 0 && dy3 == 0) {
		pt->a11 = x1 - x0;
        pt->a21 = x2 - x1;
        pt->a31 = x0;
        pt->a12 = y1 - y0;
        pt->a22 = y2 - y1;
        pt->a32 = y0;
        pt->a13 = 0;
        pt->a23 = 0;
        pt->a33 = 1;
		return pt;
	}
	else
	{
		Float dx1 = x1 - x2;
		Float dx2 = x3 - x2;
		Float dy1 = y1 - y2;
		Float dy2 = y3 - y2;
		Float denominator = dx1 * dy2 - dx2 * dy1;
		Float a13 = (dx3 * dy2 - dx2 * dy3) / denominator;
		Float a23 = (dx1 * dy3 - dx3 * dy1) / denominator;
		pt->a11 = x1 - x0 + a13 * x1;
		pt->a21 = x3 - x0 + a23 * x3;
		pt->a31 = x0;
		pt->a12 = y1 - y0 + a13 * y1;
		pt->a22 = y3 - y0 + a23 * y3;
		pt->a32 = y0;
		pt->a13 = a13;
		pt->a23 = a23;
		pt->a33 = 1;
		return pt;
	}
}

/**
 * @brief Calculate transformation matrix of quadrilateral to square
 * @param x0 the x coordinate of the 1st source point
 * @param y0 the y coordinate of the 1st source point
 * @param x1 the x coordinate of the 2nd source point
 * @param y1 the y coordinate of the 2nd source point
 * @param x2 the x coordinate of the 3rd source point
 * @param y2 the y coordinate of the 3rd source point
 * @param x3 the x coordinate of the 4th source point
 * @param y3 the y coordinate of the 4th source point
 * @return the transformation matrix
*/
J_Perspective_Transform* quad2Square( Float x0, Float y0,
										Float x1, Float y1,
										Float x2, Float y2,
										Float x3, Float y3)
{
	J_Perspective_Transform* pt = (J_Perspective_Transform*)malloc(sizeof(J_Perspective_Transform));
	if(pt == NULL)
	{
		J_REPORT_ERROR(("Memory allocation for perspective transform failed"))
		return NULL;
	}
	J_Perspective_Transform* s2q = square2Quad(x0, y0, x1, y1, x2, y2, x3, y3);
	//calculate the adjugate matrix of s2q
	pt->a11 = s2q->a22 * s2q->a33 - s2q->a23 * s2q->a32;
	pt->a21 = s2q->a23 * s2q->a31 - s2q->a21 * s2q->a33;
	pt->a31 = s2q->a21 * s2q->a32 - s2q->a22 * s2q->a31;
	pt->a12 = s2q->a13 * s2q->a32 - s2q->a12 * s2q->a33;
	pt->a22 = s2q->a11 * s2q->a33 - s2q->a13 * s2q->a31;
	pt->a32 = s2q->a12 * s2q->a31 - s2q->a11 * s2q->a32;
	pt->a13 = s2q->a12 * s2q->a23 - s2q->a13 * s2q->a22;
	pt->a23 = s2q->a13 * s2q->a21 - s2q->a11 * s2q->a23;
	pt->a33 = s2q->a11 * s2q->a22 - s2q->a12 * s2q->a21;
	free(s2q);
	return pt;
}

/**
 * @brief Calculate matrix multiplication
 * @param m1 the multiplicand
 * @param m2 the multiplier
 * @return m1 x m2
*/
J_Perspective_Transform* multiply(J_Perspective_Transform* m1, J_Perspective_Transform* m2)
{
	J_Perspective_Transform* product = (J_Perspective_Transform*)malloc(sizeof(J_Perspective_Transform));
	if(product == NULL)
	{
		J_REPORT_ERROR(("Memory allocation for perpective transform failed"))
		return NULL;
	}
	product->a11 = m1->a11 * m2->a11 + m1->a12 * m2->a21 + m1->a13 * m2->a31;
    product->a21 = m1->a21 * m2->a11 + m1->a22 * m2->a21 + m1->a23 * m2->a31;
    product->a31 = m1->a31 * m2->a11 + m1->a32 * m2->a21 + m1->a33 * m2->a31;
    product->a12 = m1->a11 * m2->a12 + m1->a12 * m2->a22 + m1->a13 * m2->a32;
    product->a22 = m1->a21 * m2->a12 + m1->a22 * m2->a22 + m1->a23 * m2->a32;
    product->a32 = m1->a31 * m2->a12 + m1->a32 * m2->a22 + m1->a33 * m2->a32;
    product->a13 = m1->a11 * m2->a13 + m1->a12 * m2->a23 + m1->a13 * m2->a33;
    product->a23 = m1->a21 * m2->a13 + m1->a22 * m2->a23 + m1->a23 * m2->a33;
    product->a33 = m1->a31 * m2->a13 + m1->a32 * m2->a23 + m1->a33 * m2->a33;
    return product;
}

/**
 * @brief Calculate transformation matrix of quadrilateral to quadrilateral
 * @param x0 the x coordinate of the 1st source point
 * @param y0 the y coordinate of the 1st source point
 * @param x1 the x coordinate of the 2nd source point
 * @param y1 the y coordinate of the 2nd source point
 * @param x2 the x coordinate of the 3rd source point
 * @param y2 the y coordinate of the 3rd source point
 * @param x3 the x coordinate of the 4th source point
 * @param y3 the y coordinate of the 4th source point
 * @param x0p the x coordinate of the 1st destination point
 * @param y0p the y coordinate of the 1st destination point
 * @param x1p the x coordinate of the 2nd destination point
 * @param y1p the y coordinate of the 2nd destination point
 * @param x2p the x coordinate of the 3rd destination point
 * @param y2p the y coordinate of the 3rd destination point
 * @param x3p the x coordinate of the 4th destination point
 * @param y3p the y coordinate of the 4th destination point
 * @return the transformation matrix
*/
J_Perspective_Transform* perspectiveTransform(Float x0, Float y0,
												Float x1, Float y1,
												Float x2, Float y2,
												Float x3, Float y3,
												Float x0p, Float y0p,
												Float x1p, Float y1p,
												Float x2p, Float y2p,
												Float x3p, Float y3p)
{
	J_Perspective_Transform* q2s = quad2Square(x0, y0, x1, y1, x2, y2, x3, y3);
	if(q2s == NULL)
	{
		return NULL;
	}
	J_Perspective_Transform* s2q = square2Quad(x0p, y0p, x1p, y1p, x2p, y2p, x3p, y3p);
	if(s2q == NULL)
	{
        free(q2s);
		return NULL;
	}
	J_Perspective_Transform* pt = multiply(q2s, s2q);
	if(pt == NULL)
	{
        free(q2s);
        free(s2q);
		return NULL;
	}
	free(q2s);
	free(s2q);
	return pt;
}

/**
 * @brief Get perspetive transformation matrix
 * @param p0 the coordinate of the 1st finder/alignment pattern
 * @param p1 the coordinate of the 2nd finder/alignment pattern
 * @param p2 the coordinate of the 3rd finder/alignment pattern
 * @param p3 the coordinate of the 4th finder/alignment pattern
 * @param side_size the side size of the symbol
 * @return the transformation matrix
*/
J_Perspective_Transform* getPerspectiveTransform(J_POINT p0,
												   J_POINT p1,
												   J_POINT p2,
												   J_POINT p3,
												   Vector2D side_size)
{
	return perspectiveTransform(3.5f, 3.5f,
								(Float)side_size.x - 3.5f, 3.5f,
								(Float)side_size.x - 3.5f, (Float)side_size.y - 3.5f,
								3.5f, (Float)side_size.y - 3.5f,
								p0.x, p0.y,
								p1.x, p1.y,
								p2.x, p2.y,
								p3.x, p3.y
							   );
}

/**
 * @brief Warp points from source image to destination image in place
 * @param pt the transformation matrix
 * @param points the source points
 * @param length the number of source points
*/
void warpPoints(J_Perspective_Transform* pt, J_POINT* points, Int32 length)
{
    for (Int32 i=0; i<length; i++) {
      Float x = points[i].x;
      Float y = points[i].y;
      Float denominator = pt->a13 * x + pt->a23 * y + pt->a33;
      points[i].x = (pt->a11 * x + pt->a21 * y + pt->a31) / denominator;
      points[i].y = (pt->a12 * x + pt->a22 * y + pt->a32) / denominator;
    }
}
