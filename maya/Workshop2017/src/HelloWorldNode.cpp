/*
* CopyMeshNode.cpp
*
*  Created on: May 11, 2017
*      Author: Andreas Schuster
*
*/

#include "HelloWorldNode.h"

#include <maya/MFnMessageAttribute.h>
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

MString hellowWorld::s_name("helloWorld");
MTypeId hellowWorld::s_id(0x18424);
MObject hellowWorld::s_objectName;
MObject hellowWorld::s_number1;
MObject hellowWorld::s_number2;
MObject hellowWorld::s_points;
MObject hellowWorld::s_transforms;
MObject hellowWorld::s_outpoints;
MObject hellowWorld::s_targetMessage;

MStatus hellowWorld::compute(const MPlug &plug, MDataBlock &data)
{
	std::cout << "In hellowWorld::compute()" << std::endl;
	MStatus stat = MStatus::kSuccess;

	if (plug == s_outpoints)
	{


		MDataHandle iSizeHandle = data.inputValue(s_number1, &stat);
		int value = iSizeHandle.asInt();
		MGlobal::displayInfo("s_number1 " + value);



		MArrayDataHandle arrayHandle = data.inputArrayValue(s_points, &stat);
		// check
		for ( int i = 0; i<arrayHandle.elementCount(); i++)
		{
			arrayHandle.jumpToArrayElement(i);
			MDataHandle knotHandle = arrayHandle.inputValue(&stat);
			double3& kData = knotHandle.asDouble3();
			MPoint point;
			point.x = kData[0];
			point.y = kData[1];
			point.z = kData[2];
			MGlobal::displayInfo(MString() + point.x + " " + point.y + " " + point.z);
		}

	}
	return stat;
}


void* hellowWorld::creator()
{
	return new hellowWorld();
}

MStatus hellowWorld::initialize()
{
	std::cout << "In hellowWorld::initialize()" << std::endl;
	MStatus stat = MStatus::kSuccess;

	// add single attributes
	MFnNumericAttribute nAttr;
	s_number1 = nAttr.create("number1", "num1", MFnNumericData::kInt, 7);

	// name
	MFnTypedAttribute tAttr;
	s_objectName = tAttr.create("objectName", "objnam", MFnData::kString);

	// point array
	s_points = nAttr.create("points", "pts", MFnNumericData::k3Double);
	nAttr.setArray(true);
	
	// array of transforms
	MFnMessageAttribute msgAttr;
	s_transforms = msgAttr.create("transformMsg", "tramsg");
	msgAttr.setArray(true);
	msgAttr.setHidden(true);


	s_outpoints = nAttr.create("outPoints", "outpoi",MFnNumericData::k3Double, 0.0, &stat);
	nAttr.setArray(true);
	// nAttr.setUsesArrayDataBuilder(true);
	nAttr.setStorable(false);
	nAttr.setWritable(false);

	addAttribute(s_number1);
	addAttribute(s_objectName);
	addAttribute(s_points);
	addAttribute(s_transforms);

	addAttribute(s_outpoints);

	
	attributeAffects(s_number1, s_outpoints);
	attributeAffects(s_objectName, s_outpoints);
	attributeAffects(s_points, s_outpoints);
	attributeAffects(s_transforms, s_outpoints);

	attributeAffects(s_number1, message);

	return stat;
}


