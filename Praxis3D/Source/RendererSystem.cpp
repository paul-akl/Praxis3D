
#include "ErrorHandlerLocator.h"
#include "RendererScene.h"
#include "RendererSystem.h"

RendererSystem::RendererSystem()
{
	for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
		m_rendererScenes[i] = nullptr;

	m_systemName = GetString(Systems::Graphics);
}

RendererSystem::~RendererSystem()
{
	for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
		if(m_rendererScenes[i] != nullptr)
			delete m_rendererScenes[i];
}

ErrorCode RendererSystem::init()
{
	ErrorCode returnCode = ErrorCode::Success;
	
	// Initialize the renderer
	returnCode = m_renderer.init();

	// Check if the renderer initialization was successful
	if(returnCode == ErrorCode::Success)
	{
		ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_RendererSystem);
	}

	return returnCode;
}
ErrorCode RendererSystem::setup(const PropertySet &p_properties)
{
	//RenderingPasses renderingPasses;

	//// Load the rendering passes
	//auto &renderPassesProperty = p_properties.getPropertySetByID(Properties::RenderPasses);
	//if(renderPassesProperty)
	//{
	//	// Iterate over the property array
	//	for(decltype(renderPassesProperty.getNumPropertySets()) objIndex = 0, objSize = renderPassesProperty.getNumPropertySets(); objIndex < objSize; objIndex++)
	//	{
	//		auto &typeProperty = renderPassesProperty.getPropertySetUnsafe(objIndex).getPropertyByID(Properties::Type);
	//		if(typeProperty)
	//		{
	//			switch(typeProperty.getID())
	//			{
	//			case Properties::AtmScatteringRenderPass:
	//				renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
	//				break;
	//			case Properties::BloomRenderPass:
	//				renderingPasses.push_back(RenderPassType::RenderPassType_Bloom);
	//				break;
	//			case Properties::GeometryRenderPass:
	//				renderingPasses.push_back(RenderPassType::RenderPassType_Geometry);
	//				break;
	//			case Properties::GUIRenderPass:
	//				renderingPasses.push_back(RenderPassType::RenderPassType_GUI);
	//				break;
	//			case Properties::LightingRenderPass:
	//				renderingPasses.push_back(RenderPassType::RenderPassType_Lighting);
	//				break;
	//			case Properties::LuminanceRenderPass:
	//				renderingPasses.push_back(RenderPassType::RenderPassType_Luminance);
	//				break;
	//			case Properties::FinalRenderPass:
	//				renderingPasses.push_back(RenderPassType::RenderPassType_Final);
	//				break;
	//			}
	//		}
	//	}
	//}

	//// Pass the loaded rendering passes to the renderer
	//m_renderer.setRenderingPasses(renderingPasses);

	return ErrorCode::Success;
}

void RendererSystem::exportSetup(PropertySet &p_propertySet)
{
	// Create the render passes Property Set entry
	auto &RenderPassesPropertySet = p_propertySet.addPropertySet(Properties::RenderPasses);

	// Get the rendering passes from the renderer
	auto renderingPasses = m_renderer.getRenderingPasses();

	// Go over each rendering pass
	for(auto renderPassType : renderingPasses)
	{
		// Convert RenderPassType to PropertyID
		Properties::PropertyID renderPassTypeProperty = Properties::Null;
		switch(renderPassType)
		{
			case RenderPassType::RenderPassType_AtmScattering:
				renderPassTypeProperty = Properties::AtmScatteringRenderPass;
				break;
			case RenderPassType::RenderPassType_Bloom:
				renderPassTypeProperty = Properties::BloomRenderPass;
				break;
			case RenderPassType::RenderPassType_Geometry:
				renderPassTypeProperty = Properties::GeometryRenderPass;
				break;
			case RenderPassType::RenderPassType_GUI:
				renderPassTypeProperty = Properties::GUIRenderPass;
				break;
			case RenderPassType::RenderPassType_Lighting:
				renderPassTypeProperty = Properties::LightingRenderPass;
				break;
			case RenderPassType::RenderPassType_Luminance:
				renderPassTypeProperty = Properties::LuminanceRenderPass;
				break;
			case RenderPassType::RenderPassType_Final:
				renderPassTypeProperty = Properties::FinalRenderPass;
				break;
		}

		// Add the rendering pass array entry and rendering pass type
		RenderPassesPropertySet.addPropertySet(Properties::ArrayEntry).addProperty(Properties::Type, renderPassTypeProperty);
	}
}

ErrorCode RendererSystem::preload()
{
	std::cout << "PRELOAD CALLED" << std::endl;
	//if(m_rendererScene != nullptr)
	//	return m_rendererScene->preload();
	//else
		return ErrorCode::Failure;
}

void RendererSystem::loadInBackground()
{
	std::cout << "LOAD IN BACKGROUND CALLED" << std::endl;
	//if(m_rendererScene != nullptr)
	//	return m_rendererScene->loadInBackground();
}

SystemScene *RendererSystem::createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
{
	if(m_rendererScenes[p_engineState] == nullptr)
	{
		// Create new scene
		m_rendererScenes[p_engineState] = new RendererScene(this, p_sceneLoader);
		ErrorCode sceneError = m_rendererScenes[p_engineState]->init();

		// Check if it initialized correctly (cannot continue without the scene)
		if(sceneError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(sceneError);
		}
		else
		{
			// Check for errors
			GLenum glError = glGetError();
		}
	}

	return m_rendererScenes[p_engineState];
}

SystemScene *RendererSystem::getScene(EngineStateType p_engineState)
{
	return m_rendererScenes[p_engineState];
}

void RendererSystem::deleteScene(EngineStateType p_engineState)
{
	if(m_rendererScenes[p_engineState] != nullptr)
		delete m_rendererScenes[p_engineState];
}
