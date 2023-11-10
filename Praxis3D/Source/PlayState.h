#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class PlayState : public EngineState
{
public:
	PlayState(Engine &p_engine);
	~PlayState();

	ErrorCode init(TaskManager *p_taskManager);
	void update(Engine &p_engine);

private:
};