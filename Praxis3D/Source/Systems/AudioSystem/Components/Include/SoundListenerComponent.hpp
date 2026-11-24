#pragma once

#include "Definitions/Include/InheritanceObjects.hpp"

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

	BitMask getSystemType() final override { return Systems::Audio; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::Audio::All; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }
	const inline int getListenerID() const { return m_listenerID; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));

		if(CheckBitmask(p_changeType, Systems::Changes::Audio::ListenerID))
			m_listenerID = p_subject->getInt(this, Systems::Changes::Audio::ListenerID);
	}

private:
	int m_listenerID;
};