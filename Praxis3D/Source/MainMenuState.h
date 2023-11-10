#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class MainMenuState : public EngineState
{
public:
	MainMenuState(Engine &p_engine);
	~MainMenuState();

	ErrorCode init(TaskManager *p_taskManager);
	void update(Engine &p_engine);

private:
};