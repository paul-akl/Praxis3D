
#include "AtmScatteringPass.h"
#include "BloomCompositePass.h"
#include "BloomPass.h"
#include "BlurPass.h"
#include "GeometryPass.h"
#include "GUIPass.h"
#include "LenseFlareCompositePass.h"
#include "LenseFlarePass.h"
#include "LightingPass.h"
#include "LuminancePass.h"
#include "FinalPass.h"
#include "HdrMappingPass.h"
#include "PostProcessPass.h"
#include "ReflectionPass.h"
#include "RendererFrontend.h"
#include "SkyPass.h"

RendererFrontend::RendererFrontend() : m_renderPassData(nullptr)
{
	// Set up the order of the rendering passes
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_Geometry);
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_AtmScattering);
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_Lighting);
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_AtmScattering);
	//m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_HdrMapping);
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_Bloom);
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_Luminance);
	//m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_Blur);
	//m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_BloomComposite);
	//m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_LenseFlare);
	//m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_Blur);
	//m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_LenseFlareComposite);
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_Final);
	m_renderingPassesTypes.push_back(RenderPassType::RenderPassType_GUI);

	// Make sure the entries of the rendering passes are set to nullptr
	for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
		m_initializedRenderingPasses[i] = nullptr;

	// Create rendering passes
	for(decltype(m_renderingPassesTypes.size()) i = 0, size = m_renderingPassesTypes.size(); i < size; i++)
	{
		switch(m_renderingPassesTypes[i])
		{
		case RenderPassType_Geometry:
			if(m_initializedRenderingPasses[RenderPassType_Geometry] == nullptr)
				m_initializedRenderingPasses[RenderPassType_Geometry] = new GeometryPass(*this);
			break;
		case RenderPassType_Lighting:
			if(m_initializedRenderingPasses[RenderPassType_Lighting] == nullptr)
				m_initializedRenderingPasses[RenderPassType_Lighting] = new LightingPass(*this);
			break;
		case RenderPassType_AtmScattering:
			if(m_initializedRenderingPasses[RenderPassType_AtmScattering] == nullptr)
				m_initializedRenderingPasses[RenderPassType_AtmScattering] = new AtmScatteringPass(*this);
			break;
		case RenderPassType_HdrMapping:
			if(m_initializedRenderingPasses[RenderPassType_HdrMapping] == nullptr)
				m_initializedRenderingPasses[RenderPassType_HdrMapping] = new HdrMappingPass(*this);
			break;
		case RenderPassType_Blur:
			if(m_initializedRenderingPasses[RenderPassType_Blur] == nullptr)
				m_initializedRenderingPasses[RenderPassType_Blur] = new BlurPass(*this);
			break;
		case RenderPassType_Bloom:
			if(m_initializedRenderingPasses[RenderPassType_Bloom] == nullptr)
				m_initializedRenderingPasses[RenderPassType_Bloom] = new BloomPass(*this);
			break;
		case RenderPassType_BloomComposite:
			if(m_initializedRenderingPasses[RenderPassType_BloomComposite] == nullptr)
				m_initializedRenderingPasses[RenderPassType_BloomComposite] = new BloomCompositePass(*this);
			break;
		case RenderPassType_LenseFlare:
			if(m_initializedRenderingPasses[RenderPassType_LenseFlare] == nullptr)
				m_initializedRenderingPasses[RenderPassType_LenseFlare] = new LenseFlarePass(*this);
			break;
		case RenderPassType_LenseFlareComposite:
			if(m_initializedRenderingPasses[RenderPassType_LenseFlareComposite] == nullptr)
				m_initializedRenderingPasses[RenderPassType_LenseFlareComposite] = new LenseFlareCompositePass(*this);
			break;
		case RenderPassType_Luminance:
			if(m_initializedRenderingPasses[RenderPassType_Luminance] == nullptr)
				m_initializedRenderingPasses[RenderPassType_Luminance] = new LuminancePass(*this);
			break;
		case RenderPassType_Final:
			if(m_initializedRenderingPasses[RenderPassType_Final] == nullptr)
				m_initializedRenderingPasses[RenderPassType_Final] = new FinalPass(*this);
			break;
		case RenderPassType_GUI:
			if(m_initializedRenderingPasses[RenderPassType_GUI] == nullptr)
				m_initializedRenderingPasses[RenderPassType_GUI] = new GUIPass(*this);
			break;
		}
	}
}

RendererFrontend::~RendererFrontend()
{
	// Delete rendering passes
	for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
	{
		// Check if has been created
		if(m_initializedRenderingPasses[i] != nullptr)
			delete m_initializedRenderingPasses[i];
	}

	delete m_renderPassData;
}

ErrorCode RendererFrontend::init()
{
	ErrorCode returnCode = ErrorCode::Success;

	// Load all default textures from the texture loader, before a scene has started to load
	auto texturesToLoad = Loaders::texture2D().getDefaultTextures();
	for(decltype(texturesToLoad.size()) size = texturesToLoad.size(), i = 0; i < size; i++)
		queueForLoading(texturesToLoad[i]);

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

	// Initialize rendering passes
	for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
	{
		// Check if has been created
		if(m_initializedRenderingPasses[i] != nullptr)
		{
			// Initialize the rendering pass and check if it was successful
			if(m_initializedRenderingPasses[i]->init() != ErrorCode::Success)
			{
				// Log an error and delete the rendering pass
				ErrHandlerLoc::get().log(ErrorType::Error, ErrorSource::Source_Renderer, m_initializedRenderingPasses[i]->getName() + " failed to load.");
				delete m_initializedRenderingPasses[i];
				m_initializedRenderingPasses[i] = nullptr;
			}
		}
	}

	// Create the render pass data struct
	m_renderPassData = new RenderPassData();

	// Reserve the required space for rendering passes array
	m_renderingPasses.reserve(m_renderingPassesTypes.size());

	// Add required rendering passes to the main array
	for(decltype(m_renderingPassesTypes.size()) i = 0, size = m_renderingPassesTypes.size(); i < size; i++)
	{
		if(m_initializedRenderingPasses[m_renderingPassesTypes[i]] != nullptr)
			m_renderingPasses.push_back(m_initializedRenderingPasses[m_renderingPassesTypes[i]]);
	}

	updateProjectionMatrix();

	passLoadCommandsToBackend();

	glViewport(0, 0, m_frameData.m_screenSize.x, m_frameData.m_screenSize.y);

	return returnCode;
}

void RendererFrontend::renderFrame(SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	if(m_frameData.m_screenSize.x != Config::graphicsVar().current_resolution_x ||
		m_frameData.m_screenSize.y != Config::graphicsVar().current_resolution_y)
	{
		// Set the new resolution
		m_frameData.m_screenSize.x = Config::graphicsVar().current_resolution_x;
		m_frameData.m_screenSize.y = Config::graphicsVar().current_resolution_y;

		// Update the projection matrix because it is dependent on the screen size
		updateProjectionMatrix();

		// Set screen size in the backend
		m_backend.setScreenSize(m_frameData);
	}

	// Clear draw commands at the beginning of each frame
	m_drawCommands.clear();
	
	// Load all the objects in the load-to-GPU queue. This needs to be done before any rendering, as objects in this
	// array might have been also added to objects-to-render arrays, so they need to be loaded first
	for(decltype(p_sceneObjects.m_loadToVideoMemory.size()) i = 0, size = p_sceneObjects.m_loadToVideoMemory.size(); i < size; i++)
	{
		switch(p_sceneObjects.m_loadToVideoMemory[i].m_objectType)
		{
		case LoadableObjectsContainer::LoadableObjectType_Model:
			queueForLoading(p_sceneObjects.m_loadToVideoMemory[i].m_loadableObject.m_model);
			break;
		case LoadableObjectsContainer::LoadableObjectType_Shader:
			queueForLoading(*p_sceneObjects.m_loadToVideoMemory[i].m_loadableObject.m_shader);
			break;
		case LoadableObjectsContainer::LoadableObjectType_Texture:
			queueForLoading(p_sceneObjects.m_loadToVideoMemory[i].m_loadableObject.m_texture);
			break;
		}
	}

	// Handle loading before any rendering takes place
	passLoadCommandsToBackend();

	// Mark all loading-to-video-memory objects as loaded
	for(decltype(p_sceneObjects.m_loadToVideoMemory.size()) i = 0, size = p_sceneObjects.m_loadToVideoMemory.size(); i < size; i++)
	{
		switch(p_sceneObjects.m_loadToVideoMemory[i].m_objectType)
		{
		case LoadableObjectsContainer::LoadableObjectType_Model:
			p_sceneObjects.m_loadToVideoMemory[i].m_loadableObject.m_model.setLoadedToVideoMemory(true);
			break;
		case LoadableObjectsContainer::LoadableObjectType_Shader:
			p_sceneObjects.m_loadToVideoMemory[i].m_loadableObject.m_shader->setLoadedToVideoMemory(true);
			break;
		case LoadableObjectsContainer::LoadableObjectType_Texture:
			p_sceneObjects.m_loadToVideoMemory[i].m_loadableObject.m_texture.setLoadedToVideoMemory(true);
			break;
		}
	}

	glm::quat qPitch = glm::angleAxis(glm::radians(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.x), glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(glm::radians(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.y), glm::vec3(0, 1, 0));
	glm::quat qRoll = glm::angleAxis(glm::radians(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.z), glm::vec3(0, 0, 1));
	glm::quat orientation = qPitch * qYaw * qRoll;
	
	orientation = glm::toQuat(p_sceneObjects.m_camera.m_spatialData.m_transformMat);

	//glm::quat orientation = qPitch * qYaw * qRoll; 
	//glm::quat orientation(glm::radians(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler));
	orientation = glm::normalize(orientation);
	glm::mat4 rotate = glm::mat4_cast(orientation);
	glm::mat4 translate = glm::mat4(1.0f);
	//translate = glm::translate(translate, -p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position);
	translate = glm::translate(translate, -glm::vec3(p_sceneObjects.m_camera.m_spatialData.m_transformMat[3]));
	//translate = glm::translate(translate, -glm::vec3(1.0f, 10.0f, 1.0f));

	m_frameData.m_viewMatrix = rotate * translate;

	glm::mat4 rotationOnly = p_sceneObjects.m_camera.m_spatialData.m_transformMat;
	rotationOnly[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	//m_frameData.m_viewMatrix = rotationOnly * translate;


	//glm::vec3 direction = glm::rotate(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationQuat, glm::vec3(0.0f, 0.0f, -1.0f));
	glm::vec3 direction = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 0.0f, -1.0f)));
	//direction = glm::normalize(direction);
	glm::vec3 up = glm::normalize(glm::rotate(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationQuat, glm::vec3(0.0f, 1.0f, 0.0f)));

	//glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), -p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position);
	//glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, glm::radians(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.x), glm::vec3(1.0f, 0.0f, 0.0f));
	//glm::mat4 View = glm::rotate(ViewRotateX, glm::radians(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.y), glm::vec3(0.0f, 1.0f, 0.0f));

	//m_frameData.m_viewMatrix = View;

	//m_frameData.m_viewMatrix = glm::lookAt(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position, direction, up);
	//m_frameData.m_viewMatrix = glm::lookAt(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position, direction, up);
	//m_frameData.m_viewMatrix = glm::lookAt(p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position, glm::vec3(0.0f, 0.0, -1.0f), glm::vec3(0.0f, 1.0, 0.0f));

	// Calculate view and view-projection matrix here, so it is only done once, since it only changes between frames
	//m_frameData.m_viewMatrix = p_sceneObjects.m_camera.m_spatialData.m_transformMat;
	m_frameData.m_viewProjMatrix = m_frameData.m_projMatrix * m_frameData.m_viewMatrix;
	
	// Convert the view matrix to row major for the atmospheric scattering shaders
	m_frameData.m_transposeViewMatrix = glm::transpose(m_frameData.m_viewMatrix);

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
	m_frameData.m_cameraPosition = p_sceneObjects.m_camera.m_spatialData.m_transformMat[3]; //p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position;
	
	// Set the camera target vector
	m_frameData.m_cameraTarget = p_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler;

	// Assign directional light values and also normalize its direction, so it's not necessary to do it in a shader
	m_frameData.m_dirLightColor = p_sceneObjects.m_directionalLight->m_color;
	m_frameData.m_dirLightIntensity = p_sceneObjects.m_directionalLight->m_intensity;
	m_frameData.m_dirLightDirection = p_sceneObjects.m_directionalLight->m_direction;
	m_frameData.m_dirLightDirection = glm::normalize(m_frameData.m_dirLightDirection);

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