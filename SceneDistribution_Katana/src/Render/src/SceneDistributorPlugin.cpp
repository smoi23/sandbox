/*
-----------------------------------------------------------------------------
This source file is part of VPET - Virtual Production Editing Tool
http://vpet.research.animationsinstitut.de/
http://github.com/FilmakademieRnd/VPET

Copyright (c) 2016 Filmakademie Baden-Wuerttemberg, Institute of Animation

This project has been realized in the scope of the EU funded project Dreamspace
under grant agreement no 610005.
http://dreamspaceproject.eu/

This program is free software; you can redistribute it and/or modify it under
the terms of the MIT License as published by the Open Source Initiative.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the MIT License for more details.

You should have received a copy of the MIT License along with
this program; if not go to
https://opensource.org/licenses/MIT
-----------------------------------------------------------------------------
*/

#include "SceneDistributorPlugin.h"
#include "SceneIterator.h"

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnRenderOutputUtils/CameraInfo.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <vector>

#include <sstream>

#include <typeinfo>

namespace Dreamspace
{
namespace Katana
{

SceneDistributorPlugin::SceneDistributorPlugin(
    FnKat::FnScenegraphIterator rootIterator,
    FnKat::GroupAttribute arguments)
    : FnKat::Render::RenderBase(rootIterator, arguments)
{
    // Query render settings
    /*int width, height;
    FnKat::RenderOutputUtils::getRenderResolution(rootIterator, &width, &height);

    // Get buffer manager handles
    FnKat::StringAttribute hostArgumentAttr = arguments.getChildByName("host");
    std::string bufferInfoAddress = hostArgumentAttr.getValue("", false);
    _sharedState.bufferInfo = FnConvertStringToBufferInfoPtr(bufferInfoAddress);
    CreateBufferFunc allocatebuffer = _sharedState.bufferInfo->_creatBufferFuncPtr;

    // Allocate buffer to output the rendered image
    _sharedState.bufferHandle = allocatebuffer(_sharedState.bufferInfo->_rendererInstance, width, height, 4);*/
}

SceneDistributorPlugin::~SceneDistributorPlugin()
{
}

int SceneDistributorPlugin::start()
{
    // Setup network material terminal names
    std::vector<std::string> terminalNames;
    _sharedState.materialTerminalNamesAttr = FnAttribute::StringAttribute(terminalNames);

    FnKat::FnScenegraphIterator rootIterator = getRootIterator();

    // get header values
    FnAttribute::FloatAttribute lightIntensityAttr = rootIterator.getAttribute("sceneDistributorGlobalStatements.lightIntensityFactor");
    if ( lightIntensityAttr.isValid())
    {

        _sharedState.vpetHeader.lightIntensityFactor = lightIntensityAttr.getValue(1.0, false);
    }
    else
    {
         std::cout << "[INFO SceneDistributor] ERROR Not found attribute: lightIntensityFactor" << std::endl;
    }

    FnAttribute::IntAttribute textureTypeAttr = rootIterator.getAttribute("sceneDistributorGlobalStatements.textureBinaryType");
    if ( textureTypeAttr.isValid())
    {

        _sharedState.vpetHeader.textureBinaryType = textureTypeAttr.getValue(0, false);
    }
    else
    {
         std::cout << "[INFO SceneDistributor] ERROR Not found attribute: textureBinaryType" << std::endl;
    }

    // Traverse the scene graph
    FnKat::FnScenegraphIterator worldIterator = rootIterator.getChildByName("world");
    if (!worldIterator.isValid())
    {
        std::cerr << "[FATAL SceneDistributor] Could not find world." << std::endl;
        exit(-1);
    }


    // set init to all
    _sharedState.lodMode = ALL;
    // get values from root
    FnAttribute::StringAttribute lodModeAttr = rootIterator.getAttribute("sceneDistributorGlobalStatements.lod.mode");
    if ( lodModeAttr.isValid() )
    {
        std::string modeValue = lodModeAttr.getValue( "all", false );
        if ( modeValue == "by tag")
        {
            FnAttribute::StringAttribute lodTagAttr = rootIterator.getAttribute("sceneDistributorGlobalStatements.lod.selectionTag");
            if ( lodTagAttr.isValid() )
            {
                _sharedState.lodMode = TAG;
                _sharedState.lodTag = lodTagAttr.getValue( "lo", false );
            }
        }
    }

    std::cout << "[INFO SceneDistributor] LodMode: " << _sharedState.lodMode << "  LodTag: " << _sharedState.lodTag << std::endl;

    std::cout << "[INFO SceneDistributor] Building scene..." << std::endl;


    SceneIterator::buildLocation(worldIterator, &_sharedState);

    // Print stats
    std::cout << "[INFO SceneDistributorPlugin.start] Texture Count: " << _sharedState.texPackList.size() << std::endl;
    std::cout << "[INFO SceneDistributorPlugin.start] Object(Mesh) Count: " << _sharedState.objPackList.size() << std::endl;
    std::cout << "[INFO SceneDistributorPlugin.start] Node Count: " << _sharedState.nodeList.size() << std::endl;
    std::cout << "[INFO SceneDistributorPlugin.start] Objects: " << _sharedState.numObjectNodes << std::endl;
    std::cout << "[INFO SceneDistributorPlugin.start] Lights: " << _sharedState.numLights << std::endl;
    std::cout << "[INFO SceneDistributorPlugin.start] Cameras: " << _sharedState.numCameras << std::endl;

    //initalize zeroMQ thread
    std::cout << "Starting zeroMQ thread." << std::endl;
    pthread_t thrd;
    // pthread_create(&thrd, NULL, &server, static_cast<void*>(&_sharedState.objPack));
	pthread_create(&thrd, NULL, &server, static_cast<void*>(&_sharedState));
	std::cout << "zeroMQ thread started." << std::endl;

    return 0;
}

int SceneDistributorPlugin::pause()
{
    // Default:
    return FnKat::Render::RenderBase::pause();
}

int SceneDistributorPlugin::resume()
{
    // Default:
    return FnKat::Render::RenderBase::resume();
}

int SceneDistributorPlugin::stop()
{
    // Default:
    return FnKat::Render::RenderBase::stop();
}

int SceneDistributorPlugin::startLiveEditing()
{
    // Default:
    return FnKat::Render::RenderBase::startLiveEditing();
}

int SceneDistributorPlugin::stopLiveEditing()
{
    // Default:
    return FnKat::Render::RenderBase::stopLiveEditing();
}

int SceneDistributorPlugin::processControlCommand(const std::string& command)
{
    // Default:
    return FnKat::Render::RenderBase::processControlCommand(command);
}

int SceneDistributorPlugin::queueDataUpdates(FnKat::GroupAttribute updateAttribute)
{
    // Loop through all updates
    for (int i = 0; i < updateAttribute.getNumberOfChildren(); i++)
    {
        FnAttribute::GroupAttribute commandAttr = updateAttribute.getChildByIndex(i);
        if (!commandAttr.isValid())
        {
            continue;
        }

        // Prepare data of a single update
        FnAttribute::StringAttribute typeAttr = commandAttr.getChildByName("type");
        FnAttribute::StringAttribute locationAttr = commandAttr.getChildByName("location");
        FnAttribute::GroupAttribute attributesAttr = commandAttr.getChildByName("attributes");

        std::string type = typeAttr.getValue("", false);
        std::string location = locationAttr.getValue("", false);

        SceneDistributorPlugin::Update update;
        update.type = type;
        update.location = location;
        update.attributesAttr = attributesAttr;

        // Queue updates by type
        if(type == "camera" && attributesAttr.isValid())
        {
            // TODO: make queue thread-safe
            _cameraUpdates.push(update);
        }
    }
}

int SceneDistributorPlugin::applyPendingDataUpdates()
{
    // Re-render frame
}

static void* server(void *scene)
{
	// std::vector<ObjectPackage> *objPack = static_cast<std::vector<ObjectPackage>*>(scene);
	SceneDistributorPluginState* sharedState = static_cast<SceneDistributorPluginState*>(scene);

    std::cout << "Thread started. " << std::endl;

    zmq::context_t* context = new  zmq::context_t(1);
	zmq::socket_t* socket = new zmq::socket_t(*context, ZMQ_REP);
	socket->bind("tcp://*:5565");
	std::cout << "zeroMQ running, now entering while." << std::endl;

	while (1)
	{
		zmq::message_t message;
		char* responseMessageContent;
		char* messageStart;
		int responseLength = 0;
        socket->recv(&message);

        std::string msgString;
        const char* msgPointer = static_cast<const char*>(message.data());
        if ( msgPointer == NULL )
        {
             std::cout << "[INFO SceneDistributorPlugin.server] Error msgPointer is NULL" << std::endl;
        }
        else
        {
            msgString = std::string( static_cast<char*>(message.data()), message.size() );
        }

        std::cout << "[INFO SceneDistributorPlugin.server] Got request string: " << msgString << std::endl;

        if (msgString == "header")
        {
            std::cout << "[INFO SceneDistributorPlugin.server] Got Header Request" << std::endl;
            responseLength =sizeof(VpetHeader);
            messageStart = responseMessageContent = (char*)malloc(responseLength);
            memcpy(responseMessageContent, (char*)&(sharedState->vpetHeader), sizeof(VpetHeader));
        }
        else if (msgString == "objects")
		{
            std::cout << "[INFO SceneDistributorPlugin.server] Got Objects Request" << std::endl;
            std::cout << "[INFO SceneDistributorPlugin.server] Object count " << sharedState->objPackList.size() << std::endl;
            responseLength = sizeof(int) * 4 * sharedState->objPackList.size();
            for (int i = 0; i < sharedState->objPackList.size(); i++)
            {
                responseLength += sizeof(float) * sharedState->objPackList[i].vertices.size();
                responseLength += sizeof(int) * sharedState->objPackList[i].indices.size();
                responseLength += sizeof(float) * sharedState->objPackList[i].normals.size();
                responseLength += sizeof(float) * sharedState->objPackList[i].uvs.size();
            }

            messageStart = responseMessageContent = (char*)malloc(responseLength);

            for (int i = 0; i < sharedState->objPackList.size(); i++)
            {
                // vSize
                int numValues = sharedState->objPackList[i].vertices.size()/3.0;
                memcpy(responseMessageContent, (char*)&numValues, sizeof(int));
                responseMessageContent += sizeof(int);
                // vertices
                memcpy(responseMessageContent, &sharedState->objPackList[i].vertices[0], sizeof(float) * sharedState->objPackList[i].vertices.size());
                responseMessageContent += sizeof(float) * sharedState->objPackList[i].vertices.size();
                // iSize
                numValues = sharedState->objPackList[i].indices.size();
                memcpy(responseMessageContent, (char*)&numValues, sizeof(int));
                responseMessageContent += sizeof(int);
                // indices
                memcpy(responseMessageContent, &sharedState->objPackList[i].indices[0], sizeof(int) * sharedState->objPackList[i].indices.size());
                responseMessageContent += sizeof(int) * sharedState->objPackList[i].indices.size();
                // nSize
                numValues = sharedState->objPackList[i].normals.size()/3.0;
                memcpy(responseMessageContent, (char*)&numValues, sizeof(int));
                responseMessageContent += sizeof(int);
                // normals
                memcpy(responseMessageContent, &sharedState->objPackList[i].normals[0], sizeof(float) * sharedState->objPackList[i].normals.size());
                responseMessageContent += sizeof(float) * sharedState->objPackList[i].normals.size();
                // uSize
                numValues = sharedState->objPackList[i].uvs.size()/2.0;
                memcpy(responseMessageContent, (char*)&numValues, sizeof(int));
                responseMessageContent += sizeof(int);
                // uvs
                memcpy(responseMessageContent, &sharedState->objPackList[i].uvs[0], sizeof(float) * sharedState->objPackList[i].uvs.size());
                responseMessageContent += sizeof(float) * sharedState->objPackList[i].uvs.size();
            }

        }
        else if (msgString == "textures")
        {
            std::cout << "[INFO SceneDistributorPlugin.server] Got Textures Request" << std::endl;
            std::cout << "[INFO SceneDistributorPlugin.server] Texture count " << sharedState->texPackList.size() << std::endl;

            responseLength =  sizeof(int) + sizeof(int)*sharedState->texPackList.size();
            for (int i = 0; i < sharedState->texPackList.size(); i++)
            {
                responseLength += sharedState->texPackList[i].colorMapDataSize;
            }

            messageStart = responseMessageContent = (char*)malloc(responseLength);

            // texture binary type (image data (0) or raw unity texture data (1))
            int textureBinaryType = sharedState->textureBinaryType;
            std::cout << " textureBinaryType: " << textureBinaryType << std::endl;
            memcpy(responseMessageContent, (char*)&textureBinaryType, sizeof(int));
            responseMessageContent += sizeof(int);

            for (int i = 0; i < sharedState->texPackList.size(); i++)
            {
                memcpy(responseMessageContent, (char*)&sharedState->texPackList[i].colorMapDataSize, sizeof(int));
                responseMessageContent += sizeof(int);
                memcpy(responseMessageContent, sharedState->texPackList[i].colorMapData, sharedState->texPackList[i].colorMapDataSize);
                responseMessageContent += sharedState->texPackList[i].colorMapDataSize;
            }
        }
        else if (msgString == "nodes")
        {

            std::cout << "[INFO SceneDistributorPlugin.server] Got Nodes Request" << std::endl;
            std::cout << "[INFO SceneDistributorPlugin.server] Node count " << sharedState->nodeList.size() << " Node Type count " << sharedState->nodeList.size() << std::endl;

            // set the size from type- and namelength
            responseLength =  sizeof(NodeType) * sharedState->nodeList.size();

            // extend with sizeof node depending on node type
            for (int i = 0; i < sharedState->nodeList.size(); i++)
            {

                if ( sharedState->nodeTypeList[i] == NodeType::GEO)
                    responseLength += sizeof_nodegeo;
                else if ( sharedState->nodeTypeList[i] == NodeType::LIGHT)
                    responseLength += sizeof_nodelight;
                else if ( sharedState->nodeTypeList[i] == NodeType::CAMERA)
                    responseLength += sizeof_nodecam;
                else
                    responseLength += sizeof_node;

            }

            // allocate memory for out byte stream
            messageStart = responseMessageContent = (char*)malloc(responseLength);

            // iterate over node list copy data to out byte stream
            for (int i = 0; i < sharedState->nodeList.size(); i++)
            {
                Node* node = sharedState->nodeList[i];


                // First Copy node type
                int nodeType = sharedState->nodeTypeList[i];
                memcpy(responseMessageContent, (char*)&nodeType, sizeof(int));
                responseMessageContent += sizeof(int);

                // Copy specific node data
                if (sharedState->nodeTypeList[i] == NodeType::GEO)
                {
                    memcpy(responseMessageContent, node, sizeof_nodegeo);
                    responseMessageContent += sizeof_nodegeo;
                }
                else if (sharedState->nodeTypeList[i] == NodeType::LIGHT)
                {
                    memcpy(responseMessageContent, node, sizeof_nodelight);
                    responseMessageContent += sizeof_nodelight;
                }
                else if (sharedState->nodeTypeList[i] == NodeType::CAMERA)
                {
                    memcpy(responseMessageContent, node, sizeof_nodecam);
                    responseMessageContent += sizeof_nodecam;
                }
                else
                {
                    memcpy(responseMessageContent, node, sizeof_node);
                    responseMessageContent += sizeof_node;
                }
            }

        }

        std::cout << "[INFO SceneDistributorPlugin.server] Send message length: " << responseLength << std::endl;
        zmq::message_t responseMessage((void*)messageStart, responseLength, NULL);
        socket->send(responseMessage);

    }

    return 0;
}

bool SceneDistributorPlugin::hasPendingDataUpdates() const
{
    // Check for any queued updates
    return !_cameraUpdates.empty();
}

void SceneDistributorPlugin::updateCamera()
{
    // Apply all queued camera updates
}


void SceneDistributorPlugin::configureDiskRenderOutputProcess(
    FnKat::Render::DiskRenderOutputProcess& diskRenderOutputProcess,
    const std::string& outputName,
    const std::string& outputPath,
    const std::string& renderMethodName,
    const float& frameTime) const
{
    // e.g.

    // The render action used for this render output:
    std::auto_ptr<FnKat::Render::RenderAction> renderAction;

    // Set the render action to do nothing:
    renderAction.reset(new FnKat::Render::NoOutputRenderAction());

    // Pass ownership of the renderAction to the diskRenderOutputProcess:
    diskRenderOutputProcess.setRenderAction(renderAction);
}

DEFINE_RENDER_PLUGIN(SceneDistributorPlugin)

}
}

void registerPlugins()
{
    REGISTER_PLUGIN(Dreamspace::Katana::SceneDistributorPlugin, "sceneDistributor", 0, 1);
}
