#pragma once

#define TBB_SUPPRESS_DEPRECATED_MESSAGES 1

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <tbb/task.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_thread.h>

#include <vector>
#include "Window.h"

#include "EngineDefinitions.h"
#include "System.h"
#include "SpinWait.h"

enum PerformanceHint
{
	Task_LongSerial = 0,
	Task_LongParallel,
	Task_Short,
	Task_NoPerformanceHint,
	Task_MAX
};

inline PerformanceHint getPerformanceHint(SystemTask *p_task)
{
	// TODO PERFORMANCE HINTS FOR SPECIFIC SYSTEMS
	return Task_NoPerformanceHint;
}

class TaskManager
{
public:
	enum JobCountInstructionHints
	{
		None,
		Generic,
		FP,
		SIMD_FP,
		SIMD_INT
	};

	typedef void(*JobFunct)(void*);
	typedef void(*ParallelForFunc)(void *p_param, unsigned int p_begin, unsigned int p_end);

	TaskManager();
	~TaskManager();

	ErrorCode init();
	void shutdown();

	void addStallTask();

	// Matches the thread ID against primary thread ID
	inline bool isPrimaryThread() { return (tbb::this_tbb_thread::get_id() == m_primaryThreadID); }

	// Sets the number of threads for task manager to utilize. Does not assign number higher than max threads
	void setNumberOfThreads(unsigned int p_numOfThreads);

	// Called from primary thread to wait until specified tasks spawned with issueJobsForSystemTasks
	// and their subtasks are complete. Work on tasks dedicated to the primary thread
	void waitForSystemTasks(SystemTask **p_tasks, unsigned int p_count);

	// Called from primary thread to wait until specified tasks spawned with issueJobsForSystemTasks
	// and their subtasks are complete. Work on tasks dedicated to the primary thread
	// Templated version
	template<typename T_Func, typename... T_Args>
	void waitForSystemTasks(SystemTask **p_tasks, unsigned int p_count, T_Func &p_func, T_Args&&... p_args)
	{
		//TODO ERROR
		assert(isPrimaryThread());
		assert(p_count > 0);
		assert(p_count <= Systems::Types::Max);

		// Execute the tasks we are waiting, now
		// Save the tasks we aren't waiting, for next time

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		for(std::vector<SystemTask *>::iterator iterator = m_primaryThreadSystemTaskList.begin(); iterator != m_primaryThreadSystemTaskList.end(); iterator++)
		{
			// Check if we are waiting for this thread
			if(std::find(p_tasks, p_tasks + p_count, *iterator))
			{
				// If we are, execute it on the primary thread
				p_func(*iterator, std::forward<T_Args>(p_args)...);
			}
			else
			{
				// If we aren't, save it for next time
				m_tempTasksList.push_back(*iterator);
			}
		}

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		m_primaryThreadSystemTaskList.clear();
		m_primaryThreadSystemTaskList.swap(m_tempTasksList);

		// Wait for the parallel calculation
		m_systemTasksRoot->wait_for_all();
	}

	// Does a callback once from each thread used by the TaskManager; Calls one at a time, and only once
	void nonStandardPerThreadCallback(JobFunct p_callback, void *p_data);

	// Assigns threads to tasks and executed them; deals with tasks that are to be run on primary thread
	void issueJobsForSystemTasks(SystemTask **p_tasks, unsigned int p_count, float p_deltaTime);

	// Assigns threads to tasks and executed them; deals with tasks that are to be run on primary thread
	// Templated version
	template<typename T_Func, typename... T_Args>
	void issueJobsForSystemTasks(SystemTask **p_tasks, unsigned int p_count, T_Func &p_func, T_Args&&... p_args)
	{
		//TODO ERROR
		assert(isPrimaryThread());
		assert(p_count > 0);

		//m_deltaTime = p_deltaTime;

		updateThreadPoolSize();

		assert(m_systemTasksRoot != nullptr);

		// Set reference count to 1, to support wait_for_all
		m_systemTasksRoot->set_ref_count(1);

		// Schedule tasks based on their performance hint order
		tbb::task_list taskList;
		unsigned int affinityCount = (unsigned int)m_affinityIDs.size();

		// TODO: implement performance hint

		for(unsigned int currentTask = 0; currentTask < p_count; currentTask++)
		{
#ifdef SETTING_MULTITHREADING_ENABLED
			if(p_tasks[currentTask]->isPrimaryThreadOnly())
			{
				m_primaryThreadSystemTaskList.push_back(p_tasks[currentTask]);
			}
			else
			{
				auto test = std::bind(p_func, p_tasks[currentTask], std::forward<T_Args>(p_args)...);

				TaskManagerGlobal::GenericCallbackTaskFunctor<decltype(test)> *systemTask
					= new(m_systemTasksRoot->allocate_additional_child_of(*m_systemTasksRoot))
					TaskManagerGlobal::GenericCallbackTaskFunctor<decltype(test)>(test);
				
				// TODO ASSERT ERROR
				assert(systemTask != nullptr);

				// Affinity will increase the chances that each SystemTask will be assigned
				// to a unique thread, regardless of PerformanceHint
				systemTask->set_affinity(m_affinityIDs[currentTask % affinityCount]);
				taskList.push_back(*systemTask);
			}
#else
			m_primaryThreadSystemTaskList.push_back(p_tasks[currentTask]);
#endif
		}

		// We only spawn system tasks here. They in their turn will spawn descendant tasks.
		// Waiting for the whole bunch completion happens in WaitForSystemTasks.
		m_systemTasksRoot->spawn(taskList);
	}

	// TBB paralle_for wrapper for job tasks
	void parallelFor(SystemTask *p_systemTask, ParallelForFunc p_jobFunc, void *p_param, unsigned int p_begin, unsigned int p_end, unsigned int p_minGrainSize = 1);

	// Passed function is executed in a parallel thread; returns before the passed function has been completed
	template<typename Function>
	inline void startBackgroundThread(const Function& p_func)
	{
		// If multi-threading is enabled 
#ifdef SETTING_MULTITHREADING_ENABLED
		m_backgroundTaskGroup.run(p_func);
#else
		p_func();
#endif
	}

	// Wrapper for a TBB parallel_for function; executed the given set of functions in parallel
	// Returns, when all the given functions have been completed
	template <typename Index, typename Function>
	inline void parallelFor(Index p_first, Index p_last, Index p_step, const Function& p_func)
	{
		// If multi-threading is enabled 
#ifdef SETTING_MULTITHREADING_ENABLED
		tbb::parallel_for(p_first, p_last, p_step, p_func);
#else
		for(Index i = p_first; i < p_last; i += p_step)
			p_func(i);
#endif
	}

	unsigned int getNumberOfThreads() { return m_numOfThreads; }
	unsigned int getRecommendedJobCount(JobCountInstructionHints p_hints)
	{
		// TODO: implement job instruction hints to issue tasks on threads more efficiently 

		return m_numOfThreads;
		//return PerformanceHint::Task_NoPerformanceHint;
	}

	// Waits for the background threads to finish their work
	void waitForBackgroundThreads()
	{
		m_backgroundTaskGroup.wait();
	}

	// Cancels all the spawned background tasks
	void cancelBackgroundThreads()
	{
		// Cancel tasks
		m_backgroundTaskGroup.cancel();
	}

	// Casts passed data to a system task and calls update on it
	static void systemTaskCallback(void *p_data);

private:
	void updateThreadPoolSize();

	static void initAffinityData(void *p_manager);
	static bool isTBBThread();

	tbb::task					*m_stallPoolParent;
	tbb::task					*m_systemTasksRoot;
	tbb::tbb_thread::id			m_primaryThreadID;
	tbb::task_scheduler_init	*m_tbbScheduler;
	tbb::task_group				m_backgroundTaskGroup;

	std::vector<tbb::task::affinity_id> m_affinityIDs;
	std::vector<SystemTask*>			m_tempTasksList;
	std::vector<SystemTask*>			m_primaryThreadSystemTaskList;

	SpinWait m_syncedCallbackMutex;
	SpinWait m_spinMutex;

	bool m_timeToQuit;
	float m_deltaTime;
	void *m_stallPoolSemaphore;

	unsigned int m_numOfThreads;
	unsigned int m_numOfMaxThreads;
	unsigned int m_numOfTargetThreads;
	unsigned int m_numOfRequestedThreads;
};

namespace TaskManagerGlobal
{
	// Used to store a single parameter for generic callback
	class GenericCallbackData
	{
	public:
		GenericCallbackData(void *p_param) : m_param(p_param) { }

	protected:
		void *m_param;
	};

	// Stores a function pointer (usually TaskManager::JobFunct) and a single parameter
	template<typename T_Func>
	class GenericCallbackTask : public tbb::task, public GenericCallbackData
	{
	public:
		GenericCallbackTask(T_Func p_ptr, void *p_param) : GenericCallbackData(p_param), m_ptr(p_ptr) { }

		tbb::task *execute()
		{
			//TODO ERROR
			assert(m_ptr != nullptr);

			m_ptr(m_param);

			return NULL;
		}

	protected:
		T_Func m_ptr;
	};

	// Stores a function pointer (to be used with std::bind, so the function pointer already contains all the parameters it needs)
	template<typename T_Func>
	class GenericCallbackTaskFunctor : public tbb::task
	{
	public:
		GenericCallbackTaskFunctor(T_Func &p_func) : m_func(p_func) { }

		tbb::task *execute()
		{
			m_func();

			return NULL;
		}

	private:
		T_Func m_func;
	};

	class SynchronizeTask : public tbb::task
	{
	public:
		SynchronizeTask() { }

		tbb::task *execute()
		{
			// TODO ERRORS
			assert(m_callback != NULL);
			assert(m_allCallbacksInvokedEvent != NULL);

			m_callback(m_callbackParam);

			if(InterlockedDecrement(&m_callbacksCount) == 0)
			{
				SetEvent(m_allCallbacksInvokedEvent);
			}
			else
			{
				WaitForSingleObject(m_allCallbacksInvokedEvent, INFINITE);
			}

			return NULL;
		}

		static void prepareCallback(TaskManager::JobFunct p_func, void *p_param, unsigned int p_count)
		{
			m_callback = p_func;
			m_callbackParam = p_param;
			m_callbacksCount = p_count;
			ResetEvent(m_allCallbacksInvokedEvent);
		}

	protected:
		friend class TaskManager;
		static void *m_callbackParam;
		static volatile long m_callbacksCount;
		static void *m_allCallbacksInvokedEvent;
		static TaskManager::JobFunct m_callback;
	};

	class StallTask : public tbb::task
	{
	public:
		StallTask(TaskManager *p_taskManager, void *p_waitFor) : m_taskManager(p_taskManager), m_waitFor(p_waitFor) { }

		tbb::task *execute()
		{
			if(m_taskManager->isPrimaryThread())
			{
				// Cannot stall a primary task, so stall some other task
				m_taskManager->addStallTask();

				// Wait a bit to give some time for some thread to pick up the stall task
				// TODO change hardcoded value
				tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(0.1));
			}
			else
			{
				//TODO ERROR
				assert(m_waitFor != nullptr);
				WaitForSingleObject(m_waitFor, INFINITE);
			}
			return NULL;
		}

	protected:
		void *m_waitFor;
		TaskManager *m_taskManager;
	};

	class ParallelFor : public GenericCallbackData
	{
	public:
		ParallelFor(TaskManager::ParallelForFunc p_pfCallback, void *p_param) : GenericCallbackData(p_param), m_parallelForCallback(p_pfCallback) { }

		void operator () (const tbb::blocked_range<unsigned int> &p_right) const
		{
			m_parallelForCallback(m_param, p_right.begin(), p_right.end());
		}

	private:
		TaskManager::ParallelForFunc m_parallelForCallback;
	};
}