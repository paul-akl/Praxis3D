#pragma once

#include "Clock.h"
#include "Config.h"
#include "ErrorCodes.h"
#include "ErrorHandlerLocator.h"
#include "PlayState.h"
#include "TaskManager.h"
#include "Window.h"

// Class containing a complete one instance of the engine.
class Engine
{
public:
	Engine();
	~Engine();

	// Initializes required systems and data. Required before entering main loop
	ErrorCode init();

	// Enters the main loop. Returns only after exiting the engine
	void run();

private:
	// Shuts all the systems, etc. down, called before returning from run()
	void shutdown();

	// Currently being executed state
	EngineState *m_currentState;

	// Different execution states
	PlayState m_playstate;

	// Various required systems
	ErrorHandler *m_errorHandler;
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

