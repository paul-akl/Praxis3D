#include "PhysicsScene.h"

PhysicsScene::PhysicsScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_physicsTask = new PhysicsTask(this);
}

ErrorCode PhysicsScene::setup(const PropertySet &p_properties)
{
	return ErrorCode();
}

void PhysicsScene::update(const float p_deltaTime)
{
}

SystemObject *PhysicsScene::createObject(const PropertySet &p_properties)
{
	return nullptr;
}

ErrorCode PhysicsScene::destroyObject(SystemObject *p_systemObject)
{
	return ErrorCode();
}
