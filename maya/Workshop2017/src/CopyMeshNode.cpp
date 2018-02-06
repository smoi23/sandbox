/*
* CopyMeshNode.cpp
*
*  Created on: May 11, 2017
*      Author: Andreas Schuster
*
*/

#include <maya/MFnMeshData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MDagPath.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>
#include <maya/MFnTransform.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include "CopyMeshNode.h"

MString CopyMeshNode::s_name("CopyMeshNode");
MTypeId CopyMeshNode::s_id(0x18524);
MObject CopyMeshNode::s_inMesh;
MObject CopyMeshNode::s_outMesh;
MObject CopyMeshNode::s_scale;


MStatus CopyMeshNode::compute(const MPlug &plug, MDataBlock &data)
{
	MStatus stat = MStatus::kSuccess;


	if (plug == s_outMesh)
	{
		MDataHandle outMeshHandle = data.outputValue(s_outMesh, &stat);
		MDataHandle inMeshHandle = data.inputValue(s_inMesh, &stat);
		MDataHandle iSizeHandle = data.inputValue(s_scale, &stat);
		m_scale = iSizeHandle.asDouble();


		/* Copy solution one*/
		outMeshHandle.copy(inMeshHandle);
		MObject outMeshData = outMeshHandle.asMesh();

		/* Copy solution two
		MObject inMeshData = inMeshHandle.asMesh();
		MFnMeshData fnMeshData;
		MObject outMeshData = fnMeshData.create(&stat);
		outMeshData = MObject(inMeshData);
		*/

		// point from where to scale
		MPoint center(0, 0, 0);
		
		// get plug at inMesh and retrieve source transform
		MPlug inMeshPlug(thisMObject(), s_inMesh);
		if (inMeshPlug.isDestination() && inMeshPlug.isConnected())
		{
			//MObject sourceNode = inMeshPlug.source().node();

			MPlugArray plugs;
			if (inMeshPlug.connectedTo(plugs, true,false) )
			{
				MPlug sourcePlug = plugs[0];
				MFnDagNode dagNodeFn(sourcePlug.node());
				if (dagNodeFn.parentCount() > 0)
				{
					MObject parent = dagNodeFn.parent(0);
					MFnDagNode parentNodeFn(dagNodeFn.parent(0));
					MDagPath parentDag;
					parentNodeFn.getPath(parentDag);
					MFnTransform transformFn(parentDag);
					center = transformFn.getTranslation(MSpace::kWorld);
				}
			}
		}


		stat = processMesh(outMeshData, center);

		outMeshHandle.set(outMeshData);
		data.setClean(plug);
	}
	else
	{
		stat = MStatus::kUnknownParameter;
	}

	return stat;
}


MStatus CopyMeshNode::processMesh(MObject &io_meshData, MPoint &i_center)
{
	MStatus stat = MStatus::kSuccess;

	MItMeshVertex iterVertex(io_meshData);
	for (iterVertex.reset(); !iterVertex.isDone(); iterVertex.next())
	{
		// project all vertices onto sphere
		MVector offset = (iterVertex.position(MSpace::kWorld) - i_center);
		offset.normalize();
		offset *= m_scale;
		MPoint pos(offset);
		iterVertex.setPosition(pos);
	}

	return stat;
}


void* CopyMeshNode::creator()
{
	return new CopyMeshNode();
}

MStatus CopyMeshNode::initialize()
{
	std::cout << "In CopyMeshNode::initialize()" << std::endl;
	MStatus stat = MStatus::kSuccess;

	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;

	s_inMesh = tAttr.create("inMesh", "in", MFnMeshData::kMesh, &stat);
	tAttr.isStorable(false);
	tAttr.isWritable(false);
	tAttr.setHidden(true);

	s_outMesh = tAttr.create("outMesh", "out", MFnMeshData::kMesh, &stat);
	tAttr.isStorable(false);
	tAttr.isWritable(false);
	tAttr.setHidden(true);

	s_scale = nAttr.create("scale", "s", MFnNumericData::kDouble, 1.0, &stat);
	nAttr.setStorable(true);
	nAttr.setWritable(true);
	nAttr.setKeyable(true);

	addAttribute(s_inMesh);
	addAttribute(s_outMesh);
	addAttribute(s_scale);

	attributeAffects(s_inMesh, s_outMesh);
	attributeAffects(s_scale, s_outMesh);

	return stat;
}


