/*
 * HelloWorld.h
 *
 *  Created on: Jul 10, 2017
 *      Author: Andreas Schuster (smoi)
 */

#ifndef INCLUDE_HELLOWORLDCMD_H_
#define INCLUDE_HELLOWORLDCMD_H_


#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>

class HelloWorld : public MPxCommand
{
public:
	MStatus doIt(const MArgList &args);
	static void* creator();
};



#endif /* INCLUDE_HELLOWORLDCMD_H_ */
