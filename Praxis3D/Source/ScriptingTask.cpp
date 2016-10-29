
#include "ScriptingScene.h"
#include "ScriptingTask.h"

ScriptingTask::ScriptingTask(ScriptingScene *p_scriptingScene)
	: SystemTask(p_scriptingScene), m_scriptingScene(p_scriptingScene)
{

}

ScriptingTask::~ScriptingTask()
{

}

void ScriptingTask::update(const float p_deltaTime)
{
	m_scriptingScene->update(p_deltaTime);
}
