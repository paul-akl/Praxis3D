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
	// Gets engine changes (if there are any) from the Universal scene of the current Engine State and process them
	void processEngineChanges();

	// Get all engine systems. Return a pointer to an array the size of Systems::NumberOfSystems
	SystemBase **getSystems() { return m_systems; }

	// Sets which engine state is currently active
	void setCurrentStateType(EngineStateType p_newStateType)
	{
		// Track whether the state failed to change
		bool changeState = true;

		// Create the state if it hasn't been created already
		if(m_engineStates[p_newStateType] == nullptr)
			createState(&m_engineStates[p_newStateType], p_newStateType);

		if(!m_engineStates[p_newStateType]->isInitialized())
		{
			// Initialize the current state
			ErrorCode stateInitError = m_engineStates[p_newStateType]->init(m_taskManager);

			// If it failed to initialize, return to the previous state and log an error
			if(stateInitError != ErrorCode::Success)
			{
				changeState = false;
				ErrHandlerLoc::get().log(stateInitError, ErrorSource::Source_Engine);
			}
			else
			{
				// Load the scene
				ErrorCode loadError = m_engineStates[p_newStateType]->load();

				// If it failed to load, log an error
				if(loadError != ErrorCode::Success)
				{
					changeState = false;
					ErrHandlerLoc::get().log(loadError, getEngineStateTypeString(m_currentStateType), ErrorSource::Source_Engine);
				}
			}
		}

		// If the state changed, deactivate the previous one, and activate the new one
		if(changeState)
		{
			if(m_engineStates[m_currentStateType] != nullptr)
				m_engineStates[m_currentStateType]->deactivate();

			if(m_engineStates[p_newStateType] != nullptr)
				m_engineStates[p_newStateType]->activate();

			m_currentStateType = p_newStateType;
			Config::m_engineVar.engineState = m_currentStateType;
		}
	}

	// Creates and initializes all the services and their locators
	ErrorCode initServices();

	// Creates and initializes all the engine systems
	ErrorCode initSystems();

	// Shuts all the systems, etc. down, called before returning from run()
	void shutdown();

	// Creates an engine state of the given type
	void createState(EngineState **p_currentState, EngineStateType p_engineStateType)
	{
		if(*p_currentState == nullptr)
			switch(p_engineStateType)
			{
				case EngineStateType_MainMenu:
					*p_currentState = new MainMenuState(*this);
					break;

				case EngineStateType_Play:
					*p_currentState = new PlayState(*this);
					break;

				case EngineStateType_Editor:
					*p_currentState = new EditorState(*this);
					break;
			}
	}

	// Converts EngineStateType into a text string
	const inline std::string getEngineStateTypeString(EngineStateType p_engineStateType) const
	{
		std::string returnString = "Invalid engine state";

		switch(p_engineStateType)
		{
			case EngineStateType_MainMenu:
				returnString = "Main menu state";
				break;
			case EngineStateType_Play:
				returnString = "Play state";
				break;
			case EngineStateType_Editor:
				returnString = "Editor state";
				break;
		}

		return returnString;
	}

	// Currently being executed state
	//EngineState *m_currentState;
	EngineStateType m_currentStateType;

	// Different execution states
	EngineState *m_engineStates[EngineStateType::EngineStateType_NumOfTypes];

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

