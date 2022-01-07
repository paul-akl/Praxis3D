#pragma once

#include "BaseScriptObject.h"
#include "CameraScript.h"
#include "DebugUIScript.h"
#include "DebugMoveScript.h"
#include "DebugRotateScript.h"
#include "NullSystemObjects.h"
#include "ScriptObject.h"
#include "ScriptTask.h"
#include "SolarTimeScript.h"
#include "System.h"
#include "SunScript.h"
#include "WorldEditObject.h"

class ScriptSystem;

class ScriptScene : public SystemScene
{
public:
	ScriptScene(ScriptSystem *p_system, SceneLoader *p_sceneLoader);
	~ScriptScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void update(const float p_deltaTime);

	void loadInBackground();

	ErrorCode preload();

	// Exports all the data of the scene (including all objects within) as a PropertySet
	virtual PropertySet exportObject();

	SystemObject *createObject(const PropertySet &p_properties);
	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType);

	BitMask getDesiredSystemChanges() { return 0; }
	BitMask getPotentialSystemChanges() { return 0; }

	// Getters
	SystemTask *getSystemTask()
	{ 
		if(m_scriptingTask != nullptr)
			return m_scriptingTask;

		return g_nullSystemBase.getScene()->getSystemTask();
	}
	Systems::TypeID getSystemType() { return Systems::Script; }

private:
	// Removes an object from a pool, by iterating checking each pool for matched index; returns true if the object was found and removed
	inline bool removeObjectFromPool(ScriptObject &p_object)
	{
		// Go over each graphics object
		for(decltype(m_scriptObjects.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_scriptObjects.getNumAllocated(),
			size = m_scriptObjects.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
		{
			// Check if the script object is allocated inside the pool container
			if(m_scriptObjects[i].allocated())
			{
				// Increment the number of allocated objects (early bail mechanism)
				numAllocObjecs++;

				// If the object matches with the one we are looking for, remove it from the script object pool
				if(*m_scriptObjects[i].getObject() == p_object)
				{
					m_scriptObjects.remove(m_scriptObjects[i].getIndex());
					return true;
				}
			}
		}

		return false;
	}

	FreeCamera *loadFreeCamera(const PropertySet &p_properties);
	DebugUIScript *loadDebugUI(const PropertySet &p_properties);
	DebugMoveScript *loadDebugMove(const PropertySet &p_properties);
	DebugRotateScript *loadDebugRotate(const PropertySet &p_properties);
	SolarTimeScript *loadSolarTime(const PropertySet &p_properties);
	SunScript *loadSun(const PropertySet &p_properties);
	WorldEditScript *loadWorldEdit(const PropertySet &p_properties);

	ScriptTask *m_scriptingTask;

	//std::vector<BaseScriptObject*> m_scriptObjects;

	// Object pools
	ObjectPool<ScriptObject> m_scriptObjects;
};