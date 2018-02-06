/** \brief geometry shader
 * lturtle.c
 *
 *  Created on: 02.10.2011
 *      Author: a.s.
 *      Version: 0.2.0 (last edit: new L-functions, cleanup)
 *  Description: iterative string interpretation.
 * 	Usage:	- copy lturtle.so in mentalray/lib
 * 			- copy lturtle.mi in mentalray/include
 * 			- create transform node and assign geometyr shader
 * 			- set attributes and render with mental ray
 * 	Chars:	- F: step forward and draw
 *			- lx,ly,lz: left rotations
 *			- rx,ry,rz: right rotations
 *			- a_: store current position and direction
 *			- z_: restore position and direction
 *
 */
#include <math.h>
#include <string.h>
#include <shader.h>
#include <geoshader.h>

#include "lturtle.h"

// production rules
#define f 				LProductionSuccessor_f(t-1);
#define F 				LProductionSuccessor_F(t-1, state, paras);
#define F2 				LProductionSuccessor_F2(t-1, state, paras);
#define X				LProductionSuccessor_X(t-1, state, paras);
// string rules (operations)
#define lz 				mi_vector_transform(&turtledir, &turtledir, R_zleft);/*mi_vector_transform(&forward, &forward, R_zleft);*/mi_vector_transform(&right, &right, R_zleft);mi_vector_transform(&up, &up, R_zleft);CalculateRotationMatrices();
#define rz 				mi_vector_transform(&turtledir, &turtledir, R_zright);/*mi_vector_transform(&forward, &forward, R_zright);*/mi_vector_transform(&right, &right, R_zright);mi_vector_transform(&up, &up, R_zright);CalculateRotationMatrices();
#define ly 				mi_vector_transform(&turtledir, &turtledir, R_yleft);mi_vector_transform(&forward, &forward, R_yleft);mi_vector_transform(&right, &right, R_yleft);/*mi_vector_transform(&up, &up, R_yleft);*/CalculateRotationMatrices();
#define ry 				mi_vector_transform(&turtledir, &turtledir, R_yright);mi_vector_transform(&forward, &forward, R_yright);mi_vector_transform(&right, &right, R_yright);/*mi_vector_transform(&up, &up, R_yright);*/CalculateRotationMatrices();
#define lx 				mi_vector_transform(&turtledir, &turtledir, R_xleft);mi_vector_transform(&forward, &forward, R_xleft);/*mi_vector_transform(&right, &right, R_xleft);*/mi_vector_transform(&up, &up, R_xleft);CalculateRotationMatrices();
#define rx 				mi_vector_transform(&turtledir, &turtledir, R_xright);mi_vector_transform(&forward, &forward, R_xright);/*mi_vector_transform(&right, &right, R_xright);*/mi_vector_transform(&up, &up, R_xright);CalculateRotationMatrices();
#define a_ 				altpos = turtlepos; altdir = turtledir; altwidth=turtlewidth; mi_matrix_copy(altxform, xformB); altforward=forward; altright=right; altup=up;
#define z_				lastturtledir=altdir; turtlepos = altpos; turtledir = altdir; turtlewidth=altwidth; lastturtlepos=turtlepos; mi_matrix_copy(xformB, altxform); forward=altforward; right=altright; up=altup; CalculateRotationMatrices();
// predefined strings
#define BRANCHES		F F lz ly a_ ly lz F ly rz rx F ly rz rx F z_ ry rz a_ ry rz F ry lz F ry lz F z_
// #define BRANCHES		F F lz a_ lz F rz F rz F z_ rz a_ rz F lz F lz F z_ // 2D
#define KOCHISLAND 		F lz F rz F rz F F lz F lz F rz F
#define KOCHLAKE		F F a_ lz F z_ F
#define KOCHLAKE2		F lz f  rz F F lz F lz F F lz F f   lz F F  rz f lz F F rz F rz F F rz F f   rz F F F
#define KOCHLAKE2_3D	F  lx F F rx  lz f  rz F F lz F lz F F lz F f   lz F  rx F F lx  F  rz f lz F F rz F rz F F rz F f   rz F F F
#define BUSH			F a_  rz F rz F z_ lz F lz F z_ z_ rx F rx F z_ lx F lx F z_ // deltaX=22, deltaZ=22

// properties
miVector lastturtledir, lastturtlepos, turtlepos, turtledir;
float startwidth, endwidth, turtlewidth, stepwidth;
float lengthFactor, widthFactor;
// rotation matrices
miMatrix R_zleft, R_zright, R_yleft, R_yright, R_xleft, R_xright;
miMatrix R90_left, R_Circle;
// current rotation matrix
miMatrix xformB;
// subdivs count, caps count
int n; //,s;
// base pyramid and prism
miVector *vPtr02;
// counter for drawn faces to get the correct index when drawing
int xcount;
// custom string and properties
char *lstring;
int selection, segmentcount, tubetype;
// basis
miVector forward, up, right;
float deltaX,deltaY,deltaZ;


static void init_obj_flags(miObject *inObj){
	inObj->visible = miTRUE;
	inObj->shadow =
	inObj->reflection =
	inObj->refraction =
	inObj->finalgather = 0x03;
}


DLLEXPORT int lturtle_version() {
	return(1);
}


DLLEXPORT miBoolean lturtle(
				miTag *result,
				miState *state,
				myParas *paras) {
	/** \brief main compute
	 *
	 */
	miObject *obj;
	miTag string_tag;
	miBoolean status;
	miBoolean close;
	/* unsued
	if (n % 2 == 0){
		n += 1;
	}
	s = (n-1)/2;
	*/
	// read attributes
	n = *mi_eval_integer(&paras->Subdivs);
	close = *mi_eval_boolean(&paras->CloseCircle);
	segmentcount = *mi_eval_boolean(&paras->Segments);
	if (segmentcount<3)
		segmentcount=3;
	selection = *mi_eval_integer(&paras->LString);
	tubetype = *mi_eval_integer(&paras->TubeType);
	miScalar startlength = *mi_eval_scalar(&paras->StartLength);
	lengthFactor = *mi_eval_scalar(&paras->ScaleLength);
	turtlewidth = *mi_eval_scalar(&paras->StartWidth);
	startwidth = turtlewidth;
	widthFactor = *mi_eval_scalar(&paras->ScaleWidth);
	endwidth = widthFactor;
	if (endwidth>=startwidth)
		endwidth = startwidth-0.01;
	deltaX = *mi_eval_scalar(&paras->AngleX);
	deltaZ = *mi_eval_scalar(&paras->AngleZ);
	deltaY = *mi_eval_scalar(&paras->AngleY);
	int depth = *mi_eval_integer(&paras->Depth);
	// read string
	string_tag = *mi_eval_tag(&paras->Type);
	lstring = (char*) mi_db_access(string_tag);
	mi_db_unpin(string_tag);

	// pyrmaid and prism base
	miVector vectors02[n];
	vPtr02 = vectors02;

	// init values
	stepwidth = (startwidth-endwidth)/depth;
	// turtle
	turtlepos.x = turtlepos.y = turtlepos.z = 0;
	turtledir.x = turtledir.z = 0; turtledir.y = 1;
	mi_vector_mul(&turtledir, startlength);
	lastturtledir = turtledir;
	lastturtlepos = turtlepos;
	// axis
	forward.x = forward.y = 0;	forward.z = 1.0;
	right.y = right.z = 0;		right.x = 1;
	up.x = up.z = 0;			up.y = 1;
	// matrices
	mi_matrix_ident(xformB);
	mi_matrix_rotate_axis(R90_left, &forward, (M_PI/2));
	mi_matrix_rotate_axis(R_zleft, &forward, (deltaZ*M_PI/180));
	mi_matrix_rotate_axis(R_zright, &forward, (-deltaZ*M_PI/180));
	mi_matrix_rotate_axis(R_yleft, &up, (deltaY*M_PI/180));
	mi_matrix_rotate_axis(R_yright, &up, (-deltaY*M_PI/180));
	mi_matrix_rotate_axis(R_xleft, &right, (deltaX*M_PI/180));
	mi_matrix_rotate_axis(R_xright, &right, (-deltaX*M_PI/180));
	mi_matrix_rotate_axis(R_Circle, &forward, 2*M_PI/segmentcount);
	// polycount
	xcount=0;
	// cunstruct base
	ConstructBasis();
	// object init
	obj = mi_api_object_begin(NULL); // ohne namen
	init_obj_flags(obj);
	// start model
	mi_api_object_group_begin(0.0);
	// start iteration
	status = LPredecessor(depth, close, state, paras);
	// end model
	mi_api_object_group_end();
	return(mi_geoshader_add_result(result, mi_api_object_end())); // gibt nen zeiger zurueck

} // End lturtle


miBoolean LPredecessor(int t, miBoolean isClose, miState *state, myParas *paras) {
	/** \brief Init method to the iteration
	 * Starts the iteration. If isClose==true, then the iteration will repeat with a degree offset around the forward axis.
	 * The closed result is a n-gon with "Segments" count of elements.
	 */
	int i;
	if (isClose) {
		for (i=0; i<segmentcount; i++) {
			LProductionSuccessor_F(t, state, paras);
			mi_vector_transform(&turtledir, &turtledir, R_Circle);
		}
	} else {
		if (selection == 6){ // special case
			LProductionSuccessor_X(t, state, paras);
		} else {
			LProductionSuccessor_F(t, state, paras);
		}
	}

	return(miTRUE);
} // End LPredecessor


miBoolean LProductionSuccessor_F(int t, miState *state, myParas *paras) {
	/** \brief Standard production rule
	 * Standard production rule iterative interpret the custom string or one of the standard choice. An operation will apply to the current postion and direction.
	 * The operation depends on the character.
	 * If t=0 (iteration count = Depth) interpretation stops and element will be drawn at current position.
	 */

	miVector altpos, altdir;
	float altwidth;
	altwidth = 0;
	miVector altforward, altright, altup;
	miMatrix altxform;

	char mystring[64];
	char *tok, *p;

	if (t>1) {
		switch (selection) {
		case 0:
			BRANCHES
			break;
		case 1:
			KOCHISLAND
			break;
		case 2:
			KOCHLAKE
			break;
		case 3:
			KOCHLAKE2
			break;
		case 4:
			KOCHLAKE2_3D
			break;
		case 5: // custom string
			strcpy(mystring, lstring);
			tok = strtok_r(mystring, " ", &p);
			while (tok != NULL){
				if ( strcmp( tok, "F")==0){ F }
				else if ( strcmp( tok, "f")==0){ f }
				else if ( strcmp( tok, "lz")==0){ lz }
				else if ( strcmp( tok, "ly")==0){ ly }
				else if ( strcmp( tok, "lx")==0){ lx }
				else if ( strcmp( tok, "rz")==0){ rz }
				else if ( strcmp( tok, "ry")==0){ ry }
				else if ( strcmp( tok, "rx")==0){ rx }
				else if ( strcmp( tok, "a_")==0){ a_ }
				else if ( strcmp( tok, "z_")==0){ z_ }
				tok = strtok_r(NULL, " ", &p);
			}
			break;
		case 7:
			BUSH
			break;
		default:
			break;
		}
	} else {
		// draw
		switch (tubetype) {
		case 0:
			DrawTriangle(0.1, state, paras);
			break;
		case 1:
			DrawPyramid(state, paras);
			break;
		case 2:
			DrawPrism(state, paras);
			break;
		}
		// last = current
		lastturtledir = turtledir;
		lastturtlepos = turtlepos;
		// move forward turtle
		mi_vector_add(&turtlepos, &turtlepos, &turtledir);
		// scale the length
		mi_vector_mul(&turtledir, lengthFactor);
		// scale the width
		turtlewidth *= widthFactor;
	}

	return(miTRUE);
} // End LProductionSuccessor_F


miBoolean LProductionSuccessor_F2(int t, miState *state, myParas *paras) {
	/** \brief Second production rule of Branches2
	 * Second production rule of a separate defined branch type. Simple repeat until depth is reached.
	 */

	if (t>1) {
		F2 F2
	} else {
		// draw
		switch (tubetype) {
		case 0:
			DrawTriangle(0.1, state, paras);
			break;
		case 1:
			DrawPyramid(state, paras);
			break;
		case 2:
			DrawPrism(state, paras);
			break;
		}
		// last = current
		lastturtledir = turtledir;
		lastturtlepos = turtlepos;
		// move forward turtle
		mi_vector_add(&turtlepos, &turtlepos, &turtledir);
		// scale the length
		mi_vector_mul(&turtledir, lengthFactor);
		// scale the width
		turtlewidth *= widthFactor;
	}
	return(miTRUE);
}


miBoolean LProductionSuccessor_X(int t, miState *state, myParas *paras) {
	/** \brief First production rule of Branches2
	 * Production rule of a separate defined branch type. The method contains additional iteration steps.
	 */

	miVector altpos, altdir;
	float altwidth;
	miVector altforward, altright, altup;
	miMatrix altxform;

	if (t>1) {
		// F2 a_ lz X z_ rz X z_ rz F2 a_ rz F2 X z_ lz X // move in 2D
		F2 a_ lz X z_ lx X z_ rx X z_ rz X z_ rz F2 a_ rz F2 X z_ lz X // move in 3D
	}

	return(miTRUE);
} // End LProductionSuccessor_X


miBoolean LProductionSuccessor_f(int t) {
	/** \brief Special production rule
	 * Production rule moves the current position in current direction six times. No element will be drawn.
	 */

	miMatrix rota;
	miVector lastN, currentN, ortho;
	double alpha, value;

	if (t>1){
		f f f f f f
	} else {
		// calculate angle and matrix, normalize
		lastN=lastturtledir; currentN = turtledir;
		mi_vector_normalize(&lastN);
		mi_vector_normalize(&currentN);
		// angle
		value = mi_vector_dot(&currentN, &lastN);
		if ( fabs(value) < 1 ) {
			alpha = acos(value);
			// orthogonal vector
			mi_vector_prod(&ortho, &lastN, &currentN);
			// matrix
			mi_vector_normalize(&ortho);
			mi_matrix_rotate_axis(rota, &ortho, alpha);
		} else {
			mi_matrix_ident(rota);
		}
		// multiply new matrix with current rotation matrix
		mi_matrix_prod(xformB, xformB, rota);
		// move without draw
		lastturtledir = turtledir;
		lastturtlepos = turtlepos;
		mi_vector_add(&turtlepos, &turtlepos, &turtledir);
	}
	return(miTRUE);

} // End LProductionSuccessor_f


miBoolean ConstructBasis() {
	/** \brief Construct basis plane
	 * Construct a basis plane for pyramid and prism and stores the border points in vPtr02[].
	 */
	miVector v;
	v.x = 1; v.y = v.z = 0;
	miMatrix R;
	int j;
	// n-count equal parts
	mi_matrix_rotate(R, 0., (360/n*M_PI/180),0.);

	for (j=0; j<n; j++){ // punkte ringsherum
		vPtr02[j] = v;
		// rotate
		mi_vector_transform(&v, &v, R);
	}

	return(miTRUE);
} //End ConstructBasis


miBoolean CalculateRotationMatrices() {
	/** \brief Calculate rotation matrices
	 * Receive the matrices to rotate around the three basis axis
	 */
	mi_matrix_rotate_axis(R_zleft, &forward, (deltaZ*M_PI/180));
	mi_matrix_rotate_axis(R_zright, &forward, (-deltaZ*M_PI/180));
	mi_matrix_rotate_axis(R_yleft, &up, (deltaY*M_PI/180));
	mi_matrix_rotate_axis(R_yright, &up, (-deltaY*M_PI/180));
	mi_matrix_rotate_axis(R_xleft, &right, (deltaX*M_PI/180));
	mi_matrix_rotate_axis(R_xright, &right, (-deltaX*M_PI/180));

	return(miTRUE);
}


// Draw functions
void DrawTriangle(float inAspect, miState *state, myParas *paras){
	/** \brief Draw triangle
	 * Draws triangle from current position to current direction.
	 */
	miVector orthogonal;
	mi_vector_transform(&orthogonal, &turtledir, R90_left);
	mi_vector_mul(&orthogonal, inAspect);
	miVector A,B,C;
	mi_vector_add(&A, &turtlepos, &orthogonal);
	mi_vector_add(&B, &turtlepos, &turtledir);
	mi_vector_mul(&orthogonal, -1.0);
	mi_vector_add(&C, &turtlepos, &orthogonal);

	mi_api_vector_xyz_add(&A);
	mi_api_vector_xyz_add(&B);
	mi_api_vector_xyz_add(&C);

	mi_api_vertex_add(xcount*3+0);
	mi_api_vertex_add(xcount*3+1);
	mi_api_vertex_add(xcount*3+2);

	mi_api_poly_begin_tag(0, *mi_eval_tag(paras)); // 0=concave, 1=convex material tag/chars
	mi_api_poly_index_add(xcount*3+0);
	mi_api_poly_index_add(xcount*3+1);
	mi_api_poly_index_add(xcount*3+2);
	mi_api_poly_end();

	xcount++;

} // End DrawTriangle


void DrawPyramid(miState *state, myParas *paras){
	/** \brief Draw pyramid
	 * Draws pyramid from current position to current direction.
	 */
	miVector osA, osB;
	miVector lastN, currentN, ortho;
	double value, alpha;
	int i;
	miVector verticesB[n];

	miMatrix rota;
	osA = turtlepos;
	mi_vector_add(&osB, &turtlepos, &turtledir);

	//normalize
	lastN=lastturtledir; currentN = turtledir;
	mi_vector_normalize(&lastN);
	mi_vector_normalize(&currentN);
	// angle
	value = mi_vector_dot(&currentN, &lastN);
	if ( fabs(value) < 1 ) {
		alpha = acos(value);
		// orthogonal vector
		mi_vector_prod(&ortho, &lastN, &currentN);
		// rotation matrix
		mi_vector_normalize(&ortho);
		mi_matrix_rotate_axis(rota, &ortho, alpha);
	} else {
		mi_matrix_ident(rota);
	}
	// multiply with current rotation matrix
	mi_matrix_prod(xformB, xformB, rota);

	for (i=0; i<n; i++) {
		verticesB[i] = *(vPtr02+i);
		// scale width
		mi_vector_mul(&verticesB[i], turtlewidth);
		mi_vector_transform(&verticesB[i], &verticesB[i], xformB);
		mi_vector_add(&verticesB[i], &verticesB[i], &turtlepos);
	}

	// add new vertices
	mi_api_vector_xyz_add(&osB);
	mi_api_vertex_add((n+1)*xcount);
	for (i=0; i<n; i++){
		mi_api_vector_xyz_add(&verticesB[i]);
		mi_api_vertex_add((n+1)*xcount+i+1); // n+1, weil oberer punkte dazu
	}
	// draw n-1 triangles
	for (i=0; i<n-1; i++) {
		mi_api_poly_begin_tag(1, *mi_eval_tag(paras)); // 0=concave, 1=convex material tag/chars ( 1 und NULL ist schneller )
		mi_api_poly_index_add((n+1)*xcount);
		mi_api_poly_index_add((n+1)*xcount+i+1);
		mi_api_poly_index_add((n+1)*xcount+i+2);
		mi_api_poly_end();
	}
	// draw last triangle (and close)
	mi_api_poly_begin_tag(1, *mi_eval_tag(paras)); // 0=concave, 1=convex material tag/chars ( 1 und NULL ist schneller )
	mi_api_poly_index_add((n+1)*xcount);
	mi_api_poly_index_add((n+1)*xcount+n);
	mi_api_poly_index_add((n+1)*xcount+1);
	mi_api_poly_end();

	xcount++;
} // End DrawPyramid


void DrawPrism(miState *state, myParas *paras){
	/** \brief Draw prism
	 * Draws prism from current position to current direction.
	 */
	miVector osA, osB;
	miVector lastN, currentN, ortho;
	double value, alpha;
	int i;
	miVector verticesB[n];
	miVector verticesA[n];
	miMatrix rota;

	osA = turtlepos;
	mi_vector_add(&osB, &turtlepos, &turtledir);

	// normalie
	lastN=lastturtledir; currentN = turtledir;
	mi_vector_normalize(&lastN);
	mi_vector_normalize(&currentN);
	// angle
	value = mi_vector_dot(&currentN, &lastN);
	if ( fabs(value) < 1 ) {
		alpha = acos(value);
		// orthogonal vector
		mi_vector_prod(&ortho, &lastN, &currentN);
		// rotation matrix
		mi_vector_normalize(&ortho);
		mi_matrix_rotate_axis(rota, &ortho, alpha);
	} else {
		mi_matrix_ident(rota);
	}
	// multiply with current rotation matrix
	mi_matrix_prod(xformB, xformB, rota);

	for (i=0; i<n; i++) {
		verticesA[i] = *(vPtr02+i);
		// scale width
		mi_vector_mul(&verticesA[i], turtlewidth);
		mi_vector_transform(&verticesA[i], &verticesA[i], xformB);
		mi_vector_add(&verticesA[i], &verticesA[i], &osA);
	}
	// second base of prism
	for (i=0; i<n; i++) {
		verticesB[i] = *(vPtr02+i);
		// scale width
		mi_vector_mul(&verticesB[i], turtlewidth*widthFactor);
		mi_vector_transform(&verticesB[i], &verticesB[i], xformB);
		mi_vector_add(&verticesB[i], &verticesB[i], &osB);
	}

	// add new vertices
	for (i=0; i<n; i++){
		mi_api_vector_xyz_add(&verticesA[i]);
		mi_api_vertex_add(n*xcount+i);
	}
	// add new vertices from second base
	for (i=0; i<n; i++){
		mi_api_vector_xyz_add(&verticesB[i]);
		mi_api_vertex_add(n*(xcount+1)+i);
	}

	// draw n-1 polygons
	for (i=0; i<n-1; i++) {
		mi_api_poly_begin_tag(1, *mi_eval_tag(paras)); // 0=concave, 1=convex material tag/chars ( 1 und NULL ist schneller )
		mi_api_poly_index_add(n*xcount+i);
		mi_api_poly_index_add(n*xcount+i+1);
		mi_api_poly_index_add(n*(xcount+1)+i+1);
		mi_api_poly_index_add(n*(xcount+1)+i);
		mi_api_poly_end();
	}
	// draw n polygon (close)
	mi_api_poly_begin_tag(1, *mi_eval_tag(paras)); // 0=concave, 1=convex material tag/chars ( 1 und NULL ist schneller )
	mi_api_poly_index_add(n*xcount+n-1);
	mi_api_poly_index_add(n*xcount);
	mi_api_poly_index_add(n*(xcount+1));
	mi_api_poly_index_add(n*(xcount+1)+n-1);
	mi_api_poly_end();

	xcount = xcount+2;
} // End DrawPrism




