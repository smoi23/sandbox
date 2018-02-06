/*
 * smoiLibMath.h
 *
 *  Created on: 13.12.2011
 *      Author: andreas schuster smoi
 *      Version: 0.2.0
 */

#ifndef SMOILIBMATH_H_
#define SMOILIBMATH_H_

// pixar
#include <ri.h>

int NormalizeVector(RtVector ioV);
int CrossProductNormalized(RtVector a, RtVector b, RtVector v);
float DotProduct(RtVector v1, RtVector v2);
float DotProductNormalize(RtVector v1, RtVector v2);
double Angle2Vectors(RtVector v1, RtVector v2);
float Magnitude(RtVector v);
double MinValue(double a, double b);
double MaxValue(double a, double b);
int RotateAxis(float angle, RtVector axis, RtVector ioV);
int RotateEuler(double x, double y, double z, RtVector v);
int MatrixPreMulPoint(RtMatrix M, RtPoint v);
// nurbs stuff
int GetBSplinePoint(float t, int *tVector, int polynomsN, int countK, int numCVs, RtPoint *CVs, RtPoint ioP);
float Polynom_N(int *tVector, int polynomsN, int i, int k, float t);
// helix stuff
int GetHelixPoint(float t, float r, float h, RtVector ioP);
/* cricle stuff */
int GetCirclePoint(float angle, float r, RtVector v);

#endif /* SMOILIBMATH_H_ */
