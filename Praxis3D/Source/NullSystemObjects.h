#pragma once

#include "System.h"

class NullSystemObject;
class NullSystemScene;
class NullSystemTask;

class NullSystemBase : public SystemBase
{
public:
	NullSystemBase() { }

	ErrorCode init() { return ErrorCode::Success; }

	ErrorCode setup(const PropertySet &p_properties) { return ErrorCode::Success; }

	//ErrorCode destroyScene(SystemScene *p_systemScene) { return ErrorCode::Success; }

	std::string getName() { return ""; }
	Systems::TypeID getSystemType() { return Systems::Null; }

	SystemScene *createScene(SceneLoader *p_sceneLoader);

	SystemScene *getScene();

	void loadInBackground() { }

private:
	static NullSystemScene m_nullSystemScene;
};

class NullSystemScene : public SystemScene
{
public:
	NullSystemScene(SystemBase *p_system);

	ErrorCode init() { return ErrorCode::Success; }

	ErrorCode setup(const PropertySet &p_properties) { return ErrorCode::Success; }

	void update(const float p_deltaTime) { }

	void loadInBackground() { }

	ErrorCode preload() { return ErrorCode::Success; }

	virtual Systems::TypeID getSystemType() { return Systems::Null; }

	//virtual std::string *getObjectTypes() { return new std::string(); }

	SystemObject *createObject(const PropertySet &p_properties);
	ErrorCode destroyObject(SystemObject *p_systemObject) { return ErrorCode::Success; }
	virtual SystemObject *getNullObject();

	SystemTask *getSystemTask();
	BitMask getDesiredSystemChanges() { return Systems::Changes::None; };
	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	BitMask getPotentialSystemChanges() { return 0; }

private:
	static NullSystemTask m_nullSystemTask;
	static NullSystemObject m_nullSystemObject;
};

class NullSystemObject : public SystemObject
{
public:
	NullSystemObject() { }

	ErrorCode init() { return ErrorCode::Success; }

	void loadToMemory() { }

	void update(const float p_deltaTime) { }

	BitMask getSystemType() { return 0; }
	BitMask getDesiredSystemChanges() { return 0; }
	BitMask getPotentialSystemChanges() { return 0; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	explicit operator bool() const { return true; }
};

class NullSystemTask : public SystemTask
{
public:
	NullSystemTask(SystemScene *p_systemScene) : SystemTask(p_systemScene) { }

	Systems::TypeID getSystemType() { return Systems::Null; }

	void update(const float p_deltaTime) { }

	bool isPrimaryThreadOnly() { return false; }

	void setSystemScene(SystemScene *p_systemScene) { m_systemScene = p_systemScene; }
};

// Global variable; null system base class with empty methods, 
// used instead of nullptr, to not halt the flow of the engine
static NullSystemBase g_nullSystemBase;