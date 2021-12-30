#pragma once

#include <atomic>

#include "Containers.h"
#include "CommonDefinitions.h"
#include "ObserverBase.h"
#include "NullObjects.h"

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
		worldSpaceNeedsUpdate = false;
		localTransformNeedsUpdate = false;
		parentTransformNeedsUpdate = false;
		worldTransformNeedsUpdate = false;
	}
	~SpatialDataManager()
	{

	}

	SpatialDataManager &operator=(const SpatialDataManager &p_spatialDataManager)
	{
		m_trackLocalChanges = p_spatialDataManager.m_trackLocalChanges;
		worldSpaceNeedsUpdate = p_spatialDataManager.worldSpaceNeedsUpdate;
		localTransformNeedsUpdate = p_spatialDataManager.localTransformNeedsUpdate;
		parentTransformNeedsUpdate = p_spatialDataManager.parentTransformNeedsUpdate;
		worldTransformNeedsUpdate = p_spatialDataManager.worldTransformNeedsUpdate;
		m_localSpace = p_spatialDataManager.m_localSpace;
		m_parentSpace = p_spatialDataManager.m_parentSpace;
		m_worldSpace = p_spatialDataManager.m_worldSpace;
		m_updateCount = p_spatialDataManager.m_updateCount;
		m_changes = p_spatialDataManager.m_changes;

		return *this;
	}

	void update()
	{	
		// Calculate the needed transform matrices
		if(localTransformNeedsUpdate)
		{
			updateTransformMatrix(m_localSpace);
			localTransformNeedsUpdate = false;
			if(m_trackLocalChanges)
				m_changes |= Systems::Changes::Spatial::LocalTransform;
		}
		if(parentTransformNeedsUpdate)
		{
			updateTransformMatrix(m_parentSpace);
			parentTransformNeedsUpdate = false;
		}
		if(worldTransformNeedsUpdate)
		{
			updateTransformMatrix(m_worldSpace);
			worldTransformNeedsUpdate = false;
		}

		// Update the world-space data if any changes have been made
		if(worldSpaceNeedsUpdate)
		{
			updateWorldSpace();
			incrementUpdateCount();
			m_changes |= Systems::Changes::Spatial::WorldTransform;
		}
	}

	// Process spatial changes from the given subject and change type
	// Returns the changes that have been made; RETURNS ONLY WORLD-SPACE CHANGES BY DEFAULT!
	BitMask changeOccurred(const ObservedSubject &p_subject, const BitMask p_changeType)
	{
		// Process the "All" changes first, as their bitmask is a combination of multiple changes, so it's faster to process the changes all at once, instead of individually
		// Checks are laid out in hierarchical sequence, starting with combinations of multiple changes and ending with each individual change
		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::All))
		{
			// Update both the local-space and parent world-space data
			updateLocalSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllLocal));
			updateParentSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllWorld));
			m_changes |= Systems::Changes::Spatial::AllWorld;
		}
		else // If the "All" flag wasn't set, check each local and world (parent) change
		{
			// Check if everything in local-space has changed
			if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllLocal))
			{
				updateLocalSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllLocal));
			}
			else
			{
				// Check if everything in local-space, except the transform matrix has changed
				if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllLocalNoTransform))
				{
					updateLocalSpatialData(p_subject.getSpatialData(&m_parent, Systems::Changes::Spatial::AllLocalNoTransform));
				}
				else // If this is reached, check each individual piece of data has changed
				{
					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalPosition))
					{
						updateLocalPosition(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::LocalPosition));
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalRotation))
					{
						updateLocalRotation(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::LocalRotation));
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalScale))
					{
						updateLocalScale(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::LocalScale));
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::LocalTransform))
					{
						updateLocalTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::LocalTransform));
					}
				}
			}

			// Check if everything in world-space of the parent object has changed
			if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllWorld))
			{
				updateParentSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllWorld));
				m_changes |= Systems::Changes::Spatial::AllWorld;
			}
			else
			{
				// Check if everything in world-space except the transform matrix, of the parent object, has changed
				if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllWorldNoTransform))
				{
					updateParentSpatialData(p_subject.getSpatialData(&m_parent, Systems::Changes::Spatial::AllWorldNoTransform));
					m_changes |= Systems::Changes::Spatial::AllWorldNoTransform;
				}
				else // If this is reached, check each individual piece of data has changed
				{
					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldPosition))
					{
						updateParentPosition(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::WorldPosition));
						m_changes |= Systems::Changes::Spatial::WorldPosition;
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldRotation))
					{
						updateParentRotation(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::WorldRotation));
						m_changes |= Systems::Changes::Spatial::WorldRotation;
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldScale))
					{
						updateParentScale(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::WorldScale));
						m_changes |= Systems::Changes::Spatial::WorldScale;
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldTransform))
					{
						updateParentTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::WorldTransform));
						m_changes |= Systems::Changes::Spatial::WorldTransform;
					}
				}
			}
		}
		
		// Update the world-space data if any changes have been made
		if(worldSpaceNeedsUpdate)
		{
			updateWorldSpace();
			incrementUpdateCount();
			m_changes |= Systems::Changes::Spatial::WorldTransform;
		}

		return m_changes;
	}

	// Get local-space spatial data
	const inline SpatialTransformData &getLocalSpaceData() const { return m_localSpace; }

	// Get world-space spatial data (combination of the local-space and the parent objects world-space data)
	const inline SpatialTransformData &getWorldSpaceData() const { return m_worldSpace; }

	const inline Math::Quaternion &getQuaternion(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalRotation:
			return m_localSpace.m_spatialData.m_rotationQuat;

		case Systems::Changes::Spatial::WorldRotation:
			return m_worldSpace.m_spatialData.m_rotationQuat;
		}

		return NullObjects::NullQuaterion;
	}
	const inline Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalPosition:
			return m_localSpace.m_spatialData.m_position;

		case Systems::Changes::Spatial::LocalRotation:
			return m_localSpace.m_spatialData.m_rotationEuler;

		case Systems::Changes::Spatial::LocalScale:
			return m_localSpace.m_spatialData.m_scale;

		case Systems::Changes::Spatial::WorldPosition:
			return m_worldSpace.m_spatialData.m_position;

		case Systems::Changes::Spatial::WorldRotation:
			return m_worldSpace.m_spatialData.m_rotationEuler;

		case Systems::Changes::Spatial::WorldScale:
			return m_worldSpace.m_spatialData.m_scale;
		}

		return NullObjects::NullVec3f;
	}
	const inline Math::Mat4f &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalTransform:
			return m_localSpace.m_transformMat;

		case Systems::Changes::Spatial::WorldTransform:
			return m_worldSpace.m_transformMat;
		}

		return NullObjects::NullMat4f;
	}
	const inline SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits) const
	{ 
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::AllLocalNoTransform:
			return m_localSpace.m_spatialData;

		case Systems::Changes::Spatial::AllWorldNoTransform:
			return m_worldSpace.m_spatialData;
		}

		return NullObjects::NullSpacialData;
	}
	const inline SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits) const 
	{ 
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::AllLocal:
			return m_localSpace;

		case Systems::Changes::Spatial::AllWorld:
			return m_worldSpace;
		}

		return NullObjects::NullSpacialTransformData;
	}

	const inline Math::Vec3f &getWorldPosition() const { return m_worldSpace.m_spatialData.m_position; }
	const inline Math::Vec3f &getWorldRotation() const { return m_worldSpace.m_spatialData.m_rotationEuler; }

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
		m_parentSpace = p_spatialDataManager.m_parentSpace;
		m_worldSpace = p_spatialDataManager.m_worldSpace;
	}

	const inline void setLocalPosition(const Math::Vec3f p_position)		{ updateLocalPosition(p_position);		}
	const inline void setLocalRotation(const Math::Vec3f p_rotation)		{ updateLocalRotation(p_rotation);		}
	const inline void setLocalRotation(const Math::Quaternion p_rotation)	{ updateLocalRotation(p_rotation);		}
	const inline void setLocalScale(const Math::Vec3f p_scale)				{ updateLocalScale(p_scale);			}
	const inline void setLocalTransform(const Math::Mat4f p_transform)		{ updateLocalTransform(p_transform);	}

	const inline void setParentPosition(const Math::Vec3f p_position)		{ updateParentPosition(p_position);		}
	const inline void setParentRotation(const Math::Vec3f p_rotation)		{ updateParentRotation(p_rotation);		}
	const inline void setParentRotation(const Math::Quaternion p_rotation)	{ updateParentRotation(p_rotation);		}
	const inline void setParentScale(const Math::Vec3f p_scale)				{ updateParentScale(p_scale);			}
	const inline void setParentTransform(const Math::Mat4f p_transform)		{ updateParentTransform(p_transform);	}

	const inline void setWorldPosition(const Math::Vec3f p_position)		
	{ 
		m_worldSpace.m_spatialData.m_position = p_position; 
		worldTransformNeedsUpdate = true;
		m_changes |= Systems::Changes::Spatial::WorldPosition;
	}
	const inline void setWorldRotation(const Math::Vec3f p_rotation)		
	{ 
		m_worldSpace.m_spatialData.m_rotationEuler = p_rotation;
		worldTransformNeedsUpdate = true;
		m_changes |= Systems::Changes::Spatial::WorldRotation;
	}
	const inline void setWorldRotation(const Math::Quaternion p_rotation)	
	{ 
		m_worldSpace.m_spatialData.m_rotationQuat = p_rotation;
		worldTransformNeedsUpdate = true;
		m_changes |= Systems::Changes::Spatial::WorldRotation;
	}
	const inline void setWorldScale(const Math::Vec3f p_scale)				
	{ 
		m_worldSpace.m_spatialData.m_scale = p_scale;
		worldTransformNeedsUpdate = true;
		m_changes |= Systems::Changes::Spatial::WorldScale;
	}
	const inline void setWorldTransform(const Math::Mat4f p_transform)		
	{ 
		m_worldSpace.m_transformMat = p_transform;
		worldTransformNeedsUpdate = false;
		m_changes |= Systems::Changes::Spatial::WorldTransform;
	}

	const inline void calculateLocalTransform() 
	{ 
		updateTransformMatrix(m_localSpace); 
	}
	const inline void calculateWorldTransform() 
	{ 
		updateTransformMatrix(m_worldSpace);
		m_changes |= Systems::Changes::Spatial::WorldRotation;
	}

	// Make up the world-space data from local and parent data
	void updateWorldSpace()
	{
		m_worldSpace = m_parentSpace + m_localSpace;
		worldSpaceNeedsUpdate = false;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalTransform;
	}

private:
	// Recalculates the model transform matrix in the world-space
	void updateTransformMatrix(SpatialTransformData &p_spacialTransformData)
	{
		p_spacialTransformData.m_transformMat = Math::createTransformMat(	p_spacialTransformData.m_spatialData.m_position, 
																			p_spacialTransformData.m_spatialData.m_rotationEuler, 
																			p_spacialTransformData.m_spatialData.m_scale);
	}

	// Functions of local spatial data
	void updateLocalSpatialTransformData(const SpatialTransformData &p_spatialTransformData)
	{
		m_localSpace = p_spatialTransformData;
		worldSpaceNeedsUpdate = true;
		localTransformNeedsUpdate = false;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::AllLocal;
	}
	void updateLocalSpatialData(const SpatialData &p_spatialData)
	{
		m_localSpace.m_spatialData = p_spatialData;
		//updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
		localTransformNeedsUpdate = false;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::AllLocalNoTransform;
	}
	void updateLocalTransform(const Math::Mat4f &p_transform)
	{
		m_localSpace.m_transformMat = p_transform;
		worldSpaceNeedsUpdate = true;
		localTransformNeedsUpdate = false;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalTransform;
	}
	void updateLocalPosition(const Math::Vec3f &p_position)
	{
		m_localSpace.m_spatialData.m_position = p_position;
		//updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
		localTransformNeedsUpdate = true;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalPosition;
	}
	void updateLocalRotation(const Math::Vec3f &p_rotation)
	{
		m_localSpace.m_spatialData.m_rotationEuler = p_rotation;
		//updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
		localTransformNeedsUpdate = true;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalRotation;
	}	
	void updateLocalRotation(const Math::Quaternion &p_rotation)
	{
		m_localSpace.m_spatialData.m_rotationQuat = p_rotation;
		//updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
		localTransformNeedsUpdate = true;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalRotation;
	}
	void updateLocalScale(const Math::Vec3f &p_scale)
	{
		m_localSpace.m_spatialData.m_scale = p_scale;
		//updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
		localTransformNeedsUpdate = true;
		if(m_trackLocalChanges)
			m_changes |= Systems::Changes::Spatial::LocalScale;
	}

	// Functions of world spatial data of the parent object
	void updateParentSpatialTransformData(const SpatialTransformData &p_spatialTransformData)
	{
		m_parentSpace = p_spatialTransformData;
		worldSpaceNeedsUpdate = true;
		parentTransformNeedsUpdate = false;
	}
	void updateParentSpatialData(const SpatialData &p_spatialData)
	{
		m_parentSpace.m_spatialData = p_spatialData;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
		parentTransformNeedsUpdate = false;
	}
	void updateParentTransform(const Math::Mat4f &p_transform)
	{
		m_parentSpace.m_transformMat = p_transform;
		worldSpaceNeedsUpdate = true;
		parentTransformNeedsUpdate = false;
	}
	void updateParentPosition(const Math::Vec3f &p_position)
	{
		m_parentSpace.m_spatialData.m_position = p_position;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
		parentTransformNeedsUpdate = true;
	}
	void updateParentRotation(const Math::Vec3f &p_rotation)
	{
		m_parentSpace.m_spatialData.m_rotationEuler = p_rotation;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
		parentTransformNeedsUpdate = true;
	}
	void updateParentRotation(const Math::Quaternion &p_rotation)
	{
		m_parentSpace.m_spatialData.m_rotationQuat = p_rotation;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
		parentTransformNeedsUpdate = true;
	}
	void updateParentScale(const Math::Vec3f &p_scale)
	{
		m_parentSpace.m_spatialData.m_scale = p_scale;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
		parentTransformNeedsUpdate = true;
	}

	// Increments the update count; should be called after any data has been changed
	inline void incrementUpdateCount() { m_updateCount++; }

	// A reference to the parent (which is the observer of all the received changes); required to retrieve the changed data from the observed subject
	const Observer &m_parent;

	// Should the local changes be tracked
	bool m_trackLocalChanges;

	// A flag to determine if the world space needs to be updated
	bool worldSpaceNeedsUpdate;
	bool localTransformNeedsUpdate;
	bool parentTransformNeedsUpdate;
	bool worldTransformNeedsUpdate;

	// Transform data in local space
	SpatialTransformData m_localSpace;
	// Transform data in world space the parent object
	SpatialTransformData m_parentSpace;
	// Transform data in world space (local and parent space added together), final transform data used for rendering
	SpatialTransformData m_worldSpace;

	// Used for update tracking; each time data is change, update count is incremented
	UpdateCount m_updateCount;

	// Holds a bit-mask of all recent changes
	BitMask m_changes;
};