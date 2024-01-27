#pragma once

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <fmod/fmod_common.h>

#include "InheritanceObjects.h"

class SoundComponent : public SystemObject
{
	friend class AudioScene;
public:
	enum SoundType : unsigned int
	{
		SoundType_Null = 0,
		SoundType_Music,
		SoundType_Ambient,
		SoundType_SoundEffect
	};
	constexpr static unsigned int SoundType_NumOfTypes = 4;

	enum SoundSourceType : unsigned int
	{
		SoundSourceType_File = 0,
		SoundSourceType_Event
	};
	constexpr static unsigned int SoundSourceType_NumOfTypes = 2;

	struct SoundComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		SoundComponentConstructionInfo()
		{
			m_soundType = SoundType::SoundType_SoundEffect;
			m_soundSourceType = SoundSourceType::SoundSourceType_File;
			m_volume = 1.0f;
			m_loop = false;
			m_spatialized = false;
			m_startPlaying = false;
		}

		// Either a filename or an event name
		std::string m_soundName;
		SoundType m_soundType;
		SoundSourceType m_soundSourceType;
		float m_volume;
		bool m_loop,
			 m_spatialized,
			 m_startPlaying;
	};

	SoundComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::SoundComponent, p_entityID)
	{
		m_sound = nullptr;
		m_channel = nullptr;
		m_soundEventDescription = nullptr;
		m_soundEventInstance = nullptr;
		m_soundType = SoundType::SoundType_Null;
		m_soundSourceType = SoundSourceType::SoundSourceType_File;
		m_volume = 1.0f;
		m_loop = false;
		m_spatialized = false;
		m_startPlaying = false;
		m_playing = false;

		m_soundExInfo = new FMOD_CREATESOUNDEXINFO();
		m_soundExInfo->cbsize = sizeof(FMOD_CREATESOUNDEXINFO);

		resetChanges();
	}
	~SoundComponent()
	{
		m_systemScene->destroyObject(this);
	}
	ErrorCode init() final override
	{
		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		setActive(true);
	}

	void update(const float p_deltaTime)
	{

	}

	void resetChanges()
	{
		m_changePending = false;
		m_loopChanged = false;
		m_reloadSound = false;
		m_spatializedChanged = false;
		m_volumeChanged = false;
	}

	BitMask getSystemType() final override { return Systems::Audio; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::Audio::All; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
		{
			// Get the active flag from the subject and set the active flag accordingly
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::SoundName))
		{
			m_soundName = p_subject->getString(this, Systems::Changes::Audio::SoundName);
			m_changePending = true;
			m_reloadSound = true;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::Loop))
		{
			m_loop = p_subject->getBool(this, Systems::Changes::Audio::Loop);
			m_changePending = true;
			m_loopChanged = true;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::Reload))
		{
			m_changePending = true;
			m_reloadSound = true;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::SoundType))
		{
			m_soundType = static_cast<SoundType>(p_subject->getUnsignedInt(this, Systems::Changes::Audio::SoundType));
			m_changePending = true;
			m_reloadSound = true;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::SoundSourceType))
		{
			m_soundSourceType = static_cast<SoundSourceType>(p_subject->getUnsignedInt(this, Systems::Changes::Audio::SoundSourceType));
			m_changePending = true;
			m_reloadSound = true;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::Spatialized))
		{
			m_spatialized = p_subject->getBool(this, Systems::Changes::Audio::Spatialized);
			m_changePending = true;
			m_spatializedChanged = true;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::StartPlaying))
		{
			m_startPlaying = p_subject->getBool(this, Systems::Changes::Audio::StartPlaying);
			m_changePending = true;
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::Volume))
		{
			m_volume = p_subject->getFloat(this, Systems::Changes::Audio::Volume);
			m_changePending = true;
			m_volumeChanged = true;
		}
	}

	const inline SoundType getSoundType() const { return m_soundType; }
	const inline SoundSourceType getSoundSourceType() const { return m_soundSourceType; }
	const inline std::string &getSoundName() const { return m_soundName; }
	const inline float getVolume() const { return m_volume; }
	const inline bool getLoop() const { return m_loop; }
	const inline bool getSpatialized() const { return m_spatialized; }
	const inline bool getStartPlaying() const { return m_startPlaying; }
	const inline bool getPlaying() const { return m_playing; }
	const inline std::vector<const char *> &getSoundTypeText() const { return m_soundTypeText; }
	const inline std::vector<const char *> &getSoundSourceTypeText() const { return m_soundSourceTypeText; }

private:
	FMOD::Sound *m_sound;
	FMOD::Channel *m_channel;
	FMOD_CREATESOUNDEXINFO *m_soundExInfo;

	FMOD::Studio::EventDescription *m_soundEventDescription; 
	FMOD::Studio::EventInstance *m_soundEventInstance;

	SoundType m_soundType;
	SoundSourceType m_soundSourceType;

	// Either a filename or an event name
	std::string m_soundName;

	float m_volume;
	bool m_loop,
		 m_spatialized,
		 m_startPlaying,
		 m_playing;

	bool	m_changePending,
			m_loopChanged,
			m_reloadSound,
			m_spatializedChanged,
			m_volumeChanged;

	std::vector<const char *> m_soundTypeText{ "null", "Music", "Ambient", "Sound effect" };
	std::vector<const char *> m_soundSourceTypeText{ "File", "Event" };
};