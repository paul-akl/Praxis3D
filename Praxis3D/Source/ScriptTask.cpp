
#include "ScriptScene.h"
#include "ScriptTask.h"

ScriptTask::ScriptTask(ScriptScene *p_scriptingScene)
	: SystemTask(p_scriptingScene), m_scriptingScene(p_scriptingScene)
{

}

ScriptTask::~ScriptTask()
{

}

void ScriptTask::update(const float p_deltaTime)
{
	m_scriptingScene->update(p_deltaTime);
}
