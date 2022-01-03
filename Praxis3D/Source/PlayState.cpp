
#include "ClockLocator.h"
#include "GUISystem.h"
#include "PlayState.h"
#include "PropertySet.h"
#include "ScriptSystem.h"
#include "RendererSystem.h"
#include "WorldSystem.h"

PlayState::PlayState()
{
	m_sceneChangeController = new ChangeController();
	m_objectChangeController = new ChangeController();
}


PlayState::~PlayState()
{
	delete m_scheduler;
	delete m_changeCtrlScene;
	delete m_sceneChangeController;
	delete m_objectChangeController;

	for(int i = 0; i < Systems::NumberOfSystems; i++)
		if(m_systems[i]->getSystemType() != Systems::Null)
			delete m_systems[i];
}

ErrorCode PlayState::init(TaskManager *p_taskManager)
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
	// |	 GUI SYSTEM INITIALIZATION	   |
	// |___________________________________|
	// Create GUI system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::GUI] = new GUISystem();
	if (m_systems[Systems::GUI]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::GUI];
		m_systems[Systems::GUI] = &g_nullSystemBase;
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
	
	// Load the default map, and log an error if it wasn't successful
	ErrorCode sceneLoaderError = m_sceneLoader.loadFromFile(Config::gameplayVar().default_map);
	if(sceneLoaderError != ErrorCode::Success)
		ErrHandlerLoc::get().log(sceneLoaderError, ErrorSource::Source_SceneLoader);

	return returnError;
}

void PlayState::update(Engine &p_engine)
{
	m_scheduler->execute(ClockLocator::get().getDeltaSecondsF());
	
	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();
}

void PlayState::shutdown()
{
	m_sceneChangeController->resetTaskManager();
	m_objectChangeController->resetTaskManager();
}
