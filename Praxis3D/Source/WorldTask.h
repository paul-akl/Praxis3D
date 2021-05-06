#pragma once

#include "System.h"

class WorldScene;

class WorldTask : public SystemTask
{
	friend class WorldScene;
public:
	WorldTask(WorldScene *p_worldScene);
	~WorldTask();

	Systems::TypeID getSystemType() { return Systems::World; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return false; }

private:
	WorldScene *m_worldScene;
};