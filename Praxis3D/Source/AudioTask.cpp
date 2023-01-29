#include "AudioScene.h"
#include "AudioTask.h"

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
