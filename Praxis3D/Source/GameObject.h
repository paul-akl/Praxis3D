#pragma once

#include "Containers.h"
#include "GraphicsObject.h"
#include "ObjectRegister.h"
#include "SceneLoader.h"
#include "ScriptObject.h"
#include "SpatialDataManager.h"
#include "System.h"

// The main object type that glues all other system objects together, forming a component system.
// Responsible for linking objects of other systems with one another, so their data changes can be synchronized.
class GameObject : public SystemObject
{
	friend class WorldScene;
public:
	GameObject(SystemScene *p_systemScene, std::string p_name, SceneLoader &p_sceneLoader, ObjectID p_gmaeObjectID = 0) : SystemObject(p_systemScene, p_name, Properties::GameObject), m_sceneLoader(p_sceneLoader), m_GameObjectID(p_gmaeObjectID), m_spatialData(*this)
	{
		m_parent = nullptr;
		m_graphicsComponent = nullptr;
		m_scriptComponent = nullptr;
		m_componentsFlag = 0;
	}
	~GameObject()
	{
		// Remove all components
		for(int i = 0; i < Systems::TypeID::NumberOfSystems; i++)
		{
			removeComponent(static_cast<Systems::TypeID>(i));
		}
	}

	ErrorCode init() { return ErrorCode::Success; }

	BitMask getSystemType() override final { return Systems::World; }
	
	void update(const float p_deltaTime)
	{
		m_spatialData.update();
		BitMask newChanges = m_spatialData.getCurrentChangesAndReset();

		// If update is needed
		if(m_updateNeeded)
		{
		
			//.m_transformMat = Math::createTransformMat(m_localSpace.m_transformMat.getPosition(), m_localSpace.m_rotationEuler, m_localSpace.m_transformMat.getScale());
			//m_worldSpace.m_transformMat = Math::createTransformMat(m_worldSpace.m_transformMat.getPosition(), m_worldSpace.m_rotationEuler, m_worldSpace.m_transformMat.getScale());

			// Mark as updated
			m_updateNeeded = false;
		}

		if(newChanges != Systems::Changes::None)
			postChanges(newChanges);
	}

	// Get the data change types that this object is interested in
	BitMask getDesiredSystemChanges() override { return Systems::Changes::Spatial::All; }

	// Get the data change types that this object might modify
	BitMask getPotentialSystemChanges() override { return Systems::Changes::Spatial::All; }

	// Notify this object of the data that has been changed
	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) override 
	{
		//assert(p_subject == nullptr);

		// Process the spatial changes and record the world-space changes to be passed to the children objects
		BitMask newChanges = m_spatialData.changeOccurred(*p_subject, p_changeType);

		//if(CheckBitmask(p_changeType, Systems::Changes::Type::Spatial))
		//	newChanges |= m_spatialData.changeOccurred(*p_subject, p_changeType);
		
		//if(CheckBitmask(p_changeType, Systems::Changes::Type::Graphics) && CheckBitmask(m_componentsFlag, Systems::GameObjectComponents::Graphics))
		//	m_graphicsComponent->changeOccurred(p_subject, p_changeType);

		// Post the world-space changes
		if(newChanges != Systems::Changes::None)
			postChanges(newChanges);

		/*if(p_changeType & Systems::Changes::Spatial::LocalRotation)
		{
			const auto &newLocalRotation = p_subject->getVec3(this, Systems::Changes::Spatial::LocalRotation);

			// If the world rotation isn't being changed, adjust it accordingly, so it has the latest rotation
			if(!(p_changeType & Systems::Changes::Spatial::WorldRotation))
			{
				m_spatialData.m_worldRotationEuler -= m_spatialData.m_localRotationEuler;
				m_spatialData.m_worldRotationEuler += newLocalRotation;
				newChanges = newChanges | Systems::Changes::Spatial::WorldRotation;
			}

			m_spatialData.m_localRotationEuler = newLocalRotation;
		}

		if(p_changeType & Systems::Changes::Spatial::WorldRotation)
		{
			m_spatialData.m_worldRotationEuler = p_subject->getVec3(this, Systems::Changes::Spatial::WorldRotation) + m_spatialData.m_localRotationEuler;
		}

		if(p_changeType & Systems::Changes::Spatial::WorldPosition)
		{
			m_spatialData.m_worldMat.setPosition(p_subject->getVec3(this, Systems::Changes::Spatial::WorldPosition) + m_spatialData.m_localMat.getPosition());
		}

		if(p_changeType & Systems::Changes::Spatial::WorldScale)
		{
			m_spatialData.m_worldMat.setScale(p_subject->getVec3(this, Systems::Changes::Spatial::WorldScale) * m_spatialData.m_localMat.getScale());
		}

		if(p_changeType & Systems::Changes::Spatial::WorldModelMatrix)
		{
			m_spatialData.m_worldMat = p_subject->getMat4(this, Systems::Changes::Spatial::WorldModelMatrix);
		}*/
	}

	// Set the parent of this object. The object can have only one parent, thus if the parent was set already, it will be overridden
	void setParent(GameObject *p_parent) { m_parent = p_parent; }

	// Child functions
	void addChild(GameObject &p_child) 
	{ 
		// Add the child to the observer list
		m_sceneLoader.getChangeController()->createObjectLink(this, &p_child);

		// Add the child to the children array
		m_children.push_back(&p_child); 
	}
	void removeChild(const std::size_t p_id)
	{
		// Loop over every child
		for(decltype(m_children.size()) size = m_children.size(), i = 0; i < size; i++)
		{
			// Match the child ID
			if(m_children[i]->m_GameObjectID == p_id)
			{
				// Unlink the child from the observer list
				m_sceneLoader.getChangeController()->removeObjectLink(this, m_children[i]);

				// Erase the child if the IDs matched
				m_children.erase(m_children.begin() + i);
			}
		}
	}
	void removeChild(const GameObject &p_child) { removeChild(p_child.m_GameObjectID); }
	void clearChildren() { m_children.clear(); }
	const std::vector<GameObject*> &getChildren() const { return m_children; }

	// Component functions
	void addComponent(GraphicsObject *p_component)
	{ 
		// Remove the old component if it exists
		removeComponent(Systems::TypeID::Graphics);

		// Assign the new graphics component
		m_graphicsComponent = p_component;

		// Share the GameObjects spatial data with the component
		m_graphicsComponent->setSpatialDataManagerReference(m_spatialData);

		// Set the flag for the graphics component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::GameObjectComponents::Graphics;

		// Set the graphics component as an observer of this game object
		m_sceneLoader.getChangeController()->createObjectLink(this, m_graphicsComponent);
	}	
	void addComponent(ScriptObject *p_component)
	{
		// Remove the old component if it exists
		removeComponent(Systems::TypeID::Script);

		// Assign the new script component
		m_scriptComponent = p_component;

		// Share the GameObjects spatial data with the component
		m_scriptComponent->setSpatialDataManagerReference(m_spatialData);

		// Set the flag for the script component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::GameObjectComponents::Script;

		m_sceneLoader.getChangeController()->createObjectLink(m_scriptComponent->getLuaComponent(), this);

		// Set the script component as an observer of this game object
		m_sceneLoader.getChangeController()->createObjectLink(this, m_scriptComponent);
	}
	void removeComponent(Systems::TypeID p_componentType) 
	{
		switch(p_componentType)
		{
			case Systems::TypeID::Graphics:
			{
				// First check if the component exists
				if(m_graphicsComponent != nullptr)
				{
					unlinkComponent(m_graphicsComponent);

					// Stop sharing the spatial data with the component
					m_graphicsComponent->removeSpatialDataManagerReference();

					// Assign the component pointer as nullptr to denote that it has been removed
					m_graphicsComponent = nullptr;

					// Remove the bit corresponding to graphics component from the componentsFlag bitmask
					m_componentsFlag &= ~Systems::GameObjectComponents::Graphics;
				}
				break;
			}
			case Systems::TypeID::Script:
			{
				// First check if the component exists
				if(m_scriptComponent != nullptr)
				{
					unlinkComponent(m_scriptComponent);

					// Stop sharing the spatial data with the component
					m_scriptComponent->removeSpatialDataManagerReference();

					// Assign the component pointer as nullptr to denote that it has been removed
					m_scriptComponent = nullptr;

					// Remove the bit corresponding to script component from the componentsFlag bitmask
					m_componentsFlag &= ~Systems::GameObjectComponents::Script;
				}
				break;
			}
		}
	}

	const SpatialDataManager &getSpatialDataChangeManager() const { return m_spatialData; }
	const glm::quat &getQuaternion(const Observer *p_observer, BitMask p_changedBits)						const override { return m_spatialData.getQuaternion(p_observer, p_changedBits); }
	const glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits)								const override { return m_spatialData.getVec3(p_observer, p_changedBits); }
	const glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits)								const override { return m_spatialData.getMat4(p_observer, p_changedBits); }
	const SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits)					const override { return m_spatialData.getSpatialData(p_observer, p_changedBits); }
	const SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits)	const override { return m_spatialData.getSpatialTransformData(p_observer, p_changedBits); }

private:
	// Set the ID of the object. The ID must be unique
	void setGameObjectID(std::size_t p_id) { m_GameObjectID = p_id; }

	// Unlinks the given component, so that it is removed from observers/listeners
	void unlinkComponent(SystemObject *p_component)
	{
		// If the component exists
		if(p_component != nullptr)
		{
			// Remove the component from the observers
			m_sceneLoader.getChangeController()->removeObjectLink(this, p_component);
		}
	}

	SceneLoader &m_sceneLoader;

	// Parent and children hierarchy
	std::vector<GameObject*> m_children;
	GameObject *m_parent;

	//Components
	GraphicsObject *m_graphicsComponent;
	ScriptObject *m_scriptComponent;

	// Stores a separate flag for each component currently present
	BitMask m_componentsFlag;

	// Position data
	SpatialDataManager m_spatialData;

	// ID of a GameObject; separate from m_objectID of a SystemObject
	ObjectID m_GameObjectID;
};