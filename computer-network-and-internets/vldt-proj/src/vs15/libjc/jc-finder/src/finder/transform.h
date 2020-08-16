// transform.c    03/03/2020
// J_Code
// brief: J_Code symbol transformer

#ifndef transform_h
#define transform_h

/**
 * @brief Perspective transform
 */
typedef struct {
	Float a11;
	Float a12;
	Float a13;
	Float a21;
	Float a22;
	Float a23;
	Float a31;
	Float a32;
	Float a33;
} J_Perspective_Transform;


// Transform
extern J_Perspective_Transform* getPerspectiveTransform(J_POINT p0, J_POINT p1,
														J_POINT p2, J_POINT p3,
														Vector2D side_size);
extern J_Perspective_Transform* perspectiveTransform(  Float x0, Float y0,
                                                       Float x1, Float y1,
                                                       Float x2, Float y2,
                                                       Float x3, Float y3,
                                                       Float x0p, Float y0p,
                                                       Float x1p, Float y1p,
                                                       Float x2p, Float y2p,
                                                       Float x3p, Float y3p);
extern void warpPoints(J_Perspective_Transform* pt, J_POINT* points, Int32 length);

#endif
