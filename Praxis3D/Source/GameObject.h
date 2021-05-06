#pragma once

#include "Containers.h"
#include "GraphicsObject.h"
#include "SceneLoader.h"
#include "System.h"

// The main object type that glues all other system objects together, forming a component system.
// Responsible for linking objects of other systems with one another, so their data changes can be synchronized.
class GameObject : public SystemObject
{
	friend class WorldScene;
public:
	GameObject(SystemScene *p_systemScene, std::string p_name, SceneLoader &p_sceneLoader, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::GameObject), m_sceneLoader(p_sceneLoader), m_id(p_id)
	{
		m_parent = nullptr;
		m_graphicsComponent = nullptr;
	}
	
	BitMask getSystemType() override final { return Systems::World; }
	
	void update(const float p_deltaTime)
	{
		// If update is needed
		if(m_updateNeeded)
		{
			
			//m_localSpace.m_transformMat = Math::createTransformMat(m_localSpace.m_transformMat.getPosition(), m_localSpace.m_rotationEuler, m_localSpace.m_transformMat.getScale());
			//m_worldSpace.m_transformMat = Math::createTransformMat(m_worldSpace.m_transformMat.getPosition(), m_worldSpace.m_rotationEuler, m_worldSpace.m_transformMat.getScale());

			// Mark as updated
			m_updateNeeded = false;
		}
	}

	// Get the data change types that this object is interested in
	BitMask getDesiredSystemChanges() override { return Systems::Changes::Spatial::All; }

	// Get the data change types that this object might modify
	BitMask getPotentialSystemChanges() override { return Systems::Changes::Spatial::All; }

	// Notify this object of the data that has been changed
	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) override 
	{
		BitMask newChanges = Systems::Changes::None;

		if(p_changeType & Systems::Changes::Spatial::Local)
		{
			m_localSpace = p_subject->getSpatialData(this, Systems::Changes::Spatial::Local);
			newChanges = newChanges | Systems::Changes::Spatial::Local;
		}

		if(p_changeType & Systems::Changes::Spatial::World)
		{
			m_worldSpace = p_subject->getSpatialData(this, Systems::Changes::Spatial::World);
			newChanges = newChanges | Systems::Changes::Spatial::World;
		}
		
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

	const Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const override final
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalPosition:
			return m_localSpace.m_position;
			break;
		case Systems::Changes::Spatial::LocalRotation:
			return m_localSpace.m_rotationEuler;
			break;
		case Systems::Changes::Spatial::LocalScale:
			return m_localSpace.m_scale;
			break;
		case Systems::Changes::Spatial::WorldPosition:
			return m_worldSpace.m_position;
			break;
		case Systems::Changes::Spatial::WorldRotation:
			return m_worldSpace.m_rotationEuler;
			break;
		case Systems::Changes::Spatial::WorldScale:
			return m_worldSpace.m_scale;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}
	const Math::Vec4f &getVec4(const Observer *p_observer, BitMask p_changedBits) const override final
	{ 
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalRotation:
			return m_localSpace.m_rotationQuat;
			break;
		case Systems::Changes::Spatial::WorldRotation:
			return m_worldSpace.m_rotationQuat;
			break;
		}

		return ObservedSubject::getVec4(p_observer, p_changedBits);
	}

	// Set the parent of this object. The object can have only one parent, thus if the parent was set already, it will be overridden
	void setParent(GameObject *p_parent) { m_parent = p_parent; }

	// Child functions
	void addChild(GameObject &p_child) 
	{ 
		// Add the child to the observer list
		m_sceneLoader.getChangeController()->createObjectLink(this, &p_child);

		// Add the child to the children array
		m_children.push_back(p_child); 
	}
	void removeChild(const std::size_t p_id)
	{
		// Loop over every child
		for(decltype(m_children.size()) size = m_children.size(), i = 0; i < size; i++)
		{
			// Match the child ID
			if(m_children[i].m_id == p_id)
			{
				// Unlink the child from the observer list
				m_sceneLoader.getChangeController()->removeObjectLink(this, &m_children[i]);

				// Erase the child if the IDs matched
				m_children.erase(m_children.begin() + i);
			}
		}
	}
	void removeChild(const GameObject &p_child) { removeChild(p_child.m_id); }
	void clearChildren() { m_children.clear(); }
	const std::vector<GameObject&> &getChildren() const { return m_children; }

	// Component functions
	void addComponent(GraphicsObject *p_graphicsComponent) { m_graphicsComponent = p_graphicsComponent; }
	void addComponent(SystemObject *p_component, Systems::TypeID p_componentType)
	{
		switch(p_componentType)
		{
			case Systems::TypeID::Graphics:
			{
				// Remove the old component if it existed
				removeComponent(p_componentType);

				// Assign the new graphics component
				m_graphicsComponent = p_component;

				// Set the graphics component as an observer of this game object
				m_sceneLoader.getChangeController()->createObjectLink(this, m_graphicsComponent);
				break;
			}
			case Systems::TypeID::Scripting:
			{
				
				break;
			}
		}
	}
	void removeComponent(Systems::TypeID p_componentType) 
	{
		switch(p_componentType)
		{
			case Systems::TypeID::Graphics:
			{
				removeComponent(m_graphicsComponent);
				break;
			}
			case Systems::TypeID::Scripting:
			{

				break;
			}
		}
	}

private:
	// Set the ID of the object. The ID must be unique
	void setID(std::size_t p_id) { m_id = p_id; }

	// Remove the given component
	void removeComponent(SystemObject *p_component)
	{
		// If the component exists
		if(p_component != nullptr)
		{
			// Remove the component from the observers
			m_sceneLoader.getChangeController()->removeObjectLink(this, p_component);

			// Assign the component pointer as nullptr to denote that it has been removed
			p_component = nullptr;
		}
	}

	SceneLoader &m_sceneLoader;

	// Parent and children hierarchy
	std::vector<GameObject&> m_children;
	GameObject *m_parent;

	//Components
	SystemObject *m_graphicsComponent;
	SystemObject *m_scriptingComponent;

	// Position data
	SpatialData m_localSpace,
				m_worldSpace;

	// ID of a GameObject; separate from m_objectID of a SystemObject
	std::size_t m_id;
};