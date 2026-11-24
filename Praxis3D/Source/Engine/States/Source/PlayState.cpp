
#include "ServiceLocators/Include/ClockLocator.hpp"
#include "Systems/GUISystem/Include/GUISystem.hpp"
#include "Systems/PhysicsSystem/Include/PhysicsSystem.hpp"
#include "Engine/States/Include/PlayState.hpp"
#include "Common/Include/PropertySet.hpp"
#include "Systems/ScriptingSystem/Include/ScriptSystem.hpp"
#include "Systems/RendererSystem/Include/RendererSystem.hpp"
#include "Systems/WorldSystem/Include/WorldSystem.hpp"

PlayState::PlayState(Engine &p_engine) : EngineState(p_engine, EngineStateType::EngineStateType_Play)
{
	m_sceneFilename = Config::gameplayVar().play_map;
}

PlayState::~PlayState()
{
}

void PlayState::update(Engine &p_engine)
{
	m_scheduler->execute<>(std::function<void(SystemTask *, float)>(&SystemTask::update), ClockLocator::get().getDeltaSecondsF());
	
	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();

	updateSceneLoadingStatus();
}
