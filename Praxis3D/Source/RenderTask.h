#pragma once

//#include "RendererFrontend.h"
#include "System.h"

class RendererScene;
class RendererFrontend;

class RenderTask : public SystemTask
{
	friend class RendererScene;
public:
	RenderTask(RendererScene *p_rendererScenem, RendererFrontend &p_renderer);
	~RenderTask();

	Systems::TypeID getSystemType() { return Systems::Graphics; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return true; }

protected:
	RendererFrontend &m_renderer;
	RendererScene *m_rendererScene;
};

