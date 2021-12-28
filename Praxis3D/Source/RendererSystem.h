#pragma once

#include "Loaders.h"
#include "RendererFrontend.h"
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

	Systems::TypeID getSystemType() { return Systems::Graphics; }

	virtual SystemScene *createScene(SceneLoader *p_sceneLoader);

	virtual SystemScene *getScene();

	RendererFrontend &getRenderer() { return m_renderer; }

protected:
	RendererFrontend m_renderer;
	RendererScene *m_rendererScene;
};

