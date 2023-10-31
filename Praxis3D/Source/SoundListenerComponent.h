#pragma once

#include "InheritanceObjects.h"

class SoundListenerComponent : public SystemObject
{
	friend class AudioScene;
public:

	struct SoundListenerComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		SoundListenerComponentConstructionInfo()
		{
			m_listenerID = 0;
		}

		int m_listenerID;
	};

	SoundListenerComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::SoundListenerComponent, p_entityID)
	{
		m_listenerID = 0;
	}
	~SoundListenerComponent()
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
	int m_listenerID;
};