/*
* CopyMeshNode.cpp
*
*  Created on: May 11, 2017
*      Author: Andreas Schuster
*
*/
#ifndef COPYMESHNODE_H_
#define COPYMESHNODE_H_

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPoint.h>

class CopyMeshNode : public MPxNode
{
public:
	CopyMeshNode() {}
	~CopyMeshNode() {}
	virtual MStatus compute(const MPlug &plug, MDataBlock &data);
	static void *creator();
	static MStatus initialize();

private:
	MStatus processMesh(MObject &io_meshData, MPoint &i_center);
	double m_scale;
public:
	static MTypeId s_id;
	static MString s_name;
	static MObject s_inMesh;
	static MObject s_outMesh;
	static MObject s_scale;
};

#endif 


