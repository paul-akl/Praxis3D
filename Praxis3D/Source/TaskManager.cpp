
#include <assert.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

//#include "Global.h"
#include "EngineDefinitions.h"
#include "ClockLocator.h"
#include "TaskManager.h"

namespace TaskManagerGlobal
{
	class GenericCallbackData
	{
	public:
		GenericCallbackData(void *p_param) : m_param(p_param) { }

	protected:
		void *m_param;
	};

	template<class ClassPointer>
	class GenericCallbackTask : public tbb::task, public GenericCallbackData
	{
	public:
		GenericCallbackTask(ClassPointer p_ptr, void *p_param) : GenericCallbackData(p_param), m_ptr(p_ptr) { }

		virtual tbb::task *execute()
		{
			//TODO ERROR
			assert(m_ptr != nullptr);

			m_ptr(m_param);

			return NULL;
		}

	protected:
		ClassPointer m_ptr;
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

			if (InterlockedDecrement(&m_callbacksCount) == 0)
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
		static void	*m_callbackParam;
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
			if (m_taskManager->isPrimaryThread())
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
		void		*m_waitFor;
		TaskManager *m_taskManager;
	};

	class ParallelFor : public GenericCallbackData
	{
	public:
		ParallelFor(TaskManager::ParallelForFunc p_pfCallback, void *p_param) : GenericCallbackData(p_param), m_parallelForCallback(p_pfCallback) { }

		void operator () (const tbb::blocked_range<unsigned int> & p_right) const
		{
			m_parallelForCallback(m_param, p_right.begin(), p_right.end());
		}

	private:
		TaskManager::ParallelForFunc m_parallelForCallback;
	};

	void *SynchronizeTask::m_callbackParam = nullptr;
	volatile long SynchronizeTask::m_callbacksCount = 0;
	void *SynchronizeTask::m_allCallbacksInvokedEvent = nullptr;
	TaskManager::JobFunct SynchronizeTask::m_callback = nullptr;

	//static TaskManager *g_taskManager = nullptr;
}

TaskManager::TaskManager()
{

	m_stallPoolParent = nullptr;
	m_systemTasksRoot = nullptr;
	m_tbbScheduler = nullptr;
	
	m_timeToQuit = false;
	m_deltaTime = 0.0f;
	m_stallPoolSemaphore = nullptr;

	m_numOfThreads = 0;
	m_numOfMaxThreads = 0;
	m_numOfTargetThreads = 0;
	m_numOfRequestedThreads = 0;
}
TaskManager::~TaskManager()
{

}

ErrorCode TaskManager::init()
{
	ErrorCode returnError = ErrorCode::Success;

	//TaskManagerGlobal::g_taskManager = this;

	m_primaryThreadID = tbb::this_tbb_thread::get_id();

	m_timeToQuit = false;
	m_numOfRequestedThreads = tbb::task_scheduler_init::default_num_threads();

	//m_numOfThreads = 0;
	//m_numOfMaxThreads = 0;
	//m_numOfTargetThreads = 0;

	m_stallPoolParent = nullptr;
	m_stallPoolSemaphore = CreateSemaphoreW(NULL, 0, m_numOfRequestedThreads, NULL);
	TaskManagerGlobal::SynchronizeTask::m_allCallbacksInvokedEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

	m_numOfThreads = m_numOfRequestedThreads;
	m_numOfMaxThreads = m_numOfRequestedThreads;
	m_numOfTargetThreads = m_numOfRequestedThreads;

	m_tbbScheduler = new tbb::task_scheduler_init(m_numOfRequestedThreads);
	m_systemTasksRoot = new(tbb::task::allocate_root()) tbb::empty_task;

	nonStandardPerThreadCallback(initAffinityData, this);

	return returnError;
}
void TaskManager::shutdown()
{
	// TODO ERROR
	assert(isPrimaryThread());

	m_timeToQuit = true;

	ReleaseSemaphore(m_stallPoolSemaphore, m_numOfMaxThreads, NULL);
	m_systemTasksRoot->destroy(*m_systemTasksRoot);

	delete m_tbbScheduler;

	CloseHandle(m_stallPoolSemaphore);
	m_stallPoolSemaphore = nullptr;
	CloseHandle(TaskManagerGlobal::SynchronizeTask::m_allCallbacksInvokedEvent);
}

void TaskManager::addStallTask()
{
	// TODO ERROR
	assert(m_stallPoolParent != nullptr);

	tbb::task *stallTask = new(m_stallPoolParent->allocate_additional_child_of(*m_stallPoolParent)) TaskManagerGlobal::StallTask(this, m_stallPoolSemaphore);
	assert(stallTask != NULL);

	m_stallPoolParent->spawn(*stallTask);
}
void TaskManager::setNumberOfThreads(unsigned int p_numOfThreads)
{
	unsigned int targetNumberOfThreads = p_numOfThreads;

	if (targetNumberOfThreads > m_numOfMaxThreads || targetNumberOfThreads == 0)
		targetNumberOfThreads = m_numOfMaxThreads;

	m_numOfTargetThreads = targetNumberOfThreads;
}
void TaskManager::waitForSystemTasks(SystemTask **p_tasks, unsigned int p_count)
{
	//TODO ERROR
	assert(isPrimaryThread());
	assert(p_count > 0);
	assert(p_count <= Systems::Types::Max);
	
	// Execute the tasks we are waiting, now
	// Save the tasks we aren't waiting, for next time

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	for (std::vector<SystemTask*>::iterator iterator = m_primaryThreadSystemTaskList.begin(); iterator != m_primaryThreadSystemTaskList.end(); iterator++)
	{
		// Check if we are waiting for this thread
		if (std::find(p_tasks, p_tasks + p_count, *iterator))
		{
			// If we are, execute it on the primary thread
			(*iterator)->update(m_deltaTime);
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
void TaskManager::nonStandardPerThreadCallback(JobFunct p_callback, void *p_data)
{
	SpinWait::Lock lock(m_syncedCallbackMutex);

	unsigned int numOfThreads = m_numOfThreads;
	if (numOfThreads != m_numOfMaxThreads)
	{
		m_numOfTargetThreads = m_numOfMaxThreads;
		updateThreadPoolSize();
	}

	TaskManagerGlobal::SynchronizeTask::prepareCallback(p_callback, p_data, m_numOfMaxThreads);

	tbb::task *broadcastParent = new(tbb::task::allocate_root()) tbb::empty_task;
	//TODO ERROR
	assert(broadcastParent != nullptr);

	// One reference for each thread, plus one for the wait_for_all call
	broadcastParent->set_ref_count(m_numOfMaxThreads + 1);

	tbb::task_list taskList;
	for (unsigned int i = 0; i < m_numOfMaxThreads; i++)
	{
		// Add a SynchronizeTask to each thread in the TBB pool (workers and master)
		tbb::task *newTask = new(broadcastParent->allocate_child()) TaskManagerGlobal::SynchronizeTask;
		// TODO ERROR
		assert(newTask != nullptr);
		taskList.push_back(*newTask);
	}

	// Run the synchronized tasks
	broadcastParent->spawn_and_wait_for_all(taskList);
	broadcastParent->destroy(*broadcastParent);

	if (numOfThreads != m_numOfMaxThreads)
	{
		m_numOfTargetThreads = numOfThreads;
		updateThreadPoolSize();
	}
}
void TaskManager::issueJobsForSystemTasks(SystemTask **p_tasks, unsigned int p_count, float p_deltaTime)
{
	//TODO ERROR
	assert(isPrimaryThread());
	assert(p_count > 0);
	
	m_deltaTime = p_deltaTime;

	updateThreadPoolSize();

	assert(m_systemTasksRoot != nullptr);

	// Set reference count to 1, to support wait_for_all
	m_systemTasksRoot->set_ref_count(1);

	// Schedule tasks based on their performance hint order
	tbb::task_list taskList;
	unsigned int affinityCount = (unsigned int)m_affinityIDs.size();
	
	for (unsigned int perfHint = 0, currentTask = 0; perfHint < PerformanceHint::Task_MAX; perfHint++)
	{
		for (currentTask = 0; currentTask < p_count; currentTask++)
		{
			if (p_tasks[currentTask]->isPrimaryThreadOnly())
			{
				// Put this task on the list of tasks to be run on the primary thread
				// only do this during the first outer loop
				//if (perfHint == 0)
				//{
					m_primaryThreadSystemTaskList.push_back(p_tasks[currentTask]);
				//}
			}
			else
			{
				// Check if it's time to dispatch this task
				if (getPerformanceHint(p_tasks[currentTask]) == (PerformanceHint)perfHint)
				{
					// This task can be run on an arbitrary thread - allocate it 
					TaskManagerGlobal::GenericCallbackTask<TaskManager::JobFunct> *systemTask
						= new(m_systemTasksRoot->allocate_additional_child_of(*m_systemTasksRoot))
						TaskManagerGlobal::GenericCallbackTask<TaskManager::JobFunct>(systemTaskCallback, p_tasks[currentTask]);

					// TODO ASSERT ERROR
					assert(systemTask != nullptr);

					// Affinity will increase the chances that each SystemTask will be assigned
					// to a unique thread, regardless of PerformanceHint
					systemTask->set_affinity(m_affinityIDs[currentTask % affinityCount]);
					taskList.push_back(*systemTask);
				}
			}
		}

		// We only spawn system tasks here. They in their turn will spawn descendant tasks.
		// Waiting for the whole bunch completion happens in WaitForSystemTasks.
		m_systemTasksRoot->spawn(taskList);
	}
}
void TaskManager::parallelFor(SystemTask *p_systemTask, ParallelForFunc p_jobFunc, void *p_param, unsigned int p_begin, unsigned int p_end, unsigned int p_minGrainSize)
{
	TaskManagerGlobal::ParallelFor parallelForBody(p_jobFunc, p_param);

	if (m_numOfThreads != 1)
	{
		tbb::parallel_for(tbb::blocked_range<unsigned int>(p_begin, p_end, p_minGrainSize), parallelForBody, tbb::auto_partitioner());
	}
	else
	{
		parallelForBody(tbb::blocked_range<unsigned int>(p_begin, p_end, p_minGrainSize));
	}
}

void TaskManager::systemTaskCallback(void *p_data)
{
	SystemTask *task = static_cast<SystemTask*>(p_data);
	task->update(ClockLocator::get().getDeltaSecondsF());
}

void TaskManager::updateThreadPoolSize()
{
	// Change the number of threads if needed, by creating some tasks which do not complete until signaled

	if (m_numOfTargetThreads != m_numOfThreads)
	{
		unsigned int numOfThreadsToWait = (m_numOfMaxThreads - m_numOfTargetThreads);
		unsigned int numOfThreadsToFree = (m_numOfMaxThreads - m_numOfThreads);

		// Free up all the threads
		if (m_stallPoolParent)
		{
			ReleaseSemaphore(m_stallPoolSemaphore, numOfThreadsToFree, NULL);

			// Make sure there are no stall tasks competing for the semaphore
			m_stallPoolParent->wait_for_all();
		}
		else
		{
			// Make a new stall parents if it doesn't exist
			m_stallPoolParent = new(tbb::task::allocate_root()) tbb::empty_task;
		}

		// TODO ERROR
		assert(m_stallPoolParent != nullptr);
		m_stallPoolParent->set_ref_count(numOfThreadsToWait + 1);

		tbb::task_list taskList;
		for (unsigned int i = 0; i < numOfThreadsToWait; i++)
		{
			tbb::task *stallTask = new(m_stallPoolParent->allocate_child()) TaskManagerGlobal::StallTask(this, m_stallPoolSemaphore);
			// TODO ERROR
			assert(stallTask != NULL);
			taskList.push_back(*stallTask);
		}

		m_stallPoolParent->spawn(taskList);
		m_numOfThreads = m_numOfTargetThreads;

		// TODO Cache thread count
	}
}

void TaskManager::initAffinityData(void *p_manager)
{
	TaskManager *manager = static_cast<TaskManager*>(p_manager);

	SpinWait::Lock lock(manager->m_spinMutex);
	manager->m_affinityIDs.push_back(tbb::task::self().affinity());
}
bool TaskManager::isTBBThread()
{
	// Determine if the calling thread is an TBB thread
	// If called not from TBB thread, task::self() will assert itself
	// TODO ERROR
	return (&tbb::task::self() != nullptr);
}