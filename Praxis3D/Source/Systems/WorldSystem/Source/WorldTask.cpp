
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/WorldSystem/Include/WorldTask.hpp"

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

