#pragma once

#include "Loaders/Include/Config.hpp"
#include "ErrorHandler/Include/ErrorCodes.hpp"
#include "Loaders/Include/SceneLoader.hpp"
#include "Multithreading/Include/TaskManager.hpp"
#include "Multithreading/Include/TaskScheduler.hpp"


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
	inline void updateSceneLoadingStatus()
	{
		bool loadingStatus = false;
		auto systemScenes = m_sceneLoader.getAllSystemScenes();

		for(unsigned int i = 0; i < Systems::TypeID::NumberOfSystems; i++)
		{
			if(systemScenes[i]->getLoadingStatus())
				loadingStatus = true;
		}

		if(m_sceneLoader.getFirstLoad())
		{
			if(!loadingStatus)
				m_sceneLoader.setFirstLoad(false);
		}

		m_sceneLoader.setSceneLoadingStatus(loadingStatus);
	}

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

