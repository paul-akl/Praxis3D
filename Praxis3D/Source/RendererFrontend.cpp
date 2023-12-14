
#include "AtmScatteringPass.h"
#include "BloomCompositePass.h"
#include "BloomPass.h"
#include "BlurPass.h"
#include "EntityViewDefinitions.h"
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
	// Disable GUI rendering until GUI render pass is set (as calling GUI functions from GUI components will crash without GUI rendering pass)
	m_guiRenderWasEnabled = Config::GUIVar().gui_render;
	Config::m_GUIVar.gui_render = false;

	m_renderingPassesSet = false;
	for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
		m_allRenderPasses[i] = nullptr;

	m_zFar = Config::graphicsVar().z_far;
	m_zNear = Config::graphicsVar().z_near;

	/*/ Set up the order of the rendering passes
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
	}*/
}

RendererFrontend::~RendererFrontend()
{
	// Delete rendering passes
	for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
	{
		// Check if has been created
		if(m_allRenderPasses[i] != nullptr)
			delete m_allRenderPasses[i];
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

	// Create the render pass data struct
	m_renderPassData = new RenderPassData();

	updateProjectionMatrix();

	passLoadCommandsToBackend();

	glViewport(0, 0, m_frameData.m_screenSize.x, m_frameData.m_screenSize.y);

	return returnCode;
}

const bool RendererFrontend::getRenderFinalToTexture() const { return (m_renderPassData != nullptr) ? m_renderPassData->getRenderFinalToTexture() : false; }

void RendererFrontend::setGUIPassFunctorSequence(FunctorSequence *p_GUIPassFunctorSequence)
{
	m_renderPassData->setGUIPassFunctorSequence(p_GUIPassFunctorSequence);
}

void RendererFrontend::setRenderFinalToTexture(const bool p_renderToTexture)
{
	if(m_renderPassData != nullptr)
		m_renderPassData->setRenderFinalToTexture(p_renderToTexture);
}

void RendererFrontend::setRenderToTextureResolution(const glm::ivec2 p_renderToTextureResolution)
{
	Config::m_graphicsVar.render_to_texture_resolution_x = p_renderToTextureResolution.x;
	Config::m_graphicsVar.render_to_texture_resolution_y = p_renderToTextureResolution.y;
}

void RendererFrontend::setRenderingPasses(const RenderingPasses &p_renderingPasses)
{
	m_renderingPassesSet = true;

	m_activeRenderPasses.clear();

	bool guiRenderPassSet = false;

	for(decltype(p_renderingPasses.size()) i = 0, size = p_renderingPasses.size(); i < size; i++)
	{
		switch(p_renderingPasses[i])
		{
			case RenderPassType_Geometry:
				if(m_allRenderPasses[RenderPassType_Geometry] == nullptr)
					m_allRenderPasses[RenderPassType_Geometry] = new GeometryPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Geometry]);
				break;
			case RenderPassType_Lighting:
				if(m_allRenderPasses[RenderPassType_Lighting] == nullptr)
					m_allRenderPasses[RenderPassType_Lighting] = new LightingPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Lighting]);
				break;
			case RenderPassType_AtmScattering:
				if(m_allRenderPasses[RenderPassType_AtmScattering] == nullptr)
					m_allRenderPasses[RenderPassType_AtmScattering] = new AtmScatteringPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_AtmScattering]);
				break;
			case RenderPassType_HdrMapping:
				if(m_allRenderPasses[RenderPassType_HdrMapping] == nullptr)
					m_allRenderPasses[RenderPassType_HdrMapping] = new HdrMappingPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_HdrMapping]);
				break;
			case RenderPassType_Blur:
				if(m_allRenderPasses[RenderPassType_Blur] == nullptr)
					m_allRenderPasses[RenderPassType_Blur] = new BlurPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Blur]);
				break;
			case RenderPassType_Bloom:
				if(m_allRenderPasses[RenderPassType_Bloom] == nullptr)
					m_allRenderPasses[RenderPassType_Bloom] = new BloomPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Bloom]);
				break;
			case RenderPassType_BloomComposite:
				if(m_allRenderPasses[RenderPassType_BloomComposite] == nullptr)
					m_allRenderPasses[RenderPassType_BloomComposite] = new BloomCompositePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_BloomComposite]);
				break;
			case RenderPassType_LenseFlare:
				if(m_allRenderPasses[RenderPassType_LenseFlare] == nullptr)
					m_allRenderPasses[RenderPassType_LenseFlare] = new LenseFlarePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_LenseFlare]);
				break;
			case RenderPassType_LenseFlareComposite:
				if(m_allRenderPasses[RenderPassType_LenseFlareComposite] == nullptr)
					m_allRenderPasses[RenderPassType_LenseFlareComposite] = new LenseFlareCompositePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_LenseFlareComposite]);
				break;
			case RenderPassType_Luminance:
				if(m_allRenderPasses[RenderPassType_Luminance] == nullptr)
					m_allRenderPasses[RenderPassType_Luminance] = new LuminancePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Luminance]);
				break;
			case RenderPassType_Final:
				if(m_allRenderPasses[RenderPassType_Final] == nullptr)
					m_allRenderPasses[RenderPassType_Final] = new FinalPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Final]);
				break;
			case RenderPassType_GUI:
				if(m_allRenderPasses[RenderPassType_GUI] == nullptr)
					m_allRenderPasses[RenderPassType_GUI] = new GUIPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_GUI]);
				guiRenderPassSet = true;
				break;
		}
	}

	// Disable GUI rendering if the GUI render pass wasn't set; re-enable GUI rendering if GUI pass was set and if it GUI rendering was enabled before
	if(!guiRenderPassSet)
		Config::m_GUIVar.gui_render = false;
	else
		if(m_guiRenderWasEnabled)
			Config::m_GUIVar.gui_render = true;

	for(decltype(m_activeRenderPasses.size()) size = m_activeRenderPasses.size(), i = 0; i < size; i++)
	{
		if(!m_activeRenderPasses[i]->isInitialized())
		{
			// Initialize the rendering pass and check if it was successful
			if(m_activeRenderPasses[i]->init() != ErrorCode::Success)
			{
				// Log an error and delete the rendering pass
				ErrHandlerLoc::get().log(ErrorType::Error, ErrorSource::Source_Renderer, m_activeRenderPasses[i]->getName() + " failed to load.");
				m_activeRenderPasses.erase(m_activeRenderPasses.begin() + i);
				i--;
			}
		}
	}

	//passLoadCommandsToBackend();
	glViewport(0, 0, m_frameData.m_screenSize.x, m_frameData.m_screenSize.y);
}

const RenderingPasses RendererFrontend::getRenderingPasses()
{
	RenderingPasses renderPasses;

	for(const auto renderPass : m_activeRenderPasses)
	{
		if(renderPass != nullptr)
		{
			renderPasses.push_back(renderPass->getRenderPassType());
		}
	}

	return renderPasses;
}

void RendererFrontend::renderFrame(SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	bool projectionMatrixNeedsUpdating = false;

	// Adjust rendering resolution if the screen size has changed
	if(m_renderPassData->m_renderFinalToTexture)
	{
		if(m_frameData.m_screenSize.x != Config::graphicsVar().render_to_texture_resolution_x ||
			m_frameData.m_screenSize.y != Config::graphicsVar().render_to_texture_resolution_y)
		{
			// Set the new resolution
			m_frameData.m_screenSize.x = Config::graphicsVar().render_to_texture_resolution_x;
			m_frameData.m_screenSize.y = Config::graphicsVar().render_to_texture_resolution_y;

			// Update the projection matrix because it is dependent on the screen size
			projectionMatrixNeedsUpdating = true;

			// Set screen size in the backend
			m_backend.setScreenSize(m_frameData);
		}
	}
	else
	{
		if(m_frameData.m_screenSize.x != Config::graphicsVar().current_resolution_x ||
			m_frameData.m_screenSize.y != Config::graphicsVar().current_resolution_y)
		{
			// Set the new resolution
			m_frameData.m_screenSize.x = Config::graphicsVar().current_resolution_x;
			m_frameData.m_screenSize.y = Config::graphicsVar().current_resolution_y;

			// Update the projection matrix because it is dependent on the screen size
			projectionMatrixNeedsUpdating = true;

			// Set screen size in the backend
			m_backend.setScreenSize(m_frameData);
		}
	}

	// If Z-buffer near or far values have changed, flag projection matrix for updating
	if(m_zFar != Config::graphicsVar().z_far || m_zNear != Config::graphicsVar().z_near)
	{
		m_zFar = Config::graphicsVar().z_far;
		m_zNear = Config::graphicsVar().z_near;
		projectionMatrixNeedsUpdating = true;
	}

	// Update the projection matrix if it was flagged
	if(projectionMatrixNeedsUpdating)
		updateProjectionMatrix();

	// Clear draw commands at the beginning of each frame
	m_drawCommands.clear();
	
	// Load all the objects in the load-to-GPU queue. This needs to be done before any rendering, as objects in this
	// array might have been also added to objects-to-render arrays, so they need to be loaded first
	for(decltype(p_sceneObjects.m_loadToVideoMemory.size()) i = 0, size = p_sceneObjects.m_loadToVideoMemory.size(); i < size; i++)
	{
		switch(p_sceneObjects.m_loadToVideoMemory[i].getType())
		{
		case LoadableObjectsContainer::LoadableObjectType_Model:
			queueForLoading(p_sceneObjects.m_loadToVideoMemory[i].getModelHandle());
			break;
		case LoadableObjectsContainer::LoadableObjectType_Shader:
			queueForLoading(*p_sceneObjects.m_loadToVideoMemory[i].getShaderProgram());
			break;
		case LoadableObjectsContainer::LoadableObjectType_Texture:
			queueForLoading(p_sceneObjects.m_loadToVideoMemory[i].getTextureHandle());
			break;
		}
	}

	// Release all objects from video memory that are in the unload-from-GPU queue
	for(decltype(p_sceneObjects.m_unloadFromVideoMemory.size()) i = 0, size = p_sceneObjects.m_unloadFromVideoMemory.size(); i < size; i++)
	{
		switch(p_sceneObjects.m_unloadFromVideoMemory[i].getType())
		{
			case LoadableObjectsContainer::LoadableObjectType_Model:
				queueForUnloading(p_sceneObjects.m_unloadFromVideoMemory[i].getModelHandle());
				break;
			case LoadableObjectsContainer::LoadableObjectType_Shader:
				queueForUnloading(*p_sceneObjects.m_unloadFromVideoMemory[i].getShaderProgram());
				break;
			case LoadableObjectsContainer::LoadableObjectType_Texture:
				queueForUnloading(p_sceneObjects.m_unloadFromVideoMemory[i].getTextureHandle());
				break;
		}
	}

	unsigned int numLoadedObjectsThisFrame = 0;
	const unsigned int maxLoadedObjectsThisFrame = Config::rendererVar().objects_loaded_per_frame;

	// Iterate over all objects that require to be loaded to video memory
	for(auto entity : p_sceneObjects.m_objectsToLoadToVideoMemory)
	{
		GraphicsLoadToVideoMemoryComponent &component = p_sceneObjects.m_objectsToLoadToVideoMemory.get<GraphicsLoadToVideoMemoryComponent>(entity);

		while(!component.m_objectsToLoad.empty())
		{
			if(numLoadedObjectsThisFrame++ >= maxLoadedObjectsThisFrame)
				goto jumpAfterLoading;

			LoadableObjectsContainer &loadableObject = component.m_objectsToLoad.front();

			switch(loadableObject.getType())
			{
			case LoadableObjectsContainer::LoadableObjectType_Model:
				queueForLoading(loadableObject.getModelHandle());
				loadableObject.getModelHandle().setLoadedToVideoMemory(true);
				break;

			case LoadableObjectsContainer::LoadableObjectType_Shader:
				queueForLoading(*loadableObject.getShaderProgram());
				loadableObject.getShaderProgram()->setLoadedToVideoMemory(true);
				break;

			case LoadableObjectsContainer::LoadableObjectType_Texture:
				queueForLoading(loadableObject.getTextureHandle());
				loadableObject.getTextureHandle().setLoadedToVideoMemory(true);
				break;
			}

			component.m_objectsToLoad.pop();
		}

		component.m_loaded = true;
	}

	jumpAfterLoading:

	// Handle loading before any rendering takes place
	passLoadCommandsToBackend();

	// Clear the load-to-GPU queue, since everything in it has been processed
	p_sceneObjects.m_loadToVideoMemory.clear();

	// Handle object unloading
	passUnloadCommandsToBackend();

	// Clear the unload-from-GPU queue
	p_sceneObjects.m_unloadFromVideoMemory.clear();
	
	// Calculate the view rotation matrix
	const glm::quat orientation = glm::normalize(glm::toQuat(p_sceneObjects.m_cameraViewMatrix));
	const glm::mat4 rotate = glm::mat4_cast(orientation);

	// Calculate the view translation matrix
	const glm::mat4 translate = glm::translate(glm::mat4(1.0f), -glm::vec3(p_sceneObjects.m_cameraViewMatrix[3]));

	// Set the full view matrix
	m_frameData.m_viewMatrix = rotate * translate;

	// Calculate the view-projection matrix here, so it is only done once, since it only changes between frames
	m_frameData.m_viewProjMatrix = m_frameData.m_projMatrix * m_frameData.m_viewMatrix;
	
	// Convert the view matrix to row major for the atmospheric scattering shaders
	m_frameData.m_transposeViewMatrix = glm::transpose(m_frameData.m_viewMatrix);

	// Set the camera position
	m_frameData.m_cameraPosition = p_sceneObjects.m_cameraViewMatrix[3];

	// Set the camera target vector
	m_frameData.m_cameraTarget = normalize(glm::vec3(0.0f, 0.0f, -1.0f) * glm::mat3(p_sceneObjects.m_cameraViewMatrix));
	
	// Prepare the geometry buffer for a new frame and a geometry pass
	m_backend.getGeometryBuffer()->initFrame();
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);		// Enable depth testing, as this is much like a regular forward render pass
	glClear(GL_DEPTH_BUFFER_BIT);	// Make sure to clear the depth buffer for the new frame

	// Set depth test function
	glDepthFunc(Config::rendererVar().depth_test_func);
	//glDisable(GL_CULL_FACE);

	for(decltype(m_activeRenderPasses.size()) i = 0, size = m_activeRenderPasses.size(); i < size; i++)
	{
		m_activeRenderPasses[i]->update(*m_renderPassData, p_sceneObjects, p_deltaTime);
	}
}

unsigned int RendererFrontend::getFramebufferTextureHandle(GBufferTextureType p_bufferType) const
{
	return m_backend.getFramebufferTextureHandle(p_bufferType); 
}
