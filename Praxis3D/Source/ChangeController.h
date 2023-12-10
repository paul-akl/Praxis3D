
#include <list>
#include <vector>

#include "ObserverBase.h"
#include "SpinWait.h"

#include <lmerr.h>
#include <tchar.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class TaskManager;

class ChangeController : public Observer
{
public:
	ChangeController();
	virtual ~ChangeController();

	ErrorCode registerSubject(ObservedSubject *p_subject, BitMask p_interestedBits, Observer *p_observer, BitMask p_observerBits = Systems::Types::All);
	ErrorCode unregisterSubject(ObservedSubject *p_subject, Observer *p_observer);

	ErrorCode removeSubject(ObservedSubject *p_subject);

	ErrorCode distributeChanges(BitMask p_systemsToNotify = Systems::Types::All, BitMask p_changesToDistribute = Systems::Changes::All);
	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType);

	// Sends a one-off notification about a change, without requiring the registration of subject-observer
	void oneTimeChange(ObservedSubject *p_subject, Observer *p_observer, BitMask p_changedBits);

	// Sends a one-off notification (without requiring the registration of subject-observer) that contains data in the message itself
	void oneTimeData(Observer *p_observer, const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving);

	ErrorCode setTaskManager(TaskManager *p_taskManager);

	void shutdown() { }

	void resetTaskManager();

private:
	class ObserverRequest
	{
	public:
		ObserverRequest(Observer *p_observer = nullptr, BitMask p_interestBits = 0, BitMask p_idBits = Systems::Changes::All) :
			m_observer(p_observer), m_interestBits(p_interestBits), m_observerIdBits(p_idBits) { }

		Observer	*m_observer;
		BitMask		m_interestBits;
		BitMask		m_observerIdBits;

		bool operator < (const ObserverRequest &p_observerRequest)	const { return m_observer < p_observerRequest.m_observer; }
		bool operator > (const ObserverRequest &p_observerRequest)	const { return m_observer > p_observerRequest.m_observer; }
		bool operator == (Observer *p_observer)						const { return m_observer == p_observer; }
	};

	struct SubjectInfo
	{
		SubjectInfo() : m_subject(nullptr), m_interestBits(0) { }

		ObservedSubject				*m_subject;
		BitMask						 m_interestBits;
		std::vector<ObserverRequest> m_observersList;
	};
	struct Notification
	{
		Notification(ObservedSubject *p_subject, BitMask p_changedBits) :
			m_subject(p_subject), m_changedBits(p_changedBits) { }

		ObservedSubject *m_subject;
		BitMask			m_changedBits;
	};
	struct OneTimeNotification
	{
		OneTimeNotification(ObservedSubject *p_subject, Observer *p_observer, BitMask p_changedBits) :
			m_subject(p_subject), m_observer(p_observer), m_changedBits(p_changedBits) { }

		ObservedSubject *m_subject;
		Observer		*m_observer;
		BitMask			m_changedBits;
	};	
	struct OneTimeData
	{
		OneTimeData(Observer *p_observer, DataType p_dataType, void *p_data, bool p_deleteAfterReceiving) :
			m_observer(p_observer), m_dataType(p_dataType), m_data(p_data), m_deleteAfterReceiving(p_deleteAfterReceiving) { }

		Observer *m_observer;
		DataType m_dataType;
		void *m_data;
		bool m_deleteAfterReceiving;
	};
	struct MappedNotification
	{
		MappedNotification(unsigned int p_ID, BitMask p_changedBits) : m_subjectID(p_ID), m_changedBits(p_changedBits) { }

		unsigned int m_subjectID;
		BitMask		 m_changedBits;
	};

	std::vector<unsigned int>						m_indexList;
	std::vector<unsigned int>						m_freeIDsList;
	std::vector<SubjectInfo>						m_subjectsList;
	std::vector<MappedNotification>					m_cumulativeNotifyList;
	std::list<std::vector<Notification>*>			m_notifyLists;
	std::list<std::vector<OneTimeNotification> *>	m_oneTimeNotifyLists;
	std::list<std::vector<OneTimeData>*>			m_oneTimeDataLists;

	// Last ID and handles for thread local storage data
	unsigned int m_lastID;
	unsigned int m_tlsNotifyList;
	unsigned int m_tlsOneTimeNotifyList;
	unsigned int m_tlsOneTimeDataList;

	BitMask m_systemsToNotify;
	BitMask m_changesToDistribute;

	SpinWait	m_spinWaitUpdate;
	TaskManager *m_taskManager;

	static void initThreadLocalData(void* p_controller);
	static void freeThreadLocalData(void* p_controller);

	static void distributionCallback(void *p_controller, unsigned int p_begin, unsigned int p_end);

	void distributeRange(unsigned int p_begin, unsigned int p_end);

	std::vector<Notification> *getNotifyList(unsigned int p_tlsIndex);
	std::vector<OneTimeNotification> *getOneTimeNotifyList(unsigned int p_tlsIndex);
	std::vector<OneTimeData> *getOneTimeDataList(unsigned int p_tlsIndex);

	//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
	std::string GetLastErrorAsString()
	{
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if(errorMessageID == 0)
			return std::string(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}
};
