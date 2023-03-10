#include "PhysicsScene.h"
#include "PhysicsTask.h"

PhysicsTask::PhysicsTask(PhysicsScene *p_physicsScene) : SystemTask(p_physicsScene), m_physicsScene(p_physicsScene)
{
}

PhysicsTask::~PhysicsTask()
{
}

void PhysicsTask::update(const float p_deltaTime)
{
	m_physicsScene->update(p_deltaTime);
}

