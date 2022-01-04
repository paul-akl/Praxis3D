#pragma once

#include "GUITask.h"
#include "System.h"

class GUISystem;

class GUIScene : public SystemScene
{
public:
	GUIScene(SystemBase* p_system, SceneLoader* p_sceneLoader);

	ErrorCode init();

	ErrorCode setup(const PropertySet& p_properties);

	void update(const float p_deltaTime);

	ErrorCode preload() { return ErrorCode::Success; }

	void loadInBackground() { }

	// Exports all the data of the scene (including all objects within) as a PropertySet (for example, used for saving to map file)
	PropertySet exportObject() { return PropertySet(); }

	SystemObject* createObject(const PropertySet& p_properties);
	ErrorCode destroyObject(SystemObject* p_systemObject);

	void changeOccurred(ObservedSubject* p_subject, BitMask p_changeType) { }

	SystemTask* getSystemTask() { return m_GUITask; };
	Systems::TypeID getSystemType() { return Systems::TypeID::GUI; };
	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::CreateObject || Systems::Changes::Generic::DeleteObject; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

private:
	GUITask *m_GUITask;
};