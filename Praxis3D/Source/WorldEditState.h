#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class WorldEditState : public EngineState
{
public:
	WorldEditState();
	~WorldEditState();

	ErrorCode init(TaskManager *p_taskManager);
	void update(Engine &p_engine);

	void shutdown();

private:
	SystemBase *m_systems[Systems::NumberOfSystems];
	SceneLoader m_sceneLoader;

	TaskScheduler *m_scheduler;
	UniversalScene *m_changeCtrlScene;

	ChangeController *m_sceneChangeController;
	ChangeController *m_objectChangeController;
};