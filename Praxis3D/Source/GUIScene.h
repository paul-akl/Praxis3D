#pragma once

#include "GUIObject.h"
#include "GUITask.h"
#include "ObjectPool.h"
#include "System.h"

class GUISystem;

class GUIScene : public SystemScene
{
public:
	GUIScene(SystemBase* p_system, SceneLoader* p_sceneLoader);
	~GUIScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet& p_properties);

	void update(const float p_deltaTime);

	ErrorCode preload();

	void loadInBackground();

	// Exports all the data of the scene (including all objects within) as a PropertySet (for example, used for saving to map file)
	PropertySet exportObject() { return PropertySet(); }

	SystemObject *createComponent(const EntityID &p_entityID, const std::string &p_entityName, const PropertySet &p_properties);
	SystemObject *createObject(const PropertySet& p_properties);
	ErrorCode destroyObject(SystemObject* p_systemObject);

	void changeOccurred(ObservedSubject* p_subject, BitMask p_changeType) { }

	SystemTask *getSystemTask() { return m_GUITask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::GUI; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:	
	// Removes an object from a pool, by iterating checking each pool for matched index; returns true if the object was found and removed
	inline bool removeObjectFromPool(GUIObject &p_object)
	{
		// Go over each GUI object
		for(decltype(m_GUIObjects.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_GUIObjects.getNumAllocated(),
			size = m_GUIObjects.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
		{
			// Check if the GUI object is allocated inside the pool container
			if(m_GUIObjects[i].allocated())
			{
				// Increment the number of allocated objects (early bail mechanism)
				numAllocObjecs++;

				// If the object matches with the one we are looking for, remove it from the GUI object pool
				if(*m_GUIObjects[i].getObject() == p_object)
				{
					m_GUIObjects.remove(m_GUIObjects[i].getIndex());
					return true;
				}
			}
		}

		return false;
	}

	GUITask *m_GUITask;

	// Object pools
	ObjectPool<GUIObject> m_GUIObjects;
};