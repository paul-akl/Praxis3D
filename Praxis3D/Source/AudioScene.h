#pragma once

#include <fmod/fmod_studio.hpp>
#include <random>

#include "AudioTask.h"
#include "FmodErrorCodes.h"
#include "SoundComponent.h"
#include "SoundListenerComponent.h"
#include "System.h"

class AudioSystem;
struct ComponentsConstructionInfo;

struct AudioComponentsConstructionInfo
{
	AudioComponentsConstructionInfo()
	{
		m_soundConstructionInfo = nullptr;
		m_soundListenerConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const AudioComponentsConstructionInfo &p_other)
	{
		Utilities::performCopy<SoundComponent::SoundComponentConstructionInfo>(&m_soundConstructionInfo, &p_other.m_soundConstructionInfo);
		Utilities::performCopy<SoundListenerComponent::SoundListenerComponentConstructionInfo>(&m_soundListenerConstructionInfo, &p_other.m_soundListenerConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		if(m_soundConstructionInfo != nullptr)
			delete m_soundConstructionInfo;

		if(m_soundListenerConstructionInfo != nullptr)
			delete m_soundListenerConstructionInfo;
	}

	SoundComponent::SoundComponentConstructionInfo *m_soundConstructionInfo;
	SoundListenerComponent::SoundListenerComponentConstructionInfo *m_soundListenerConstructionInfo;
};

struct SingleSound
{
	SingleSound() : m_sound(nullptr), m_loaded(false) { }

	FMOD::Sound *m_sound;
	std::string m_filename;
	bool m_loaded;
};

class AudioScene : public SystemScene
{
public:
	AudioScene(SystemBase *p_system, SceneLoader *p_sceneLoader);
	~AudioScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void exportSetup(PropertySet &p_propertySet);

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

		if(p_constructionInfo.m_soundListenerConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_soundListenerConstructionInfo, p_startLoading));

		return components;
	}

	void exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo);
	void exportComponents(const EntityID p_entityID, AudioComponentsConstructionInfo &p_constructionInfo);

	SystemObject *createComponent(const EntityID &p_entityID, const SoundComponent::SoundComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	SystemObject *createComponent(const EntityID &p_entityID, const SoundListenerComponent::SoundListenerComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);

	void exportComponent(SoundComponent::SoundComponentConstructionInfo &p_constructionInfo, const SoundComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_constructionInfo.m_loop = p_component.m_loop;
		p_constructionInfo.m_soundFilename = p_component.m_soundFilename;
		p_constructionInfo.m_soundType = p_component.m_soundType;
		p_constructionInfo.m_spatialized = p_component.m_spatialized;
		p_constructionInfo.m_startPlaying = p_component.m_startPlaying;
		p_constructionInfo.m_volume = p_component.m_volume;
	}
	void exportComponent(SoundListenerComponent::SoundListenerComponentConstructionInfo &p_constructionInfo, const SoundListenerComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_constructionInfo.m_listenerID = p_component.getListenerID();
	}

	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	SystemTask *getSystemTask() { return m_audioTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::Audio; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	void loadParameterGUIDs();

	AudioTask *m_audioTask;
	AudioSystem *m_audioSystem;

	// FMOD studio and core system handles
	FMOD::Studio::System *m_studioSystem;
	FMOD::System *m_coreSystem;

	// Sound events for object impacts
	FMOD::Studio::EventDescription *m_impactEvents[ObjectMaterialType::NumberOfMaterialTypes];

	SingleSound m_collisionSounds[ObjectMaterialType::NumberOfMaterialTypes];

	// Volume values of different buses
	float volume_ambient;
	float volume_master;
	float volume_music;
	float volume_sfx;

	// All banks that this scene have loaded
	std::vector<std::pair<std::string, FMOD::Studio::Bank *>> m_bankFilenames;
};