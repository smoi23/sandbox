/*
* CopyMeshNode.cpp
*
*  Created on: May 11, 2017
*      Author: Andreas Schuster
*
*/
#ifndef HELLOWORLDNODE_H_
#define HELLOWORLDNODE_H_

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPoint.h>

class hellowWorld : public MPxNode
{
public:
	hellowWorld() {}
	~hellowWorld() {}
	virtual MStatus compute(const MPlug &plug, MDataBlock &data);
	static void *creator();
	static MStatus initialize();
private:

public:
	static MTypeId s_id;
	static MString s_name;
	static MObject s_objectName;
	static MObject s_number1;
	static MObject s_number2;
	static MObject s_points;
	static MObject s_transforms;
	static MObject s_outpoints;
	static MObject s_targetMessage;
};

#endif 


