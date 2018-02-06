/*
* pluginMain.cpp
*
*  Created on: May 10, 2017
*      Author: Andreas Schuster
*
*/
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxTransformationMatrix.h>

#include "SceneModifierCmd.h"
#include "BBoxCubeCmd.h"
#include "CopyMeshNode.h"
#include "SphereTransformNode.h"
#include "AttractorDeformerNode.h"

MStatus initializePlugin(MObject obj)
{
	MStatus stat;

	MFnPlugin plugin(obj, "Andreas Schuster", "1.0.0", "Any");

	/*
	stat = plugin.registerCommand("helloWorld", HelloWorld::creator);
	if (!stat)
	{
		stat.perror("registerCommand");
		return stat;
	}

	stat = plugin.registerCommand(HelloWorldArgs::s_name, HelloWorldArgs::creator, HelloWorldArgs::newSyntax);
	if (!stat)
	{
		stat.perror("registerCommand");
		return stat;
	}
	*/

	stat = plugin.registerCommand(SceneModifier::s_name, SceneModifier::creator, SceneModifier::newSyntax);
	if (!stat)
	{
		stat.perror("registerCommand");
		return stat;
	}

	stat = plugin.registerCommand(BBoxCubeCmd::s_name, BBoxCubeCmd::creator, BBoxCubeCmd::newSyntax);
	if (!stat)
	{
		stat.perror("registerCommand");
		return stat;
	}

	/*
	stat = plugin.registerUI("Workshop2017CreateUI", "Workshop2017DeleteUI");
	if (!stat) {
		stat.perror("registerUI");
		return stat;
	}
	*/

	/*
	stat = plugin.registerNode(hellowWorld::s_name, hellowWorld::s_id, hellowWorld::creator, hellowWorld::initialize);
	if (!stat)
	{
		stat.perror("registerNode");
		return stat;
	}
	*/

	stat = plugin.registerNode(CopyMeshNode::s_name, CopyMeshNode::s_id, CopyMeshNode::creator, CopyMeshNode::initialize);
	if (!stat)
	{
		stat.perror("registerNode");
		return stat;
	}
	

	// const MString classification = "drawdb/geometry/transform/sphereTransform";
	stat = plugin.registerTransform(SphereTransformNode::s_name, SphereTransformNode::s_id, SphereTransformNode::creator, SphereTransformNode::initialize,  SphereTransformationMatrix::creator, SphereTransformationMatrix::s_id, NULL); // , &classification);
	if (!stat)
	{
		stat.perror("registerTransformNode");
		return stat;
	}

	
	stat = plugin.registerNode(AttractorDeformer::s_name, AttractorDeformer::s_id, AttractorDeformer::creator, AttractorDeformer::initialize, MPxNode::kDeformerNode);
	if (!stat)
	{
		stat.perror("registerNode");
		return stat;
	}
	

	return stat;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus stat;
	MFnPlugin plugin(obj);

	/*
	stat = plugin.deregisterCommand("helloWorld");
	if (!stat)
	{
		stat.perror("deregisterCommand");
		return stat;
	}

	stat = plugin.deregisterCommand(HelloWorldArgs::s_name);
	if (!stat)
	{
		stat.perror("deregisterCommand");
		return stat;
	}
	*/

	stat = plugin.deregisterCommand(SceneModifier::s_name);
	if (!stat)
	{
		stat.perror("deregisterCommand");
		return stat;
	}

	/*
	stat = plugin.deregisterNode(hellowWorld::s_id);
	if (!stat)
	{
		stat.perror("deregisterNode");
		return stat;
	}
	*/

	stat = plugin.deregisterCommand(BBoxCubeCmd::s_name);
	if (!stat)
	{
		stat.perror("deregisterCommand");
		return stat;
	}


	stat = plugin.deregisterNode(CopyMeshNode::s_id);
	if (!stat)
	{
		stat.perror("deregisterNode");
		return stat;
	}
	
	stat = plugin.deregisterNode(SphereTransformNode::s_id);
	if (!stat)
	{
		stat.perror("deregisterTransformNode");
		return stat;
	}

	
	stat = plugin.deregisterNode(AttractorDeformer::s_id);
	if (!stat)
	{
		stat.perror("deregisterNode");
		return stat;
	}
	
	return stat;
}
