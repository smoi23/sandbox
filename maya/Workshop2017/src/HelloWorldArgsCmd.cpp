/*
 * HelloWorldArgs.cpp
 *
 *  Created on: Jul 10, 2017
 *      Author: Andreas Schuster (smoi)
 */

#include "HelloWorldArgsCmd.h"
#include <stdio.h>
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


#define kHelpFlag "-h"
#define kHelpFlagLong "-help"

#define kSizeFlag "s"
#define kSizeFlagLong "size"

MString HelloWorldArgs::s_name("helloWorldArgs");

HelloWorldArgs::HelloWorldArgs()
:
m_doHelp(false),
m_size(1.0)
{
	std::cout << "In HelloWorldArgs::HelloWorldArgs()" << std::endl;
}

HelloWorldArgs::~HelloWorldArgs()
{
	std::cout << "In HelloWorldArgs::~HelloWorldArgs()" << std::endl;
}

MSyntax HelloWorldArgs::newSyntax()
{
	MSyntax syntax;
	std::cout << "In HelloWorldArgs::newSyntax()" << std::endl;

	syntax.enableQuery(false);
	syntax.enableEdit(false);
	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1);
	syntax.addFlag(kHelpFlag, kHelpFlagLong, MSyntax::kNoArg);
	syntax.addFlag(kSizeFlag, kSizeFlagLong, MSyntax::kDouble);

	return syntax;
}

MStatus HelloWorldArgs::doIt(const MArgList &args)
{
	MStatus stat = MS::kSuccess;
	std::cout << "In HelloWorldArgs::doIt()" << std::endl;

	stat = parseArgs(args);

	if (m_doHelp)
	{
		MGlobal::displayInfo("Show help");
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

MStatus HelloWorldArgs::redoIt()
{
	std::cout << "In HelloWorldArgs::redoIt()" << std::endl;

	MStatus stat;

	MItSelectionList iter(m_selection);
	for (iter.reset(); !iter.isDone(); iter.next())
	{
		MDagPath dagpath;
		MObject component;
		iter.getDagPath(dagpath, component);
		MGlobal::displayInfo(dagpath.fullPathName());

		MItMeshVertex vertexIter(dagpath, component, &stat);
		if (stat == MS::kSuccess)
		{
			for (; !vertexIter.isDone(); vertexIter.next())
			{
				MPoint _p = vertexIter.position();
				MGlobal::displayInfo(MString() + vertexIter.index() + ": " +_p.x + " " + _p.y + " " + _p.z);
			}
		}
		else
		{
			MGlobal::displayWarning("Cant iterate vertices at: " + dagpath.fullPathName());
		}

	}

	return MS::kSuccess;
}
MStatus HelloWorldArgs::undoIt()
{
	std::cout << "In HelloWorldArgs::undoIt()" << std::endl;
	return MS::kSuccess;
}

bool HelloWorldArgs::isUndoable() const
{
	std::cout << "In HelloWorldArgs::isUndoable()" << std::endl;
	return false;
}

void* HelloWorldArgs::creator()
{
	std::cout << "In HelloWorldArgs::creator()" << std::endl;
	return new HelloWorldArgs();
}

MStatus HelloWorldArgs::parseArgs(const MArgList &args)
{
	MStatus stat = MS::kSuccess;

	//
	/*
	unsigned int pos = 0;
	MStringArray objects = args.asStringArray(pos, &stat);

	if (stat != MS::kSuccess)
	{
		MGlobal::displayError("Cant get input object list");
		return stat;
	}
	for (int i = 0; i < objects.length(); i++)
	{
		MGlobal::displayInfo("Object: " + objects[i]);
	}

	pos++;
	MString h = args.asString(pos, &stat);
	if (stat != MS::kSuccess)
	{
		MGlobal::displayError("Cant get flag key");
		return stat;
	}

	pos++;
	if (h == kHelpFlag)
	{
		m_doHelp = args.asBool(pos, &stat);
		if (stat != MS::kSuccess)
		{
			MGlobal::displayError("Cant get helper flag");
			return stat;
		}
	}
	*/

	MArgDatabase argData(syntax(), args);
	m_doHelp = argData.isFlagSet(kHelpFlag);

	if (argData.isFlagSet(kSizeFlagLong) && !argData.isQuery())
		argData.getFlagArgument(kSizeFlagLong, 0, m_size);

	argData.getObjects(m_selection);


	return stat;
}


