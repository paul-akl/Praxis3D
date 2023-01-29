#pragma once

#include "Config.h"
#include "ErrorCodes.h"
#include "SceneLoader.h"
#include "TaskManager.h"
#include "TaskScheduler.h"


class Engine;
class SystemBase;

class EngineState
{
public:
	EngineState();
	virtual ~EngineState();

	virtual ErrorCode init(TaskManager *p_taskManager);
	virtual void update(Engine &p_engine) = 0;

	virtual void shutdown();

	virtual const EngineStateType getEngineStateType() = 0;

	const inline bool isInitialized() const { return m_initialized; }

protected:
	// Creates and initializes all the engine systems
	ErrorCode initSystems();

	bool m_initialized;

	// All engine systems
	SystemBase *m_systems[Systems::NumberOfSystems];

	// System scenes register
	SceneLoader m_sceneLoader;

	// Multi-threading task scheduler
	TaskScheduler *m_scheduler;
	UniversalScene *m_changeCtrlScene;

	// Subject - observer messaging systems
	ChangeController *m_sceneChangeController;
	ChangeController *m_objectChangeController;
};

