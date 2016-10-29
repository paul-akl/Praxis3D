#pragma once

#include "Config.h"
#include "ErrorCodes.h"
#include "TaskManager.h"

class Engine;

class EngineState
{
public:
	virtual ~EngineState() { }

	virtual ErrorCode init(TaskManager *p_taskManager) = 0;
	virtual void update(Engine &p_engine) = 0;

	virtual void shutdown() = 0;
};

