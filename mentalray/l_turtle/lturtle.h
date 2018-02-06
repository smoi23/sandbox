/** \brief geometry shader
 * lturtle.c
 *
 *  Created on: 02.10.2011
 *      Author: a.s.
 *      Version: 0.2.0 (last edit: new L-functions, cleanup)
 *  Description: iterative string interpretation.
 * 	Usage:	- copy lturtle.so in mentalray/lib
 * 			- copy lturtle.mi in mentalray/include
 * 			- create transform node and assign geometry shader
 * 			- set attributes and render with mental ray
 * 	Chars:	- F: step forward and draw
 *			- lx,ly,lz: left rotations
 *			- rx,ry,rz: right rotations
 *			- a_: store current position and direction
 *			- z_: restore position and direction
 *
 */

#ifndef LTURTLE_H_
#define LTURTLE_H_


typedef struct {
	miInteger DebugMe;
	miInteger LString;
	miScalar StartLength;
	miScalar ScaleLength;
	miScalar StartWidth;
	miScalar ScaleWidth;
	miScalar AngleX;
	miScalar AngleY;
	miScalar AngleZ;
	miInteger Depth;
	miInteger Subdivs;
	miInteger TubeType;
	miBoolean CloseCircle;
	miInteger Segments;
	miTag Type;
}myParas;


miBoolean LPredecessor(int t, miBoolean isClose, miState *state, myParas *paras);
miBoolean LProductionSuccessor_F(int t, miState *state, myParas *paras);
miBoolean LProductionSuccessor_F2(int t, miState *state, myParas *paras);
miBoolean LProductionSuccessor_X(int t, miState *state, myParas *paras);
miBoolean LProductionSuccessor_f(int t);
miBoolean ConstructBasis();
miBoolean CalculateRotationMatrices();
void DrawTriangle(float inAspect, miState *state, myParas *paras);
void DrawPyramid(miState *state, myParas *paras);
void DrawPrism(miState *state, myParas *paras);

#endif /* LTURTLE_H_ */
