#include "AudioSystem.h"

ErrorCode AudioSystem::init()
{
	ErrorCode returnError = ErrorCode::Success;

	// Create the FMOD studio system
	auto fmodError = FMOD::Studio::System::create(&m_studioSystem);

	// Check if the sound system was created successfully
	if(fmodError != FMOD_OK)
	{
		ErrHandlerLoc::get().log(ErrorCode::Audio_system_init_failed, ErrorSource::Source_AudioScene);
		returnError = ErrorCode::Audio_system_init_failed;
	}
	else
	{
		// Get the FMOD core system
		m_studioSystem->getCoreSystem(&m_coreSystem);

		// Get the number of sound drivers
		m_coreSystem->getNumDrivers(&m_numSoundDrivers);

		// Do not continue if there are no sound drivers
		if(m_numSoundDrivers == 0)
		{
			ErrHandlerLoc::get().log(ErrorCode::Audio_no_drivers, ErrorSource::Source_AudioScene);
			returnError = ErrorCode::Audio_no_drivers;
		}
		else
		{
			// Initialize our Instance with 36 Channels
			m_studioSystem->initialize(Config::audioVar().num_audio_channels, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL | FMOD_INIT_VOL0_BECOMES_VIRTUAL | FMOD_INIT_3D_RIGHTHANDED, NULL);

			FMOD::Studio::Bank *defaultSoundBank = nullptr;

			// Load default sound bank
			if(fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + Config::audioVar().default_sound_bank).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &defaultSoundBank), Config::audioVar().default_sound_bank))
				if(fmodErrorLog(defaultSoundBank->loadSampleData(), Config::audioVar().default_sound_bank))
					addImpactSoundBank(defaultSoundBank);

			// Load default string bank
			if(fmodErrorLog(m_studioSystem->loadBankFile((Config::filepathVar().sound_path + Config::audioVar().default_sound_bank_string).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &defaultSoundBank), Config::audioVar().default_sound_bank_string))
				fmodErrorLog(defaultSoundBank->loadSampleData(), Config::audioVar().default_sound_bank_string);

			// Create channel groups
			for(unsigned int i = 0; i < AudioBusType::AudioBusType_NumOfTypes; i++)
			{
				switch(i)
				{
					case AudioBusType_Ambient:
						m_coreSystem->createChannelGroup(Config::audioVar().bus_name_ambient.c_str(), &m_channelGroups[i]);
						break;
					case AudioBusType_Master:
						m_coreSystem->createChannelGroup(Config::audioVar().channel_name_master.c_str(), &m_channelGroups[i]);
						break;
					case AudioBusType_Music:
						m_coreSystem->createChannelGroup(Config::audioVar().bus_name_music.c_str(), &m_channelGroups[i]);
						break;
					case AudioBusType_SFX:
						m_coreSystem->createChannelGroup(Config::audioVar().bus_name_sfx.c_str(), &m_channelGroups[i]);
						break;
				}
			}

			// Add all channel groups (other than master) to the master channel group as a nested group
			//for(unsigned int i = 0; i < AudioBusType::AudioBusType_NumOfTypes; i++)
			//	if(i != AudioBusType::AudioBusType_Master)
			//		m_channelGroups[AudioBusType::AudioBusType_Master]->addGroup(m_channelGroups[i]);
		}
	}

	if(returnError == ErrorCode::Success)
		ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_AudioSystem);

	return returnError;
}

ErrorCode AudioSystem::setup(const PropertySet &p_properties)
{
	ErrorCode returnCode = ErrorCode::Success;

	return returnCode;
}
