#pragma once

#include "System.h"
#include "PhysicsTask.h"

class PhysicsSystem;

class PhysicsScene : public SystemScene
{
public:
	PhysicsScene(SystemBase *p_system, SceneLoader *p_sceneLoader);

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

	SystemTask *getSystemTask() { return m_physicsTask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::Physics; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	PhysicsTask *m_physicsTask;
};