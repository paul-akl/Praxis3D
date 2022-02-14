#pragma once

#include "Containers.h"
#include "ObserverBase.h"

// Manages physics data updates
class PhysicsDataManager
{
public:
	PhysicsDataManager(const Observer &p_parent) : m_parent(p_parent)
	{
		m_updateCount = 0;
		m_changes = Systems::Changes::None;
	}
	~PhysicsDataManager()
	{

	}

	void update()
	{
		if(m_changes != Systems::Changes::None)
			incrementUpdateCount();
	}

	// Process GUI data changes from the given subject and change type
	// Returns the changes that have been made
	BitMask changeOccurred(const ObservedSubject *p_subject, const BitMask p_changeType)
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Type::Physics))
		{
			// Update functors
		}

		return m_changes;
	}

	inline const PhysicsData &getPhysicsData() { return m_physicsData; }

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

private:
	// Increments the update count; should be called after any data has been changed
	inline void incrementUpdateCount() { m_updateCount++; }

	// Physics data
	PhysicsData m_physicsData;

	// A reference to the parent (which is the observer of all the received changes); required to retrieve the changed data from the observed subject
	const Observer &m_parent;

	// Used for update tracking; each time data is change, update count is incremented
	UpdateCount m_updateCount;

	// Holds a bit-mask of all recent changes
	BitMask m_changes;
};