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
	EngineState(Engine &p_engine, EngineStateType p_engineState);
	virtual ~EngineState();

	virtual ErrorCode init(TaskManager *p_taskManager);

	virtual ErrorCode load() = 0;

	virtual ErrorCode load(const PropertySet &p_sceneProperty) = 0;

	virtual void update(Engine &p_engine) = 0;

	virtual void activate();

	virtual void deactivate();

	virtual void shutdown();

	const inline EngineStateType getEngineStateType() { return m_engineStateType; }

	inline UniversalScene *getChangeControllerScene() { return m_changeCtrlScene; }

	inline SceneLoader &getSceneLoader() { return m_sceneLoader; }

	const inline bool isInitialized() const { return m_initialized; }

	const inline std::string &getSceneFilename() const { return m_sceneFilename; }

	inline void setSceneFilename(const std::string &p_filename) { m_sceneFilename = p_filename; }

protected:
	bool m_initialized;
	bool m_loaded;
	EngineStateType m_engineStateType;

	// Reference to engine, used for getting systems
	Engine &m_engine;

	// System scenes loader and register
	SceneLoader m_sceneLoader;

	// Multi-threading task scheduler
	TaskScheduler *m_scheduler;
	UniversalScene *m_changeCtrlScene;

	// Subject - observer messaging systems
	ChangeController *m_sceneChangeController;
	ChangeController *m_objectChangeController;

	std::string m_sceneFilename;
};

