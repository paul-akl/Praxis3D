#pragma once

#pragma comment(lib, "fmod/fmod_vc.lib")
//#pragma comment(lib, "fmod/fmodL_vc.lib")
#pragma comment(lib, "fmod/fmodstudio_vc.lib")
//#pragma comment(lib, "fmod/fmodstudioL_vc.lib")
//#pragma comment(lib, "fmod/fsbank_vc.lib")

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <fmod/fmod_studio.hpp>
#include <random>

#include "AudioTask.h"
#include "FmodErrorCodes.h"
#include "SoundComponent.h"
#include "SoundListenerComponent.h"
#include "System.h"

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
	// Returns true if the operation was successful; returns falls if operation failed and logs an error
	const inline bool fmodErrorLog(const FMOD_RESULT p_fmodResult, const std::string &p_objectName = "", const ErrorType p_errorType = ErrorType::Warning, const ErrorSource p_errorSource = ErrorSource::Source_AudioScene) const noexcept
	{
		// Check if the operation result is OK
		if(p_fmodResult != FMOD_RESULT::FMOD_OK)
		{
			// Convert the FMOD error to a string of the actual error name
			// If the object name was given, include it in the error
			if(p_objectName.empty())
				ErrHandlerLoc::get().log(p_errorType, p_errorSource, GetString(static_cast<FmodErrorCodes>(p_fmodResult)));
			else
				ErrHandlerLoc::get().log(p_errorType, p_errorSource, "\'" + p_objectName + "\': " + GetString(static_cast<FmodErrorCodes>(p_fmodResult)));

			// Operation failed - return false
			return false;
		}
		else // Operation successful - return true
			return true;
	}

	void addImpactSoundBank(FMOD::Studio::Bank *p_soundBank)
	{
		// Get the number of sound events
		int numOfEvents = 0;
		p_soundBank->getEventCount(&numOfEvents);

		if(numOfEvents > 0)
		{
			// Get the list of sound events
			FMOD::Studio::EventDescription **events = new FMOD::Studio::EventDescription * [numOfEvents];
			p_soundBank->getEventList(events, numOfEvents, &numOfEvents);

			// Go over each sound event
			for(int eventIndex = 0; eventIndex < numOfEvents; eventIndex++)
			{
				// Get sound event path
				char path[512];
				events[eventIndex]->getPath(path, 512, nullptr);

				// Extract the sound event name from the path, and assign the event itself to the event name entry in the impact sound map
				m_impactSounds[Utilities::splitStringAfterDelimiter(Config::audioVar().pathDelimiter, std::string(path))] = events[eventIndex];
			}

			// Delete the pointer to an array of pointers that was created
			delete events;
		}
	}

	void loadParameterGUIDs();

	AudioTask *m_audioTask;
	FMOD::Studio::System *m_studioSystem;
	FMOD::System *m_coreSystem;

	FMOD::ChannelGroup *m_masterChannelGroup;
	FMOD::ChannelGroup *m_ambientChannelGroup;
	FMOD::ChannelGroup *m_sfxChannelGroup;
	FMOD::ChannelGroup *m_musicChannelGroup;

	// Sound effects for object impacts
	FMOD::Studio::Bank *m_impactBank;

	// Sound events for object impacts
	FMOD::Studio::EventDescription *m_impactEvents[ObjectMaterialType::NumberOfMaterialTypes];

	SingleSound m_collisionSounds[ObjectMaterialType::NumberOfMaterialTypes];

	bool m_defaultImpactSoundsLoaded;

	int m_numSoundDrivers;

	FMOD::Studio::Bank *m_testBank1;
	FMOD::Studio::EventDescription *m_eventDescr1;
	FMOD::Studio::EventInstance *m_eventInstance1;
	FMOD::Sound *m_metalImpactSound[3];
	FMOD::Channel *m_metalImpactChannel[3];
	FMOD::DSP *m_pitch;
	std::random_device m_randomDevice;
	std::default_random_engine m_randomEngine;
	std::uniform_real_distribution<float> m_distribution;
	int m_lastPlayedImpactSound;

	//std::vector<FMOD::Studio::EventDescription*> m_impactSounds;
	//std::unordered_map<std::string, std::size_t> m_impactSoundsIndexMap;
	std::unordered_map<std::string, FMOD::Studio::EventDescription*> m_impactSounds;
	std::vector<std::pair<std::string, FMOD::Studio::Bank *>> m_bankFilenames;

	float m_dTime;
};