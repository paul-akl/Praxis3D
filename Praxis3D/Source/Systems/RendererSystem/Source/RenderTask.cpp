
#include <iostream>

#include "Systems/RendererSystem/Include/RendererFrontend.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Systems/RendererSystem/Include/RenderTask.hpp"

RenderTask::RenderTask(RendererScene *p_rendererScene, RendererFrontend &p_renderer)
	: SystemTask(p_rendererScene), m_rendererScene(p_rendererScene), m_renderer(p_renderer)
{

}

RenderTask::~RenderTask()
{

}

void RenderTask::update(const float p_deltaTime)
{
	m_rendererScene->update(p_deltaTime);
	m_renderer.renderFrame(m_rendererScene->getSceneObjects(), p_deltaTime);
}