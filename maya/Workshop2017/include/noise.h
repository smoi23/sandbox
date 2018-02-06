//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+


//
//  Class: noise
//
//  Description:
//      The noise class is used for generating pseudo-random continuous noise.
//      The noise values generated are always between 0 and 1.
//
//      The technique used is a simple lattice noise algorithm based upon one 
//      by Ken Perlin.  This particular implementation is adapted from 
//      Darwyn Peachey's (Texturing and Modeling: a Procedural Approach,
//		S. Ebert Editor, 1994).
//

#define TABLE_SIZE 256

extern const int kTableMask;
#define MODPERM(x) permtable[(x)&kTableMask]

typedef struct {
	float x;
	float y;
	float z;
} pnt;

class noise {
public:

	static float  atValue( float x );
	static float  atPoint( float x, float y, float z ); 

	static pnt    atPointAndTime( float x, float y, float z, float t );

	static void   initTable( long seed );

private:

	static int    permtable   [256];
	static float  valueTable1 [256];
	static float  valueTable2 [256];
	static float  valueTable3 [256];
	static int    isInitialized;
	
	static float  spline( float x, float knot0, float knot1, float knot2, 
						  float knot3 );

	static float  value( int x, int y, int z, float table[] = valueTable1 );
	static float  value( int x, int y, int z, int t,
						 float table[] = valueTable1 );
	static float  value( int x, float table[] = valueTable1 );


};
 

inline float noise::value( int x, int y, int z, int t, float table[] ) {
	return table[MODPERM( x + MODPERM( y + MODPERM( z + MODPERM( t ) ) ) )];
}

inline float noise::value( int x, int y, int z, float table[] ) {
	return table[MODPERM( x + MODPERM( y + MODPERM( z ) ) )];
}

inline float noise::value( int x, float table[] ) {
	return table[MODPERM( x )];
}
