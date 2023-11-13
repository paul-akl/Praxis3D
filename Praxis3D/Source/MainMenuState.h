#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class MainMenuState : public EngineState
{
public:
	MainMenuState(Engine &p_engine);
	~MainMenuState();

	ErrorCode load()
	{
		ErrorCode returnError = ErrorCode::Initialize_failure;

		if(m_initialized)
		{
			// Load the scene map, and log an error if it wasn't successful
			returnError = m_sceneLoader.loadFromFile(m_sceneFilename);
			if(returnError != ErrorCode::Success)
				ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);
		}

		return returnError;
	}

	void update(Engine &p_engine);

private:
};