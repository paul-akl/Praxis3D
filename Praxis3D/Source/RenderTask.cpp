
#include <iostream>

#include "RendererFrontend.h"
#include "RendererScene.h"
#include "RenderTask.h"

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