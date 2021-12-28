#pragma once

#define TBB_SUPPRESS_DEPRECATED_MESSAGES 1

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <tbb/task.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_thread.h>

#include <vector>
#include "Window.h"

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

	// Does a callback once from each thread used by the TaskManager; Calls one at a time, and only once
	void nonStandardPerThreadCallback(JobFunct p_callback, void *p_data);

	// Assigns threads to tasks and executed them; deals with tasks that are to be run on primary thread
	void issueJobsForSystemTasks(SystemTask **p_tasks, unsigned int p_count, float p_deltaTime);

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

		//return m_numOfThreads;
		return PerformanceHint::Task_NoPerformanceHint;
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

/*#include <tbb\tbb.h>
#include <wtypes.h>

#include "TaskSet.h"

typedef void(*TaskSetFunc)(void*, int, unsigned int, unsigned int);
typedef unsigned int TaskSetHandle;

#define TASKSETHANDLE_INVALID			0xFFFFFFFF
#define MAX_SUCCESSORS                  5
#define MAX_TASKSETS                    256
#define MAX_TASKSETNAMELENGTH           512

tbb::atomic<INT>						g_contextIdCount;
tbb::enumerable_thread_specific<INT>	g_contextId;

class TbbContextId;
class GenericTask;
class TaskSet;

class TaskManager
{
public:
TaskManager();
~TaskManager();

void init();
void shutdown();

bool createTaskSet(TaskSetFunc p_func, void *p_arg, unsigned int p_taskCount, TaskSetHandle *p_depends,
unsigned int p_dependsCount, OPTIONAL char* p_name, OUT TaskSetHandle *p_outHandle);

void releaseHandle(TaskSetHandle p_taskSet);
void releaseHandles(TaskSetHandle *p_taskSetList, unsigned int p_taskSetCount);

void waitForSet(TaskSetHandle p_taskSet);
void waitForAll();

bool isSetComplete(TaskSetHandle p_set);

private:
friend class GenericTask;

TaskSetHandle allocateTaskSet();

void completeTaskSet(TaskSetHandle p_set);

TaskSet			*m_taskSets[MAX_TASKSETS];
TbbContextId	*m_tbbContextId;
void			*m_tbbInit;
unsigned int	m_nextFreeSet;
};

class TbbContextId : public tbb::task_scheduler_observer
{
void on_scheduler_entry(bool)
{
INT context = g_contextIdCount.fetch_and_increment();
g_contextId.local() = context;
}

public:
TbbContextId()
{
g_contextIdCount = 0;
observe(true);
}
};

class SpinLock
{
public:
SpinLock() : m_lock(0)	{ }
~SpinLock()				{ }

void lock()
{
while (_InterlockedCompareExchange((long*)&m_lock, 1, 0) == 1)
{

}
}

void unlock()
{
m_lock = 0;
}

private:
volatile unsigned int m_lock;
};

class GenericTask : public tbb::task
{
public:
GenericTask();
GenericTask(TaskSetFunc p_func, void* p_arg, unsigned int p_id, unsigned int p_size, char* p_setName, TaskSetHandle p_taskSet) :
m_func(p_func), m_arg(p_arg), m_id(p_id), m_size(p_size), m_setName(p_setName), m_taskSet(p_taskSet)
{

}

task* execute();
//{
//	m_func(m_arg, g_contextId.local(), m_id, m_size);
//	g_taskManager.completeTaskSet(m_taskSet);
//}

private:
TaskSetFunc		m_func;
void*			m_arg;
unsigned int	m_id;
unsigned int	m_size;
char*			m_setName;
TaskSetHandle		m_taskSet;
};

class TaskSet : public tbb::task
{
public:
TaskSet() :
m_func(NULL),
m_arg(0),
m_size(0),
m_taskSet(TASKSETHANDLE_INVALID),
m_hasBeenWaitedOn(false)
{
m_setName[0] = 0;
memset(m_successors, 0, sizeof(m_successors));
}

~TaskSet() { }

task* execute()
{
set_ref_count(m_size + 1);

for (unsigned int i = 0; i < m_size; i++)
{
spawn(*new(allocate_child()) GenericTask(m_func, m_arg, i, m_size, m_setName, m_taskSet));
}

return NULL;
}

TaskSet			*m_successors[MAX_SUCCESSORS];
TaskSetHandle	m_taskSet;
bool			m_hasBeenWaitedOn;

TaskSetFunc	m_func;
void		*m_arg;

volatile unsigned int	m_startCount;
volatile unsigned int	m_completionCount;
volatile unsigned int	m_refCount;

unsigned int	m_size;
SpinLock		m_successorsLock;

char	m_setName[MAX_TASKSETNAMELENGTH];
};*/