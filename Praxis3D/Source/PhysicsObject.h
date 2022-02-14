#pragma once

#include "InheritanceObjects.h"
#include "PhysicsDataManager.h"
#include "PhysicsMotionState.h"
#include "RigidBodyComponent.h"
#include "System.h"

enum PhysicsComponentType : std::size_t
{
	PhysicsComponentType_RigidBodyComponent = 0,
	PhysicsComponentType_SoftBodyComponent,
	PhysicsComponentType_NumOfComponents
};

class PhysicsObject : public SystemObject, public SpatialDataManagerObject, public LoadableGraphicsObject, public PhysicsMotionState
{
public:
	PhysicsObject(SystemScene *p_systemScene, const std::string &p_name)
		: SystemObject(p_systemScene, p_name, Properties::PhysicsObject), m_physicsData(*this)
	{
		m_componentsFlag = 0;
		m_rigidBodyComponent = nullptr;
	}
	~PhysicsObject()
	{
		// Iterate over all component types and delete components if they have been created
		//for(std::size_t i = 0; i < ScriptComponentType::ScriptComponentType_NumOfComponents; i++)
		//	removeComponent(static_cast<ScriptComponentType>(i));
	}

	ErrorCode init() 
	{

		return ErrorCode::Success; 
	}

	void loadToMemory()
	{

		setActive(true);
		setLoadedToMemory(true);
		setLoadedToVideoMemory(true);
	}

	ErrorCode importObject(const PropertySet &p_properties)
	{
		ErrorCode returnError = ErrorCode::Success;

		// Check if the property set is valid and the script object hasn't been loaded already
		if(p_properties)
		{
			if(!isLoadedToMemory())
			{
				//// Check if there is a property set for sequence and load the GUI sequence component if there is
				//auto const &sequenceComponentProperty = p_properties.getPropertySetByID(Properties::Sequence);
				//if(sequenceComponentProperty)
				//{
				//	// Create the GUI sequence component
				//	addComponent(new GUISequenceComponent(m_systemScene, m_name + Config::componentVar().lua_component_name));

				//	// Try to initialize the lua component
				//	auto componentInitError = m_GUISequenceComponent->init();
				//	if(componentInitError == ErrorCode::Success)
				//	{
				//		// Try to import the component
				//		auto const &componentImportError = m_GUISequenceComponent->importObject(sequenceComponentProperty);

				//		// Remove the component if it failed to import
				//		if(componentImportError != ErrorCode::Success)
				//		{
				//			removeComponent(GUIComponentType::GUIComponentType_Sequence);
				//			ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_GUISequenceComponent, m_name);
				//		}
				//	}
				//	else // Remove the component if it failed to initialize
				//	{
				//		removeComponent(GUIComponentType::GUIComponentType_Sequence);
				//		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_GUISequenceComponent, m_name);
				//	}
				//}
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

	void update(const float p_deltaTime)
	{
		BitMask newChanges = Systems::Changes::None;

		// If the motion state has been changed
		if(getMotionStateDirtyFlag())
		{
			// Reset the motion state dirty flag
			resetMotionStateDirtyFlag();

			// Update the motion state transforms
			updateMotionStateTrans();

			// Set the changes from the motion state
			newChanges |= Systems::Changes::Spatial::LocalTransform;
		}

		// Post changes if there are any
		if(newChanges != Systems::Changes::None)
			postChanges(newChanges);
	}

	// System type is Physics
	BitMask getSystemType() { return Systems::Physics; }

	BitMask getDesiredSystemChanges() { return Systems::Changes::Spatial::All | Systems::Changes::Physics::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::All; }

	const glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits)	const override { return getWorldTransform(); }

	inline RigidBodyComponent *getRigidBodyComponent() { return m_rigidBodyComponent; }

	inline const bool rigidBodyComponentPresent() const { return (m_rigidBodyComponent == nullptr) ? false : true; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		// Process the spatial changes
		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransform))
		{
			setWorldTransform(p_subject->getMat4(this, Systems::Changes::Spatial::LocalTransform));
		}

		// If the change type is GUI, send the change to the GUI Data Manager
		if(CheckBitmask(p_changeType, Systems::Changes::Type::Physics))
		{
			m_physicsData.changeOccurred(p_subject, p_changeType);
		}

		// If any data has been updated, post the changes to listeners
		if(newChanges != Systems::Changes::None)
		{
			postChanges(newChanges);
		}
	}

	void addComponent(RigidBodyComponent *p_component)
	{
		// Make sure that this component isn't assigned already
		removeComponent(PhysicsComponentType::PhysicsComponentType_RigidBodyComponent);
		m_rigidBodyComponent = p_component;

		// Set the flag for the Rigid Body component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::PhysicsObjectComponents::RigidBody;
	}

	void removeComponent(const PhysicsComponentType p_compType)
	{
		switch(p_compType)
		{
		case PhysicsComponentType::PhysicsComponentType_RigidBodyComponent:
		{
			if(rigidBodyComponentPresent())
			{
				// Delete the component
				delete m_rigidBodyComponent;

				// Assign the component pointer as nullptr to denote that it has been removed
				m_rigidBodyComponent = nullptr;

				// Remove the bit corresponding to Rigid Body component from the componentsFlag bitmask
				m_componentsFlag &= ~Systems::PhysicsObjectComponents::RigidBody;
			}
			break;
		}
		}
	}

	// Returns true if the physics object contains any components
	inline const bool containsComponents()
	{
		if(rigidBodyComponentPresent())
			return true;

		return false;
	}

private:
	// Components
	RigidBodyComponent *m_rigidBodyComponent;

	// Physics data
	PhysicsDataManager m_physicsData;

	// Stores a separate flag for each component currently present
	BitMask m_componentsFlag;
};

