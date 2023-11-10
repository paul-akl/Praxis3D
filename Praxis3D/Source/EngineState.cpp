
#include "AudioSystem.h"
#include "Engine.h"
#include "EngineState.h"
#include "GUISystem.h"
#include "PhysicsSystem.h"
#include "RendererSystem.h"
#include "ScriptSystem.h"
#include "WorldSystem.h"

EngineState::EngineState(Engine &p_engine, EngineStateType p_engineState) : m_engine(p_engine), m_engineStateType(p_engineState), m_initialized(false)
{
	m_sceneChangeController = new ChangeController();
	m_objectChangeController = new ChangeController();

	m_scheduler = nullptr;
	m_changeCtrlScene = nullptr;

	//for(int i = 0; i < Systems::NumberOfSystems; i++)
	//	m_systems[i] = nullptr;
}

EngineState::~EngineState()
{
	shutdown();

	//delete m_sceneChangeController;
	//delete m_objectChangeController;
}

ErrorCode EngineState::init(TaskManager *p_taskManager)
{
	ErrorCode returnError = ErrorCode::Success;

	// Clear the engine state first, if it has already been initialized
	if(m_initialized)
		shutdown();

	// Get all engine systems
	auto systems = m_engine.getSystems();

	// Create new scene change controller
	m_changeCtrlScene = new UniversalScene(m_sceneChangeController, m_objectChangeController);

	// Create new scheduler for updating (executing) scenes
	m_scheduler = new TaskScheduler(p_taskManager);

	// Use the created scene change controller in our scene loader
	m_sceneLoader.registerChangeController(m_changeCtrlScene);

	// Create new scenes from each system to be used by this play state
	// Register systems with change controller and scenes with scene loader
	for(int i = 0; i < Systems::NumberOfSystems; i++)
	{
		// Create new system scene
		auto newScene = systems[i]->createScene(&m_sceneLoader, m_engineStateType);

		m_sceneLoader.registerSystemScene(newScene);
		auto extendError = m_changeCtrlScene->extend(newScene);
		if(extendError != ErrorCode::Success)
			returnError = extendError;
	}

	if(returnError == ErrorCode::Success)
	{
		// Register change control scene with the scheduler
		m_scheduler->setScene(m_changeCtrlScene);

		// Register task manager with object and scene change controllers
		returnError = m_objectChangeController->setTaskManager(p_taskManager);

		if(returnError == ErrorCode::Success)
			returnError = m_sceneChangeController->setTaskManager(p_taskManager);
	}

	return returnError;
}

void EngineState::activate()
{
	m_scheduler->execute<>(std::function<void(SystemTask *)>(&SystemTask::activate));

	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();
}

void EngineState::deactivate()
{
	m_scheduler->execute<>(std::function<void(SystemTask *)>(&SystemTask::deactivate));

	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();
}

void EngineState::shutdown()
{
	if(m_initialized)
	{
		//m_objectChangeController->resetTaskManager();
		//m_sceneChangeController->resetTaskManager();

		if(m_scheduler != nullptr)
			delete m_scheduler;
		if(m_changeCtrlScene != nullptr)
			delete m_changeCtrlScene;
		if(m_sceneChangeController != nullptr)
			delete m_sceneChangeController;
		if(m_objectChangeController != nullptr)
			delete m_objectChangeController;

		// Get all engine systems
		auto systems = m_engine.getSystems();

		// Delete all system scenes that belong to this engine state
		for(int i = 0; i < Systems::NumberOfSystems; i++)
		{
			systems[i]->deleteScene(m_engineStateType);
		}

		m_initialized = false;
	}
}