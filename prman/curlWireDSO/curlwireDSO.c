/*
 * curlwireDSO.c
 *
 *  Created on: 12.12.2011
 *      Author: andreas schuster smoi
 *      Version: 0.3.1
 */
#ifndef PI
#define PI 3.14159265
#endif
// standard
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// pixar
#include <ri.h>
#include <rx.h>
// smoi
#include "smoiLibMath.h"
// declarations
typedef struct _curledWireData {
	RtFloat 	openHead;   // unused
	RtFloat		openEnd;	// unused
	RtFloat		curl;
	RtInt 		numWire;
	RtFloat		width;
	RtFloat		subwidth;
	RtInt		style;
	RtInt		stepCount;
	RtInt 		numCVs;
	RtPoint		*CVs;
} curledWireData;

typedef curledWireData *curledWireDataP;


// pixar
RtPointer ConvertParameters(RtString paramstr);
RtVoid Subdivide(RtPointer data, RtFloat detail);
RtVoid Free(RtPointer data);

RtPointer ConvertParameters(RtString paramstr){

	char *tok, *p;
	int i;

	curledWireDataP myData;
	myData = (curledWireDataP)malloc(sizeof(curledWireData));

//printf("alloca myData: %p\n", myData);
//printf("sizeof(curledWireData): %lu\n", sizeof(curledWireData));

    // convert the string to a params
    tok = strtok_r(paramstr," ", &p);
    myData->openHead = (RtFloat)atof(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->openEnd = (RtFloat)atof(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->curl = (RtFloat)atof(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->numWire = (RtInt)atoi(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->width = (RtFloat)atof(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->subwidth = (RtFloat)atof(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->style = (RtInt)atoi(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->stepCount = (RtInt)atoi(tok);
    tok = strtok_r(NULL, " ", &p);
    myData->numCVs = (RtInt)atoi(tok);

    myData->CVs = (RtPoint *)calloc(myData->numCVs, sizeof(RtPoint));

    for (i=0; i<myData->numCVs; i++){
    	tok = strtok_r(NULL, " ", &p);
    	myData->CVs[i][0] = atof(tok);
    	tok = strtok_r(NULL, " ", &p);
    	myData->CVs[i][1] = atof(tok);
    	tok = strtok_r(NULL, " ", &p);
    	myData->CVs[i][2] = atof(tok);
    }

    // return  pointer
	return (RtPointer)myData;
}


RtVoid Subdivide(RtPointer data, RtFloat detail) {

	curledWireDataP myData;
	int i,j,k;
	RtPoint point, point2, pointCV[1];
	RtVector tangent, right, up, forward, helixV, cross, tangent2, face, uplocal, rightlocal, forwardlocal;
	double alpha, beta, gamma, gammaY, uplocalangle, dotProductUp;
	float curlpower, subangle;
	RtColor farbe;
	myData = (curledWireDataP)data;

	// init curve properties
	if (myData->curl < 0) myData->curl = 0;
	curlpower = 1+myData->curl;
	subangle = 2*PI/(float)myData->numWire;

	// b-spline declare
	int 	countK;			// einflussanzahl
	int 	polynomsN;		// anzahl polynome,kurvenabschnitte
	int 	Stepcount = myData->stepCount;
	float 	t, tEnd, tStepsize;
	RtPoint HelixCVs[myData->numWire*Stepcount];
	int 	counter = 0;
	int		counter2 = 0;
	RtInt	nv[myData->numWire];

	for (i=0; i<myData->numWire; i++) {
		nv[i] = Stepcount;
	}

	// patch stuff
	int nu = 5;
	RtVector *circle;
	RtVector circleV;
	RtPoint pts[Stepcount*nu];
	circle = (RtVector *)malloc(nu*sizeof(RtVector));
	// einen kreis bereit halten / store a cirlce
	for (i=0; i<nu; i++) {
		GetCirclePoint(2*PI/nu*i, (myData->subwidth/2), circle[i]);
	}

	// b-spline init
	countK = 4; //  same as maya 3 cubic!!;
	polynomsN = myData->numCVs-1;
	int tVector[polynomsN+countK+1]; // knotvektor
	// knotenvektor erstellen
	for (j = 0; j < (polynomsN+countK+1); j++) {
		if (j < countK) {
			tVector[j] = 0;
		} else if (j <= polynomsN) {
			tVector[j] = j - countK + 1;
		} else {
			tVector[j] = polynomsN - countK + 2;
		}
	}
	tEnd = tVector[polynomsN+countK]; // t goes from 0 to tEnd
	tEnd = tEnd - 0.0001;
	tStepsize = tEnd/Stepcount;

	// init axis
	right[0] 	= 1; right[1] 	= 0; right[2] 	= 0;
	up[0] 		= 0; up[1] 		= 1; up[2] 		= 0;
	forward[0] 	= 0; forward[1] = 0; forward[2] = 1;
	// local axis wich could be rotate to
	uplocal[0] 		= 0; uplocal[1] 	 = 1; uplocal[2] 	  = 0;
	rightlocal[0] 	= 1; rightlocal[1] 	 = rightlocal[2] 	  = 0;
	forwardlocal[0] = 0; forwardlocal[1] = 0; forwardlocal[2] = 1;


	///////////////
	// End Setup //
	///////////////

	// compute
	// choose style (patches/points)
	if (myData->style == 0){ // patches
		// set basis matrix
		RiBasis(RiBSplineBasis, 1, RiBSplineBasis, 1); //RiBasis(RiCatmullRomBasis, 1, RiCatmullRomBasis, 1);
		// --> for each subwire
		for (j=0; j<myData->numWire; j++) {

			counter2 = 0;
			for (i=0; i<Stepcount; i++) {
				// increase
				t = i * tStepsize;
				// get point on curve at t
				GetBSplinePoint(t, tVector, polynomsN, countK, myData->numCVs, myData->CVs, point);
				GetBSplinePoint((t+tStepsize*0.01), tVector, polynomsN, countK, myData->numCVs, myData->CVs, point2);

				tangent[0] = point2[0] - point[0];
				tangent[0] *= 1.0001; // --> != right
				tangent[1] = point2[1] - point[1];
				tangent[1] *= 1.0001; // --> != up
				tangent[2] = point2[2] - point[2];
				tangent[2] *= 1.0001; // --> != forward
				NormalizeVector(tangent);

				// rotate the local coordination system
				rightlocal[0] 	= 1; rightlocal[1] 	= 0; rightlocal[2] 	= 0;
				uplocal[0] 		= 0; uplocal[1] 	= 1; uplocal[2] 	= 0;
				forwardlocal[0] = 0; forwardlocal[1] = 0; forwardlocal[2] = 1;
				tangent2[0] = tangent[0]; tangent2[1] = tangent[1]; tangent2[2] = 0;
				NormalizeVector(tangent2);
				dotProductUp = DotProduct(up, tangent2);
				uplocalangle = asin(dotProductUp);
				uplocalangle *= -1.0;
				// rotate the local axis around forward
				RotateAxis(-1*uplocalangle, forward, uplocal);
				RotateAxis(-1*uplocalangle, forward, rightlocal);
				// nun ist das Koordinatensystem was im folgendem benutzt wird (XYZlocal) gedreht
				tangent2[0] = tangent[0]; tangent2[1] = 0; tangent2[2] = tangent[2];
				RotateAxis(-1*uplocalangle, forward, tangent2);
				NormalizeVector(tangent2);
				// angle to rotate around up-axis
				beta = Angle2Vectors(rightlocal, tangent2);
				// check direction and correct the angle
				if (DotProduct(tangent, forwardlocal)<0) {
					beta *= -1;
				}

				// angle to rotate around face-axis
				face[0] = rightlocal[0];	face[1] = rightlocal[1];	face[2] = rightlocal[2];
				RotateAxis(beta, uplocal, face);
				alpha = Angle2Vectors(face, tangent);
				alpha *= -1;
				// cross vector
				if (fabs(alpha) > 0.01) {
					CrossProductNormalized(face, tangent, cross);
				} else {
					alpha = 0;
					cross[0] = forwardlocal[0]; cross[1] = forwardlocal[1]; cross[2] = forwardlocal[2];
				}

				// calculate helix point at t with h=0 because height will move along the input curve
				GetHelixPoint(t*curlpower+j*subangle, myData->width, 0, helixV);
				// nach neuem up
				RotateAxis(-1*uplocalangle, forward, helixV);
				// rotate the vector
				RotateAxis(beta, uplocal, helixV);
				RotateAxis(alpha, cross, helixV);

				// nun ist der kreis in orthogonal zur kurve ausgerichtet (und muss noch in Richtung gedrehtes subwire ausgerichtet werden)
				// calculate rotation in direction of subwire
				gamma = PI/2*(1-1/curlpower) * sin(t*curlpower+j*subangle);
				gammaY = PI/2*(1-1/curlpower) * cos(t*curlpower+j*subangle);

				// move every circle vector
				for (k=0; k<nu; k++){
					circleV[0] = circle[k][0]; circleV[1] = circle[k][1]; circleV[2] = circle[k][2];
					RotateAxis(-1*uplocalangle, forward, circleV);
  					RotateAxis(gammaY, uplocal, circleV);
 					RotateAxis(gamma, forwardlocal, circleV);
					// die anderen rotationen ausfuehren
					RotateAxis(beta, uplocal, circleV);
					RotateAxis(alpha, cross, circleV);
					// position on curve + curled vector + circle vector (the curled vector points to the middle of subwire and the circle vector shape the cylinder)
					pts[counter2][0] = point[0] + helixV[0] + circleV[0] ;
					pts[counter2][1] = point[1] + helixV[1] + circleV[1];
					pts[counter2][2] = point[2] + helixV[2] + circleV[2];
					counter2++;
				} // End for

			} // End for (i=0; i<=Stepcount; i++)

			// draw a wire
			RiPatchMesh("bicubic", nu, "periodic", Stepcount, "nonperiodic", "P", (RtPointer)pts, RI_NULL);

		} //End for (j=0; j<myData->numWire; j++) // End one wire

	} else { // End if (STYLE == 0) // points

		for (i=0; i<Stepcount; i++) {
			// increase t
			t = i * tStepsize;
			// get point on curve at t
			GetBSplinePoint(t, tVector, polynomsN, countK, myData->numCVs, myData->CVs, point);
			GetBSplinePoint((t+tStepsize*0.01), tVector, polynomsN, countK, myData->numCVs, myData->CVs, point2);

			tangent[0] = point2[0] - point[0];
			tangent[0] *= 1.0001; // --> != right
			tangent[1] = point2[1] - point[1];
			tangent[2] = point2[2] - point[2];
			NormalizeVector(tangent);
			// angle between y-axis and tangent
			tangent2[0] = tangent[0]; tangent2[1] = 0; tangent2[2] = tangent[2];

			NormalizeVector(tangent2);
			beta = Angle2Vectors(right, tangent2);
			// correct the angle - stulle?
			if (tangent[2]<0) {
				beta *= -1;
			}
			// new stuff
			face[0] = right[0];	face[1] = right[1];	face[2] = right[2];
			RotateAxis(beta, up, face);
			alpha = Angle2Vectors(face, tangent);
			alpha *= -1;
			if ( fabs(alpha)>0.01) {
				CrossProductNormalized(face, tangent, cross);
			} else {
				alpha = 0;
				cross[0] = forward[0]; cross[1] = forward[1]; cross[2] = forward[2];
			}

			// --> for each subwire
			for (j=0; j<myData->numWire; j++) {

				// calculate helix point at t with y=0 because this direction will move along the input curve
				GetHelixPoint(t*curlpower+j*subangle, myData->width, 0, helixV);
				// rotate the vector
				RotateAxis(beta, up, helixV);
				RotateAxis(alpha, cross, helixV);
				// move and store the vector
				HelixCVs[counter][0] = point[0] + helixV[0];
				HelixCVs[counter][1] = point[1] + helixV[1];
				HelixCVs[counter][2] = point[2] + helixV[2];

				counter++;

			} // <-- end for each subwire

		} // End for t=0 // <-- repeat for next t

		// draw the subwire as points
		farbe[0] = 0.3; farbe[1] = 0.1; farbe[2]=0.77;
		RiColor(farbe);
		for (j=0; j<(myData->numWire*Stepcount); j++) {
			farbe[1] = (float)j / ((float)myData->numWire*(float)Stepcount);
			farbe[2] = 0.9 / ( (j%myData->numWire)+1 );
			RiColor(farbe);
			pointCV[0][0] = HelixCVs[j][0];
			pointCV[0][1] = HelixCVs[j][1];
			pointCV[0][2] = HelixCVs[j][2];
			RiPoints(1, "P", pointCV, "constantwidth", &myData->subwidth, RI_NULL);
		}

	} // End if else STYLE

	// free
	free(circle);
	// return
	return;

} // End Subdivide


RtVoid Free(RtPointer data){

	//printf("free:%p\n",data);
	free((curledWireDataP)data);

}



