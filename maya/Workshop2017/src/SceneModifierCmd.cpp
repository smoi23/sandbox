/*
 * SceneModifierCmd.cpp
 *
 *  Created on: Jul 10, 2017
 *      Author: Andreas Schuster (smoi)
 */

#include "DebugRoutines.cpp"
#include "SceneModifierCmd.h"
#include <stdio.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>

#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>
#include <maya/MStringArray.h>
#include <maya/MGlobal.h>
#include <maya/MFnTransform.h>
#include <maya/MArgDatabase.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPoint.h>


#include <maya/MItDag.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnSet.h>

#define kHelpFlag "-h"
#define kHelpFlagLong "-help"

#define kSizeFlag "s"
#define kSizeFlagLong "size"


MString SceneModifier::s_name("sceneModifier");


SceneModifier::SceneModifier()
:
m_doHelp(false),
m_size(1.0)
{
	std::cout << "In SceneModifier::SceneModifier()" << std::endl;
}

SceneModifier::~SceneModifier()
{
	std::cout << "In SceneModifier::~SceneModifier()" << std::endl;
}

MSyntax SceneModifier::newSyntax()
{
	MSyntax syntax;
	std::cout << "In SceneModifier::newSyntax()" << std::endl;

	syntax.enableQuery(false);
	syntax.enableEdit(false);
	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1);
	syntax.addFlag(kHelpFlag, kHelpFlagLong, MSyntax::kNoArg);
	syntax.addFlag(kSizeFlag, kSizeFlagLong, MSyntax::kDouble);


	return syntax;
}

MStatus SceneModifier::doIt(const MArgList &args)
{
	MStatus stat = MS::kSuccess;
	std::cout << "In SceneModifier::doIt()" << std::endl;

	stat = parseArgs(args);

	if (m_doHelp)
	{
		MGlobal::displayInfo("Show help");
	}
	else
	{
		// collect all cameras in scene
		MItDependencyNodes nodeIter;
		m_cameras = MSelectionList();
		for(nodeIter.reset(); !nodeIter.isDone(); nodeIter.next())
		{
			MObject item = nodeIter.thisNode();
			if (item.hasFn(MFn::kCamera))
			{
				m_cameras.add(item);
			}
		}

		// get node by name
		MSelectionList selection;
		selection.add("|root");
		if (selection.length() > 0)
		{
			selection.getDependNode(0, m_root);
		}
		else
		{
			m_root = MObject::kNullObj;
		}

		// setup scene changes
		//
		// create root node if not exist
		selection.clear();
		selection.add("|root");
		if (selection.length() > 0)
		{
			selection.getDependNode(0, m_root);
		}
		else
		{
			m_root = m_dagmodifier.createNode("transform", MObject::kNullObj, &stat);
			m_dagmodifier.renameNode(m_root, "root");
			m_dagmodifier.doIt();
			CHECK_ERR(stat, "Create root");
		}

		// create mesh node
		MObject cubeMesh = m_dagmodifier.createNode("mesh", m_root);

		// create polySphere and connect it to mesh
		MObject polySphere = m_dgmodifier.createNode("polySphere", &stat);
		CHECK_ERR(stat, "Create PolySphere");
		MFnDependencyNode depFn(polySphere);
		MPlug outPlug = depFn.findPlug("output");

		depFn.setObject(cubeMesh);
		MPlug inPlug = depFn.findPlug("inMesh");

		m_dagmodifier.connect(outPlug, inPlug);

		stat = redoIt();
	}
	return stat;
}

MStatus SceneModifier::redoIt()
{
	MStatus stat = MS::kSuccess;
	std::cout << "In SceneModifier::redoIt()" << std::endl;

	m_dagmodifier.doIt();

	// print cameras
	MItSelectionList iterSelect(m_cameras);
	for (; !iterSelect.isDone(); iterSelect.next())
	{
		MObject item;
		iterSelect.getDependNode(item);
		MFnDependencyNode depnodeFn(item);
		MGlobal::displayInfo("Camera: " + depnodeFn.name() );
	}

	// display root name
	MFnDagNode dagFn(m_root);
	MGlobal::displayInfo("Got root " + dagFn.fullPathName());

	// move and scale root
	MFnTransform transformFn(m_root);
	transformFn.setTranslation(MVector (1, 2, 3), MSpace::kObject);
	const double scl[3] = { m_size,m_size,m_size };
	transformFn.setScale(scl);
	
	// assign initial shadingg group to the shape below root
	MDagPath dagpath;
	dagFn.getPath(dagpath);
	dagpath.extendToShape();
	MObject shape = dagpath.node();
	MSelectionList selection;
	stat = selection.add("initialShadingGroup");
	if (stat == MS::kSuccess)
	{
		MObject shaderNode;
		selection.getDependNode(0, shaderNode);
		MFnSet setFn(shaderNode);
		setFn.addMember(shape);
	}

	// print all child of type tranform below root
	if (!m_root.isNull())
	{
		MItDag dagIter(MItDag::kDepthFirst, MFn::kTransform);
		dagIter.reset(m_root);
		for( ; !dagIter.isDone(); dagIter.next())
		{
			MGlobal::displayInfo("DGraphIter: " + dagIter.fullPathName());
		}
	}

	return MS::kSuccess;
}
MStatus SceneModifier::undoIt()
{
	std::cout << "In SceneModifier::undoIt()" << std::endl;
	m_dgmodifier.undoIt();
	m_dagmodifier.undoIt();
	return MS::kSuccess;
}

bool SceneModifier::isUndoable() const
{
	std::cout << "In SceneModifier::isUndoable()" << std::endl;
	return true;
}

void* SceneModifier::creator()
{
	std::cout << "In SceneModifier::creator()" << std::endl;
	return new SceneModifier();
}

MStatus SceneModifier::parseArgs(const MArgList &args)
{
	MStatus stat = MS::kSuccess;

	MArgDatabase argData(syntax(), args);
	m_doHelp = argData.isFlagSet(kHelpFlag);

	if (argData.isFlagSet(kSizeFlagLong))
		argData.getFlagArgument(kSizeFlagLong, 0, m_size);

	return stat;
}

