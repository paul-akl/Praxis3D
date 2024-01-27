#pragma once

#pragma comment(lib, "fmod/fmod_vc.lib")
//#pragma comment(lib, "fmod/fmodL_vc.lib")
#pragma comment(lib, "fmod/fmodstudio_vc.lib")
//#pragma comment(lib, "fmod/fmodstudioL_vc.lib")
//#pragma comment(lib, "fmod/fsbank_vc.lib")

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <fmod/fmod_studio.hpp>

#include "AudioScene.h"
#include "ErrorHandlerLocator.h"
#include "System.h"

class AudioSystem : public SystemBase
{
	friend class AudioScene;
public:
	AudioSystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			m_audioScenes[i] = nullptr;

		m_systemName = GetString(Systems::Audio);
		m_coreSystem = nullptr;
		m_studioSystem = nullptr;
		m_impactBank = nullptr;
		m_numSoundDrivers = 0;

		for(unsigned int i = 0; i < AudioBusType::AudioBusType_NumOfTypes; i++)
		{
			m_buses[i] = nullptr;
			m_channelGroups[i] = nullptr;
		}
	}
	~AudioSystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			if(m_audioScenes[i] != nullptr)
				delete m_audioScenes[i];

		if(m_studioSystem != nullptr)
		{
			if(m_coreSystem != nullptr)
				m_coreSystem->release();

			m_studioSystem->release();
		}
	}

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	virtual ErrorCode preload()
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}

	void loadInBackground() { }

	Systems::TypeID getSystemType() { return Systems::Audio; }
	FMOD::Studio::System *getStudioSystem() { return m_studioSystem; }
	FMOD::System *getCoreSystem() { return m_coreSystem; }

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
	{
		if(m_audioScenes[p_engineState] == nullptr)
		{
			// Create new scene
			m_audioScenes[p_engineState] = new AudioScene(this, p_sceneLoader);
			ErrorCode sceneError = m_audioScenes[p_engineState]->init();

			// Check if it initialized correctly (cannot continue without the scene)
			if(sceneError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(sceneError, ErrorSource::Source_AudioScene);
				delete m_audioScenes[p_engineState];
				m_audioScenes[p_engineState] = nullptr;
			}
		}

		return m_audioScenes[p_engineState];
	}

	SystemScene *getScene(EngineStateType p_engineState) { return m_audioScenes[p_engineState]; }

	void deleteScene(EngineStateType p_engineState)
	{
		if(m_audioScenes[p_engineState] != nullptr)
		{
			// Shutdown the scene before destroying it
			m_audioScenes[p_engineState]->shutdown();

			delete m_audioScenes[p_engineState];
			m_audioScenes[p_engineState] = nullptr;
		}
	}

protected:
	// Returns true if the operation was successful; returns false if operation failed and logs an error
	static inline bool fmodErrorLog(const FMOD_RESULT p_fmodResult, const std::string &p_objectName = "", const ErrorType p_errorType = ErrorType::Warning, const ErrorSource p_errorSource = ErrorSource::Source_AudioScene) noexcept
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

	inline void registerEvents(FMOD::Studio::Bank *p_soundBank)
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

				// Extract the sound event name from the path, and assign the event itself to the event name entry in the sound event map
				m_soundEvents[Utilities::splitStringAfterDelimiter(Config::audioVar().pathDelimiter, std::string(path))] = events[eventIndex];
			}

			// Delete the pointer to an array of pointers that was created
			delete events;
		}
	}

	// Search through all loaded sound events and return a match; if no match is found, return a nullptr
	inline FMOD::Studio::EventDescription *getEvent(const std::string &p_eventName)
	{
		auto event = m_soundEvents.find(p_eventName);

		if(event == m_soundEvents.end())
			return nullptr;
		else
			return event->second;
	}

	FMOD::Studio::Bank *loadBankFile(const std::string p_filename, FMOD_STUDIO_LOAD_BANK_FLAGS p_flags);

	// Load all the audio buses from the studio system into a buses array
	void assignBusses()
	{
		for(unsigned int busType = 0; busType < AudioBusType::AudioBusType_NumOfTypes; busType++)
		{
			switch(busType)
			{
				case AudioBusType_Ambient:
					m_studioSystem->getBus((Config::audioVar().bus_name_prefix + Config::audioVar().bus_name_ambient).c_str(), &m_buses[busType]);
					break;
				case AudioBusType_Master:
					m_studioSystem->getBus((Config::audioVar().bus_name_prefix + Config::audioVar().bus_name_master).c_str(), &m_buses[busType]);
					break;
				case AudioBusType_Music:
					m_studioSystem->getBus((Config::audioVar().bus_name_prefix + Config::audioVar().bus_name_music).c_str(), &m_buses[busType]);
					break;
				case AudioBusType_SFX:
					m_studioSystem->getBus((Config::audioVar().bus_name_prefix + Config::audioVar().bus_name_sfx).c_str(), &m_buses[busType]);
					break;
				case AudioBusType_NumOfTypes:
				default:
					// Log an error if this portion is reached, as more Audio Bus Types might have been added without modifying this method
					ErrHandlerLoc::get().log(ErrorCode::Audio_invalid_bus_type, ErrorSource::Source_AudioSystem);
					break;
			}
		}
	}

	FMOD::Studio::Bus *getBus(const AudioBusType p_busType) { return m_buses[p_busType]; }
	FMOD::ChannelGroup *getChannelGroup(const AudioBusType p_busType) { return m_channelGroups[p_busType]; }

	AudioScene *m_audioScenes[EngineStateType::EngineStateType_NumOfTypes];

	// Sound banks for object impacts
	FMOD::Studio::Bank *m_impactBank;

	// All loaded sound events
	std::unordered_map<std::string, FMOD::Studio::EventDescription *> m_soundEvents;

	// All banks that have been loaded
	std::vector<std::pair<std::string, FMOD::Studio::Bank *>> m_loadedBanks;

	// FMOD studio and core system handles
	FMOD::Studio::System *m_studioSystem;
	FMOD::System *m_coreSystem;

	// Holds all the audio bus types, used for manipulating grouped sounds (like changing volume)
	FMOD::Studio::Bus *m_buses[AudioBusType::AudioBusType_NumOfTypes];
	FMOD::ChannelGroup *m_channelGroups[AudioBusType::AudioBusType_NumOfTypes];

	int m_numSoundDrivers;
};