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
		worldSpaceNeedsUpdate = false;
	}
	~SpatialDataManager()
	{

	}

	// Process spatial changes from the given subject and change type
	// Returns the changes that have been made; RETURNS ONLY WORLD-SPACE CHANGES!
	BitMask changeOccurred(const ObservedSubject &p_subject, const BitMask p_changeType)
	{
		BitMask newChanges = Systems::Changes::None;

		// Process the "All" changes first, as their bitmask is a combination of multiple changes, so it's faster to process the changes all at once, instead of individually
		// Checks are laid out in hierarchical sequence, starting with combinations of multiple changes and ending with each individual change
		if(CheckBitmask(p_changeType, Systems::Changes::Spatial::All))
		{
			// Update both the local-space and parent world-space data
			updateLocalSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllLocal));
			updateParentSpatialTransformData(p_subject.getSpatialTransformData(&m_parent, Systems::Changes::Spatial::AllWorld));
			newChanges |= Systems::Changes::Spatial::AllWorld;
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
				newChanges |= Systems::Changes::Spatial::AllWorld;
			}
			else
			{
				// Check if everything in world-space except the transform matrix, of the parent object, has changed
				if(CheckBitmask(p_changeType, Systems::Changes::Spatial::AllWorldNoTransform))
				{
					updateParentSpatialData(p_subject.getSpatialData(&m_parent, Systems::Changes::Spatial::AllWorldNoTransform));
					newChanges |= Systems::Changes::Spatial::AllWorldNoTransform;
				}
				else // If this is reached, check each individual piece of data has changed
				{
					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldPosition))
					{
						updateParentPosition(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::WorldPosition));
						newChanges |= Systems::Changes::Spatial::WorldPosition;
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldRotation))
					{
						updateParentRotation(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::WorldRotation));
						newChanges |= Systems::Changes::Spatial::WorldRotation;
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldScale))
					{
						updateParentScale(p_subject.getVec3(&m_parent, Systems::Changes::Spatial::WorldScale));
						newChanges |= Systems::Changes::Spatial::WorldScale;
					}

					if(CheckBitmask(p_changeType, Systems::Changes::Spatial::WorldTransform))
					{
						updateParentTransform(p_subject.getMat4(&m_parent, Systems::Changes::Spatial::WorldTransform));
						newChanges |= Systems::Changes::Spatial::WorldTransform;
					}
				}
			}
		}
		
		// Update the world-space data if any changes have been made
		if(worldSpaceNeedsUpdate)
		{
			updateWorldSpace();
			newChanges |= Systems::Changes::Spatial::WorldTransform;
			incrementUpdateCount();
		}

		return newChanges;
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

	// Returns the current update count; each time data is changed, update count is incremented
	const inline UpdateCount getUpdateCount() const { return m_updateCount; }

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
	}
	void updateLocalSpatialData(const SpatialData &p_spatialData)
	{
		m_localSpace.m_spatialData = p_spatialData;
		updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
	}
	void updateLocalTransform(const Math::Mat4f &p_transform)
	{
		m_localSpace.m_transformMat = p_transform;
		worldSpaceNeedsUpdate = true;
	}
	void updateLocalPosition(const Math::Vec3f &p_position)
	{
		m_localSpace.m_spatialData.m_position = p_position;
		updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
	}
	void updateLocalRotation(const Math::Vec3f &p_rotation)
	{
		m_localSpace.m_spatialData.m_rotationEuler = p_rotation;
		updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
	}
	void updateLocalScale(const Math::Vec3f &p_scale)
	{
		m_localSpace.m_spatialData.m_scale = p_scale;
		updateTransformMatrix(m_localSpace);
		worldSpaceNeedsUpdate = true;
	}

	// Functions of world spatial data of the parent object
	void updateParentSpatialTransformData(const SpatialTransformData &p_spatialTransformData)
	{
		m_parentSpace = p_spatialTransformData;
		worldSpaceNeedsUpdate = true;
	}
	void updateParentSpatialData(const SpatialData &p_spatialData)
	{
		m_parentSpace.m_spatialData = p_spatialData;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
	}
	void updateParentTransform(const Math::Mat4f &p_transform)
	{
		m_parentSpace.m_transformMat = p_transform;
		worldSpaceNeedsUpdate = true;
	}
	void updateParentPosition(const Math::Vec3f &p_position)
	{
		m_parentSpace.m_spatialData.m_position = p_position;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
	}
	void updateParentRotation(const Math::Vec3f &p_rotation)
	{
		m_parentSpace.m_spatialData.m_rotationEuler = p_rotation;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
	}
	void updateParentScale(const Math::Vec3f &p_scale)
	{
		m_parentSpace.m_spatialData.m_scale = p_scale;
		updateTransformMatrix(m_parentSpace);
		worldSpaceNeedsUpdate = true;
	}

	// Make up the world-space data from local and parent data
	void updateWorldSpace()
	{
		m_worldSpace = m_parentSpace + m_localSpace;
		worldSpaceNeedsUpdate = false;
	}

	// Increments the update count; should be called after any data has been changed
	inline void incrementUpdateCount() { m_updateCount++; }

	// A reference to the parent (which is the observer of all the received changes); required to retrieve the changed data from the observed subject
	const Observer &m_parent;

	// A flag to determine if the world space needs to be updated
	bool worldSpaceNeedsUpdate;

	// Transform data in local space
	SpatialTransformData m_localSpace;
	// Transform data in world space the parent object
	SpatialTransformData m_parentSpace;
	// Transform data in world space (local and parent space added together), final transform data used for rendering
	SpatialTransformData m_worldSpace;

	// Used for update tracking; each time data is change, update count is incremented
	UpdateCount m_updateCount;
};