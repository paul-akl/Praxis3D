
#include "ClockLocator.h"
#include "PropertySet.h"
#include "ScriptingSystem.h"
#include "RendererSystem.h"
#include "WorldEditState.h"

WorldEditState::WorldEditState()
{
	m_sceneChangeController = new ChangeController();
	m_objectChangeController = new ChangeController();
}

WorldEditState::~WorldEditState()
{
	delete m_scheduler;
	delete m_changeCtrlScene;
	delete m_sceneChangeController;
	delete m_objectChangeController;

	for(int i = 0; i < Systems::NumberOfSystems; i++)
		if(m_systems[i]->getSystemType() != Systems::Null)
			delete m_systems[i];
}

ErrorCode WorldEditState::init(TaskManager *p_taskManager)
{
	ErrorCode returnError = ErrorCode::Success;

	m_changeCtrlScene = new UniversalScene(m_sceneChangeController, m_objectChangeController);

	m_scheduler = new TaskScheduler(p_taskManager);

	m_sceneLoader.registerChangeController(m_changeCtrlScene);

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
	// |  SCRIPTING SYSTEM INITIALIZATION  |
	// |___________________________________|
	// Create scripting system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Scripting] = new ScriptingSystem();
	if(m_systems[Systems::Scripting]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Scripting];
		m_systems[Systems::Scripting] = &g_nullSystemBase;
	}

	// Register systems with change controller and scenes with scene loader
	for(int i = 0; i < Systems::NumberOfSystems; i++)
	{
		m_changeCtrlScene->extend(m_systems[i]);
		m_sceneLoader.registerSystemScene(m_systems[i]->createScene(&m_sceneLoader));
	}

	// Register change control scene with the scheduler
	m_scheduler->setScene(m_changeCtrlScene);

	// Register task manager with object and scene change controllers
	m_objectChangeController->setTaskManager(p_taskManager);
	m_sceneChangeController->setTaskManager(p_taskManager);

	// Load the default map, and log an error if it wasn't successful
	ErrorCode sceneLoaderError = m_sceneLoader.loadFromFile(Config::gameplayVar().default_map);
	if(sceneLoaderError != ErrorCode::Success)
		ErrHandlerLoc::get().log(sceneLoaderError, ErrorSource::Source_SceneLoader);

	return returnError;
}

void WorldEditState::update(Engine &p_engine)
{
	m_scheduler->execute(ClockLocator::get().getDeltaSecondsF());

	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();
}

void WorldEditState::shutdown()
{
	m_sceneChangeController->resetTaskManager();
	m_objectChangeController->resetTaskManager();
}
