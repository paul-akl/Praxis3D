
#include "ClockLocator.h"
#include "GUISystem.h"
#include "MainMenuState.h"
#include "PhysicsSystem.h"
#include "PropertySet.h"
#include "ScriptSystem.h"
#include "RendererSystem.h"
#include "WorldSystem.h"

MainMenuState::MainMenuState() : EngineState()
{
}

MainMenuState::~MainMenuState()
{
}

ErrorCode MainMenuState::init(TaskManager *p_taskManager)
{
	ErrorCode returnError = EngineState::init(p_taskManager);

	if(returnError == ErrorCode::Success)
	{
		// Load the default map, and log an error if it wasn't successful
		returnError = m_sceneLoader.loadFromFile(Config::gameplayVar().main_menu_map);
		if(returnError != ErrorCode::Success)
			ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);

		m_initialized = true;
	}

	return returnError;
}

void MainMenuState::update(Engine &p_engine)
{
	m_scheduler->execute<>(std::function<void(SystemTask*, float)>(&SystemTask::update), ClockLocator::get().getDeltaSecondsF());

	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();
}
