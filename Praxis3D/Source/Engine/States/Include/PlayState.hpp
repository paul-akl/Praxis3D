#pragma once

#include "Loaders/Include/SceneLoader.hpp"
#include "Engine/States/Include/EngineState.hpp"
#include "Multithreading/Include/TaskManager.hpp"
#include "Multithreading/Include/TaskScheduler.hpp"

class PlayState : public EngineState
{
public:
	PlayState(Engine &p_engine);
	~PlayState();

	ErrorCode load()
	{
		ErrorCode returnError = ErrorCode::Success;

		if(m_initialized && !m_loaded)
		{
			// Load the scene map, and log an error if it wasn't successful
			returnError = m_sceneLoader.loadFromFile(m_sceneFilename);
			if(returnError != ErrorCode::Success)
				ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);
			else
				m_loaded = true;
		}

		return returnError;
	}

	ErrorCode load(const PropertySet &p_sceneProperty)
	{
		ErrorCode returnError = ErrorCode::Success;

		if(m_initialized && !m_loaded)
		{
			// Load the scene map, and log an error if it wasn't successful
			returnError = m_sceneLoader.loadFromProperties(p_sceneProperty);
			if(returnError != ErrorCode::Success)
				ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);
			else
				m_loaded = true;
		}

		return returnError;
	}

	void update(Engine &p_engine);

private:
};