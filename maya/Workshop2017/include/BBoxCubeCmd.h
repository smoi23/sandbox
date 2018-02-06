/*
* WorkshopCmd3.h
*
*  Created on: May 15, 2017
*      Author: Andreas Schuster
*
*/
#pragma once
#ifndef BBOXCUBECMD_H
#define BBOXCUBECMD_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MSyntax.h>

#include <maya/MBoundingBox.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>

//
// Definition
//
class BBoxCubeCmd : public MPxCommand
{

public:
	BBoxCubeCmd();
	~BBoxCubeCmd();
	static MSyntax newSyntax();
	MStatus doIt(const MArgList &args);
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;
	static void* creator();

private:
	MStatus parseArgs(const MArgList &args);
	void addCubeFromBbox(MPointArray &o_vertexBuffer, MIntArray &o_vertexCount, MIntArray &o_vertexIndex, MBoundingBox &i_bbox);
	MStatus setMeshData(MObject transform, MObject dataWrapper);

public:
	static MString s_name;

private:
	bool m_doHelp;
	static MObject m_meshTransform;
	double m_size;
	MSelectionList m_selection;
	MDagModifier dagModifier;
	bool m_isQuery;

};



#endif
