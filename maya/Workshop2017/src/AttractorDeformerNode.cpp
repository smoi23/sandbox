/*
 * AttractorDeformerNode.cpp
 *
 *  Created on: Jul 6, 2017
 *      Author: Andreas Schuster
 */

#include <cmath>
#include <AttractorDeformerNode.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MItGeometry.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTransform.h>

MTypeId AttractorDeformer::s_id(0x18538);
MString AttractorDeformer::s_name("attractorDeformer");
MObject AttractorDeformer::s_matrix;
MObject AttractorDeformer::s_falloff;

AttractorDeformer::AttractorDeformer():
		m_falloff(0)
{}

void* AttractorDeformer::creator()
{
	std::cout << "In AttractorDeformer::creator()" << std::endl;
	return new AttractorDeformer();
}

MStatus AttractorDeformer::initialize()
{
	std::cout << "In AttractorDeformer::initialize()" << std::endl;

	MStatus stat = MStatus::kSuccess;


	MFnMatrixAttribute mAttr;
	s_matrix = mAttr.create("attractorMatrix", "attmat", MFnMatrixAttribute::kDouble,  &stat);
	mAttr.setStorable(false);
	mAttr.setConnectable(true);


	MFnNumericAttribute nAttr;
	s_falloff = nAttr.create("attractorFalloff", "attfal", MFnNumericData::kDouble, 1.5, &stat);
	nAttr.setStorable(true);
	nAttr.setWritable(true);
	nAttr.setKeyable(true);
	nAttr.setMin(1.0);

	addAttribute(s_matrix);
	addAttribute(s_falloff);

	attributeAffects(s_matrix, outputGeom);
	attributeAffects(s_falloff, outputGeom);

	return stat;
}

MStatus AttractorDeformer::deform(MDataBlock& 	block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex)
{
	std::cout << "In AttractorDeformer::deform()" << std::endl;

	MStatus stat = MStatus::kSuccess;

	MDataHandle envHandle = block.inputValue(envelope, &stat);
	float env = envHandle.asFloat();

	MDataHandle falloffHandle = block.inputValue(s_falloff, &stat);
	m_falloff = falloffHandle.asDouble();

	std::cout << "FallOff " << m_falloff << std::endl;

	MDataHandle matrixHandle = block.inputValue(s_matrix, &stat);
	MMatrix matLocator  = matrixHandle.asMatrix();

	MPoint attractorPoint;
	attractorPoint *= matLocator;
	attractorPoint *= env;

	for(iter.reset(); !iter.isDone(); iter.next())
	{
		MPoint p = iter.position();

		MVector attractorOffset(attractorPoint - p);
		// std::cout << "p: " << attractorOffset.x << " " << attractorOffset.y <<  " " << attractorOffset.z << std::endl;
		double stretch =  std::sqrt(attractorOffset.length()) / m_falloff;
		attractorOffset.normalize();

		float weight = weightValue(block, multiIndex, iter.index());

		p = p + attractorOffset * weight * stretch * env;

		iter.setPosition(p);
	}

	return stat;
}

MObject& AttractorDeformer::accessoryAttribute() const
{
	return s_matrix;
}

MStatus AttractorDeformer::accessoryNodeSetup(MDagModifier& dagM)
{
	std::cout << "In AttractorDeformer::accessoryNodeSetup()" << std::endl;

	MStatus stat;

	MObject objLoc = dagM.createNode(MString("locator"), MObject::kNullObj, &stat);

	if (MS::kSuccess == stat)
	{
		MFnDependencyNode fnLoc(objLoc);
		fnLoc.setName("attractorDeformerLocator");
		MString attrName("matrix");
		MObject attrLoc = fnLoc.attribute(attrName);
		stat = dagM.connect(objLoc, attrLoc, this->thisMObject(), s_matrix);
	}
	return stat;
}
