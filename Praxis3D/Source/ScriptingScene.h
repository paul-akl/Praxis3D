#pragma once

#include "BaseScriptObject.h"
#include "CameraScript.h"
#include "DebugUIScript.h"
#include "DebugMoveScript.h"
#include "DebugRotateScript.h"
#include "NullSystemObjects.h"
#include "ScriptingTask.h"
#include "SolarTimeScript.h"
#include "System.h"
#include "WorldEditObject.h"

class ScriptingSystem;

class ScriptingScene : public SystemScene
{
public:
	ScriptingScene(ScriptingSystem *p_system, SceneLoader *p_sceneLoader);
	~ScriptingScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void update(const float p_deltaTime);

	void loadInBackground();

	// Exports all the data of the scene (including all objects within) as a PropertySet
	virtual PropertySet exportObject();
	
	ErrorCode preload();

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
	Systems::TypeID getSystemType() { return Systems::Scripting; }

private:
	FreeCamera *loadFreeCamera(const PropertySet &p_properties);
	DebugUIScript *loadDebugUI(const PropertySet &p_properties);
	DebugMoveScript *loadDebugMove(const PropertySet &p_properties);
	DebugRotateScript *loadDebugRotate(const PropertySet &p_properties);
	SolarTimeScript *loadSolarTime(const PropertySet &p_properties);
	WorldEditScript *loadWorldEdit(const PropertySet &p_properties);

	ScriptingTask *m_scriptingTask;

	std::vector<BaseScriptObject*> m_scriptObjects;
};