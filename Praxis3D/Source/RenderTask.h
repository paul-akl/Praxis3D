#pragma once

#include "Renderer.h"
#include "System.h"

class RendererScene;

class RenderTask : public SystemTask
{
	friend class RendererScene;
public:
	RenderTask(RendererScene *p_rendererScenem, Renderer *p_renderer);
	~RenderTask();

	Systems::TypeID getSystemType() { return Systems::Graphics; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return true; }

protected:
	Renderer *m_renderer;
	RendererScene *m_rendererScene;
};

