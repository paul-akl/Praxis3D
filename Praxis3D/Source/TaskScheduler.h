#pragma once

#include <map>

#include "System.h"
#include "Universal.h"

class TaskManager;

class TaskScheduler
{
public:
	TaskScheduler(TaskManager *p_taskManager);
	~TaskScheduler();

	void setScene(const UniversalScene *p_scene);

	void execute(float p_deltaTime);

protected:
	static const float m_defaultClockFrequency;
	TaskManager *m_taskManager;

	float m_clockFrequency;
	void *m_executionTimer;
	bool m_multithreadingEnabled;

	UniversalScene::SystemSceneMap m_systemScenes;
};