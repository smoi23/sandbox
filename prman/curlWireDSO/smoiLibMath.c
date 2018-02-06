/*
 * smoiLibMath.c
 *
 *  Created on: 10.12.2011
 *      Author: andreas schuster smoi
 *      Version: 0.2.0
 */
#ifndef PI
#define PI 3.14159265
#endif
// std
#include <math.h>
#include <stdio.h>
// smoi
#include "smoiLibMath.h"
// pixar
#include <ri.h>



int NormalizeVector(RtVector ioV) {

	float length;

	length = sqrt(ioV[0]*ioV[0] + ioV[1]*ioV[1] + ioV[2]*ioV[2]);
	ioV[0] = ioV[0] / length;
	ioV[1] = ioV[1] / length;
	ioV[2] = ioV[2] / length;

	return(0);
}

int CrossProductNormalized(RtVector a, RtVector b, RtVector v) {

	float length;

	v[0] = a[1]*b[2] - a[2]*b[1];
	v[1] = a[2]*b[0] - a[0]*b[2];
	v[2] = a[0]*b[1] - a[1]*b[0];

	length = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	v[0] = v[0] / length;
	v[1] = v[1] / length;
	v[2] = v[2] / length;

	return(0);
}


float DotProduct(RtVector v1, RtVector v2){

	return(v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);

}

float DotProductNormalize(RtVector v1, RtVector v2){

	return ( v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] ) / sqrt(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]) / sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);

}

double Angle2Vectors(RtVector v1, RtVector v2) {

	return acos((double)( v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] ) / sqrt(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]) / sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]));

}

float Magnitude(RtVector v) {

	return(sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]));

}

double MinValue(double a, double b) {
	return a < b ? a : b;
}

double MaxValue(double a, double b) {
	return a > b ? a : b;
}

int RotateAxis(float angle, RtVector axis, RtVector ioV) {

	// left handed (beim right handed alle vorzeichen ausser auf der diagonalen vertauschen)

	RtMatrix R;

	// lefthanded
	R[0][0] = (1-cos(angle))*axis[0]*axis[0]+cos(angle);			R[0][1] = (1-cos(angle))*axis[0]*axis[1]-sin(angle)*axis[2];	R[0][2] = (1-cos(angle))*axis[0]*axis[2]+sin(angle)*axis[1];	R[0][3] = 0;
	R[1][0] = (1-cos(angle))*axis[0]*axis[1]+sin(angle)*axis[2];	R[1][1] = (1-cos(angle))*axis[1]*axis[1]+cos(angle);			R[1][2] = (1-cos(angle))*axis[1]*axis[2]-sin(angle)*axis[0];	R[1][3] = 0;
	R[2][0] = (1-cos(angle))*axis[0]*axis[2]-sin(angle)*axis[1];	R[2][1] = (1-cos(angle))*axis[1]*axis[2]+sin(angle)*axis[0];	R[2][2] = (1-cos(angle))*axis[2]*axis[2]+cos(angle);			R[2][3] = 0;
	// vierte zeile brauchmer nich
	/*
	// righthanded
	R[0][0] = (1-cos(angle))*axis[0]*axis[0]+cos(angle);			R[0][1] = (1-cos(angle))*axis[0]*axis[1]+sin(angle)*axis[2];	R[0][2] = (1-cos(angle))*axis[0]*axis[2]-sin(angle)*axis[1];	R[0][3] = 0;
	R[1][0] = (1-cos(angle))*axis[0]*axis[1]-sin(angle)*axis[2];	R[1][1] = (1-cos(angle))*axis[1]*axis[1]+cos(angle);			R[1][2] = (1-cos(angle))*axis[1]*axis[2]+sin(angle)*axis[0];	R[1][3] = 0;
	R[2][0] = (1-cos(angle))*axis[0]*axis[1]+sin(angle)*axis[1];	R[2][1] = (1-cos(angle))*axis[1]*axis[2]-sin(angle)*axis[0];	R[2][2] = (1-cos(angle))*axis[2]*axis[2]+cos(angle);			R[2][3] = 0;
	// vierte zeile brauchmer nich
*/
	MatrixPreMulPoint(R, ioV);

	return(0);
}


int MatrixPreMulPoint(RtMatrix M, RtPoint v){

	RtPoint		v2;

	v2[0] = M[0][0] * v[0] + M[1][0] * v[1] + M[2][0] * v[2];
	v2[1] = M[0][1] * v[0] + M[1][1] * v[1] + M[2][1] * v[2];
	v2[2] = M[0][2] * v[0] + M[1][2] * v[1] + M[2][2] * v[2];

	v[0] = v2[0];
	v[1] = v2[1];
	v[2] = v2[2];

	return(0);

} // End MatrixMulVector


/* B-Spline stuff */
int GetBSplinePoint(float t, int *tVector, int polynomsN, int countK, int numCVs, RtPoint *CVs, RtPoint ioP) {

	int i;
	float w;

	ioP[0] = ioP[1] = ioP[2] = 0;

	for (i = 0; i < numCVs; i++) {

		//nurbs
		// BSpline
		w = Polynom_N(tVector, polynomsN, i, countK, t);

		ioP[0] = ioP[0]+w*CVs[i][0];
		ioP[1] = ioP[1]+w*CVs[i][1];
		ioP[2] = ioP[2]+w*CVs[i][2];

	}

	return(0);

} // End GetBSplinePoint


float Polynom_N(int *tVector, int polynomsN, int i, int k, float t) {

	float weight = 0.0;

	if (i > polynomsN) {
		return weight;
	}

	if (k == 1) {

		if (tVector[i] <= t && t < tVector[i + 1]) { // < vs <= beim zweiten !?!?
			weight = 1.0;
		}

	} else {

		if ((tVector[i + k - 1] - tVector[i]) != 0) {
			weight += (t - tVector[i]) / (tVector[i + k - 1] - tVector[i]) * Polynom_N(tVector, polynomsN, i, k - 1, t);
		}

		if ((tVector[i + k] - tVector[i + 1]) !=0) {
			weight += (tVector[i + k] - t) / (tVector[i + k] - tVector[i + 1]) * Polynom_N(tVector, polynomsN, i + 1, k - 1, t);
		}

	}

	return weight;

} //  End Polynom_N

/* Helix stuff */
int GetHelixPoint(float t, float r, float h, RtVector ioP) {

	ioP[0] = h * t / 2 / PI; //r * cos(t);
	ioP[1] = r * cos(t); // h * t / 2 / PI;
	ioP[2] = r * sin(t);

	return(0);

} // End GetHelixPoint


/* circle stuff */
int GetCirclePoint(float angle, float r, RtVector v) {

	v[0] = 0;
	v[1] = r*cos(angle);
	v[2] = r*sin(angle);

	return(0);

} // End GetCirclePoint





