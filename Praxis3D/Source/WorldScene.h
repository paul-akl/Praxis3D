#pragma once

#include "GameObject.h"
#include "ObjectPool.h"
#include "ObjectRegister.h"
#include "System.h"
#include "WorldTask.h"

class WorldSystem;

class WorldScene : public SystemScene
{
public:
	WorldScene(SystemBase *p_system, SceneLoader *p_sceneLoader);

	ErrorCode init() { return ErrorCode::Success; }

	ErrorCode setup(const PropertySet &p_properties);

	void update(const float p_deltaTime);

	ErrorCode preload() { return ErrorCode::Success; }

	void loadInBackground() { }

	// Exports all the data of the scene (including all objects within) as a PropertySet (for example, used for saving to map file)
	PropertySet exportObject() { return PropertySet(); }

	SystemObject *createObject(const PropertySet &p_properties);
	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	SystemTask *getSystemTask() { return m_worldTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::World; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	struct GameObjectAndParent
	{
		GameObjectAndParent()
		{
			m_gameObject = nullptr;
			m_parent = 0;
		}		
		GameObjectAndParent(GameObject *p_gameObject, decltype(GameObject::m_GameObjectID) p_parent)
		{
			m_gameObject = p_gameObject;
			m_parent = p_parent;
		}

		GameObject *m_gameObject;
		decltype(GameObject::m_GameObjectID) m_parent;
	};
	struct GameObjectAndChildren
	{
		GameObjectAndChildren()
		{
			m_gameObject = nullptr;
		}		
		GameObjectAndChildren(GameObject *p_gameObject)
		{
			m_gameObject = p_gameObject;
		}

		GameObject *m_gameObject;
		std::vector<decltype(GameObject::m_GameObjectID)> m_children;
	};

	// Removes an object from a pool, by iterating checking each pool for matched index; returns true if the object was found and removed
	inline bool removeObjectFromPool(GameObject &p_object)
	{
		// Go over each game object
		for(decltype(m_gameObjects.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_gameObjects.getNumAllocated(),
			size = m_gameObjects.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
		{
			// Check if the game object is allocated inside the pool container
			if(m_gameObjects[i].allocated())
			{
				// Increment the number of allocated objects (early bail mechanism)
				numAllocObjecs++;

				// If the object matches with the one we are looking for, remove it from the game object pool
				if(*m_gameObjects[i].getObject() == p_object)
				{
					m_gameObjects.remove(m_gameObjects[i].getIndex());
					return true;
				}
			}
		}

		return false;
	}

	std::vector<GameObjectAndParent> m_unassignedParents;
	std::vector<GameObjectAndChildren> m_unassignedChildren;

	ObjectPool<GameObject> m_gameObjects;
	WorldTask *m_worldTask;

	ObjectRegisterConcurrent<GameObject*> m_objectRegister;
};
// UniqueID.h
// UIDRegister.h
// UniqueIDRegister.h