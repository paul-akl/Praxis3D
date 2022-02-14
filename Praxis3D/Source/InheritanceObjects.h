#pragma once

#include "CommonDefinitions.h"
#include "GraphicsDataSets.h"
#include "GUIDataManager.h"
#include "NullObjects.h"
#include "PhysicsDataManager.h"
#include "SpatialDataManager.h"

// Contains a const pointer to a SpatialDataManager
// Used for objects that do not have their own SpatialDataManager, but uses another objects SpatialDataManager to get spatial values
// Always has a valid pointer to SpatialDataManager (assigns a static empty SpatialDataManager upon construction) and only accepts valid references to SpatialDataManager (UNLESS THE DATA THAT THE POINTER IS ADDRESSING IS DELETED!)
// Intended to be used for components, so they can get specific data from their parent object, instead of the component having a copy of the data
class SpatialDataManagerObject
{
public:
	SpatialDataManagerObject() : m_spatialData(&NullObjects::NullSpatialDataManager)
	{
		m_updateCount = 0;
	}
	~SpatialDataManagerObject() { }

	// Assign a pointer to a const SpatialDataChangeManager, so the object can use it for its spatial data
	virtual void setSpatialDataManagerReference(const SpatialDataManager &p_spatialData) { m_spatialData = &p_spatialData; }

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

// Contains a const pointer to a SpatialData struct
// Used for objects that do not have their own SpatialData, but uses another objects SpatialData to get required values
// Always has a valid pointer to SpatialData (assigns a static empty SpatialData upon construction) and only accepts valid references to SpatialData (UNLESS THE DATA THAT THE POINTER IS ADDRESSING IS DELETED!)
// Intended to be used for components, so they can get specific data from their parent object, instead of the component having a copy of the data
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

// Contains a const pointer to a GUIDataManager
// Used for objects that do not have their own GUIDataManager, but uses another objects GUIDataManager to get required values
// Always has a valid pointer to GUIDataManager (assigns a static empty GUIDataManager upon construction) and only accepts valid references to GUIDataManager (UNLESS THE DATA THAT THE POINTER IS ADDRESSING IS DELETED!)
// Intended to be used for components, so they can get specific data from their parent object, instead of the component having a copy of the data
class GUIDataManagerObject
{
public:
	GUIDataManagerObject() : m_GUIData(&NullObjects::NullGUIDataManager)
	{
		m_updateCount = 0;
	}
	~GUIDataManagerObject() { }

	// Assign a pointer to a const GUIDataManager, so the object can use it for its GUI data
	virtual void setGUIDataManagerReference(const GUIDataManager &p_GUIData) { m_GUIData = &p_GUIData; }

	const GUIDataManager &getGUIDataManagerReference() { return *m_GUIData; }

	// Replaces the pointer to the GUIDataManager with an empty one, so that this object cannot use the GUI data from the current pointer anymore
	void removeGUIDataManagerReference() { m_GUIData = &NullObjects::NullGUIDataManager; }

protected:
	// Returns true if the GUIDataManager data has changed since the last check; resets the update counter, so the function will not return true until the GUI data changes again
	inline bool hasGUIDataUpdated()
	{
		auto GUIDataUpdateCount = m_GUIData->getUpdateCount();

		if(GUIDataUpdateCount != m_updateCount)
		{
			m_updateCount = GUIDataUpdateCount;
			return true;
		}
		else
		{
			return false;
		}
	}

	const GUIDataManager *m_GUIData;

private:
	UpdateCount m_updateCount;
};

// Contains a const pointer to a PhysicsDataManager
// Used for objects that do not have their own PhysicsDataManager, but uses another objects PhysicsDataManager to get required values
// Always has a valid pointer to PhysicsDataManager (assigns a static empty GUIDataManager upon construction) and only accepts valid references to PhysicsDataManager (UNLESS THE DATA THAT THE POINTER IS ADDRESSING IS DELETED!)
// Intended to be used for components, so they can get specific data from their parent object, instead of the component having a copy of the data
class PhysicsDataManagerObject
{
public:
	PhysicsDataManagerObject() : m_physicsData(&NullObjects::NullPhysicsDataManager)
	{
		m_updateCount = 0;
	}
	~PhysicsDataManagerObject() { }

	// Assign a pointer to a const PhysicsDataManager, so the object can use it for its physics data
	virtual void setPhysicsDataManagerReference(const PhysicsDataManager &p_physicsData) { m_physicsData = &p_physicsData; }

	const PhysicsDataManager &getPhysicsDataManagerReference() { return *m_physicsData; }

	// Replaces the pointer to the PhysicsDataManager with an empty one, so that this object cannot use the physics data from the current pointer anymore
	void removeGUIDataManagerReference() { m_physicsData = &NullObjects::NullPhysicsDataManager; }

protected:
	// Returns true if the GUIDataManager data has changed since the last check; resets the update counter, so the function will not return true until the GUI data changes again
	inline bool hasPhysicsDataUpdated()
	{
		auto physicsDataUpdateCount = m_physicsData->getUpdateCount();

		if(physicsDataUpdateCount != m_updateCount)
		{
			m_updateCount = physicsDataUpdateCount;
			return true;
		}
		else
		{
			return false;
		}
	}

	const PhysicsDataManager *m_physicsData;

private:
	UpdateCount m_updateCount;
};

// Contains flags denoting whether the object has been loaded to memory and video memory
// Sets the flags to false if the default constructor is called
class LoadableGraphicsObject
{
public:
	LoadableGraphicsObject()
	{
		m_loadedToMemory = false;
		m_loadedToVideoMemory = false;
	}	
	LoadableGraphicsObject(bool p_loadedToMemory, bool p_loadedToVideoMemory)
	{
		m_loadedToMemory = p_loadedToMemory;
		m_loadedToVideoMemory = p_loadedToVideoMemory;
	}
	~LoadableGraphicsObject() { }

	virtual std::vector<LoadableObjectsContainer> getLoadableObjects() { return std::vector<LoadableObjectsContainer>(); }

	virtual void performCheckIsLoadedToMemory() { }
	virtual void performCheckIsLoadedToVideoMemory() { }

	inline const bool isLoadedToMemory()		const { return m_loadedToMemory;		}
	inline const bool isLoadedToVideoMemory()	const { return m_loadedToVideoMemory;	}

	inline void setLoadedToMemory(bool p_loaded)		{ m_loadedToMemory = p_loaded;		}
	inline void setLoadedToVideoMemory(bool p_loaded)	{ m_loadedToVideoMemory = p_loaded; }

protected:
	bool	m_loadedToMemory,
			m_loadedToVideoMemory;
};