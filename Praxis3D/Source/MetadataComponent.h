#pragma once

#include "CommonDefinitions.h"
#include "InheritanceObjects.h"

// A component containing data on an entity itself (i.e. a game object), like game object name, hierarchy (parent and children objects)
class MetadataComponent : public SystemObject
{
	friend class WorldScene;
public:

	MetadataComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::MetadataComponent, p_entityID)
	{
		m_parent = NULL_ENTITY_ID;
	}
	~MetadataComponent()
	{

	}
	ErrorCode init() final override
	{
		m_initialized = true;
		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		setActive(true);
	}

	void update(const float p_deltaTime)
	{

	}

	BitMask getSystemType() final override { return Systems::World; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::None; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }
	inline EntityID getParentEntityID() const { return m_parent; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{

	}

private:
	inline void setParent(const EntityID p_parentEntityID) { m_parent = p_parentEntityID; }

	EntityID m_parent;
};