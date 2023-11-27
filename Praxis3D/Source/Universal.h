#pragma once

#include <list>
#include <map>
#include <string>

#include "ChangeController.h"
#include "ObserverBase.h"
#include "SpinWait.h"
#include "System.h"

class UniversalObject;

class UniversalScene : public Observer
{
public:
	UniversalScene(ChangeController *p_sceneChangeController, ChangeController *p_objectChangeController);
	~UniversalScene();

	SystemScene *extend(SystemBase *p_system);
	ErrorCode extend(SystemScene *p_scene);
	ErrorCode unextend(SystemScene *p_scene);

	UniversalObject *createObject(std::string p_name = "");
	ErrorCode destroyObject(UniversalObject *p_object);

	UniversalObject *getObject(std::string p_name);
	
	// Creates a link between the subject and the observer, so that the observer can be notified of any data changes within the subject
	void createObjectLink(ObservedSubject *p_subject, SystemObject *p_observer);
	
	// Removes the link between the subject and the observer, so that the observer will no longer be notified of any data changes within the subject
	void removeObjectLink(ObservedSubject *p_subject, SystemObject *p_observer);

	// Queues an engine change that is processed before the next frame by the Engine
	//void sendEngineChange(EngineChangeData &p_engineChangeData)
	//{
	//	// Make sure calls from other threads are locked, while current call is in progress
	//	// This is needed so the changes list isn't being written to simultaneously from different threads
	//	SpinWait::Lock lock(m_mutex);

	//	m_engineChangeQueue.push_back(p_engineChangeData);
	//	m_engineChangePending = true;
	//}

	// Queues an engine change that is processed before the next frame by the Engine
	template<class... T_Args>
	void sendEngineChange(T_Args&&... p_args)
	{
		// Make sure calls from other threads are locked, while current call is in progress
		// This is needed so the changes list isn't being written to simultaneously from different threads
		SpinWait::Lock lock(m_mutex);

		m_engineChangeQueue.emplace_back(std::forward<T_Args>(p_args)...);
		m_engineChangePending = true;
	}

	// Sends a one-off notification about a change, without requiring the object linking
	void sendChange(ObservedSubject *p_subject, Observer *p_observer, BitMask p_changedBits)
	{
		// Check if any changes have been done
		//if(p_changedBits)
		m_objectChangeController->oneTimeChange(p_subject, p_observer, p_changedBits);
	}

	// Sends a one-off notification to a system object containing data, without requiring the object linking
	inline void sendData(SystemObject *p_observer, DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving = false) { m_objectChangeController->oneTimeData(p_observer, p_dataType, p_data, p_deleteAfterReceiving); }

	// Sends a one-off notification to a system scene containing data, without requiring the object linking
	inline void sendData(SystemScene *p_observer, DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving = false) { m_sceneChangeController->oneTimeData(p_observer, p_dataType, p_data, p_deleteAfterReceiving); }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changes);

	const inline std::list<UniversalObject*> &getObjects() const { return m_objects; }
	const inline std::map<BitMask, SystemScene*> &getSystemScenes() const { return m_systemScenes; }
	const inline std::list<EngineChangeData> &getEngineChangeQueue() const { return m_engineChangeQueue; }
	const inline bool getEngineChangePending() const { return m_engineChangePending; }

	void inline clearEngineChangeQueue()
	{
		m_engineChangeQueue.clear();
		m_engineChangePending = false;
	}

	typedef std::map<BitMask, SystemScene*> SystemSceneMap;
	typedef std::list<UniversalObject*>		UniversalObjectList;

	struct ObjectLinkData
	{
		Observer		*m_observer;
		ObservedSubject *m_subject;

		const std::string &m_observerName;
		const std::string &m_subjectName;
	};
	typedef std::list<ObjectLinkData> ObjectLinkList;

protected:
	ChangeController *m_sceneChangeController;
	ChangeController *m_objectChangeController;

	SystemSceneMap		m_systemScenes;
	ObjectLinkList		m_objectLinks;
	UniversalObjectList	m_objects;

	SpinWait m_mutex;

	bool m_engineChangePending;
	std::list<EngineChangeData> m_engineChangeQueue;

public:
	const ObjectLinkList &getObjectLinksList() { return m_objectLinks; }
};

class UniversalObject : public Observer, public ObservedSubject
{
	friend class UniversalScene;
protected:
	UniversalObject(UniversalScene *p_universalScene, std::string p_name = "");
	~UniversalObject();

public:
	std::string getName() { return m_name; }
	void setName(std::string p_name) { }

	SystemObject *extend(SystemScene *p_systemScene, PropertySet &p_properties);

	bool extend(SystemObject *p_systemObject);
	void unextend(SystemScene *p_systemScene);

	SystemObject *getExtension(BitMask p_system);

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changes);

	virtual BitMask getPotentialSystemChanges();

	typedef std::map <BitMask, SystemObject*> SystemObjectMap;

	const SystemObjectMap &getExtensions() { return m_objectExtensions; }

protected:
	std::string m_name;

	UniversalScene		*m_scene;
	ChangeController	*m_objectChangeController;
	SystemObjectMap		m_objectExtensions;
};