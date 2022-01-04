
#include "ClockLocator.h"
#include "GUISystem.h"
#include "PhysicsSystem.h"
#include "PlayState.h"
#include "PropertySet.h"
#include "ScriptSystem.h"
#include "RendererSystem.h"
#include "WorldSystem.h"

PlayState::PlayState() : EngineState()
{
	m_sceneChangeController = new ChangeController();
	m_objectChangeController = new ChangeController();

	m_scheduler = nullptr;
	m_changeCtrlScene = nullptr;
}

PlayState::~PlayState()
{
}

ErrorCode PlayState::init(TaskManager *p_taskManager, SystemBase *p_systems[Systems::NumberOfSystems])
{
	ErrorCode returnError = ErrorCode::Success;

	// Clear the engine state first, if it has already been initialized
	if(m_initialized)
		shutdown();

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
		m_sceneLoader.registerSystemScene(p_systems[i]->createScene(&m_sceneLoader));
		m_changeCtrlScene->extend(p_systems[i]);
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

	m_initialized = true;

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

	m_initialized = false;
}
