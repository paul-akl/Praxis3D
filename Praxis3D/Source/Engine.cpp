
#include <GL\glew.h>
#include <sstream>

#include <iostream>

#include "AudioSystem.h"
#include "ClockLocator.h"
#include "Engine.h"
#include "GUIHandlerLocator.h"
#include "GUISystem.h"
#include "ObjectDirectory.h"
#include "PhysicsSystem.h"
#include "RendererSystem.h"
#include "ScriptSystem.h"
#include "TaskManagerLocator.h"
#include "WindowLocator.h"
#include "WorldSystem.h"

int Engine::m_instances = 0;

#define ENUMTOSTRING(ENUM) #ENUM

Engine::Engine()
{
	m_instances++;
	m_initialized = false;
	m_errorHandler = nullptr;
	m_GUIHandler = nullptr;
	m_window = nullptr;
	m_clock = nullptr;
	m_taskManager = nullptr;
	m_scheduler = nullptr;
	m_changeCtrlScene = nullptr;
	m_sceneChangeController = nullptr;
	m_objectChangeController = nullptr;
	m_currentState = nullptr;
	m_currentStateType = Config::engineVar().engineState;
}

Engine::~Engine()
{
	// Delete systems
	delete m_taskManager;
	delete m_errorHandler;
	delete m_clock;
}

// Some of the initialization sequences are order sensitive. Do not change the order of calls.
ErrorCode Engine::init()
{
	// Allow only one instance. If there's more, someone is doing something wrong.
	if(m_instances > 1)
	{
		printf("Error: Attempting to create multiple engine instances.\n");
		return ErrorCode::Failure;
	}

	// Initialize all services and their locators
	auto servicesError = initServices();
	if(servicesError != ErrorCode::Success)
		return servicesError;

	//  ___________________________________
	// |								   |
	// |	ENGINE STATE INITIALIZATION	   |
	// |___________________________________|
	// Set and initialize the current engine state
	setCurrentStateType();
	if(!m_currentState->isInitialized())
		return ErrorCode::Failure;

	// If this point is reached, all initializations passed, mark the engine as initialized
	m_initialized = true;

	return ErrorCode::Success;
}

void Engine::run()
{
	// Make sure the engine is initialized before entering the main loop
	if(!m_initialized)
		return;

	// Infinite main loop
	while(true)
	{
		// Update the clock
		m_clock->update();

		// Handle window and input events
		m_window->handleEvents();

		// If engine is still running
		if(Config::engineVar().running == true)
		{
			// Load a different engine state, if it has been changed
			if(m_currentStateType != Config::engineVar().engineState)
			{
				m_currentStateType = Config::engineVar().engineState;
				setCurrentStateType();
			}

			// Call update on the current engine state
			m_currentState->update(*this);

			// Swap buffers. If v-sync is enabled, this call should halt for appropriate time
			m_window->swapBuffers();
		}
		else
		{
			// If engine is not running anymore, break the loop
			break;
		}
	}

	// Call shutdown before returning
	shutdown();
}

ErrorCode Engine::initServices()
{
	ErrorCode returnError = ErrorCode::Success;

	//  ___________________________________
	// |								   |
	// |  SERVICE LOCATORS INITIALIZATION  |
	// |___________________________________|
	// Initialize all locators before providing them with real instances
	ErrHandlerLoc::init();
	ClockLocator::init();
	GUIHandlerLocator::init();
	TaskManagerLocator::init();
	WindowLocator::init();

	//  ___________________________________
	// |								   |
	// |	ERROR HANDLER INITIALIZATION   |
	// |___________________________________|
	// Create and initialize error handler
	m_errorHandler = new ErrorHandler();
	ErrorCode errHandlerError = m_errorHandler->init();

	// If error handler was initialized successfully, pass it to the locator, if not, log an error
	if(errHandlerError == ErrorCode::Success)
	{
		ErrHandlerLoc::provide(m_errorHandler);
	}
	else
		printf("Error: Error handler has failed to initialize. Error code: %i\n", errHandlerError);

	//  ___________________________________
	// |								   |
	// |    SET CONFIGURATION VARIABLES    |
	// |___________________________________|
	// Initialize configuration variables
	Config::init();
	Config::loadFromFile(Config::configFileVar().config_file);

	//  ___________________________________
	// |								   |
	// |	  CLOCK INITIALIZATION		   |
	// |___________________________________|
	// Initialize clock. Still continue if there's an error, nothing would move but the engine would still run.
	// The error handler will be responsible for outputting error and asking user if they want to continue in this case.
	m_clock = new Clock();
	ErrorCode clockError = m_clock->init();

	// Check if clock was initialized successfully
	if(clockError == ErrorCode::Success)
		ClockLocator::provide(m_clock);
	else
		ErrHandlerLoc::get().log(clockError, ErrorSource::Source_Engine);

	//  ___________________________________
	// |								   |
	// |	  WINDOW INITIALIZATION		   |
	// |___________________________________|
	// Initialize window system and then attempt to spawn a window
	m_window = new Window();
	ErrorCode windowError = m_window->init();

	if(windowError == ErrorCode::Success)
	{
		windowError = m_window->createWindow();

		// If the window creation failed, we don't want to continue with the engine, so return an error
		if(windowError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(windowError, ErrorSource::Source_Engine);
			return windowError;
		}

		// Assign window class to the service locator
		WindowLocator::provide(m_window);
	}
	else
		ErrHandlerLoc::get().log(windowError, ErrorSource::Source_Engine);

	//  ___________________________________
	// |								   |
	// |	  GLEW INITIALIZATION		   |
	// |___________________________________|
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();

	// It falsely gives error 1280, putting a call here clears the error, so it won't trigger anything
	glGetError();

	// If GLEW failed to initialize, return failure, as the engine would not be able to continue
	if(glewError != GLEW_OK)
	{
		// Get the GLEW error before returning
		std::stringstream stringstreamGlewError;
		stringstreamGlewError << glewGetErrorString(glewError);
		ErrHandlerLoc::get().log(Glew_failed, ErrorSource::Source_Engine, stringstreamGlewError.str());

		return ErrorCode::Failure;
	}

	//  ___________________________________
	// |								   |
	// |	TASK MANAGER INITIALIZATION	   |
	// |___________________________________|
	// Create new task manager and initialize it
	m_taskManager = new TaskManager();
	ErrorCode taskMgrError = m_taskManager->init();

	// If task manager initialized successfully, provide it to the locator, otherwise log an error
	if(taskMgrError == ErrorCode::Success)
		TaskManagerLocator::provide(m_taskManager);
	else
		ErrHandlerLoc::get().log(taskMgrError, ErrorSource::Source_Engine);

	//  ___________________________________
	// |								   |
	// |	  LOADERS INITIALIZATION	   |
	// |___________________________________|
	// Initialize all global loaders and check for errors
	ErrorCode loaderError = ErrorCode::Success;
	// Initialize model loaders
	loaderError = Loaders::model().init();
	if(loaderError != ErrorCode::Success)
		ErrHandlerLoc::get().log(loaderError, ErrorSource::Source_Engine);

	// Initialize shader loader
	loaderError = Loaders::shader().init();
	if(loaderError != ErrorCode::Success)
		ErrHandlerLoc::get().log(loaderError, ErrorSource::Source_Engine);

	// Initialize texture2D loader
	loaderError = Loaders::texture2D().init();
	if(loaderError != ErrorCode::Success)
		ErrHandlerLoc::get().log(loaderError, ErrorSource::Source_Engine);

	// Initialize textureCubemap loader
	loaderError = Loaders::textureCubemap().init();
	if(loaderError != ErrorCode::Success)
		ErrHandlerLoc::get().log(loaderError, ErrorSource::Source_Engine);

	// Initialize the object directory
	loaderError = ObjectDirectory::init();
	if(loaderError != ErrorCode::Success)
		ErrHandlerLoc::get().log(loaderError, ErrorSource::Source_Engine);

	//  ___________________________________
	// |								   |
	// |  OBJECT DIRECTORY INITIALIZATION  |
	// |___________________________________|
	// Initialize the object directory, return failure if it wasn't successful
	ErrorCode objectDirectoryError = ObjectDirectory::init();
	if(objectDirectoryError != ErrorCode::Success)
	{
		ErrHandlerLoc::get().log(objectDirectoryError, ErrorSource::Source_ObjectDirectory);
		return objectDirectoryError;
	}

	//  ___________________________________
	// |								   |
	// |	GUI HANDLER INITIALIZATION	   |
	// |___________________________________|
	// Initialize GUI handler. Still continue if there's an error, the GUI would just be missing.
	// The error handler will be responsible for outputting error and asking user if they want to continue in this case.
	m_GUIHandler = new GUIHandler();
	ErrorCode guiError = m_GUIHandler->init();

	// Check if gui handler was initialized successfully
	// If so, register it in GUI Handler locator and Window system
	// and enable GUI in Window system
	if(guiError == ErrorCode::Success)
	{
		GUIHandlerLocator::provide(m_GUIHandler);
		m_window->registerGUIHandler(m_GUIHandler);
		m_window->setEnableGUI(true);
	}
	else
		ErrHandlerLoc::get().log(guiError, ErrorSource::Source_Engine);

	return returnError;
}

void Engine::shutdown()
{
	// Shutdown engine states
	m_playstate.shutdown();

	// Cancel all the tasks in background threads
	m_taskManager->cancelBackgroundThreads();

	// Make sure they are canceled, by waiting for them
	m_taskManager->waitForBackgroundThreads();

	// Shutdown the task manager
	m_taskManager->shutdown();

	// Set the initialized flag to false, so the engine is not run without initializing again
	m_initialized = false;
}
