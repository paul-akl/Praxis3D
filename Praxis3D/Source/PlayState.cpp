
#include "ClockLocator.h"
#include "GUISystem.h"
#include "PhysicsSystem.h"
#include "PlayState.h"
#include "PropertySet.h"
#include "ScriptSystem.h"
#include "RendererSystem.h"
#include "WorldSystem.h"

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
}
