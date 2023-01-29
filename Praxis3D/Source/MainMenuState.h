#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class MainMenuState : public EngineState
{
public:
	MainMenuState();
	~MainMenuState();

	ErrorCode init(TaskManager *p_taskManager);
	void update(Engine &p_engine);

	const EngineStateType getEngineStateType() final { return EngineStateType::EngineStateType_MainMenu; }

private:
};