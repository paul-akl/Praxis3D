
#include "GeometryPass.h"
#include "LightingPass.h"
#include "FinalPass.h"
#include "ReflectionPass.h"
#include "RendererFrontend.h"

ErrorCode RendererFrontend::init()
{
	ErrorCode returnCode = ErrorCode::Success;

	// Get the current screen size
	m_frameData.m_screenSize.x = Config::graphicsVar().current_resolution_x;
	m_frameData.m_screenSize.y = Config::graphicsVar().current_resolution_y;

	// Initialize renderer backend and check if it was successful
	if(!ErrHandlerLoc::get().ifSuccessful(m_backend.init(m_frameData), returnCode))
		return returnCode;
	
	// Add geometry rendering pass, if it was initialized successfuly
	GeometryPass *geometryPass = new GeometryPass(*this);
	if(geometryPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(geometryPass);

	// Add lighting rendering pass, if it was initialized successfuly
	LightingPass *lightingPass = new LightingPass(*this);
	if(lightingPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(lightingPass);

	// Add final rendering pass, if it was initialized successfuly
	FinalPass *finalPass = new FinalPass(*this);
	if(finalPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(finalPass);

	passDrawCommandsToBackend();

	return returnCode;
}

void RendererFrontend::renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	// Clear draw commands at the beggining of each frame
	m_drawCommands.clear();

	// Load all the objects in the load-to-gpu queue. This needs to be done before any rendering, as objects in this
	// array might have been also added to objects-to-render arrays, so they need to be loaded first
	for(decltype(p_sceneObjects.m_objectsToLoad.size()) i = 0, size = p_sceneObjects.m_objectsToLoad.size(); i < size; i++)
	{
		queueForLoading(*p_sceneObjects.m_objectsToLoad[i]);
	}

	// Calculate view-projection matrix here, so it is only done once, since it only changes between frames
	Math::Mat4f viewProjMatrix = m_frameData.m_projMatrix * p_sceneObjects.m_camera->getBaseObjectData().m_modelMat;

	for(decltype(m_renderingPasses.size()) i = 0, size = m_renderingPasses.size(); i < size; i++)
	{
		m_renderingPasses[i]->update(p_sceneObjects, p_deltaTime);
	}

	passDrawCommandsToBackend();
}

