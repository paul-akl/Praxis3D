
#include "BlurPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "FinalPass.h"
#include "ReflectionPass.h"
#include "RendererFrontend.h"

RendererFrontend::~RendererFrontend()
{
	// Delete rendering passes
	//for(decltype(m_renderingPasses.size()) i = 0, size = m_renderingPasses.size(); i < size; i++)
	//{
		//delete m_renderingPasses[i];
	//}
}

ErrorCode RendererFrontend::init()
{
	ErrorCode returnCode = ErrorCode::Success;

	// If eye adaption is disabled, eye adaption rate should be set to 0
	if(!Config::graphicsVar().eye_adaption)
	{
		Config::m_graphicsVar.eye_adaption_rate = 0.0f;
	}

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

	// Add blur rendering pass, if it was initialized successfully
	//BlurPass *blurPass = new BlurPass(*this);
	//if(blurPass->init() == ErrorCode::Success)
	//	m_renderingPasses.push_back(blurPass);

	// Add final rendering pass, if it was initialized successfuly
	FinalPass *finalPass = new FinalPass(*this);
	if(finalPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(finalPass);

	updateProjectionMatrix();

	passLoadCommandsToBackend();

	return returnCode;
}

void RendererFrontend::renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	if(m_frameData.m_screenSize.x != Config::graphicsVar().current_resolution_x ||
		m_frameData.m_screenSize.y != Config::graphicsVar().current_resolution_y)
	{
		// Set the new resolution
		m_frameData.m_screenSize.x = Config::graphicsVar().current_resolution_x;
		m_frameData.m_screenSize.y = Config::graphicsVar().current_resolution_y;

		// Update the projection matrix because it is dependant on the screen size
		updateProjectionMatrix();

		// Set screen size in the backend
		m_backend.setScreenSize(m_frameData);
	}

	// Clear draw commands at the beggining of each frame
	m_drawCommands.clear();

	//if(p_sceneObjects.m_staticSkybox.)

	// Load all the objects in the load-to-gpu queue. This needs to be done before any rendering, as objects in this
	// array might have been also added to objects-to-render arrays, so they need to be loaded first
	for (decltype(p_sceneObjects.m_objectsToLoad.size()) i = 0, size = p_sceneObjects.m_objectsToLoad.size(); i < size; i++)
	{
		queueForLoading(*p_sceneObjects.m_objectsToLoad[i]);
	}

	// Handle loading before any rendering takes place
	passLoadCommandsToBackend();

	// Calculate view and view-projection matrix here, so it is only done once, since it only changes between frames
	m_frameData.m_viewMatrix = p_sceneObjects.m_camera->getBaseObjectData().m_modelMat;
	m_frameData.m_viewProjMatrix = m_frameData.m_projMatrix * p_sceneObjects.m_camera->getBaseObjectData().m_modelMat;
	
	// Set the camera position
	m_frameData.m_cameraPosition = p_sceneObjects.m_camera->getVec3(nullptr, Systems::Changes::Spacial::Position);
	
	// Assign directional light values and also normalize its direction, so it's not neccessary to do it in a shader
	m_frameData.m_dirLightColor = p_sceneObjects.m_directionalLight->m_color;
	m_frameData.m_dirLightIntensity = p_sceneObjects.m_directionalLight->m_intensity;
	m_frameData.m_dirLightDirection = p_sceneObjects.m_directionalLight->m_direction;
	m_frameData.m_dirLightDirection.normalize();

	// Set number of lights so they can be send to the shader
	m_frameData.m_numPointLights = (decltype(m_frameData.m_numPointLights))p_sceneObjects.m_pointLights.size();
	m_frameData.m_numSpotLights = (decltype(m_frameData.m_numSpotLights))p_sceneObjects.m_spotLights.size();
	
	// Prepare the geometry buffer for a new frame and a geometry pass
	m_backend.getGeometryBuffer()->initFrame();
	m_backend.getGeometryBuffer()->initGeometryPass();

	glEnable(GL_DEPTH_TEST);		// Enable depth testing, as this is much like a regular forward render pass
	glClear(GL_DEPTH_BUFFER_BIT);	// Make sure to clear the depth buffer for the new frame
	glDisable(GL_CULL_FACE);

	//m_renderingPasses[0]->update(p_sceneObjects, p_deltaTime);

	for(decltype(m_renderingPasses.size()) i = 0, size = m_renderingPasses.size(); i < size; i++)
	{
		m_renderingPasses[i]->update(p_sceneObjects, p_deltaTime);
	}
}

