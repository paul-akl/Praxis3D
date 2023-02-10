
#include "TaskManager.h"
#include "TaskScheduler.h"

// TODO DATA DRIVEN
//const float TaskScheduler::m_defaultClockFrequency = 1.0f / 120.0f;      // Set the timer to 120Hz

TaskScheduler::TaskScheduler(TaskManager *p_taskManager) 
	: m_taskManager(p_taskManager),
	m_clockFrequency(1.0f / Config::engineVar().task_scheduler_clock_frequency),
	m_executionTimer(nullptr),
	m_multithreadingEnabled(true) // TODO DATA DRIVEN
{
	m_multithreadingEnabled = (p_taskManager != nullptr);
}
TaskScheduler::~TaskScheduler()
{

}

void TaskScheduler::setScene(const UniversalScene *p_scene)
{
	// Create temp containers to hold current tasks (to make sure none of them are currently being executed)
	SystemTask *tasksToWaitFor[Systems::Types::Max];
	unsigned int numTasksToWaitFor = 0;

	// Iterate over all the registered system scenes and get their tasks
	for(UniversalScene::SystemSceneMap::iterator it = m_systemScenes.begin(); it != m_systemScenes.end(); it++)
	{
		SystemScene *currentScene = it->second;
		tasksToWaitFor[numTasksToWaitFor++] = currentScene->getSystemTask();
	}

	// Clear the system scene list (so it can be re-filled with the newly added scene)
	m_systemScenes.clear();

	// If there are any tasks retrieved from the scenes
	if(numTasksToWaitFor > 0)
	{
		// Wait for any executing scenes to finish their work
		m_taskManager->waitForSystemTasks(tasksToWaitFor, numTasksToWaitFor);
	}

	// Get the new system scenes from the parameter
	const UniversalScene::SystemSceneMap &systemScenes = p_scene->getSystemScenes();

	// Iterate over new scenes and add them to the internal list
	for(UniversalScene::SystemSceneMap::const_iterator it = systemScenes.begin(); it != systemScenes.end(); it++)
	{
		// Make sure the system is valid (by checking if it has a task)
		if(it->second->getSystemTask() != nullptr)
		{
			// Add the scene
			m_systemScenes[it->first] = it->second;
		}
	}
}

void TaskScheduler::execute(float p_deltaTime)
{
	// If multithreading is enabled, execute tasks over threads in parallel
	if(m_multithreadingEnabled)
	{
		// Create temp containers to hold current tasks (to execute in this time-step)
		SystemTask *tasksToExecute[Systems::TypeID::NumberOfSystems];
		unsigned int numTasksToExecute = 0;

		// Iterate over all the system scenes and get their tasks
		for(auto it = m_systemScenes.begin(); it != m_systemScenes.end(); it++)
		{
			// Get the scene
			SystemScene *currentScene = it->second;

			// Get the scene's task (and increment the count)
			tasksToExecute[numTasksToExecute++] = currentScene->getSystemTask();
		}

		// Execute the tasks in parallel, by passing them to the task manager
		m_taskManager->issueJobsForSystemTasks(tasksToExecute, numTasksToExecute, p_deltaTime);

		m_taskManager->waitForSystemTasks(tasksToExecute, numTasksToExecute);
	}
	// If multithreading is disabled, execute tasks in serial
	else
	{
		// Iterate over all the system scenes and update them
		for(UniversalScene::SystemSceneMap::iterator it = m_systemScenes.begin(); it != m_systemScenes.end(); it++)
		{
			// Get the scene
			SystemScene *currentScene = it->second;

			// Execute the task straight away (hence in serial)
			currentScene->getSystemTask()->update(p_deltaTime);
		}
	}
}