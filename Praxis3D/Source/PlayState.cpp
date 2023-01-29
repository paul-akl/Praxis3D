
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
}

PlayState::~PlayState()
{
}

ErrorCode PlayState::init(TaskManager *p_taskManager)
{
	ErrorCode returnError = EngineState::init(p_taskManager);
	
	if(returnError == ErrorCode::Success)
	{
		// Load the default map, and log an error if it wasn't successful
		returnError = m_sceneLoader.loadFromFile(Config::gameplayVar().default_map);
		if(returnError != ErrorCode::Success)
			ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);

		m_initialized = true;
	}

	return returnError;
}

void PlayState::update(Engine &p_engine)
{
	m_scheduler->execute(ClockLocator::get().getDeltaSecondsF());
	
	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();
}
