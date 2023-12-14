#pragma once

#include "InheritanceObjects.h"
#include "SpatialDataManager.h"
#include "System.h"

// A component containing all spatial data (i.e. position, rotation, scale, etc.)
class SpatialComponent : public SystemObject
{
	friend class WorldScene;
public:
	struct SpatialComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		SpatialComponentConstructionInfo()
		{
			m_localScale = glm::vec3(1.0f);
		}

		glm::vec3 m_localPosition;
		glm::vec3 m_localRotationEuler;
		glm::quat m_localRotationQuaternion;
		glm::vec3 m_localScale;
	};

	SpatialComponent(SystemScene *p_systemScene, const std::string &p_name, EntityID p_entityID)
		: SystemObject(p_systemScene, p_name, Properties::SpatialComponent, p_entityID), m_spatialData(*this)
	{
	}
	~SpatialComponent()
	{
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
		// Update spatial data
		m_spatialData.update();

		// Get any changes of the spatial data
		BitMask newChanges = m_spatialData.getCurrentChangesAndReset();

		// If update is needed
		if(m_updateNeeded)
		{
			// Mark as updated
			m_updateNeeded = false;
		}

		// Post changes to listeners, if anything has changed
		if(newChanges != Systems::Changes::None)
			postChanges(newChanges);
	}

	// Get the data change types that this object is interested in
	BitMask getDesiredSystemChanges() override { return Systems::Changes::Spatial::All; }

	// Get the data change types that this object might modify
	BitMask getPotentialSystemChanges() override { return Systems::Changes::Spatial::All; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
		{
			// Get the active flag from the subject and set the active flag accordingly
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
		}

		// Process the spatial changes and record the world-space changes
		newChanges = m_spatialData.changeOccurred(*p_subject, p_changeType & Systems::Changes::Spatial::All);

		// Update spatial data
		m_spatialData.update();

		// Get any changes of the spatial data
		newChanges = m_spatialData.getCurrentChangesAndReset();

		// If any data has been updated, post the changes to listeners
		if(newChanges != Systems::Changes::None)
		{
			postChanges(newChanges);
		}
	}

	const SpatialDataManager &getSpatialDataChangeManager() const { return m_spatialData; }
	const glm::quat &getQuaternion(const Observer *p_observer, BitMask p_changedBits)						const override { return m_spatialData.getQuaternion(p_observer, p_changedBits); }
	const glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits)								const override { return m_spatialData.getVec3(p_observer, p_changedBits); }
	const glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits)								const override { return m_spatialData.getMat4(p_observer, p_changedBits); }
	const SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits)					const override { return m_spatialData.getSpatialData(p_observer, p_changedBits); }
	const SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits)	const override { return m_spatialData.getSpatialTransformData(p_observer, p_changedBits); }

private:
	SpatialDataManager m_spatialData;
};