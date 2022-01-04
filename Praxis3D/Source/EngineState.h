#pragma once

#include "Config.h"
#include "ErrorCodes.h"
#include "TaskManager.h"

class Engine;
class SystemBase;

class EngineState
{
public:
	EngineState() : m_initialized(false) { }
	virtual ~EngineState() { }

	virtual ErrorCode init(TaskManager *p_taskManager, SystemBase *p_systems[Systems::NumberOfSystems]) = 0;
	virtual void update(Engine &p_engine) = 0;

	virtual void shutdown() = 0;

protected:
	bool m_initialized;
};

