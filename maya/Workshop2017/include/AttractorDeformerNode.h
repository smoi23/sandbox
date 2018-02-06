/*
 * AttractorDeformerNode.h
 *
 *  Created on: Jul 6, 2017
 *      Author: Andreas Schusters
 */

#ifndef ATTRACTORDEFORMERNODE_H_
#define ATTRACTORDEFORMERNODE_H_

#include <maya/MPxDeformerNode.h>

class AttractorDeformer : public MPxDeformerNode
{

public:
	AttractorDeformer();
	virtual ~AttractorDeformer() {}
	static void* creator();
	static MStatus initialize();

	virtual MStatus deform(MDataBlock& 	block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex);
    virtual MObject& accessoryAttribute() const;
    virtual MStatus	 accessoryNodeSetup(MDagModifier& dagM);

public:
	static MTypeId s_id;
	static MString s_name;
	static MObject s_matrix;
	static MObject s_falloff;
private:
	double m_falloff;
};


#endif /* ATTRACTORDEFORMERNODE_H_ */
