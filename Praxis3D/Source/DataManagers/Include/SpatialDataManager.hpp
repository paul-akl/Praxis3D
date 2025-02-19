#pragma once

#include <atomic>
#include <iostream>

#include "Definitions/Include/Containers.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"
#include "Communication/Include/ObserverBase.hpp"
#include "Systems/Base/Include/NullObjects.hpp"

// Manages spatial data updates. Contains:
// *Local-space data		- local data that is every object has individually 
// *Parent world-space data - world data of the parent object, stored separately from the world data of this objects, in case the local data is changed and new world data needs to be calculated
// *World-space data		- spatial data comprising of local and parent data, used for rendering
class SpatialDataManager
{
public:
	SpatialDataManager(const Observer &p_parent) : m_parent(p_parent)
	{
		m_updateCount = 0;
		m_trackLocalChanges = false;

		m_localEverythingUpToDate = true;
		m_localPositionUpToDate = true;
		m_localEulerUpToDate = true;
		m_localQuaternionUpToDate = true;
		m_localScaleUpToDate = true;
		m_localTransformUpToDate = true;
		m_worldTransformUpToDate = true;

		m_changes = Systems::Changes::None;
	}
	~SpatialDataManager()
	{

	}

	SpatialDataManager &operator=(const SpatialDataManager &p_spatialDataManager)
	{
		m_trackLocalChanges = p_spatialDataManager.m_trackLocalChanges;
		m_localEverythingUpToDate = p_spatialDataManager.m_localEverythingUpToDate;
		m_localPositionUpToDate = p_spatialDataManager.m_localPositionUpToDate;
		m_localEulerUpToDate = p_spatialDataManager.m_localEulerUpToDate;
		m_localQuaternionUpToDate = p_spatialDataManager.m_localQuaternionUpToDate;
		m_localScaleUpToDate = p_spatialDataManager.m_localScaleUpToDate;
		m_localTransformUpToDate = p_spatialDataManager.m_localTransformUpToDate;
		m_worldTransformUpToDate = p_spatialDataManager.m_worldTransformUpToDate;
		m_localSpace = p_spatialDataManager.m_localSpace;
		m_parentTransform = p_spatialDataManager.m_parentTransform;
		m_localTransformWithScale = p_spatialDataManager.m_localTransformWithScale;
		m_worldTransformWithScale = p_spatialDataManager.m_worldTransformWithScale;
		m_worldTransformNoScale = p_spatialDataManager.m_worldTransformNoScale;
		m_updateCount = p_spatialDataManager.m_updateCount;
		m_changes = p_spatialDataManager.m_changes;

		return *this;
	}

	void update()
	{	
		// Calculate local spatial variables if any of them are out-of-date
		if(!m_localEverythingUpToDate)
		{
			if(!m_localPositionUpToDate)
			{
				m_localSpace.m_spatialData.m_position = m_localSpace.m_transformMatNoScale[3];
				m_localPositionUpToDate = true;

				if(m_trackLocalChanges)
					m_changes |= Systems::Changes::Spatial::LocalPosition;
			}

			//if(!m_localEulerUpToDate)
			//{
			//	m_localSpace.m_spatialData.m_rotationEuler = glm::degrees(glm::eulerAngles(m_localSpace.m_spatialData.m_rotationQuat));
			//	m_localEulerUpToDate = true;
			//}

			if(!m_localQuaternionUpToDate)
			{
				if(m_localTransformUpToDate)
				{
					m_localSpace.m_spatialData.m_rotationQuat = glm::toQuat(m_localSpace.m_transformMatNoScale);
					const glm::mat3 rotMtx(
						glm::vec3(m_localSpace.m_transformMatNoScale[0]),
						glm::vec3(m_localSpace.m_transformMatNoScale[1]),
						glm::vec3(m_localSpace.m_transformMatNoScale[2]));
					m_localSpace.m_spatialData.m_rotationQuat = glm::quat_cast(rotMtx);
				}
				else
					m_localSpace.m_spatialData.m_rotationQuat = Math::eulerDegreesToQuaterion(m_localSpace.m_spatialData.m_rotationEuler);

				m_localQuaternionUpToDate = true;

				if(m_trackLocalChanges)
					m_changes |= Systems::Changes::Spatial::LocalRotation;
			}

			// Scale SHOULD NEVER need an update, but in case it somehow happens, update it anyway
			/*/ VERY computationally expensive!
			if(!m_localScaleUpToDate)
			{
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;

				glm::decompose(m_localSpace.m_transformMatNoScale, m_localSpace.m_spatialData.m_scale, rotation, translation, skew, perspective);
				m_localScaleUpToDate = true;

				if(m_trackLocalChanges)
					m_changes |= Systems::Changes::Spatial::LocalScale;
			}*/

			if(!m_localTransformUpToDate)
			{
				m_localSpace.m_transformMatNoScale = Math::createTransformMat(
					m_localSpace.m_spatialData.m_position, 
					m_localSpace.m_spatialData.m_rotationQuat);

				m_localTransformUpToDate = true;

				if(m_trackLocalChanges)
				{
					m_changes |= Systems::Changes::Spatial::LocalTransformNoScale;
				}
			}

			m_localEverythingUpToDate = true;
		}

		// Calculate the world transform if it's outdated
		if(!m_worldTransformUpToDate)
		{
			m_worldTransformNoScale = m_parentTransform * m_localSpace.m_transformMatNoScale;
			m_worldTransformWithScale = glm::scale(m_worldTransformNoScale, m_localSpace.m_spatialData.m_scale);

			m_worldTransformUpToDate = true;

			incrementUpdateCount();

			if(!m_trackLocalChanges)
				m_changes |= Systems::Changes::Spatial::WorldTransformNoScale;
		}
	}

	// Process spatial changes from the given subject and change type
	// Returns the changes that have been made; RETURNS ONLY WORLD-SPACE CHANGES BY DEFAULT!
	BitMask changeOccurred(const ObservedSubject &p_subject, const BitMask p_changeType)
	{
		bool localOrWorldTransformChanged = false;

		// Process the "All" changes first, as their bitmask is a combination of multiple changes, so it's faster to process the changes all at once, instead of individually
		// Checks are laid out in hierarchical sequence, starting with combinations of multiple changes and ending with each individual change
		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::All))
		{
			// Update both the local-space and parent world-space data
			setLocalSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllLocal));
			setWorldTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::WorldTransformNoScale));
			localOrWorldTransformChanged = true;
		}
		else // If the "All" flag wasn't set, check each local and world (parent) change
		{
			// Check if everything in local-space has changed
			if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllLocal))
			{
				setLocalSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllLocal));
				localOrWorldTransformChanged = true;
			}
			else
			{
				// Check if everything in local-space, except the transform matrix has changed
				if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllLocalNoTransform))
				{
					setLocalSpatialData(p_subject.getSpatialData(&m_parent, Systems::Changes::Spatial::AllLocalNoTransform));
				}
				else // If this is reached, check each individual piece of data has changed
				{
					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalPosition))
					{
						setLocalPosition(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::LocalPosition));
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalRotation))
					{
						setLocalRotation(p_subject.getQuaternion(&m_parent, Systems::Changes::Spatial::LocalRotation));
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalScale))
					{
						setLocalScale(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::LocalScale));
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransform))
					{
						setLocalTransformWithScale(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::LocalTransform));
						localOrWorldTransformChanged = true;
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransformNoScale))
					{
						//setLocalTransform(glm::scale(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::LocalTransform), m_localSpace.m_spatialData.m_scale));
						setLocalTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::LocalTransformNoScale));

						localOrWorldTransformChanged = true;
					}
				}
			}

			// Check if everything in world-space of the parent object has changed
			if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllWorld))
			{
				setWorldTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::WorldTransformNoScale));
				localOrWorldTransformChanged = true;
			}
			else
			{
				//if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldTransform))
				//{
				//	setParentTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::WorldTransform));
				//	localOrWorldTransformChanged = true;
				//}

				if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldTransformNoScale))
				{
					setParentTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::WorldTransformNoScale));
					localOrWorldTransformChanged = true;
				}
			}
		}
		
		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::Velocity))
			setVelocity(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::Velocity));

		// Update the world-space data if any changes have been made
		if(localOrWorldTransformChanged)
		{
			m_worldTransformNoScale = m_parentTransform * m_localSpace.m_transformMatNoScale;
			m_worldTransformWithScale = glm::scale(m_worldTransformNoScale, m_localSpace.m_spatialData.m_scale);

			m_worldTransformUpToDate = true;

			incrementUpdateCount();
			m_changes |= Systems::Changes::Spatial::WorldTransformNoScale;
		}

		return m_changes;
	}

	// Get local-space spatial data
	const inline SpatialTransformData &getLocalSpaceData() const { return m_localSpace; }
	inline SpatialTransformData &getLocalSpaceDataNonConst() { return m_localSpace; }

	// Get local-space transform
	const inline glm::mat4 &getLocalTransform() const { return m_localSpace.m_transformMatNoScale; }

	// Get world-space transform of the parent
	const inline glm::mat4 &getParentTransform() const { return m_parentTransform; }

	// Get world-space transform (combination of the local-space and the parent objects world-space)
	const inline glm::mat4 &getWorldTransform() const { return m_worldTransformNoScale; }

	// Get world-space transform with scale applied (combination of the local-space and the parent objects world-space)
	const inline glm::mat4 &getWorldTransformWithScale() const { return m_worldTransformWithScale; }

	// Get the current velocity
	const inline glm::vec3 &getVelocity() const { return m_velocity; }

	const inline glm::quat &getQuaternion(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Spatial::LocalRotation))
			return m_localSpace.m_spatialData.m_rotationQuat; 
		else
			return NullObjects::NullQuaterion;
	}
	const inline glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalPosition:
			return m_localSpace.m_spatialData.m_position;

		case Systems::Changes::Spatial::LocalRotation:
			return m_localSpace.m_spatialData.m_rotationEuler;

		case Systems::Changes::Spatial::LocalScale:
			return m_localSpace.m_spatialData.m_scale;

		case Systems::Changes::Spatial::Velocity:
			return m_velocity;
		}

		return NullObjects::NullVec3f;
	}
	const inline glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		// Apply scale to the local transform matrix
		case Systems::Changes::Spatial::LocalTransform:

			m_localTransformWithScale = glm::scale(m_localSpace.m_transformMatNoScale, m_localSpace.m_spatialData.m_scale);
			return m_localTransformWithScale;

		case Systems::Changes::Spatial::LocalTransformNoScale:
			return m_localSpace.m_transformMatNoScale;
		
		case Systems::Changes::Spatial::WorldTransform:
			return m_worldTransformNoScale;

		case Systems::Changes::Spatial::WorldTransformNoScale:
			return m_worldTransformNoScale;
		}

		return NullObjects::NullMat4f;
	}
	const inline SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Spatial::AllLocalNoTransform))
			return m_localSpace.m_spatialData;
		else
			return NullObjects::NullSpacialData;
	}
	const inline SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits) const 
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Spatial::AllLocal))
			return m_localSpace;
		else
			return NullObjects::NullSpacialTransformData;
	}

	// Returns the current update count; each time data is changed, update count is incremented
	const inline UpdateCount getUpdateCount() const { return m_updateCount; }

	// Returns all the changes made since the last reset of changes
	const inline BitMask getCurrentChanges() const { return m_changes; }

	// Returns all the changes made since the last reset of changes; and resets the changes
	const inline BitMask getCurrentChangesAndReset() 
	{ 
		auto returnChanges = m_changes;
		resetChanges();
		return returnChanges;
	}
	
	// Reset the tracking of changes
	inline void resetChanges() { m_changes = Systems::Changes::None; }

	// Should the local changes be tracked and returned when getting current changes
	inline void setTrackLocalChanges(const bool p_trackLocalChanges) { m_trackLocalChanges = p_trackLocalChanges; }

	// Set only the local, parent and world spatial data
	inline void setSpatialData(const SpatialDataManager &p_spatialDataManager)
	{
		m_localSpace = p_spatialDataManager.m_localSpace;
		m_parentTransform = p_spatialDataManager.m_parentTransform;
		m_worldTransformNoScale = p_spatialDataManager.m_worldTransformNoScale;

		m_localEverythingUpToDate = p_spatialDataManager.m_localEverythingUpToDate;
		m_localPositionUpToDate = p_spatialDataManager.m_localPositionUpToDate;
		m_localEulerUpToDate = p_spatialDataManager.m_localEulerUpToDate;
		m_localQuaternionUpToDate = p_spatialDataManager.m_localQuaternionUpToDate;
		m_localScaleUpToDate = p_spatialDataManager.m_localScaleUpToDate;
		m_localTransformUpToDate = p_spatialDataManager.m_localTransformUpToDate;
		m_worldTransformUpToDate = p_spatialDataManager.m_worldTransformUpToDate;
	}

	// Functions of local spatial data
	const inline void setLocalPosition(const glm::vec3 p_position)
	{
		m_localSpace.m_spatialData.m_position = p_position;

		// Updated variables
		m_localPositionUpToDate = true;

		m_localEverythingUpToDate = false;
		m_localTransformUpToDate = false;
		m_worldTransformUpToDate = false;

		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalPosition;
	}
	const inline void setLocalRotation(const glm::vec3 p_rotation)
	{
		m_localSpace.m_spatialData.m_rotationEuler = p_rotation; 
		m_localSpace.m_spatialData.m_rotationQuat = Math::eulerDegreesToQuaterion(m_localSpace.m_spatialData.m_rotationEuler);

		// Updated variables
		m_localEulerUpToDate = true;
		m_localQuaternionUpToDate = true;

		// Variables that became outdated
		m_localEverythingUpToDate = false;
		m_localTransformUpToDate = false;
		m_worldTransformUpToDate = false;

		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalRotation;
	}
	const inline void setLocalRotation(const glm::quat p_rotation)
	{
		m_localSpace.m_spatialData.m_rotationQuat = p_rotation;

		// Updated variables
		m_localQuaternionUpToDate = true;

		// Variables that became outdated
		m_localEverythingUpToDate = false;
		m_localEulerUpToDate = false;
		m_localTransformUpToDate = false;
		m_worldTransformUpToDate = false;

		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalRotation;
	}
	const inline void setLocalScale(const glm::vec3 p_scale)
	{
		m_localSpace.m_spatialData.m_scale = p_scale;

		// Updated variables
		m_localScaleUpToDate = true;

		// Variables that became outdated
		//m_localEverythingUpToDate = false;
		//m_localTransformUpToDate = false;
		m_worldTransformUpToDate = false;

		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalScale;
	}
	const inline void setLocalTransform(const glm::mat4 p_transform)
	{
		m_localSpace.m_transformMatNoScale = p_transform;

		// Updated variables
		m_localTransformUpToDate = true;

		// Variables that became outdated
		m_localEverythingUpToDate = false;
		m_worldTransformUpToDate = false;
		m_localPositionUpToDate = false;
		m_localEulerUpToDate = false;
		m_localQuaternionUpToDate = false;
		m_localScaleUpToDate = false;

		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalTransformNoScale;
	}
	const inline void setLocalTransformWithScale(const glm::mat4 p_transform)
	{
		m_localTransformWithScale = p_transform;

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(m_localTransformWithScale, scale, rotation, translation, skew, perspective);

		m_localSpace.m_spatialData.m_position = translation;
		m_localSpace.m_spatialData.m_rotationQuat = rotation;
		m_localSpace.m_spatialData.m_scale = scale;

		setLocalTransform(Math::createTransformMat(m_localSpace.m_spatialData.m_position, m_localSpace.m_spatialData.m_rotationQuat));
	}
	const inline void setLocalSpatialData(const SpatialData &p_spatialData)
	{
		m_localSpace.m_spatialData = p_spatialData;

		// Updated variables
		m_localPositionUpToDate = true;
		m_localEulerUpToDate = true;
		m_localQuaternionUpToDate = true;
		m_localScaleUpToDate = true;

		// Variables that became outdated
		m_localEverythingUpToDate = false;
		m_localTransformUpToDate = false;
		m_worldTransformUpToDate = false;

		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::AllLocalNoTransform;
	}
	const inline void setLocalSpatialTransformData(const SpatialTransformData &p_spatialTransformData)
	{
		m_localSpace = p_spatialTransformData;

		// Updated variables
		m_localEverythingUpToDate = true;
		m_localPositionUpToDate = true;
		m_localEulerUpToDate = true;
		m_localQuaternionUpToDate = true;
		m_localScaleUpToDate = true;
		m_localTransformUpToDate = true;

		// Variables that became outdated
		m_worldTransformUpToDate = false;

		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::AllLocal;
	}
	const inline void setVelocity(const glm::vec3 &p_velocity) { m_velocity = p_velocity; }

	// Set (world) transform of the parent object, that gets multiplied by the local transform to get world transform
	const inline void setParentTransform(const glm::mat4 &p_transform)
	{
		m_parentTransform = p_transform;

		m_worldTransformUpToDate = false;
	}

	// Set world transform, that should be the combination of local and parent transforms
	const inline void setWorldTransform(const glm::mat4 p_transform)		
	{ 
		m_worldTransformNoScale = p_transform;

		m_changes |= Systems::Changes::Spatial::WorldTransformNoScale;
	}

	//const inline void calculateLocalTransform() 
	//{ 
	//	//updateTransformMatrix(m_localSpace); 
	//}

	// Manually updates the local rotation Euler angles, as they are not automatically updated
	inline void calculateLocalRotationEuler()
	{
		if(!m_localQuaternionUpToDate)
		{
			if(m_localTransformUpToDate)
				m_localSpace.m_spatialData.m_rotationQuat = glm::toQuat(m_localSpace.m_transformMatNoScale);
			else
				m_localSpace.m_spatialData.m_rotationQuat = Math::eulerDegreesToQuaterion(m_localSpace.m_spatialData.m_rotationEuler);

			m_localQuaternionUpToDate = true;
		}

		m_localSpace.m_spatialData.m_rotationEuler = glm::degrees(glm::eulerAngles(m_localSpace.m_spatialData.m_rotationQuat));

		m_localEulerUpToDate = true;
	}

	// Manually updates the local rotation quaternion
	inline void calculateLocalRotationQuaternion()
	{
		if(m_localTransformUpToDate)
		{
			m_localSpace.m_spatialData.m_rotationQuat = glm::toQuat(m_localSpace.m_transformMatNoScale);
			const glm::mat3 rotMtx(
				glm::vec3(m_localSpace.m_transformMatNoScale[0]),
				glm::vec3(m_localSpace.m_transformMatNoScale[1]),
				glm::vec3(m_localSpace.m_transformMatNoScale[2]));
			m_localSpace.m_spatialData.m_rotationQuat = glm::quat_cast(rotMtx);
		}
		else
			m_localSpace.m_spatialData.m_rotationQuat = Math::eulerDegreesToQuaterion(m_localSpace.m_spatialData.m_rotationEuler);

		m_localQuaternionUpToDate = true;
	}

private:
	const inline void localDataChanged()
	{
		m_localTransformUpToDate = false;
		m_worldTransformUpToDate = false;
	}

	// Recalculates the model transform matrix in the world-space
	//void updateTransformMatrix(SpatialTransformData &p_spacialTransformData)
	//{
	//	p_spacialTransformData.m_transformMatNoScale = Math::createTransformMat(p_spacialTransformData.m_spatialData.m_position, 
	//																			p_spacialTransformData.m_spatialData.m_rotationEuler);
	//}

	// Increments the update count; should be called after any data has been changed
	inline void incrementUpdateCount() { m_updateCount++; }

	// A reference to the parent (which is the observer of all the received changes); required to retrieve the changed data from the observed subject
	const Observer &m_parent;

	// Should the local changes be tracked
	bool m_trackLocalChanges;

	// Flags to determine which local data is currently up to date
	// Certain local data becomes out of date when incomplete local data is changed
	bool m_localEverythingUpToDate;
	bool m_localPositionUpToDate;
	bool m_localEulerUpToDate;
	bool m_localQuaternionUpToDate;
	bool m_localScaleUpToDate;
	bool m_localTransformUpToDate;

	// Flag to determine if the world transform is up to date (combined local and parent transforms)
	bool m_worldTransformUpToDate;

	// Transform data in local space
	SpatialTransformData m_localSpace;
	// Transform data in world space the parent object
	glm::mat4 m_parentTransform;
	// Transform data in world space (local and parent space added together), with scale applied, final transform data used for rendering
	glm::mat4 m_worldTransformWithScale;
	// Transform data in world space (local and parent space added together)
	glm::mat4 m_worldTransformNoScale;
	// A local transform matrix constructed while scaling it
	mutable glm::mat4 m_localTransformWithScale;
	// Current velocity vector
	glm::vec3 m_velocity;

	// Used for update tracking; each time data is change, update count is incremented
	UpdateCount m_updateCount;

	// Holds a bit-mask of all recent changes
	BitMask m_changes;
};