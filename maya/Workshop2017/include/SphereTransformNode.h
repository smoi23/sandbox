/*
* SphereTransform.h
*
*  Created on: May 12, 2017
*      Author: Andreas Schuster
*
*/
#ifndef SPHERETRANSFORMNODE_H_
#define SPHERETRANSFORMNODE_H_

#include <maya/MPxTransform.h>
#include <maya/MObject.h>

#include <maya/MPxTransformationMatrix.h>

class SphereTransformationMatrix : public MPxTransformationMatrix
{
public:
	SphereTransformationMatrix();
	~SphereTransformationMatrix() {}
	static void *creator();

    virtual MMatrix asMatrix() const;
    virtual MMatrix asMatrix(double percent) const;

    // Degrees
    double  getRadius() const;
    void    setRadius( double rock );

    static MTypeId s_id;
protected:
    double m_radius;
};


class SphereTransformNode : public MPxTransform
{
public:
	SphereTransformNode();
	SphereTransformNode(MPxTransformationMatrix* tm);
	virtual ~SphereTransformNode() {}
	virtual void postConstructor();
    virtual MPxTransformationMatrix* createTransformationMatrix();
    virtual void  resetTransformation (MPxTransformationMatrix *matrix);
    virtual void  resetTransformation (const MMatrix &matrix);
	SphereTransformationMatrix *getSphereTransformMatrix();
	virtual MStatus	compute(const MPlug& plug, MDataBlock& data);
	virtual MStatus validateAndSetValue(const MPlug& plug, const MDataHandle& handle, const MDGContext& context);
	static void *creator();
	static MStatus initialize();

public:
	static MTypeId s_id;
	static MString s_name;

protected:
	static MObject s_radius;

};

#endif 


