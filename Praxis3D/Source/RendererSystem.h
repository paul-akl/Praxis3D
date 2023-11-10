#pragma once

#include "GUIHandler.h"
#include "Loaders.h"
#include "RendererFrontend.h"
#include "System.h"

class RendererScene;

class RendererSystem : public SystemBase
{
public:
	RendererSystem();
	~RendererSystem();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void exportSetup(PropertySet &p_propertySet);

	//virtual ErrorCode destroyScene(SystemScene *p_systemScene);

	ErrorCode preload();
	void loadInBackground();

	Systems::TypeID getSystemType() { return Systems::Graphics; }

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState);

	SystemScene *getScene(EngineStateType p_engineState);

	void deleteScene(EngineStateType p_engineState);

	RendererFrontend &getRenderer() { return m_renderer; }

	void setRenderingPasses(const RenderingPasses &p_renderingPasses)
	{
		m_renderer.setRenderingPasses(p_renderingPasses);
	}

protected:
	RendererFrontend m_renderer;
	RendererScene *m_rendererScenes[EngineStateType::EngineStateType_NumOfTypes];
};

