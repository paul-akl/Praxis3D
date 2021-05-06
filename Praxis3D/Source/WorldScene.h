#pragma once

#include "GameObject.h"
#include "ObjectPool.h"
#include "ObjectRegister.h"
#include "System.h"
#include "WorldTask.h"

class WorldScene : public SystemScene
{
public:
		
	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void update(const float p_deltaTime);

	ErrorCode preload();

	void loadInBackground();

	// Exports all the data of the scene (including all objects within) as a PropertySet (for example, used for saving to map file)
	PropertySet exportObject();

	SystemObject *createObject(const PropertySet &p_properties) override;
	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	SystemTask *getSystemTask() { return m_worldTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::World; };
	BitMask getDesiredSystemChanges();

private:
	struct GameObjectAndParent
	{
		GameObjectAndParent()
		{
			m_gameObject = nullptr;
			m_parent = 0;
		}		
		GameObjectAndParent(GameObject *p_gameObject, decltype(GameObject::m_id) p_parent)
		{
			m_gameObject = p_gameObject;
			m_parent = p_parent;
		}

		GameObject *m_gameObject;
		decltype(GameObject::m_id) m_parent;
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
		std::vector<decltype(GameObject::m_id)> m_children;
	};

	std::vector<GameObjectAndParent> m_unassignedParents;
	std::vector<GameObjectAndChildren> m_unassignedChildren;

	ObjectPool<GameObject> m_gameObjects;
	WorldTask *m_worldTask;

	ObjectRegisterConcurrent<GameObject*> m_objectRegister;
};
// UniqueID.h
// UIDRegister.h
// UniqueIDRegister.h