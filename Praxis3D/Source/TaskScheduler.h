#pragma once

#include <map>

#include "System.h"
#include "Universal.h"

class SystemScene;
class TaskManager;

class TaskScheduler
{
public:
	TaskScheduler(TaskManager *p_taskManager);
	~TaskScheduler();

	void setScene(const UniversalScene *p_scene);

	// Calls update (passing the delta time) on all registered scenes
	void execute(float p_deltaTime);

	// Calls any member function of SystemTask that is passed as an argument, and forwards all passed parameters
	template<typename T_Func, typename... T_Args>
	void execute(T_Func &p_func, T_Args&&... p_args)
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
			m_taskManager->issueJobsForSystemTasks(tasksToExecute, numTasksToExecute, p_func, std::forward<T_Args>(p_args)...);

			m_taskManager->waitForSystemTasks(tasksToExecute, numTasksToExecute, p_func, std::forward<T_Args>(p_args)...);
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
				p_func(currentScene->getSystemTask(), std::forward<T_Args>(p_args)...);
			}
		}
	}

	// Calls any member function of SystemTask that is passed as an argument, and forwards all passed parameters
	template <typename... T_Type>
	void executeViaBackgroundThreads(std::function<void(SystemTask *, T_Type...)> p_func, T_Type&&... p_args)
	{
		// Contains tasks that can only be executed in the primary thread
		std::vector<SystemTask*> primaryThreadTasks;

		// Iterate over all the system scenes and get their tasks
		for(auto it = m_systemScenes.begin(); it != m_systemScenes.end(); it++)
		{
			// Get the scene
			SystemScene *currentScene = it->second;

			// Get the scene's task (and increment the count)
			SystemTask *currentTask = currentScene->getSystemTask();

			// If the task is only to be executed in primary thread, add it to an array; otherwise execute the member function in a background thread
			if(currentTask->isPrimaryThreadOnly())
				primaryThreadTasks.push_back(currentTask);
			else
				m_taskManager->startBackgroundThread(std::bind(p_func, currentTask, std::forward<T_Type>(p_args)...));
		}

		// Temporary elevate the primary thread priority
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		// Execute all tasks that are set for primary thread
		for(decltype(primaryThreadTasks.size()) i = 0, size = primaryThreadTasks.size(); i < size; i++)
			p_func(primaryThreadTasks[i], std::forward<T_Type>(p_args)...);

		// Return the primary thread priority to normal
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

		// Wait for previously spawned tasks to finish
		m_taskManager->waitForBackgroundThreads();
	}

protected:
	static const float m_defaultClockFrequency;
	TaskManager *m_taskManager;

	float m_clockFrequency;
	void *m_executionTimer;
	bool m_multithreadingEnabled;

	UniversalScene::SystemSceneMap m_systemScenes;
};