
#include "ClockLocator.h"
#include "GUISystem.h"
#include "MainMenuState.h"
#include "PhysicsSystem.h"
#include "PropertySet.h"
#include "ScriptSystem.h"
#include "RendererSystem.h"
#include "WorldSystem.h"

MainMenuState::MainMenuState(Engine &p_engine) : EngineState(p_engine, EngineStateType::EngineStateType_MainMenu)
{
	m_sceneFilename = Config::gameplayVar().main_menu_map;
}

MainMenuState::~MainMenuState()
{
}

void MainMenuState::update(Engine &p_engine)
{
	m_scheduler->execute(ClockLocator::get().getDeltaSecondsF());
	//m_scheduler->execute<>(std::function<void(SystemTask*, float)>(&SystemTask::update), ClockLocator::get().getDeltaSecondsF());

	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();

	updateSceneLoadingStatus();
}
