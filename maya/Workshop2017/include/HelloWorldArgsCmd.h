/*
 * HelloWorldArgs.h
 *
 *  Created on: Jul 10, 2017
 *      Author: Andreas Schuster (smoi)
 */
#pragma once
#ifndef INCLUDE_HELLOWORLDARGSCMD_H_
#define INCLUDE_HELLOWORLDARGSCMD_H_


#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MSelectionList.h>


class HelloWorldArgs : public MPxCommand
{

public:

	HelloWorldArgs();
	~HelloWorldArgs();
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
	MSelectionList m_selection;
	MSelectionList m_myselected;
};



#endif /* INCLUDE_HELLOWORLDARGSCMD_H_ */
