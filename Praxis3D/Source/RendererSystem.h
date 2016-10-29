#pragma once

#include "DeferredRenderer.h"
#include "Loaders.h"
#include "System.h"

class RendererScene;

class RendererSystem : public SystemBase
{
public:
	RendererSystem();
	~RendererSystem();

	virtual ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	//virtual ErrorCode destroyScene(SystemScene *p_systemScene);

	virtual ErrorCode preload();
	void loadInBackground();

	virtual std::string getName();
	Systems::TypeID getSystemType() { return Systems::Graphics; }

	virtual SystemScene *createScene(SceneLoader *p_sceneLoader);

	virtual SystemScene *getScene();

	Renderer *getRenderer() { return m_renderer; }

protected:
	Renderer *m_renderer;
	RendererScene *m_rendererScene;
};

