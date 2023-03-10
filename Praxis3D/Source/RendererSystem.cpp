
#include "ErrorHandlerLocator.h"
#include "RendererScene.h"
#include "RendererSystem.h"

RendererSystem::RendererSystem()
{
	m_rendererScene = nullptr;
	m_systemName = GetString(Systems::Graphics);
}

RendererSystem::~RendererSystem()
{

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
	RenderingPasses renderingPasses;

	// Load the rendering passes
	auto &renderPassesProperty = p_properties.getPropertySetByID(Properties::RenderPasses);
	if(renderPassesProperty)
	{
		// Iterate over the property array
		for(decltype(renderPassesProperty.getNumPropertySets()) objIndex = 0, objSize = renderPassesProperty.getNumPropertySets(); objIndex < objSize; objIndex++)
		{
			auto &typeProperty = renderPassesProperty.getPropertySetUnsafe(objIndex).getPropertyByID(Properties::Type);
			if(typeProperty)
			{
				switch(typeProperty.getID())
				{
				case Properties::AtmScatteringRenderPass:
					renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
					break;
				case Properties::BloomRenderPass:
					renderingPasses.push_back(RenderPassType::RenderPassType_Bloom);
					break;
				case Properties::GeometryRenderPass:
					renderingPasses.push_back(RenderPassType::RenderPassType_Geometry);
					break;
				case Properties::GUIRenderPass:
					renderingPasses.push_back(RenderPassType::RenderPassType_GUI);
					break;
				case Properties::LightingRenderPass:
					renderingPasses.push_back(RenderPassType::RenderPassType_Lighting);
					break;
				case Properties::LuminanceRenderPass:
					renderingPasses.push_back(RenderPassType::RenderPassType_Luminance);
					break;
				case Properties::FinalRenderPass:
					renderingPasses.push_back(RenderPassType::RenderPassType_Final);
					break;
				}
			}
		}
	}

	// Pass the loaded rendering passes to the renderer
	m_renderer.setRenderingPasses(renderingPasses);

	return ErrorCode::Success;
}

ErrorCode RendererSystem::preload()
{
	if(m_rendererScene != nullptr)
		return m_rendererScene->preload();
	else
		return ErrorCode::Failure;
}

void RendererSystem::loadInBackground()
{
	if(m_rendererScene != nullptr)
		return m_rendererScene->loadInBackground();
}

SystemScene *RendererSystem::createScene(SceneLoader *p_sceneLoader)
{
	if(m_rendererScene == nullptr)
	{
		// Create new scene
		m_rendererScene = new RendererScene(this, p_sceneLoader);
		ErrorCode sceneError = m_rendererScene->init();

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

	return m_rendererScene;
}

SystemScene *RendererSystem::getScene()
{
	return m_rendererScene;
}
