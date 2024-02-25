
#include "AmbientOcclusionPass.h"
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
#include "ShadowMappingPass.h"
#include "SkyPass.h"
#include "TonemappingPass.h"

RendererFrontend::RendererFrontend() : m_renderPassData(nullptr)
{
	m_antialiasingType = AntiAliasingType::AntiAliasingType_None;

	// Disable GUI rendering until GUI render pass is set (as calling GUI functions from GUI components will crash without GUI rendering pass)
	m_guiRenderWasEnabled = Config::GUIVar().gui_render;
	Config::m_GUIVar.gui_render = false;

	m_renderingPassesSet = false;
	for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
		m_allRenderPasses[i] = nullptr;
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

	// Set the anti-aliasing type from Config (if it is valid)
	if(Config::graphicsVar().antialiasing_type >= 0 && Config::graphicsVar().antialiasing_type < AntiAliasingType::AntiAliasingType_NumOfTypes)
		m_antialiasingType = static_cast<AntiAliasingType>(Config::graphicsVar().antialiasing_type);

	// Set the anti-aliasing method based on specified type
	setAntialiasingMethod(m_antialiasingType);

	// Load all default textures from the texture loader, before a scene has started to load
	auto texturesToLoad = Loaders::texture2D().getDefaultTextures();
	for(decltype(texturesToLoad.size()) size = texturesToLoad.size(), i = 0; i < size; i++)
	{
		queueForLoading(texturesToLoad[i]);
		texturesToLoad[i].setLoadedToVideoMemory(true);
	}

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
	bool shadowMappingPassSet = false;

	for(decltype(p_renderingPasses.size()) i = 0, size = p_renderingPasses.size(); i < size; i++)
	{
		switch(p_renderingPasses[i])
		{
			case RenderPassType::RenderPassType_Geometry:
				if(m_allRenderPasses[RenderPassType::RenderPassType_Geometry] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_Geometry] = new GeometryPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Geometry]);
				break;
			case RenderPassType::RenderPassType_ShadowMapping:
				if(m_allRenderPasses[RenderPassType::RenderPassType_ShadowMapping] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_ShadowMapping] = new ShadowMappingPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_ShadowMapping]);
				shadowMappingPassSet = true;
				break;
			case RenderPassType::RenderPassType_Lighting:
				if(m_allRenderPasses[RenderPassType::RenderPassType_Lighting] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_Lighting] = new LightingPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType_Lighting]);
				break;
			case RenderPassType::RenderPassType_AtmScattering:
				if(m_allRenderPasses[RenderPassType::RenderPassType_AtmScattering] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_AtmScattering] = new AtmScatteringPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_AtmScattering]);
				break;
			case RenderPassType::RenderPassType_HdrMapping:
				if(m_allRenderPasses[RenderPassType::RenderPassType_HdrMapping] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_HdrMapping] = new HdrMappingPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_HdrMapping]);
				break;
			case RenderPassType::RenderPassType_Blur:
				if(m_allRenderPasses[RenderPassType::RenderPassType_Blur] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_Blur] = new BlurPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_Blur]);
				break;
			case RenderPassType::RenderPassType_Bloom:
				if(m_allRenderPasses[RenderPassType::RenderPassType_Bloom] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_Bloom] = new BloomPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_Bloom]);
				break;
			case RenderPassType::RenderPassType_BloomComposite:
				if(m_allRenderPasses[RenderPassType::RenderPassType_BloomComposite] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_BloomComposite] = new BloomCompositePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_BloomComposite]);
				break;
			case RenderPassType::RenderPassType_LenseFlare:
				if(m_allRenderPasses[RenderPassType::RenderPassType_LenseFlare] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_LenseFlare] = new LenseFlarePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_LenseFlare]);
				break;
			case RenderPassType::RenderPassType_LenseFlareComposite:
				if(m_allRenderPasses[RenderPassType::RenderPassType_LenseFlareComposite] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_LenseFlareComposite] = new LenseFlareCompositePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_LenseFlareComposite]);
				break;
			case RenderPassType::RenderPassType_Luminance:
				if(m_allRenderPasses[RenderPassType::RenderPassType_Luminance] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_Luminance] = new LuminancePass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_Luminance]);
				break;
			case RenderPassType::RenderPassType_Final:
				if(m_allRenderPasses[RenderPassType::RenderPassType_Final] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_Final] = new FinalPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_Final]);
				break;
			case RenderPassType::RenderPassType_GUI:
				if(m_allRenderPasses[RenderPassType::RenderPassType_GUI] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_GUI] = new GUIPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_GUI]);
				guiRenderPassSet = true;
				break;
			case RenderPassType::RenderPassType_AmbientOcclusion:
				if(m_allRenderPasses[RenderPassType::RenderPassType_AmbientOcclusion] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_AmbientOcclusion] = new AmbientOcclusionPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_AmbientOcclusion]);
				break;
			case RenderPassType::RenderPassType_Tonemapping:
				if(m_allRenderPasses[RenderPassType::RenderPassType_Tonemapping] == nullptr)
					m_allRenderPasses[RenderPassType::RenderPassType_Tonemapping] = new TonemappingPass(*this);
				m_activeRenderPasses.push_back(m_allRenderPasses[RenderPassType::RenderPassType_Tonemapping]);
				break;
		}
	}

	// Disable GUI rendering if the GUI render pass wasn't set; re-enable GUI rendering if GUI pass was set and if it GUI rendering was enabled before
	if(!guiRenderPassSet)
		Config::m_GUIVar.gui_render = false;
	else
		if(m_guiRenderWasEnabled)
			Config::m_GUIVar.gui_render = true;

	// Set the shadow mapping enabled flag
	m_frameData.m_shadowMappingData.m_shadowMappingEnabled = shadowMappingPassSet;

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

	// Check if the anti-aliasing type has changed
	if(m_antialiasingType != Config::graphicsVar().antialiasing_type)
	{
		// Set the anti-aliasing type from Config (if it is valid)
		if(Config::graphicsVar().antialiasing_type >= 0 && Config::graphicsVar().antialiasing_type < AntiAliasingType::AntiAliasingType_NumOfTypes)
		{
			m_antialiasingType = static_cast<AntiAliasingType>(Config::graphicsVar().antialiasing_type);

			// Set the anti-aliasing method based on specified type
			setAntialiasingMethod(m_antialiasingType);
		}
	}

	// Adjust rendering resolution if the screen size has changed
	if(m_renderPassData->m_renderFinalToTexture)
	{
		if(m_frameData.m_screenSize.x != Config::graphicsVar().render_to_texture_resolution_x ||
			m_frameData.m_screenSize.y != Config::graphicsVar().render_to_texture_resolution_y)
		{
			// Set the new resolution
			m_frameData.m_screenSize.x = Config::graphicsVar().render_to_texture_resolution_x;
			m_frameData.m_screenSize.y = Config::graphicsVar().render_to_texture_resolution_y;

			// Set the inverse of the new resolution
			m_frameData.m_inverseScreenSize.x = 1.0f / (float)m_frameData.m_screenSize.x;
			m_frameData.m_inverseScreenSize.y = 1.0f / (float)m_frameData.m_screenSize.y;

			// Update the projection matrix because it is dependent on the screen size
			projectionMatrixNeedsUpdating = true;

			// Set screen size in the backend
			m_backend.setScreenSize(m_frameData);
		}
	}
	else
	{
		// Set the gobal variables for the viewport position
		Config::m_rendererVar.current_viewport_position_x = 0.0f;
		Config::m_rendererVar.current_viewport_position_y = 0.0f;

		if(m_frameData.m_screenSize.x != Config::graphicsVar().current_resolution_x ||
			m_frameData.m_screenSize.y != Config::graphicsVar().current_resolution_y)
		{
			// Set the new resolution
			m_frameData.m_screenSize.x = Config::graphicsVar().current_resolution_x;
			m_frameData.m_screenSize.y = Config::graphicsVar().current_resolution_y;

			// Set the inverse of the new resolution
			m_frameData.m_inverseScreenSize.x = 1.0f / (float)m_frameData.m_screenSize.x;
			m_frameData.m_inverseScreenSize.y = 1.0f / (float)m_frameData.m_screenSize.y;

			// Update the projection matrix because it is dependent on the screen size
			projectionMatrixNeedsUpdating = true;

			// Set screen size in the backend
			m_backend.setScreenSize(m_frameData);
		}
	}

	// Set the global variables for the current viewport size
	Config::m_rendererVar.current_viewport_size_x = m_frameData.m_screenSize.x;
	Config::m_rendererVar.current_viewport_size_y = m_frameData.m_screenSize.y;

	// If Z-buffer near or far or FOV values have changed, flag projection matrix for updating
	if(m_frameData.m_zFar != p_sceneObjects.m_zFar || m_frameData.m_zNear != p_sceneObjects.m_zNear || m_frameData.m_fov != p_sceneObjects.m_fov)
	{
		m_frameData.m_zFar = p_sceneObjects.m_zFar;
		m_frameData.m_zNear = p_sceneObjects.m_zNear;
		m_frameData.m_fov = p_sceneObjects.m_fov;
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
			if(!p_sceneObjects.m_loadToVideoMemory[i].getModelHandle().isLoadedToVideoMemory())
			{
				queueForLoading(p_sceneObjects.m_loadToVideoMemory[i].getModelHandle());
				p_sceneObjects.m_loadToVideoMemory[i].getModelHandle().setLoadedToVideoMemory(true);
			}
			break;
		case LoadableObjectsContainer::LoadableObjectType_Shader:
			if(!p_sceneObjects.m_loadToVideoMemory[i].getShaderProgram()->isLoadedToVideoMemory())
			{
				queueForLoading(*p_sceneObjects.m_loadToVideoMemory[i].getShaderProgram());
				p_sceneObjects.m_loadToVideoMemory[i].getShaderProgram()->setLoadedToVideoMemory(true);
			}
			break;
		case LoadableObjectsContainer::LoadableObjectType_Texture:
			//if(!p_sceneObjects.m_loadToVideoMemory[i].getTextureHandle().isLoadedToVideoMemory())
			{
				queueForLoading(p_sceneObjects.m_loadToVideoMemory[i].getTextureHandle());
				p_sceneObjects.m_loadToVideoMemory[i].getTextureHandle().setLoadedToVideoMemory(true);
			}
			break;
		}
	}

	// Release all objects from video memory that are in the unload-from-GPU queue
	for(decltype(p_sceneObjects.m_unloadFromVideoMemory.size()) i = 0, size = p_sceneObjects.m_unloadFromVideoMemory.size(); i < size; i++)
	{
		queueForUnloading(p_sceneObjects.m_unloadFromVideoMemory[i]);
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
				if(!loadableObject.getModelHandle().isLoadedToVideoMemory())
				{
					queueForLoading(loadableObject.getModelHandle());
					loadableObject.getModelHandle().setLoadedToVideoMemory(true);
				}
				break;

			case LoadableObjectsContainer::LoadableObjectType_Shader:
				if(!loadableObject.getShaderProgram()->isLoadedToVideoMemory())
				{
					queueForLoading(*loadableObject.getShaderProgram());
					loadableObject.getShaderProgram()->setLoadedToVideoMemory(true);
				}
				break;

			case LoadableObjectsContainer::LoadableObjectType_Texture:
				if(!loadableObject.getTextureHandle().isLoadedToVideoMemory())
				{
					queueForLoading(loadableObject.getTextureHandle());
					loadableObject.getTextureHandle().setLoadedToVideoMemory(true);
				}
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

	// Calculate the transpose inverse view matrix needed for converting normals (in the normal g-buffer) to view space
	m_frameData.m_transposeInverseViewMatrix = glm::transpose(glm::inverse(m_frameData.m_viewMatrix));

	// Set the camera position
	m_frameData.m_cameraPosition = p_sceneObjects.m_cameraViewMatrix[3];

	// Set the camera target vector
	m_frameData.m_cameraTarget = normalize(glm::vec3(0.0f, 0.0f, -1.0f) * glm::mat3(p_sceneObjects.m_cameraViewMatrix));
	
	for(decltype(m_activeRenderPasses.size()) i = 0, size = m_activeRenderPasses.size(); i < size; i++)
	{
		m_activeRenderPasses[i]->update(*m_renderPassData, p_sceneObjects, p_deltaTime);
	}
}

unsigned int RendererFrontend::getFramebufferTextureHandle(GBufferTextureType p_bufferType) const
{
	return m_backend.getFramebufferTextureHandle(p_bufferType); 
}
