#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class PlayState : public EngineState
{
public:
	PlayState();
	~PlayState();

	ErrorCode init(TaskManager *p_taskManager, SystemBase *p_systems[Systems::NumberOfSystems]);
	void update(Engine &p_engine);

	void shutdown();
private:
	SceneLoader m_sceneLoader;

	TaskScheduler *m_scheduler;
	UniversalScene *m_changeCtrlScene;

	ChangeController *m_sceneChangeController;
	ChangeController *m_objectChangeController;
};