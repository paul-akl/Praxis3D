#include "GUIScene.h"
#include "GUITask.h"

GUITask::GUITask(GUIScene* p_GUIScene) : SystemTask(p_GUIScene), m_GUIScene(p_GUIScene)
{
}

GUITask::~GUITask()
{
}

void GUITask::update(const float p_deltaTime)
{
	m_GUIScene->update(p_deltaTime);
}

