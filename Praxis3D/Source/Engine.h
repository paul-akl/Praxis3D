#pragma once

#include "Clock.h"
#include "Config.h"
#include "EditorState.h"
#include "ErrorCodes.h"
#include "ErrorHandlerLocator.h"
#include "GUIHandlerLocator.h"
#include "MainMenuState.h"
#include "PlayState.h"
#include "TaskManager.h"
#include "Window.h"

// Class containing a complete one instance of the engine.
class Engine
{
	friend class EngineState;
public:
	Engine();
	~Engine();

	// Initializes required systems and data. Required before entering main loop
	ErrorCode init();

	// Enters the main loop. Returns only after exiting the engine
	void run();

private:
	// Get all engine systems. Return a pointer to an array the size of Systems::NumberOfSystems
	SystemBase **getSystems() { return m_systems; }

	// Sets which engine state is currently active
	void setCurrentStateType()
	{
		EngineState *previousState = m_currentState;

		// Set the current state
		switch(m_currentStateType)
		{
		case EngineStateType_MainMenu:
			m_currentState = &m_mainMenuState;
			break;

		case EngineStateType_Play:
			m_currentState = &m_playstate;
			break;

		case EngineStateType_Editor:
			m_currentState = &m_editorState;
			break;

		default:
			m_currentStateType = m_currentState->getEngineStateType();
			break;
		}

		if(!m_currentState->isInitialized())
		{
			// Initialize the current state
			ErrorCode stateInitError = m_currentState->init(m_taskManager);

			// If it failed to initialize, return to the previous state and log an error
			if(stateInitError != ErrorCode::Success)
			{
				m_currentState = previousState;
				if(m_currentState != nullptr)
					m_currentStateType = m_currentState->getEngineStateType();
				ErrHandlerLoc::get().log(stateInitError, ErrorSource::Source_Engine);
			}
		}

		// If the state changed, deactivate the previous one, and activate the new one
		if(m_currentState != previousState)
		{
			if(previousState != nullptr)
				previousState->deactivate();

			if(m_currentState != nullptr)
				m_currentState->activate();
		}
	}

	// Creates and initializes all the services and their locators
	ErrorCode initServices();

	// Creates and initializes all the engine systems
	ErrorCode initSystems();

	// Shuts all the systems, etc. down, called before returning from run()
	void shutdown();

	// Currently being executed state
	EngineState *m_currentState;
	EngineStateType m_currentStateType;

	// Different execution states
	MainMenuState m_mainMenuState;
	PlayState m_playstate;
	EditorState m_editorState;

	// All engine systems
	SystemBase *m_systems[Systems::NumberOfSystems];
	
	// Various required services
	ErrorHandler *m_errorHandler;
	GUIHandler *m_GUIHandler;
	Window *m_window;
	Clock *m_clock;

	// Task manager and scheduler for multi-threading
	TaskManager *m_taskManager;
	TaskScheduler *m_scheduler;

	// Universal change controller, stores subject-observer messages between linked objects
	UniversalScene *m_changeCtrlScene;

	// Scene and object change controllers, distribute changes
	ChangeController *m_sceneChangeController;
	ChangeController *m_objectChangeController;

	// Flag to make sure engine is not run without initialization call
	bool m_initialized;

	// Make sure there is only one instance running
	static int m_instances;
};

