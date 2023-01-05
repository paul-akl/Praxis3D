#pragma once

#include "Containers.h"
#include "ObserverBase.h"

// Manages GUI data updates
class GUIDataManager
{
public:
	GUIDataManager(const Observer &p_parent) : m_parent(&p_parent)
	{
		m_updateCount = 0;
		m_changes = Systems::Changes::None;
	}
	~GUIDataManager()
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
		if(CheckBitmask(p_changeType, Systems::Changes::GUI::Sequence))
		{
			// Update functors
			m_GUIData.m_functors = p_subject->getFunctors(m_parent, Systems::Changes::GUI::Sequence);
			m_changes |= Systems::Changes::GUI::Sequence;
		}

		return m_changes;
	}

	template<typename Functor>
	inline void addFunctor(Functor p_functor)
	{
		m_GUIData.m_functors.push_back(p_functor);
		m_changes |= Systems::Changes::GUI::Sequence;
	}

	inline void clearFunctors() { m_GUIData.m_functors.clear(); }

	inline const GUIData &getGUIData() { return m_GUIData; }

	const Functors &getFunctors(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::GUI::Sequence))
			return m_GUIData.m_functors;

		return NullObjects::NullFunctors;
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

private:
	// Increments the update count; should be called after any data has been changed
	inline void incrementUpdateCount() { m_updateCount++; }

	// GUI data
	GUIData m_GUIData;

	// A pointer to the parent (which is the observer of all the received changes); required to retrieve the changed data from the observed subject
	const Observer *m_parent;

	// Used for update tracking; each time data is change, update count is incremented
	UpdateCount m_updateCount;

	// Holds a bit-mask of all recent changes
	BitMask m_changes;
};