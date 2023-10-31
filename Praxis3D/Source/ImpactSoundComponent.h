#pragma once

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <fmod/fmod_common.h>

#include "InheritanceObjects.h"

class ImpactSoundComponent : public SystemObject
{
	friend class AudioScene;
public:

	struct ImpactSoundComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		ImpactSoundComponentConstructionInfo()
		{

		}

	};

	ImpactSoundComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::SoundComponent, p_entityID)
	{

	}
	~ImpactSoundComponent()
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
	std::string m_soundName;
};