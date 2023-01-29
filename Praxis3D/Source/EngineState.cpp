
#include "AudioSystem.h"
#include "EngineState.h"
#include "GUISystem.h"
#include "PhysicsSystem.h"
#include "RendererSystem.h"
#include "ScriptSystem.h"
#include "WorldSystem.h"

EngineState::EngineState() : m_initialized(false)
{
	m_sceneChangeController = new ChangeController();
	m_objectChangeController = new ChangeController();

	m_scheduler = nullptr;
	m_changeCtrlScene = nullptr;

	for(int i = 0; i < Systems::NumberOfSystems; i++)
		m_systems[i] = nullptr;
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

	// Initialize all engine systems
	returnError = initSystems();
	if(returnError == ErrorCode::Success)
	{
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
			m_sceneLoader.registerSystemScene(m_systems[i]->createScene(&m_sceneLoader));
			m_changeCtrlScene->extend(m_systems[i]);
		}

		// Register change control scene with the scheduler
		m_scheduler->setScene(m_changeCtrlScene);

		// Register task manager with object and scene change controllers
		m_objectChangeController->setTaskManager(p_taskManager);
		m_sceneChangeController->setTaskManager(p_taskManager);
	}

	return returnError;
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

		// Delete all created systems
		for(int i = 0; i < Systems::NumberOfSystems; i++)
			if(m_systems[i]->getSystemType() != Systems::Null)
				delete m_systems[i];

		m_initialized = false;
	}
}

ErrorCode EngineState::initSystems()
{
	ErrorCode returnError = ErrorCode::Success;

	//  __________________________________
	// |								  |
	// |   AUDIO SYSTEM INITIALIZATION	  |
	// |__________________________________|
	// Create audio system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Audio] = new AudioSystem();
	if(m_systems[Systems::Audio]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Audio];
		m_systems[Systems::Audio] = &g_nullSystemBase;
	}

	//  __________________________________
	// |								  |
	// |  RENDERER SYSTEM INITIALIZATION  |
	// |__________________________________|
	// Create graphics system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Graphics] = new RendererSystem();
	if(m_systems[Systems::Graphics]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Graphics];
		m_systems[Systems::Graphics] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |	 GUI SYSTEM INITIALIZATION	   |
	// |___________________________________|
	// Create GUI system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::GUI] = new GUISystem();
	if(m_systems[Systems::GUI]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::GUI];
		m_systems[Systems::GUI] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |   PHYSICS SYSTEM INITIALIZATION   |
	// |___________________________________|
	// Create scripting system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Physics] = new PhysicsSystem();
	if(m_systems[Systems::Physics]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Physics];
		m_systems[Systems::Physics] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |  SCRIPTING SYSTEM INITIALIZATION  |
	// |___________________________________|
	// Create scripting system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Script] = new ScriptSystem();
	if(m_systems[Systems::Script]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Script];
		m_systems[Systems::Script] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |	WORLD SYSTEM INITIALIZATION	   |
	// |___________________________________|
	// Create scripting system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::World] = new WorldSystem();
	if(m_systems[Systems::World]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::World];
		m_systems[Systems::World] = &g_nullSystemBase;
	}

	return returnError;
}