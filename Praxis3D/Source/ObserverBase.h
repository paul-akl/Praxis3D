#pragma once

// THREAD SAFETY NOTE
//
// Affects CSubject class implementation
//
// Currently there are no concurrent initial attach or detach operations on subject 
// objects. Thus corresponding locking operations (protecting the integrity of 
// the observers list) are disabled.
//
// It is not recommended to introduce concurrency of this sort, because it not only
// would result in additional performance impact to _all_ operations with subject
// (including frequent PostChanges), but also would introduce the risk of deadlocks
// (implementation of the Change Control Manager will have to be carefully revised).
// Most probably eliminating the deadlocks threat will further increase the overhead.
// One of the ways to avoid deadlock is posting change notifications (ChangeOccured)
// after the lock is released, but this would require ref counting on the IObserver
// interface.
// 
// Yet concurrent _repeated_ attaches (updating interest bits for already registered 
// CCM observer) are possible, so the protection against race conditions introduced
// by them must always be in place. 
#define ENABLE_CONCURRENT_SUBJECT_OPERATIONS 1

#include <list>

#include "Config.h"
#include "Containers.h"
#include "NullObjects.h"
#include "Math.h"
#include "SpinWait.h"

typedef unsigned int SystemObjectID;

class ObservedSubject;

class Observer
{
public:
	//virtual ~Observer() = 0;

	// This method gets called when data that we are interested changed in observed subject
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) = 0;
};

class ObservedSubject
{
public:
	ObservedSubject();
	virtual ~ObservedSubject();

	virtual ErrorCode attach(Observer *p_observer, BitMask p_interestedBits, SystemObjectID p_ID, unsigned int p_shiftBits = 0);
	virtual ErrorCode detach(Observer *p_observer);

	virtual ErrorCode updateInterestBits(Observer *p_observer, BitMask p_interestedBits);
	virtual SystemObjectID getID(Observer *p_observer) const;
	virtual BitMask getPotentialSystemChanges() = 0;

	virtual void postChanges(BitMask p_changedBits);
	virtual void preDestruct();

	const static SystemObjectID g_invalidID = static_cast<unsigned int>(-1);

	const virtual Math::Quaternion	&getQuaternion(const Observer *p_observer, BitMask p_changedBits)	const { return NullObjects::NullQuaterion;	}
	const virtual Math::Vec3f		&getVec3(const Observer *p_observer, BitMask p_changedBits)			const { return NullObjects::NullVec3f;		}
	const virtual Math::Vec4f		&getVec4(const Observer *p_observer, BitMask p_changedBits)			const { return NullObjects::NullVec4f;		}
	const virtual Math::Mat4f		&getMat4(const Observer *p_observer, BitMask p_changedBits)			const { return NullObjects::NullMat4f;		}

	const virtual bool					getBool(const Observer *p_observer, BitMask p_changedBits) const					{ return NullObjects::NullBool;					}
	const virtual int					getInt(const Observer *p_observer, BitMask p_changedBits) const						{ return NullObjects::NullInt;					}
	const virtual float					getFloat(const Observer *p_observer, BitMask p_changedBits) const					{ return NullObjects::NullFloat;				}
	const virtual double				&getDouble(const Observer *p_observer, BitMask p_changedBits) const					{ return NullObjects::NullDouble;				}
	const virtual std::string			&getString(const Observer *p_observer, BitMask p_changedBits) const					{ return NullObjects::NullString;				}
	const virtual SpatialData			&getSpatialData(const Observer *p_observer, BitMask p_changedBits) const			{ return NullObjects::NullSpacialData;			}
	const virtual SpatialTransformData	&getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits) const	{ return NullObjects::NullSpacialTransformData;	}

	// Stores observer information required for postChanges messages
	struct ObserverData
	{
		ObserverData(Observer *p_observer = nullptr, BitMask p_interestedBits = 0, SystemObjectID p_ID = 0)
			:	m_observer(p_observer),
				m_interestedBits(p_interestedBits),
				m_ID(p_ID) { }

		bool operator == (Observer *p_observer) const { return m_observer == p_observer; }

		Observer		*m_observer;
		SystemObjectID	m_ID;
		BitMask			m_interestedBits;
	};

private:
	inline BitMask getBitsToPost(ObservedSubject::ObserverData &p_observer, BitMask p_changedBits)
	{
		return (p_observer.m_interestedBits & p_changedBits);
	}

	typedef std::list<ObserverData> ObserverDataList;

	ObserverDataList m_observerList;

	#if ENABLE_CONCURRENT_SUBJECT_OPERATIONS
		SpinWait m_observerListMutex;
	#endif
};