#pragma once

#pragma comment(lib, "fmod/fmod_vc.lib")
//#pragma comment(lib, "fmod/fmodL_vc.lib")
//#pragma comment(lib, "fmod/fmodstudio_vc.lib")
//#pragma comment(lib, "fmod/fmodstudioL_vc.lib")
//#pragma comment(lib, "fmod/fsbank_vc.lib")

#include <fmod/core/fmod.hpp>
#include <fmod/core/fmod_errors.h>

#include "AudioTask.h"
#include "SoundComponent.h"
#include "System.h"

struct ComponentsConstructionInfo;

struct AudioComponentsConstructionInfo
{
	AudioComponentsConstructionInfo()
	{
		m_soundConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const AudioComponentsConstructionInfo &p_other)
	{
		Utilities::performCopy<SoundComponent::SoundComponentConstructionInfo>(&m_soundConstructionInfo, &p_other.m_soundConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		if(m_soundConstructionInfo != nullptr)
			delete m_soundConstructionInfo;
	}

	SoundComponent::SoundComponentConstructionInfo *m_soundConstructionInfo;
};

class AudioScene : public SystemScene
{
public:
	AudioScene(SystemBase *p_system, SceneLoader *p_sceneLoader);
	~AudioScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void activate();

	void deactivate();

	void update(const float p_deltaTime);

	ErrorCode preload();

	void loadInBackground();

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const AudioComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		std::vector<SystemObject*> components;

		if(p_constructionInfo.m_soundConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_soundConstructionInfo, p_startLoading));

		return components;
	}

	SystemObject *createComponent(const EntityID &p_entityID, const SoundComponent::SoundComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	SystemTask *getSystemTask() { return m_audioTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::Audio; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	AudioTask *m_audioTask;
	FMOD::System *m_fmodSystem;

	FMOD::ChannelGroup *m_masterChannelGroup;
	FMOD::ChannelGroup *m_musicChannelGroup;
	FMOD::ChannelGroup *m_ambientChannelGroup;

	int m_numSoundDrivers;
};