/*
* WorkshopCmd3.cpp
*
*  Created on: May 15, 2017
*      Author: Andreas Schuster
*
*/
#include <stdio.h>
#include <iostream>

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>

#include <maya/MSyntax.h>
#include <maya/MStringArray.h>
#include <maya/MGlobal.h>

#include <maya/MArgDatabase.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MFnDagNode.h>

#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MFnTransform.h>
#include <maya/MFnMeshData.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MPlug.h>
#include <maya/MBoundingBox.h>
#include <maya/MFnSet.h>

#include "BBoxCubeCmd.h"

// 
#define kHelpFlag "-h"
#define kHelpFlagLong "-help"

// //
#define kSizeFlag "s"
#define kSizeFlagLong "size"


MObject BBoxCubeCmd::m_meshTransform;
MString BBoxCubeCmd::s_name("BBoxCube");


BBoxCubeCmd::BBoxCubeCmd():
m_doHelp(false),
m_size(1.0),
m_isQuery(false)
{
	std::cout << "In BBoxCubeCmd::BBoxCubeCmd()" << std::endl;
}

BBoxCubeCmd::~BBoxCubeCmd()
{
	std::cout << "In BBoxCubeCmd::~BBoxCubeCmd()" << std::endl;
}

MSyntax BBoxCubeCmd::newSyntax()
{
	MSyntax syntax;
	std::cout << "In BBoxCubeCmd::BBoxCubeCmd()" << std::endl;

	// //
	syntax.enableQuery(true);
	syntax.enableEdit(false);
	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1);
	syntax.addFlag(kHelpFlag, kHelpFlagLong, MSyntax::kNoArg);
	syntax.addFlag(kSizeFlag, kSizeFlagLong, MSyntax::kDouble);

	return syntax;
}

MStatus BBoxCubeCmd::doIt(const MArgList &args)
{
	MStatus stat = MS::kSuccess;
	std::cout << "In BBoxCubeCmd::doIt()" << std::endl;

	stat = parseArgs(args);

	if (m_doHelp)
	{
		MGlobal::displayInfo("Show help");
	}
	else if (m_isQuery)
	{
		MString  selectionString("");
		MItSelectionList iter(m_selection);
		for (iter.reset(); !iter.isDone(); iter.next())
		{
			MDagPath dagpath;
			iter.getDagPath(dagpath);
			selectionString += dagpath.fullPathName();

		}
		MGlobal::displayInfo(selectionString);
	}
	else
	{
		if (m_selection.length() > 0)
		{
			stat = redoIt();
		}
		else
		{
			MGlobal::displayError("Pass or select at least one polygon object.");
		}
	}
	return stat;
}

MStatus BBoxCubeCmd::redoIt()
{
	std::cout << "In BBoxCubeCmd::redoIt()" << std::endl;

	MStatus stat;

	// start new buffers
	MPointArray vertexBuffer;
	MIntArray faceCount;
	MIntArray vertexIndex;


	MItSelectionList iter(m_selection, MFn::kTransform);
	for (iter.reset(); !iter.isDone(); iter.next())
	{

		MDagPath dagpath;
		iter.getDagPath(dagpath);
		MGlobal::displayInfo(dagpath.fullPathName());

		MMatrix dagMatrix = dagpath.inclusiveMatrix();

		stat = dagpath.extendToShape();
		if (stat == MS::kSuccess)
		{
			MFnDagNode fnDagNode(dagpath);
			MBoundingBox bbox = fnDagNode.boundingBox();
			bbox.transformUsing(dagMatrix);
			addCubeFromBbox(vertexBuffer, faceCount, vertexIndex, bbox);
		}
	}

	MGlobal::displayInfo(MString("VertexBufferlength: ") + vertexBuffer.length());

	// create place for the data
	MFnMeshData    dataFn;
	MObject        dataWrapper = dataFn.create();

	// create the mesh from the mesh function set
	MFnMesh fnMesh;
	fnMesh.create(vertexBuffer.length(), faceCount.length(), vertexBuffer, faceCount, vertexIndex, dataWrapper, &stat);
	if (stat != MS::kSuccess)
	{
		MGlobal::displayError("Failed to create mesh: " + stat.errorString());
		return stat;
	}

	// set normals to make the cube hard edged
	int numFaceVertices = faceCount.length() * 4; // assuming quads!
	MVectorArray normals(numFaceVertices);
	MIntArray faces(numFaceVertices);
	MIntArray vertices(numFaceVertices);
	MItMeshPolygon iterFace(dataWrapper);
	int idx = 0;
	for (iterFace.reset(); !iterFace.isDone(); iterFace.next())
	{
		MIntArray vertexIds;
		iterFace.getVertices(vertexIds);
		//std::cout << "faceVertices count " << vertexIds.length() << std::endl;
		//std::cout << vertexIds[0] << " " << vertexIds[1] << " " << vertexIds[2] << " " << vertexIds[3] << std::endl;
	
		MVector faceNormal;
		iterFace.getNormal(faceNormal);
		//std::cout << "Face normal: " << faceNormal.x << " " << faceNormal.y << " " << faceNormal.z << std::endl;
		
		for (int i = 0; i < 4; i++)  // assuming quads!
		{
			faces.set(iterFace.index(), idx * 4+i);
			vertices.set(vertexIds[i], idx * 4 + i);
			normals.set(faceNormal, idx * 4 + i);
			// fnMesh.setFaceVertexNormal(faceNormal, iterFace.index(), vertexIds[i]); // set normal for every face vertex separately
		}
		idx++;
	}

	fnMesh.setFaceVertexNormals(normals, faces, vertices);

	fnMesh.updateSurface();

	MObject transformNode = dagModifier.createNode("mesh", MObject::kNullObj, &stat);
	if (stat != MS::kSuccess)
	{
		MGlobal::displayError("Failed to create transform: " + stat.errorString());
		return stat;
	}
	dagModifier.doIt();

	// Set the mesh node to use the geometry we created for it.
	setMeshData(transformNode, dataWrapper);


	// assign to shading group
	MSelectionList selectionList;
	stat = selectionList.add("initialShadingGroup");
	if (stat == MS::kSuccess)
	{
		MObject shaderNode;
		selectionList.getDependNode(0, shaderNode);
		MFnSet setFn(shaderNode);
		MFnDagNode  dagFn(transformNode);
		MObject     mesh = dagFn.child(0);
		setFn.addMember(mesh);
	}


	return MS::kSuccess;
}

void BBoxCubeCmd::addCubeFromBbox(MPointArray &o_vertexBuffer, MIntArray &o_vertexCount, MIntArray &o_vertexIndex, MBoundingBox &i_bbox)
{
	int offset = o_vertexBuffer.length();

	// 8 vertices
	o_vertexBuffer.append(i_bbox.max());
	o_vertexBuffer.append(MPoint(i_bbox.max().x, i_bbox.max().y, i_bbox.min().z));
	o_vertexBuffer.append(MPoint(i_bbox.min().x, i_bbox.max().y, i_bbox.min().z));
	o_vertexBuffer.append(MPoint(i_bbox.min().x, i_bbox.max().y, i_bbox.max().z));
	o_vertexBuffer.append(i_bbox.min());
	o_vertexBuffer.append(MPoint(i_bbox.max().x, i_bbox.min().y, i_bbox.min().z));
	o_vertexBuffer.append(MPoint(i_bbox.max().x, i_bbox.min().y, i_bbox.max().z));
	o_vertexBuffer.append(MPoint(i_bbox.min().x, i_bbox.min().y, i_bbox.max().z));

	// every face has 4 vertices
	for (int i = 0; i < 6; i++)
	{
		o_vertexCount.append(4);
	}

	// face top
	o_vertexIndex.append(offset + 0);
	o_vertexIndex.append(offset + 1);
	o_vertexIndex.append(offset + 2);
	o_vertexIndex.append(offset + 3);
	// face front
	o_vertexIndex.append(offset + 0);
	o_vertexIndex.append(offset + 3);
	o_vertexIndex.append(offset + 7);
	o_vertexIndex.append(offset + 6);
	// face right
	o_vertexIndex.append(offset + 1);
	o_vertexIndex.append(offset + 0);
	o_vertexIndex.append(offset + 6);
	o_vertexIndex.append(offset + 5);
	// face left
	o_vertexIndex.append(offset + 3);
	o_vertexIndex.append(offset + 2);
	o_vertexIndex.append(offset + 4);
	o_vertexIndex.append(offset + 7);
	// face back
	o_vertexIndex.append(offset + 2);
	o_vertexIndex.append(offset + 1);
	o_vertexIndex.append(offset + 5);
	o_vertexIndex.append(offset + 4);
	// face bottom
	o_vertexIndex.append(offset + 4);
	o_vertexIndex.append(offset + 5);
	o_vertexIndex.append(offset + 6);
	o_vertexIndex.append(offset + 7);

}



MStatus BBoxCubeCmd::setMeshData(MObject transform, MObject dataWrapper)
{
	MStatus st;

	// Get the mesh node.
	MFnDagNode  dagFn(transform);
	MObject     mesh = dagFn.child(0);

	// The mesh node has two geometry inputs: 'inMesh' and 'cachedInMesh'.
	// 'inMesh' is only used when it has an incoming connection, otherwise
	// 'cachedInMesh' is used. Unfortunately, the docs say that 'cachedInMesh'
	// is for internal use only and that changing it may render Maya
	// unstable.
	//
	// To get around that, we do the little dance below...

	// Use a temporary MDagModifier to create a temporary mesh attribute on
	// the node.
	MFnTypedAttribute  tAttr;
	MObject            tempAttr = tAttr.create("tempMesh", "tmpm", MFnData::kMesh);
	MDagModifier       tempMod;

	st = tempMod.addAttribute(mesh, tempAttr);

	st = tempMod.doIt();

	// Set the geometry data onto the temp attribute.
	dagFn.setObject(mesh);

	MPlug  tempPlug = dagFn.findPlug(tempAttr);

	st = tempPlug.setValue(dataWrapper);

	// Use the temporary MDagModifier to connect the temp attribute to the
	// node's 'inMesh'.
	MPlug  inMeshPlug = dagFn.findPlug("inMesh");

	st = tempMod.connect(tempPlug, inMeshPlug);

	st = tempMod.doIt();

	// Force the mesh to update by grabbing its output geometry.
	dagFn.findPlug("outMesh").asMObject();

	// Undo the temporary modifier.
	st = tempMod.undoIt();

	return st;
}


MStatus BBoxCubeCmd::undoIt()
{
	std::cout << "In BBoxCubeCmd::undoIt()" << std::endl;
	dagModifier.undoIt();
	return MS::kSuccess;
}

bool BBoxCubeCmd::isUndoable() const
{
	std::cout << "In BBoxCubeCmd::isUndoable()" << std::endl;
	return !m_isQuery;
}

void* BBoxCubeCmd::creator()
{
	std::cout << "In BBoxCubeCmd::creator()" << std::endl;
	return new BBoxCubeCmd();
}

MStatus BBoxCubeCmd::parseArgs(const MArgList &args)
{
	MStatus stat = MS::kSuccess;

	MArgDatabase argData(syntax(), args);
	m_doHelp = argData.isFlagSet(kHelpFlag);

	m_isQuery = argData.isQuery();

	if (!m_isQuery) // only update selection if not in query mode
	{
		argData.getObjects(m_selection);
	}
	
	return stat;
}

