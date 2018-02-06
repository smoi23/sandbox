/*
* SphereTransform.cpp
*
*  Created on: May 12, 2017
*      Author: Andreas Schuster
*
*/

#include <maya/MPxNode.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MDataHandle.h>

#include <maya/MPxTransform.h>


#include "SphereTransformNode.h"



MTypeId SphereTransformationMatrix::s_id(0x17529);


SphereTransformationMatrix::SphereTransformationMatrix() :
	m_radius(1)
{}

void* SphereTransformationMatrix::creator()
{
	return new SphereTransformationMatrix();
}

MMatrix SphereTransformationMatrix::asMatrix() const
{
	// std::cout << "SphereTransformationMatrix::asMatrix()" << std::endl;
	MMatrix parentM = MPxTransformationMatrix::asMatrix();
	MTransformationMatrix tm(parentM);

	MVector trans = tm.translation(MSpace::kWorld);
	trans.normalize();
	trans *= m_radius;
	tm.setTranslation(trans, MSpace::kWorld);

	return tm.asMatrix();
}

MMatrix SphereTransformationMatrix::asMatrix(double percent) const
{
	// std::cout << "SphereTransformationMatrix::asMatrix(percent)" << std::endl;
	return asMatrix();
}

double SphereTransformationMatrix::getRadius() const
{
	return m_radius;
}

void SphereTransformationMatrix::setRadius(double radius)
{
	m_radius = radius;
}



MString SphereTransformNode::s_name("sphereTransform");
MTypeId SphereTransformNode::s_id(0x18528);
MObject SphereTransformNode::s_radius;

SphereTransformNode::SphereTransformNode() :
	MPxTransform()
{
}

SphereTransformNode::SphereTransformNode(MPxTransformationMatrix* tm):
	MPxTransform(tm)
{
}

void SphereTransformNode::postConstructor()
{
	MPxTransform::postConstructor();

}

MPxTransformationMatrix* SphereTransformNode::createTransformationMatrix()
{
	return new SphereTransformationMatrix();
}

void  SphereTransformNode::resetTransformation(const MMatrix &matrix)
{
	MPxTransform::resetTransformation(matrix);
}


void  SphereTransformNode::resetTransformation(MPxTransformationMatrix *matrix)
{
	MPxTransform::resetTransformation(matrix);
}

SphereTransformationMatrix* SphereTransformNode::getSphereTransformMatrix()
{
	SphereTransformationMatrix *tm = (SphereTransformationMatrix *)baseTransformationMatrix;
	return tm;
}


MStatus SphereTransformNode::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus stat = MStatus::kSuccess;
	std::cout << "In SphereTransform::compute()" << std::endl;

	if ((plug.attribute() == MPxTransform::matrix)
		|| (plug.attribute() == MPxTransform::inverseMatrix)
		|| (plug.attribute() == MPxTransform::worldMatrix)
		|| (plug.attribute() == MPxTransform::worldInverseMatrix)
		|| (plug.attribute() == MPxTransform::parentMatrix)
		|| (plug.attribute() == MPxTransform::parentInverseMatrix))
	{

		MPlug radiusPlug(thisMObject(), s_radius);
		MDataHandle radiusHandle = block.inputValue(radiusPlug, &stat);
		double radius = radiusHandle.asDouble();

		// update custom transformation matrix
		// TODO: error checking
		SphereTransformationMatrix* tm = getSphereTransformMatrix();
		tm->setRadius(radius);
	}

	return MPxTransform::compute(plug, block);
}

MStatus SphereTransformNode::validateAndSetValue(const MPlug& plug,	const MDataHandle& handle,	const MDGContext& context)
{
	MStatus stat = MStatus::kSuccess;
	// std::cout << "In SphereTransform::validateAndSetValue()" << std::endl;

	if (plug.isNull())
	{
		return MStatus::kFailure;
	}
	else if (plug == s_radius)
	{
		double radius = handle.asDouble();

		// get data block
		MDataBlock block = forceCache(*(MDGContext *)&context);
		MDataHandle blockHandle = block.outputValue(plug, &stat);
		blockHandle.set(radius);

		// update custom transformation matrix
		SphereTransformationMatrix* tm = getSphereTransformMatrix();		
		tm->setRadius(radius);

		// mark as clean after changing the data
		blockHandle.setClean();

		// mark matrix as dirty so DG will be updated 
		dirtyMatrix();

		return stat;
	}

	return MPxTransform::validateAndSetValue(plug, handle, context);
}


void* SphereTransformNode::creator()
{
	std::cout << "In SphereTransform::creator()" << std::endl;
	return new SphereTransformNode();
}


MStatus SphereTransformNode::initialize()
{
	std::cout << "In SphereTransform::initialize()" << std::endl;

	MStatus stat = MStatus::kSuccess;

	MFnNumericAttribute nAttr;
	s_radius = nAttr.create("radiusSphere", "radsph", MFnNumericData::kDouble, 1.0, &stat);
	nAttr.setKeyable(true);
	nAttr.setAffectsWorldSpace(true);

	addAttribute(s_radius);

	mustCallValidateAndSet(s_radius);

	return stat;
}



