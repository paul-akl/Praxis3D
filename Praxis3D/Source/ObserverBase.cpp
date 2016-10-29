#include "ObserverBase.h"

const Math::Vec3f ObservedSubject::m_nullVec3 = Math::Vec3f(1.0f);
const Math::Vec4f ObservedSubject::m_nullVec4 = Math::Vec4f(1.0f);
const Math::Mat4f ObservedSubject::m_nullMat4 = Math::Mat4f();
const bool ObservedSubject::m_nullBool = false;
const int ObservedSubject::m_nullInt = 0;
const float ObservedSubject::m_nullFloat = 0.0f;
const double ObservedSubject::m_nullDouble = 0.0;
const std::string ObservedSubject::m_nullString;

ObservedSubject::ObservedSubject()
{

}
ObservedSubject::~ObservedSubject()
{
	preDestruct();
}

// Attach an observer for this subject
ErrorCode ObservedSubject::attach(Observer *p_observer, unsigned int p_interestedBits, unsigned int p_ID, unsigned int p_shiftBits)
{
	// If the concurency is enabled, lock the current subject and wait for it to be free
	#if ENABLE_CONCURRENT_SUBJECT_OPERATIONS
		SpinWait::Lock lock(m_observerListMutex);
	#endif

	// TODO REMOVE p_shiftBits
	// TODO ASSERT ERROR
	// ChangeController does not allow duplicates, so this error should never be called
	_ASSERT(std::find(m_observerList.begin(), m_observerList.end(), p_observer) == m_observerList.end());

	// Add the new observer. If we have reached this point, p_observer is valid
	m_observerList.push_back(ObserverData(p_observer, p_interestedBits, p_ID));

	return ErrorCode::Success;
}

// Remove the given observer from the internal observer list of this subject
ErrorCode ObservedSubject::detach(Observer *p_observer)
{
	ErrorCode returnError = ErrorCode::Failure;

	// If the concurency is enabled, lock the current subject and wait for it to be free
	#if ENABLE_CONCURRENT_SUBJECT_OPERATIONS 
		SpinWait::Lock lock(m_observerListMutex); 
	#endif

	// No need to check if p_observer is non-null, since the find() guarantees it will be valid
	ObserverDataList::iterator it = std::find(m_observerList.begin(), m_observerList.end(), p_observer);

	// If the observer was found
	if(it != m_observerList.end())
	{
		// Remove observer from the list, return success
		m_observerList.erase(it);
		returnError = ErrorCode::Success;
	}

	return returnError;
}

ErrorCode ObservedSubject::updateInterestBits(Observer *p_observer, unsigned int p_interestedBits)
{
	ErrorCode returnError = ErrorCode::Failure;

	// If the concurency is enabled, lock the current subject and wait for it to be free
	#if ENABLE_CONCURRENT_SUBJECT_OPERATIONS
		SpinWait::Lock lock(m_observerListMutex);
	#endif

	// Find the observer
	ObserverDataList::iterator it = std::find(m_observerList.begin(), m_observerList.end(), p_observer);

	// If the observer was found
	if(it != m_observerList.end())
	{
		#if ENABLE_CONCURRENT_SUBJECT_OPERATIONS
			it->m_interestedBits |= p_interestedBits;
		#else
			// Updates can happen concurrently, so usage of interlocked operation is needed
			long previousBits;
			long newBits = (long)(it->m_interestedBits | p_interestedBits);

			do
			{
				previousBits = it->m_interestedBits;
			} while(_InterlockedCompareExchange((long*)&it->m_interestedBits, newBits, previousBits) != previousBits);
		#endif

		returnError = ErrorCode::Success;
	}

	return returnError;
}
BitMask ObservedSubject::getID(Observer *p_observer) const
{
	// Iterate through all the observers
	for(ObserverDataList::const_iterator it = m_observerList.begin(); it != m_observerList.end(); it++)
	{
		// If we match the observer, return its ID
		if(it->m_observer == p_observer)
		{
			return it->m_ID;
		}
	}

	// If this point is reached, observer was not found, so return an invalid ID
	return g_invalidID;
}

// NOTE: If concurrent operations are enabled, the implementation of postChanges would need to be changed.
void ObservedSubject::postChanges(BitMask p_changedBits)
{
	// If the concurency is enabled, lock the current subject and wait for it to be free
	#if ENABLE_CONCURRENT_SUBJECT_OPERATIONS
		SpinWait::Lock lock(m_observerListMutex);
	#endif

	// Send the changes to all observers
	for(ObserverDataList::iterator it = m_observerList.begin(); it != m_observerList.end(); it++)
	{
		BitMask changedInterestedBits = getBitsToPost(*it, p_changedBits);

		// Check if there are any changes to the data we are interested in
		if(changedInterestedBits)
		{
			it->m_observer->changeOccurred(this, changedInterestedBits);
		}
	}
}

// Gets called from the destructor
void ObservedSubject::preDestruct()
{
	// "preDestruct" is only called from the destructor (hence never concurrently).
	// Thus if it ever gets called concurrently, locking must be added.
	for(ObserverDataList::iterator it = m_observerList.begin(); it != m_observerList.end(); it++)
	{
		it->m_observer->changeOccurred(this, 0);
	}

	m_observerList.clear();
}

namespace Interface
{
	inline BitMask getBitsToPost(ObservedSubject::ObserverData &p_observer, BitMask p_changedBits)
	{
		BitMask changedInterestedBits = p_observer.m_interestedBits & p_changedBits;

		return changedInterestedBits;
	}
}