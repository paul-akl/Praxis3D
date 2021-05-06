
#include "WorldScene.h"
#include "WorldTask.h"

WorldTask::WorldTask(WorldScene *p_worldScene)
	: SystemTask(p_worldScene), m_worldScene(p_worldScene)
{

}

WorldTask::~WorldTask()
{
}

void WorldTask::update(const float p_deltaTime)
{
	m_worldScene->update(p_deltaTime);
}

