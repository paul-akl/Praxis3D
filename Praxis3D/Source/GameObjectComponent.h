#pragma once

//#include "InheritanceObjects.h"
//#include "SpatialDataManager.h"
//#include "SceneLoader.h"
#include "System.h"

// A component containing data on an entity itself (i.e. a game object), like game object name, hierarchy (parent and children objects)
class GameObjectComponent : public SystemObject
{
	friend class WorldScene;
public:
	GameObjectComponent(SystemScene *p_systemScene, const std::string &p_name, EntityID p_entityID)
		: SystemObject(p_systemScene, p_name, Properties::SpatialComponent), m_entityID(p_entityID)
	{
		m_parent = 0;
	}
	~GameObjectComponent()
	{
		// Iterate over all component types and delete components if they have been created
		//for(std::size_t i = 0; i < ScriptComponentType::ScriptComponentType_NumOfComponents; i++)
		//	removeComponent(static_cast<ScriptComponentType>(i));
	}

	ErrorCode init() { return ErrorCode::Success; }

	void loadToMemory()
	{
		setActive(true);
	}

	// System type is World
	BitMask getSystemType() { return Systems::World; }

	void update(const float p_deltaTime)
	{

	}

	// Get the data change types that this object is interested in
	BitMask getDesiredSystemChanges() override { return Systems::Changes::Generic::Name; }

	// Get the data change types that this object might modify
	BitMask getPotentialSystemChanges() override { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		// Process the spatial changes and record the world-space changes
		//newChanges = m_spatialData.changeOccurred(*p_subject, p_changeType & Systems::Changes::Spatial::All);

		// If any data has been updated, post the changes to listeners
		if(newChanges != Systems::Changes::None)
		{
			//postChanges(newChanges);
		}
	}

	ErrorCode importObject(const PropertySet &p_properties)
	{
		ErrorCode returnError = ErrorCode::Success;

		// Check if the property set is valid
		if(p_properties)
		{
			// Load property data
			for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
			{
				switch(p_properties[i].getPropertyID())
				{

				case Properties::LocalPosition:
					//m_spatialData.setLocalPosition(p_properties[i].getVec3f());
					break;
				case Properties::LocalRotation:
					//m_spatialData.setLocalRotation(p_properties[i].getVec3f());
					break;
				case Properties::LocalRotationQuaternion:
					//m_spatialData.setLocalRotation(glm::quat(p_properties[i].getVec4f()));
					break;
				case Properties::LocalScale:
					//m_spatialData.setLocalScale(p_properties[i].getVec3f());
					break;
				}
			}
		}
		else
		{
			returnError = ErrorCode::Failure;
		}

		return returnError;
	}

	PropertySet exportObject()
	{
		return PropertySet();
	}

	inline void setParent(const EntityID p_parent) { m_parent = p_parent; }
	inline void addChild(const EntityID p_child) { m_children.push_back(p_child); }

	inline void clearChildren() { m_children.clear(); }
	inline std::vector<EntityID> &getChildren() { return m_children; }

private:
	EntityID m_entityID;

	// Parent and children hierarchy
	std::vector<EntityID> m_children;
	EntityID m_parent;
};

