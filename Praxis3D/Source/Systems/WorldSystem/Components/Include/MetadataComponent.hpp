#pragma once

#include "Definitions/Include/CommonDefinitions.hpp"
#include "Definitions/Include/InheritanceObjects.hpp"

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
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::Generic::Name; }
	inline EntityID getParentEntityID() const { return m_parent; }
	const inline std::string getPrefabName() const { return m_prefab; }
	const inline bool getUsesPrefab() const { return !m_prefab.empty(); }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
		{
			// Get the active flag from the subject and set the active flag accordingly
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Name))
		{
			setName(p_subject->getString(this, Systems::Changes::Generic::Name));
		}

		if(CheckBitmask(p_changeType, Systems::Changes::World::PrefabName))
		{
			setPrefabName(p_subject->getString(this, Systems::Changes::World::PrefabName));
		}
	}

private:
	inline void setParent(const EntityID p_parentEntityID) { m_parent = p_parentEntityID; }
	inline void setPrefabName(const std::string p_prefabName) { m_prefab = p_prefabName; }

	EntityID m_parent;

	std::string m_prefab;
};