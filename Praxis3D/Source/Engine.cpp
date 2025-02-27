
#include <GL\glew.h>
#include <sstream>

#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

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

// Try to run on a dedicated GPU (in cases where there's integrated and dedicated GPUs in the system),
// by requesting the high performance power settings from both nVidia and AMD GPUs
#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif

int Engine::m_instances = 0;

#define ENUMTOSTRING(ENUM) #ENUM

Engine::Engine()
{
	m_instances++;
	m_initialized = false;
	m_changeCtrlScene = nullptr;
	m_sceneChangeController = nullptr;
	m_objectChangeController = nullptr;
	m_currentStateType = Config::engineVar().engineState;

	// Seed the random number generator
	srand((unsigned int)time(NULL));
	rand();

	for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
		m_engineStates[i] = nullptr;

	for(unsigned int i = 0; i < Systems::NumberOfSystems; i++)
		m_systems[i] = nullptr;
}

Engine::~Engine()
{
	// Delete engine states
	for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
		if(m_engineStates[i] != nullptr)
			delete m_engineStates[i];

	// Delete systems
	for(unsigned int i = 0; i < Systems::NumberOfSystems; i++)
		if(m_systems[i] != nullptr)
			delete m_systems[i];
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

	// Initialize all engine systems
	auto systemsError = initSystems();
	if(systemsError != ErrorCode::Success)
		return systemsError;

	//  ___________________________________
	// |								   |
	// |	ENGINE STATE INITIALIZATION	   |
	// |___________________________________|
	// Set and initialize the current engine state
	setCurrentStateType(m_currentStateType);
	if(m_engineStates[m_currentStateType] == nullptr || !m_engineStates[m_currentStateType]->isInitialized())
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
		m_clock.update();

		// Handle window and input events
		m_window.handleEvents();

		// Update all loaders
		updateLoaders();

		// If engine is still running
		if(Config::engineVar().running == true)
		{
			// Process any queued changes
			processEngineChanges();

			// Call update on the current engine state
			m_engineStates[m_currentStateType]->update(*this);

			// Swap buffers. If v-sync is enabled, this call should halt for appropriate time
			m_window.swapBuffers();
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

void Engine::processEngineChanges()
{
	// Check if there are any engine changes
	auto changeControllerScene = m_engineStates[m_currentStateType]->getChangeControllerScene();
	if(changeControllerScene->getEngineChangePending())
	{
		// Save the current engine state type, in case it gets changed
		auto previousStateType = m_currentStateType;

		// Go over each engine change
		auto engineChanges = changeControllerScene->getEngineChangeQueue();
		for(auto const &change : engineChanges)
		{
			switch(change.m_changeType)
			{
				case EngineChangeType::EngineChangeType_SceneFilename:
					{
						// Create the state if it hasn't been created already
						if(m_engineStates[change.m_engineStateType] == nullptr)
							createState(&m_engineStates[change.m_engineStateType], change.m_engineStateType);

						m_engineStates[change.m_engineStateType]->setSceneFilename(change.m_filename);
					}
					break;
				case EngineChangeType::EngineChangeType_SceneLoad:
					{
						if(initializeState(change.m_engineStateType))
						{
							if(change.m_sceneProperties)
								loadState(change.m_engineStateType, change.m_sceneProperties);
							else
								loadState(change.m_engineStateType, change.m_filename);
						}
					}
					break;
				case EngineChangeType::EngineChangeType_SceneUnload:
					{
						if(m_engineStates[change.m_engineStateType] != nullptr)
						{
							auto engineStateType = change.m_engineStateType;

							m_engineStates[engineStateType]->shutdown();
							delete m_engineStates[engineStateType];
							m_engineStates[engineStateType] = nullptr;
							if(engineStateType == previousStateType)
								return;
						}
					}
					break;
				case EngineChangeType::EngineChangeType_SceneReload:
					{
						std::string filename = change.m_filename;

						// Delete the current scene
						if(m_engineStates[change.m_engineStateType] != nullptr)
						{
							filename = m_engineStates[change.m_engineStateType]->getSceneFilename();
							delete m_engineStates[change.m_engineStateType];
							m_engineStates[change.m_engineStateType] = nullptr;
						}

						if(initializeState(change.m_engineStateType))
						{
							bool stateLoaded = false;

							if(change.m_sceneProperties)
								stateLoaded = loadState(change.m_engineStateType, change.m_sceneProperties);
							else
								stateLoaded = loadState(change.m_engineStateType, filename);

							if(stateLoaded)
								setCurrentState(change.m_engineStateType);
						}
						return;
					}
					break;
				case EngineChangeType::EngineChangeType_StateChange:
					{
						if(initializeState(change.m_engineStateType))
						{
							bool stateLoaded = false;

							if(change.m_sceneProperties)
								stateLoaded = loadState(change.m_engineStateType, change.m_sceneProperties);
							else
								stateLoaded = loadState(change.m_engineStateType, change.m_filename);

							if(stateLoaded)
								setCurrentState(change.m_engineStateType);
						}
					}
					break;
			}
		}

		// Mark engine changes as being processed by clearing the queue
		changeControllerScene->clearEngineChangeQueue();
	}
}

void Engine::updateLoaders()
{
	Loaders::model().processReleaseQueue(m_engineStates[m_currentStateType]->getSceneLoader());
	Loaders::texture2D().processReleaseQueue(m_engineStates[m_currentStateType]->getSceneLoader());
	// Not in use for the moment
	//Loaders::textureCubemap().processReleaseQueue(); 
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
	ErrorCode errHandlerError = m_errorHandler.init();

	// If error handler was initialized successfully, pass it to the locator, if not, log an error
	if(errHandlerError == ErrorCode::Success)
	{
		ErrHandlerLoc::provide(&m_errorHandler);
	}
	else
		printf("Error: Error handler has failed to initialize. Error code: %s\n", GetString(errHandlerError));

	//  ___________________________________
	// |								   |n
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
	ErrorCode clockError = m_clock.init();

	// Check if clock was initialized successfully
	if(clockError == ErrorCode::Success)
		ClockLocator::provide(&m_clock);
	else
		ErrHandlerLoc::get().log(clockError, ErrorSource::Source_Engine);
	
	//  ___________________________________
	// |								   |
	// |	  WINDOW INITIALIZATION		   |
	// |___________________________________|
	// Initialize window system and then attempt to spawn a window
	ErrorCode windowError = m_window.init();

	if(windowError == ErrorCode::Success)
	{
		windowError = m_window.createWindow();

		// If the window creation failed, we don't want to continue with the engine, so return an error
		if(windowError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(windowError, ErrorSource::Source_Engine);
			return windowError;
		}

		// Assign window class to the service locator
		WindowLocator::provide(&m_window);
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
	// Initialize the task manager
	ErrorCode taskMgrError = m_taskManager.init();

	// If task manager initialized successfully, provide it to the locator, otherwise log an error
	if(taskMgrError == ErrorCode::Success)
		TaskManagerLocator::provide(&m_taskManager);
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
	ErrorCode guiError = m_GUIHandler.init();

	// Check if the GUI handler was initialized successfully
	// If so, register it in GUI Handler locator and Window system
	// and enable GUI in Window system
	if(guiError == ErrorCode::Success)
	{
		GUIHandlerLocator::provide(&m_GUIHandler);
		m_window.registerGUIHandler(&m_GUIHandler);
		m_window.setEnableGUI(true);
	}
	else
		ErrHandlerLoc::get().log(guiError, ErrorSource::Source_Engine);

	return returnError;
}

ErrorCode Engine::initSystems()
{
	ErrorCode returnError = ErrorCode::Success;

	//  __________________________________
	// |								  |
	// |   AUDIO SYSTEM INITIALIZATION	  |
	// |__________________________________|
	// Create audio system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Audio] = new AudioSystem();
	if(m_systems[Systems::Audio]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Audio];
		m_systems[Systems::Audio] = &g_nullSystemBase;
	}

	//  __________________________________
	// |								  |
	// |  RENDERER SYSTEM INITIALIZATION  |
	// |__________________________________|
	// Create graphics system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Graphics] = new RendererSystem();
	if(m_systems[Systems::Graphics]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Graphics];
		m_systems[Systems::Graphics] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |	 GUI SYSTEM INITIALIZATION	   |
	// |___________________________________|
	// Create GUI system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::GUI] = new GUISystem();
	if(m_systems[Systems::GUI]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::GUI];
		m_systems[Systems::GUI] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |   PHYSICS SYSTEM INITIALIZATION   |
	// |___________________________________|
	// Create scripting system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Physics] = new PhysicsSystem();
	if(m_systems[Systems::Physics]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Physics];
		m_systems[Systems::Physics] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |  SCRIPTING SYSTEM INITIALIZATION  |
	// |___________________________________|
	// Create scripting system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::Script] = new ScriptSystem();
	if(m_systems[Systems::Script]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::Script];
		m_systems[Systems::Script] = &g_nullSystemBase;
	}

	//  ___________________________________
	// |								   |
	// |	WORLD SYSTEM INITIALIZATION	   |
	// |___________________________________|
	// Create scripting system and check if it was successful (if not, assign a null system in it's place)
	m_systems[Systems::World] = new WorldSystem();
	if(m_systems[Systems::World]->init() != ErrorCode::Success)
	{
		delete m_systems[Systems::World];
		m_systems[Systems::World] = &g_nullSystemBase;
	}

	return returnError;
}

void Engine::shutdown()
{
	// Shutdown engine states
	for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
		if(m_engineStates[i] != nullptr)
			m_engineStates[i]->shutdown();

	// Cancel all the tasks in background threads
	m_taskManager.cancelBackgroundThreads();

	// Make sure they are canceled, by waiting for them
	m_taskManager.waitForBackgroundThreads();

	// Shutdown the task manager
	m_taskManager.shutdown();

	// Set the initialized flag to false, so the engine is not run without initializing again
	m_initialized = false;
}
