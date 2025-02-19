#pragma once

#include "Engine/Include/EngineDefinitions.hpp"
#include "Multithreading/Include/TaskManager.hpp"

// Service locator provides global access to specific instantiated classes (i.e. services),
// but retains the ability to switch instances. Uses dependency injection pattern.
// This particular instance is using a wrapper around the service. Hence instead of giving
// direct access to the service, it only allows certain functions to be called (for safety reasons).
class TaskManagerLocator
{
public:
	class TaskManagerWrapper
	{
		friend class TaskManagerLocator;
	public:
		template<typename Function>
		void startBackgroundThread(const Function& p_func)
		{
			if(m_validTaskManager)
				m_taskManager->startBackgroundThread(p_func);
			else
				p_func();
		}

		template <typename Index, typename Function>
		inline void parallelFor(Index p_first, Index p_last, Index p_step, const Function& p_func)
		{
			if(m_validTaskManager)
				m_taskManager->parallelFor(p_first, p_last, p_step, p_func);
			else
				for(Index i = p_first; i < p_last; i += p_step)
					p_func(i);

		}

	private:
		TaskManagerWrapper() : m_taskManager(nullptr), m_validTaskManager(false) { }
		TaskManagerWrapper(TaskManager *p_taskManager) : m_taskManager(p_taskManager) { m_validTaskManager = (p_taskManager != nullptr); }

		void setTaskManager(TaskManager *p_taskManager)
		{
			m_taskManager = p_taskManager;
			m_validTaskManager = (p_taskManager != nullptr); 
		}

		TaskManager *m_taskManager;
		bool m_validTaskManager;
	};

	// Get the service
	inline static TaskManagerWrapper &get() { return m_taskMgrWrapper; }

	// Initialize the service locator to use null services
	inline static ErrorCode init() { return ErrorCode::Success; }

	inline static void provide(TaskManager *p_taskManager) { m_taskMgrWrapper.setTaskManager(p_taskManager); }

private:
	static TaskManagerWrapper m_taskMgrWrapper;
};