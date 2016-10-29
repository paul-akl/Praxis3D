#pragma once

#include "Config.h"
#include "RendererFrontend.h"

class RenderPass
{
public:
	RenderPass(RendererFrontend &p_renderer) : m_renderer(p_renderer) { }

	virtual ErrorCode init() = 0;

	virtual void update(const SceneObjects &p_sceneObjects, const float p_deltaTime) = 0;

	virtual std::string getName() = 0;

protected:
	RendererFrontend &m_renderer;
};