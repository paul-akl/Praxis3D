#pragma once

#include "System.h"

class PhysicsScene;

class PhysicsTask : public SystemTask
{
	friend class GUIScene;
public:
	PhysicsTask(PhysicsScene *p_physicsScene);
	~PhysicsTask();

	Systems::TypeID getSystemType() { return Systems::Physics; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return false; }

private:
	PhysicsScene *m_physicsScene;
};