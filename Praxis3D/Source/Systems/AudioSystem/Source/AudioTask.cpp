#include "Systems/AudioSystem/Include/AudioScene.hpp"
#include "Systems/AudioSystem/Include/AudioTask.hpp"

AudioTask::AudioTask(AudioScene *p_audioScene) : SystemTask(p_audioScene), m_audioScene(p_audioScene)
{
}

AudioTask::~AudioTask()
{
}

void AudioTask::update(const float p_deltaTime)
{
	m_audioScene->update(p_deltaTime);
}
