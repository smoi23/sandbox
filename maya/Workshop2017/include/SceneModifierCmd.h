/*
 * SceneModifierCmd.h
 *
 *  Created on: Jul 10, 2017
 *      Author: Andreas Schuster
 */

#ifndef INCLUDE_SCENEMODIFIERCMD_H_
#define INCLUDE_SCENEMODIFIERCMD_H_


#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MSelectionList.h>
#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>

\
class SceneModifier : public MPxCommand
{

public:

	SceneModifier();
	~SceneModifier();
	static MSyntax newSyntax();
	MStatus doIt(const MArgList &args);
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;
	static void* creator();
public:
	static MString s_name;
private:
	MStatus parseArgs(const MArgList &args);

private:
	bool m_doHelp;
	double m_size;
	MSelectionList m_cameras;
	MDagModifier m_dagmodifier;
	MObject m_root;
};


#endif /* INCLUDE_SCENEMODIFIERCMD_H_ */
