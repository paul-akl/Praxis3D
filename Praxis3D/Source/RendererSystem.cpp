
#include "ErrorHandlerLocator.h"
#include "RendererScene.h"
#include "RendererSystem.h"

RendererSystem::RendererSystem()
{
	m_renderer = new DeferredRenderer();
	m_rendererScene = nullptr;
}

RendererSystem::~RendererSystem()
{

}

ErrorCode RendererSystem::init()
{
	ErrorCode returnCode = ErrorCode::Success;

	// Initialize the renderer
	ErrorCode rendererError = m_renderer->init();

	// Check if the renderer initialization was successful
	// If it failed, assign a new null renderer
	if(rendererError != ErrorCode::Success)
	{
		ErrHandlerLoc::get().log(rendererError);

		// Delete the failed renderer and assign the base class as a null renderer
		// It will not draw anything on screen, but the rest of the engine will still work,
		// if the error type for this error code wasn't set to fatal (for example when debugging)
		delete m_renderer;
		m_renderer = new Renderer();
	}
	else
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

std::string RendererSystem::getName()
{
	return "";
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
