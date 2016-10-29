
#include <iostream>

#include "RenderTask.h"
#include "RendererScene.h"

RenderTask::RenderTask(RendererScene *p_rendererScene, Renderer *p_renderer)
	: SystemTask(p_rendererScene), m_rendererScene(p_rendererScene), m_renderer(p_renderer)
{

}

RenderTask::~RenderTask()
{

}

void RenderTask::update(const float p_deltaTime)
{
	m_rendererScene->update(p_deltaTime);

	m_renderer->renderFrame(m_rendererScene->getSceneObjects(), p_deltaTime);
}