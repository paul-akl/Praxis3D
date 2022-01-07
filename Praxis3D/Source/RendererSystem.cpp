
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
		ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Renderer, "Renderer has been initialized");
	}

	return returnCode;
}
ErrorCode RendererSystem::setup(const PropertySet &p_properties)
{
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
