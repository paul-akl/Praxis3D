
#include "AtmScatteringPass.h"
#include "BlurPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "FinalPass.h"
#include "HdrMappingPass.h"
#include "PostProcessPass.h"
#include "ReflectionPass.h"
#include "RendererFrontend.h"
#include "SkyPass.h"

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

	// Create the render pass data struct
	m_renderPassData = new RenderPassData();

	m_renderingPasses.reserve(7);

	// Add geometry rendering pass, if it was initialized successfuly
	GeometryPass *geometryPass = new GeometryPass(*this);
	if(geometryPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(geometryPass);
	
	// Add SKY atmospheric scattering pass, if it was initialized successfuly
	AtmScatteringPass *atmScatteringPass = new AtmScatteringPass(*this);
	ErrorCode atmScatPassError = atmScatteringPass->init();
	if(atmScatPassError == ErrorCode::Success)
		m_renderingPasses.push_back(atmScatteringPass);

	// Add lighting rendering pass, if it was initialized successfuly
	LightingPass *lightingPass = new LightingPass(*this);
	if(lightingPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(lightingPass);

	// Add GROUND atmospheric scattering pass, if it was initialized successfuly
	if(atmScatPassError == ErrorCode::Success)
		m_renderingPasses.push_back(atmScatteringPass);

	// Add HDR mapping rendering pass, if it was initialized successfully
	HdrMappingPass *hdrPass = new HdrMappingPass(*this);
	if(hdrPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(hdrPass);

	// Add blur rendering pass, if it was initialized successfully
	BlurPass *blurPass = new BlurPass(*this);
	if(blurPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(blurPass);

	// Add post process rendering pass, if it was initialized successfully
	PostProcessPass *postProcessPass = new PostProcessPass(*this);
	if(postProcessPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(postProcessPass);

	// Add final rendering pass, if it was initialized successfuly
	FinalPass *finalPass = new FinalPass(*this);
	if(finalPass->init() == ErrorCode::Success)
		m_renderingPasses.push_back(finalPass);

	//if(atmScatteringPass->init() == ErrorCode::Success)
	//	m_renderingPasses.push_back(atmScatteringPass);

	updateProjectionMatrix();

	passLoadCommandsToBackend();

	glViewport(0, 0, m_frameData.m_screenSize.x, m_frameData.m_screenSize.y);

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
	
	// Convert the view matrix to row major for the atmospheric scattering shaders
	m_frameData.m_transposeViewMatrix = Math::transpose(m_frameData.m_viewMatrix);

	/*auto tempMatrix = m_frameData.m_viewMatrix;
	
	m_frameData.m_viewMatrix.m[0] = tempMatrix.m[0];
	m_frameData.m_viewMatrix.m[1] = tempMatrix.m[4];
	m_frameData.m_viewMatrix.m[2] = tempMatrix.m[8];
	m_frameData.m_viewMatrix.m[3] = tempMatrix.m[12];
	m_frameData.m_viewMatrix.m[4] = tempMatrix.m[1];
	m_frameData.m_viewMatrix.m[5] = tempMatrix.m[5];
	m_frameData.m_viewMatrix.m[6] = tempMatrix.m[9];
	m_frameData.m_viewMatrix.m[7] = tempMatrix.m[13];
	m_frameData.m_viewMatrix.m[8] = tempMatrix.m[2];
	m_frameData.m_viewMatrix.m[9] = tempMatrix.m[6];
	m_frameData.m_viewMatrix.m[10] = tempMatrix.m[10];
	m_frameData.m_viewMatrix.m[11] = tempMatrix.m[14];
	m_frameData.m_viewMatrix.m[12] = tempMatrix.m[3];
	m_frameData.m_viewMatrix.m[13] = tempMatrix.m[7];
	m_frameData.m_viewMatrix.m[14] = tempMatrix.m[11];
	m_frameData.m_viewMatrix.m[15] = tempMatrix.m[15];*/

	/*std::cout << "VIEW MATRIX:" << std::endl;
	std::cout << m_frameData.m_viewMatrix.m[0] << " : " << m_frameData.m_viewMatrix.m[1] << " : " << m_frameData.m_viewMatrix.m[2] << " : " << m_frameData.m_viewMatrix.m[3] << std::endl;
	std::cout << m_frameData.m_viewMatrix.m[4] << " : " << m_frameData.m_viewMatrix.m[5] << " : " << m_frameData.m_viewMatrix.m[6] << " : " << m_frameData.m_viewMatrix.m[7] << std::endl;
	std::cout << m_frameData.m_viewMatrix.m[8] << " : " << m_frameData.m_viewMatrix.m[9] << " : " << m_frameData.m_viewMatrix.m[10] << " : " << m_frameData.m_viewMatrix.m[11] << std::endl;
	std::cout << m_frameData.m_viewMatrix.m[12] << " : " << m_frameData.m_viewMatrix.m[13] << " : " << m_frameData.m_viewMatrix.m[14] << " : " << m_frameData.m_viewMatrix.m[15] << std::endl;
	*/
	// Set the camera position
	m_frameData.m_cameraPosition = p_sceneObjects.m_camera->getVec3(nullptr, Systems::Changes::Spacial::Position);
	
	// Set the camera target vector
	m_frameData.m_cameraTarget = p_sceneObjects.m_camera->getVec3(nullptr, Systems::Changes::Spacial::Rotation);

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
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);		// Enable depth testing, as this is much like a regular forward render pass
	glClear(GL_DEPTH_BUFFER_BIT);	// Make sure to clear the depth buffer for the new frame

	// Set depth test function
	glDepthFunc(Config::rendererVar().depth_test_func);
	//glDisable(GL_CULL_FACE);

	//m_renderingPasses[0]->update(p_sceneObjects, p_deltaTime);

	for(decltype(m_renderingPasses.size()) i = 0, size = m_renderingPasses.size(); i < size; i++)
	{
		m_renderingPasses[i]->update(*m_renderPassData, p_sceneObjects, p_deltaTime);
	}
}

