
#include "ChangeController.h"
#include "ErrorHandlerLocator.h"
#include "TaskManager.h"

ChangeController::ChangeController() : m_lastID(0), m_tlsNotifyList(TLS_OUT_OF_INDEXES), m_taskManager(nullptr)
{
	// Reserve space to avoid multiple reallocations
	m_cumulativeNotifyList.reserve((size_t)Config::engineVar().change_ctrl_cml_notify_list_reserv);
	m_subjectsList.reserve((size_t)Config::engineVar().change_ctrl_subject_list_reserv);
	m_subjectsList.resize(1);

	// Setup per thread local storage of notify and one-off-notify lists
	m_tlsNotifyList = ::TlsAlloc();
	m_tlsOneTimeNotifyList = ::TlsAlloc();

	//_ASSERT(m_tlsNotifyList != TLS_OUT_OF_INDEXES && "ChangeController: Not enough space in TLS");
	//_ASSERT(m_tlsOneOffNotifyList != TLS_OUT_OF_INDEXES && "ChangeController: Not enough space in TLS");

	// Reserve space and prepare notify lists for the main (this) thread
	std::vector<Notification> *list = new std::vector<Notification>;
	list->reserve((size_t)Config::engineVar().change_ctrl_notify_list_reserv);
	::TlsSetValue(m_tlsNotifyList, list);
	m_notifyLists.push_back(list);

	std::cout << "TlsSetValue: " << m_tlsNotifyList << ", " << list->size() << std::endl;

	// Reserve space and prepare one-off notify lists for the main (this) thread
	std::vector<OneTimeNotification> *oneOffList = new std::vector<OneTimeNotification>;
	oneOffList->reserve((size_t)Config::engineVar().change_ctrl_oneoff_notify_list_reserv);
	::TlsSetValue(m_tlsOneTimeNotifyList, oneOffList);
	m_oneTimeNotifyLists.push_back(oneOffList);

	std::cout << "TlsSetValue: " << m_tlsOneTimeNotifyList << ", " << oneOffList->size() << std::endl;
}
ChangeController::~ChangeController()
{
	// Clean up all the subjects and their observers
	// Iterate over all subjects
	for(decltype(m_subjectsList.size()) subjectIndex = 0; subjectIndex < m_subjectsList.size(); subjectIndex++)
	{
		// If subject is valid
		if(m_subjectsList[subjectIndex].m_subject)
		{
			// Iterate over all subject's observers
			for(decltype(m_subjectsList[subjectIndex].m_observersList.size()) observerIndex = 0;
				observerIndex < m_subjectsList[subjectIndex].m_observersList.size(); observerIndex++)
			{
				// Unregister the subject
				unregisterSubject(m_subjectsList[subjectIndex].m_subject, m_subjectsList[subjectIndex].m_observersList[observerIndex].m_observer);
			}
		}
	}

	// Free thread local storage
	if(m_tlsNotifyList != TLS_OUT_OF_INDEXES)
		::TlsFree(m_tlsNotifyList);

	if(m_tlsOneTimeNotifyList != TLS_OUT_OF_INDEXES)
		::TlsFree(m_tlsOneTimeNotifyList);

	// Free main thread (this) storage
	std::vector<Notification> *list = (std::vector<Notification>*)m_notifyLists.back();
	if(list != nullptr)
		delete list;

	std::vector<OneTimeNotification> *oneOffList = (std::vector<OneTimeNotification>*)m_oneTimeNotifyLists.back();
	if(oneOffList != nullptr)
		delete oneOffList;
}

ErrorCode ChangeController::registerSubject(ObservedSubject *p_subject, BitMask p_interestedBits, Observer *p_observer, BitMask p_observerBits)
{
	// Current return error is "failed" untill the registering has been completed
	ErrorCode returnError = ErrorCode::Failure;

	if(p_subject && p_observer)
	{
		// Stop the updates and lock, during the subject registration
		SpinWait::Lock lock(m_spinWaitUpdate);

		unsigned int ID = p_subject->getID(this);

		// If subject has already been registered
		if(ID != ObservedSubject::g_invalidID)
		{
			// Add a new observer
			SubjectInfo &subjectInfo = m_subjectsList[ID];
			subjectInfo.m_observersList.push_back(ObserverRequest(p_observer, p_interestedBits, p_observerBits));

			p_interestedBits &= ~subjectInfo.m_interestBits;

			if(p_interestedBits)
			{
				subjectInfo.m_interestBits |= p_interestedBits;
				p_subject->updateInterestBits(this, p_interestedBits);
			}
		}
		// If the subject is new
		else
		{
			if(m_freeIDsList.empty())
			{
				ID = ++m_lastID;
				// TODO ASSERT ERROR
				_ASSERT(ID == m_subjectsList.size());
				m_subjectsList.resize(ID + 1);
			}
			else
			{
				ID = m_freeIDsList.back();
				m_freeIDsList.pop_back();
			}

			SubjectInfo &subjectInfo = m_subjectsList[ID];
			subjectInfo.m_subject = p_subject;
			subjectInfo.m_observersList.push_back(ObserverRequest(p_observer, p_interestedBits, p_observerBits));
			subjectInfo.m_interestBits = p_interestedBits;

			p_subject->attach(this, p_interestedBits, ID);
		}

		returnError = ErrorCode::Success;
	}

	return returnError;
}
ErrorCode ChangeController::unregisterSubject(ObservedSubject *p_subject, Observer *p_observer)
{
	// Current return error is "failed" untill the unregistering has been completed
	ErrorCode returnError = ErrorCode::Failure;

	if(p_subject && p_observer)
	{
		// Stop the updates and lock, during the subject registration
		SpinWait::Lock lock(m_spinWaitUpdate);

		unsigned int ID = p_subject->getID(this);

		if(m_subjectsList.size() <= ID || m_subjectsList[ID].m_subject != p_subject)
		{
			return ErrorCode::Failure;
		}

		// TODO ASSERT ERROR
		_ASSERT(m_subjectsList[ID].m_subject == p_subject);

		std::vector<ObserverRequest> &observerList = m_subjectsList[ID].m_observersList;
		std::vector<ObserverRequest>::iterator observerListIterator = std::find(observerList.begin(), observerList.end(), p_observer);

		if(observerListIterator != observerList.end())
		{
			observerList.erase(observerListIterator);

			if(observerList.empty())
			{
				m_subjectsList[ID].m_subject = nullptr;
				m_freeIDsList.push_back(ID);
				p_subject->detach(this);
			}

			returnError = ErrorCode::Success;
		}
	}

	return returnError;
}

ErrorCode ChangeController::distributeChanges(BitMask p_systemsToNotify, BitMask p_changesToDistribute)
{
	// Store the parameters so they can be used by multiple threads
	m_systemsToNotify = p_systemsToNotify;
	m_changesToDistribute = p_changesToDistribute;

	// Loop through the notifications.
	// Some of them might generate more notifications, so it might need to loop through multiple times
	while(true)
	{
		// iterate over every thread-specific one-time notification list
		for(decltype(m_oneTimeNotifyLists)::iterator listIterator = m_oneTimeNotifyLists.begin(); listIterator != m_oneTimeNotifyLists.end(); listIterator++)
		{
			// Loop over ever notification in this list
			auto &currentList = **listIterator;
			for(decltype(currentList.size()) i = 0, size = currentList.size(); i < size; i++)
			{
				// Notify the observer about the change. We cannot check if the change is desired, since the
				// observer is not registered with the change controller in one-time changes.
				currentList[i].m_observer->changeOccurred(currentList[i].m_subject, currentList[i].m_changedBits);
			}

			// Clear out the list before moving to the next one
			currentList.clear();
		}

		// Make sure index list is big enough to contain all subjects
		m_indexList.resize(m_subjectsList.size());

		// Iterate over the every list (from thread local storage) and generate cumulative notify list
		for(decltype(m_notifyLists)::iterator listIterator = m_notifyLists.begin(); listIterator != m_notifyLists.end(); listIterator++)
		{
			auto *currentList = *listIterator;
			for(decltype(currentList->size()) i = 0, listSize = currentList->size(); i < listSize; i++)
			{
				// Get notification
				Notification &notification = currentList->at(i);

				// Get subject's ID
				unsigned int ID = notification.m_subject->getID(this);

				// TODO ASSERT ERROR
				_ASSERT(ID != ObservedSubject::g_invalidID);

				// If the ID is valid
				if(ID != ObservedSubject::g_invalidID)
				{
					// Get subject's index (flag)
					unsigned int index = m_indexList[ID];

					// If index flag is set, then subject is already in cumulative notify list
					if(index)
					{
						// A subject only needs to be notified once for all the changes
						// If it already added to the notify list, combine the changes
						m_cumulativeNotifyList[index].m_changedBits |= notification.m_changedBits;
					}
					else
					{
						// Set the subject's index flag
						m_indexList[ID] = (unsigned int)(m_cumulativeNotifyList.size());

						// Add the notification to the cumulative notify list
						m_cumulativeNotifyList.push_back(MappedNotification(ID, notification.m_changedBits));
					}
				}
			}

			//std::cout << currentList->size() << "; ";

			// Clear out the list before moving to the next one
			currentList->clear();
		}

		// Get the number of notifications we need to process
		std::size_t numberOfChanges = m_cumulativeNotifyList.size();

		// If there are no messages to process, exit the loop
		if(numberOfChanges == 0)
			break;

		// If there are more changes to distribute than grain size, do it in parallel
		if((unsigned int)(numberOfChanges > Config::engineVar().change_ctrl_grain_size && m_taskManager != nullptr))
		{
			// Process the notifications in a parallel loop
			m_taskManager->parallelFor(nullptr, distributionCallback, this, 0, (unsigned int)numberOfChanges, Config::engineVar().change_ctrl_grain_size);
		}
		else
		{
			// Not enough notifications to distribute them in parallel, so process them in serial
			distributeRange(0, (unsigned int)numberOfChanges);
		}

		if(m_changesToDistribute == Systems::Changes::All)
		{
			// Clear out all the lists after the distribution of the notifications
			m_cumulativeNotifyList.clear();
			m_indexList.clear();
		}
		else
		{
			// Some of the notifications may need to be distributed later,
			// in that case, do not clear the lists and just exit the loop
			break;
		}
	}

	return ErrorCode::Success;
}
void ChangeController::changeOccurred(ObservedSubject *p_subject, BitMask p_changedBits)
{
	// Current return error is "failed" until the task has been completed
	//ErrorCode returnError = ErrorCode::Failure;

	// TODO ASSERT ERROR
	_ASSERT(p_subject);

	// Check is subject is valid
	if(p_subject)
	{
		// If the subject is shutting down
		if(!p_changedBits)
		{
			ErrorCode error = removeSubject(p_subject);
			if(error != ErrorCode::Success)
				ErrHandlerLoc::get().log(error, ErrorSource::Source_Engine);
		}
		else
		{
			// Get thread local notification list
			auto *notifyList = getNotifyList(m_tlsNotifyList);

			//std::cout << "test ( ";

			//std::cout << m_tlsNotifyList << " ";

			if(notifyList != nullptr)
			{
				// Don't check for duplicates, for performance reasons
				notifyList->push_back(Notification(p_subject, p_changedBits));

				//std::cout << "size: " << notifyList->size() << " ";
			}
			else
			{
				std::cout << std::endl << GetLastErrorAsString() << std::endl;
				std::cout << "nullptr ";
			}
		}
	}

	//std::cout << ") test" << std::endl;
}

void ChangeController::oneTimeChange(ObservedSubject *p_subject, Observer *p_observer, BitMask p_changedBits)
{
	// TODO ASSERT ERROR
	_ASSERT(p_subject);
	_ASSERT(p_observer);

	// Check if both subject and observer are valid
	if(p_subject != nullptr && p_observer != nullptr)
	{
		// Get thread local notification list
		auto &oneTimeNotifyList = getOneTimeNotifyList(m_tlsOneTimeNotifyList);

		// Don't check for duplicates, for performance reasons
		oneTimeNotifyList.push_back(OneTimeNotification(p_subject, p_observer, p_changedBits));
	}
}

ErrorCode ChangeController::setTaskManager(TaskManager *p_taskManager)
{
	if(!p_taskManager)
		return ErrorCode::Undefined;

	if(m_taskManager != nullptr)
		resetTaskManager();

	// Set up pre-thread tsl notify list
	if(m_tlsNotifyList != TLS_OUT_OF_INDEXES)
	{
		m_taskManager = p_taskManager;

		// Make a callback to InitThreadLocalData from each thread
		m_taskManager->nonStandardPerThreadCallback(initThreadLocalData, this);

		return ErrorCode::Success;
	}

	return ErrorCode::Failure;
}

void ChangeController::resetTaskManager()
{
	// Free up data task manager data
	if(m_taskManager)
	{
		// Make a callback to FreeThreadLocalData from each thread
		m_taskManager->nonStandardPerThreadCallback(freeThreadLocalData, this);
		m_notifyLists.clear();
		m_taskManager = nullptr;

		// Restore main thread data
		std::vector<Notification> *notificationList = new std::vector<Notification>();
		::TlsSetValue(m_tlsNotifyList, notificationList);
		m_notifyLists.push_back(notificationList);
	}
}

void ChangeController::initThreadLocalData(void* p_controller)
{
	// TODO ERROR
	// ASSERT( arg && "ChangeManager: No manager pointer passed to InitThreadLocalNotifyList" );

	// Cast the passed controller to its type
	ChangeController *controller = (ChangeController*)p_controller;

	// Check if TLS handle is valid
	if(::TlsGetValue(controller->m_tlsNotifyList) == NULL)
	{
		// Create a new notify array
		std::vector<Notification> *notifyList = new std::vector<Notification>;

		// Reserve space in new array and set it as a tread local storage for current (this) thread
		notifyList->reserve((unsigned int)Config::engineVar().change_ctrl_notify_list_reserv);
		::TlsSetValue(controller->m_tlsNotifyList, notifyList);

		//std::cout << "TlsSetValue: " << controller->m_tlsNotifyList << ", " << notifyList->size() << std::endl;

		// Lock muted while adding the new array to the list
		SpinWait::Lock lock(controller->m_spinWaitUpdate);
		controller->m_notifyLists.push_back(notifyList);
	}

	// Check if TLS handle is valid
	if(::TlsGetValue(controller->m_tlsOneTimeNotifyList) == NULL)
	{
		// Create a new notify array
		std::vector<OneTimeNotification> *oneTimeNotifyList = new std::vector<OneTimeNotification>;

		// Reserve space in new array and set it as a tread local storage for current (this) thread
		oneTimeNotifyList->reserve((unsigned int)Config::engineVar().change_ctrl_oneoff_notify_list_reserv);
		::TlsSetValue(controller->m_tlsOneTimeNotifyList, oneTimeNotifyList);

		//sstd::cout << "TlsSetValue: " << controller->m_tlsOneTimeNotifyList << ", " << oneTimeNotifyList->size() << std::endl;

		// Lock muted while adding the new array to the list
		SpinWait::Lock lock(controller->m_spinWaitUpdate);
		controller->m_oneTimeNotifyLists.push_back(oneTimeNotifyList);
	}
}
void ChangeController::freeThreadLocalData(void* p_controller)
{
	// TODO ERROR
	// ASSERT(arg && "ChangeManager: No manager pointer passed to FreeThreadLocalNotifyList");

	// Cast the passed controller to its type
	ChangeController *controller = (ChangeController*)p_controller;

	// If the array in current thread local storage exists
	if(controller->m_tlsNotifyList != TLS_OUT_OF_INDEXES)
	{
		// Cast array to its type and delete it
		delete static_cast<std::vector<Notification>*>(::TlsGetValue(controller->m_tlsNotifyList));
		// Set the TLS handle to null
		::TlsSetValue(controller->m_tlsNotifyList, NULL);
	}

	// If the array in current thread local storage exists
	if(controller->m_tlsOneTimeNotifyList != TLS_OUT_OF_INDEXES)
	{
		// Cast array to its type and delete it
		delete static_cast<std::vector<OneTimeNotification>*>(::TlsGetValue(controller->m_tlsOneTimeNotifyList));
		// Set the TLS handle to null
		::TlsSetValue(controller->m_tlsOneTimeNotifyList, NULL);
	}

}

void ChangeController::distributionCallback(void *p_controller, unsigned int p_begin, unsigned int p_end)
{
	// Process the given range (this will be called from multiple threads)
	ChangeController *controller = (ChangeController*)p_controller;
	controller->distributeRange(p_begin, p_end);
}

void ChangeController::distributeRange(unsigned int p_begin, unsigned int p_end)
{
	// Loop through all the notification in the given range
	for(size_t i = p_begin; i < p_end; i++)
	{
		// Get the notification and the subject
		MappedNotification &notification = m_cumulativeNotifyList[i];
		SubjectInfo &subject = m_subjectsList[notification.m_subjectID];

		// Distribute any desired changes
		BitMask activeChanges = notification.m_changedBits & m_changesToDistribute;

		if(activeChanges)
		{
			// Clear the bit for the changes we are distributing
			notification.m_changedBits &= ~activeChanges;

			// Loop through all the observers and let them process the notification
			std::vector<ObserverRequest> &observerList = subject.m_observersList;
			for(size_t j = 0; j != observerList.size(); j++)
			{
				// Determine if this observer is interested in this notification
				BitMask changesToSend = observerList[j].m_interestBits & activeChanges;
				if(changesToSend)
				{
					// If this observer is part of the systems to be notified then we can pass it this notification
					if(observerList[j].m_observerIdBits & m_systemsToNotify)
					{
						// Have the observer process this change (notification)
						observerList[j].m_observer->changeOccurred(subject.m_subject, changesToSend);
					}
				}
			}
		}
	}
}

ErrorCode ChangeController::removeSubject(ObservedSubject *p_subject)
{
	// TODO ERRORS
	ErrorCode returnError = ErrorCode::Failure;

	std::vector<ObserverRequest> observerList;
	{
		SpinWait::Lock lock(m_spinWaitUpdate);

		unsigned int ID = p_subject->getID(this);
		_ASSERT(ID != unsigned int(-1));
		_ASSERT(m_subjectsList[ID].m_subject == p_subject);

		if(m_subjectsList.size() <= ID || m_subjectsList[ID].m_subject != p_subject)
		{
			return ErrorCode::Failure;
			// TODO ERROR FAILURE
		}
		observerList = m_subjectsList[ID].m_observersList;
		m_subjectsList[ID].m_subject = NULL;
		m_freeIDsList.push_back(ID);
		returnError = ErrorCode::Success;
		// TODO ERROR SUCCESS
	}

	std::vector<ObserverRequest>::iterator listIterator = observerList.begin();
	for(; listIterator != observerList.end(); listIterator++)
	{
		p_subject->detach(listIterator->m_observer);
	}

	return returnError;
}

inline std::vector<ChangeController::Notification> *ChangeController::getNotifyList(unsigned int p_tlsIndex)
{
	LPVOID test = ::TlsGetValue(p_tlsIndex);

	unsigned int test2 = (unsigned int)test;

	//std::cout << test2 << " ";

	if(test2 > 0)
		return static_cast<std::vector<ChangeController::Notification>*>(::TlsGetValue(p_tlsIndex));
	else
		return nullptr;
}
inline std::vector<ChangeController::OneTimeNotification> &ChangeController::getOneTimeNotifyList(unsigned int p_tlsIndex)
{
	return *static_cast<std::vector<ChangeController::OneTimeNotification>*>(::TlsGetValue(p_tlsIndex));
}
