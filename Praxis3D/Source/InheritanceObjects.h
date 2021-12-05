#pragma once

#include "CommonDefinitions.h"
#include "NullObjects.h"
#include "SpatialDataManager.h"

// Contains a pointer to a const SpatialDataManager
// Used for objects that do not have their own SpatialDataManager, but uses another objects SpatialDataManager to get spatial data
// Always has a valid pointer to SpatialDataManager (assigns a static empty SpatialDataManager upon construction) and only accepts valid references to SpatialDataManager (UNLESS THE DATA THAT THE POINTER IS ADDRESSING IS DELETED!)
class SpatialDataManagerObject
{
public:
	SpatialDataManagerObject() : m_spatialData(&NullObjects::NullSpatialDataManager)
	{
		m_updateCount = 0;
	}
	~SpatialDataManagerObject() { }

	// Assign a pointer to a const SpatialDataChangeManager, so the object can use it for its spatial data
	void setSpatialDataManagerReference(const SpatialDataManager &p_spatialData) { m_spatialData = &p_spatialData; }

	const SpatialDataManager &getSpatialDataManagerReference() { return *m_spatialData; }

	// Replaces the pointer to the SpatialDataChangeManager with an empty one, so that this object cannot use the spatial data from the current pointer anymore
	void removeSpatialDataManagerReference() { m_spatialData = &NullObjects::NullSpatialDataManager; }

protected:
	// Returns true if the spatial data has changed since the last check; resets the update counter, so the function will not return true until the spatial data changes again
	inline bool hasSpatialDataUpdated() 
	{ 
		auto spatialDataUpdateCount = m_spatialData->getUpdateCount();

		if(spatialDataUpdateCount != m_updateCount)
		{
			m_updateCount = spatialDataUpdateCount;
			return true;
		}
		else
		{
			return false;
		}
	}

	const SpatialDataManager *m_spatialData;

private:
	UpdateCount m_updateCount;
};

// Contains a pointer to a const SpatialData struct
// Used for objects that do not have their own SpatialData, but uses another objects SpatialData to get required data
// Always has a valid pointer to SpatialData (assigns a static empty SpatialData upon construction) and only accepts valid references to SpatialData (UNLESS THE DATA THAT THE POINTER IS ADDRESSING IS DELETED!)
class SpatialDataObject
{
public:
	SpatialDataObject() : m_spatialData(&NullObjects::NullSpacialData){ }
	~SpatialDataObject() { }

	// Assign a pointer to a const SpatialData, so the object can use it for its spatial data
	void setSpatialDataReference(const SpatialData &p_spatialData) { m_spatialData = &p_spatialData; }

	const SpatialData &getSpatialDataReference() { return *m_spatialData; }

	// Replaces the pointer to the SpatialData with an empty one, so that this object cannot use the spatial data from the current pointer anymore
	void removeSpatialDataReference() { m_spatialData = &NullObjects::NullSpacialData; }

protected:
	const SpatialData *m_spatialData;
};