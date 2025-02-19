
#include "ServiceLocators/Include/ClockLocator.hpp"
#include "Systems/GUISystem/Include/GUISystem.hpp"
#include "Engine/States/Include/MainMenuState.hpp"
#include "Systems/PhysicsSystem/Include/PhysicsSystem.hpp"
#include "Common/Include/PropertySet.hpp"
#include "Systems/ScriptingSystem/Include/ScriptSystem.hpp"
#include "Systems/RendererSystem/Include/RendererSystem.hpp"
#include "Systems/WorldSystem/Include/WorldSystem.hpp"

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
