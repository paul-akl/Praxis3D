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

	ErrorCode init(TaskManager *p_taskManager);
	void update(Engine &p_engine);

	const EngineStateType getEngineStateType() final { return EngineStateType::EngineStateType_Play; }

private:
};