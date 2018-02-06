/*
 * HelloWorld.cpp
 *
 *  Created on: Jul 10, 2017
 *      Author: Andreas Schuster (smoi)
 */

#include "HelloWorldCmd.h"
#include <stdio.h>
#include <maya/MGlobal.h>


MStatus HelloWorld::doIt(const MArgList &args)
{
	MStatus stat = MS::kSuccess;
	std::cout << "In HelloWorld::doIt()" << std::endl;
	MGlobal::displayInfo("In HelloWorld::doIt()");
	return stat;
}

void* HelloWorld::creator()
{
	return new HelloWorld();
}




