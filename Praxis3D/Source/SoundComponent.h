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

	struct SoundComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		SoundComponentConstructionInfo()
		{
			m_soundType = SoundType::SoundType_Music;
			m_volume = 1.0f;
			m_loop = false;
			m_spatialized = false;
			m_startPlaying = false;
		}

		std::string m_soundFilename;
		SoundType m_soundType;
		float m_volume;
		bool m_loop,
			 m_spatialized,
			 m_startPlaying;
	};

	SoundComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::SoundComponent, p_entityID)
	{
		m_sound = nullptr;
		m_channel = nullptr;
		m_soundType = SoundType::SoundType_Null;
		m_volume = 1.0f;
		m_loop = false;
		m_spatialized = false;
		m_startPlaying = false;
		m_playing = false;

		m_soundExInfo = new FMOD_CREATESOUNDEXINFO();
		m_soundExInfo->cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	}
	~SoundComponent()
	{

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

	BitMask getSystemType() final override { return Systems::Audio; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::Audio::All; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{

	}

private:
	FMOD::Sound *m_sound;
	FMOD::Channel *m_channel;
	FMOD_CREATESOUNDEXINFO *m_soundExInfo;

	SoundType m_soundType;
	std::string m_soundFilename;
	float m_volume;
	bool m_loop,
		 m_spatialized,
		 m_startPlaying,
		 m_playing;
};