#pragma once

#include "System.h"

class AudioScene;

class AudioTask : public SystemTask
{
	friend class GUIScene;
public:
	AudioTask(AudioScene *p_audioScene);
	~AudioTask();

	Systems::TypeID getSystemType() { return Systems::Audio; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return false; }

private:
	AudioScene *m_audioScene;
};