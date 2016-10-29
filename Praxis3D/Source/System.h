#pragma once

#include <string>

#include "Config.h"
#include "ObserverBase.h"
#include "PropertySet.h"

class SceneLoader;
class SystemObject;
class SystemScene;
class SystemTask;

class SystemBase
{
public:
	SystemBase() : m_initialized(false) { }
	//~SystemBase();

	virtual ErrorCode init() = 0;
	//virtual ErrorCode destroyScene(SystemScene *p_systemScene) = 0;

	virtual ErrorCode setup(const PropertySet &p_properties) = 0;

	virtual Systems::TypeID getSystemType() = 0;
	virtual std::string getName() = 0;

	// Passes scene loader as argument so that scenes could have access to other system scenes 
	// that are register with this scene loader
	virtual SystemScene *createScene(SceneLoader *p_sceneLoader) = 0;

	virtual SystemScene *getScene() = 0;

	virtual void loadInBackground() = 0;
	
protected:
	bool m_initialized;
};

class SystemScene : public ObservedSubject, public Observer
{
	friend SystemBase;
public:
	SystemScene(SystemBase *p_system, SceneLoader *p_sceneLoader)
		: m_initialized(false), m_system(p_system), m_sceneLoader(p_sceneLoader){ }
	//~SystemScene();

	SystemBase *getSystem() { return m_system; }

	virtual ErrorCode init() = 0;

	virtual ErrorCode setup(const PropertySet &p_properties) = 0;

	virtual void update(const float p_deltaTime) = 0;

	virtual ErrorCode preload() = 0;

	virtual void loadInBackground() = 0;

	// Exports all the data of the scene (including all objects within) as a PropertySet (for example, used for saving to map file)
	virtual PropertySet exportObject() { return PropertySet(Properties::Null); }

	virtual SystemObject *createObject(const PropertySet &p_properties) = 0;
	virtual ErrorCode destroyObject(SystemObject *p_systemObject) = 0;

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	virtual SystemTask *getSystemTask() = 0;
	virtual Systems::TypeID getSystemType() = 0;
	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::None; };
	
protected:
	bool m_initialized;
	SystemBase *m_system;
	SceneLoader *m_sceneLoader;
};

class SystemObject : public ObservedSubject, public Observer
{
	friend SystemBase;
	friend SystemScene;
public:
	SystemObject() : m_initialized(false), m_systemScene(nullptr), m_objectType(Properties::Null) { }
	SystemObject(SystemScene *p_systemScene, const std::string &p_name, Properties::PropertyID p_objectType)
		: m_initialized(false), m_systemScene(p_systemScene), m_objectType(p_objectType)
	{
		setName(p_name);
	}

	virtual ErrorCode init() = 0;

	virtual void loadToMemory() = 0;

	virtual BitMask getSystemType() = 0;

	virtual void update(const float p_deltaTime) = 0;

	virtual BitMask getDesiredSystemChanges() = 0;

	// Exports all the data of the object as a PropertySet (for example, used for saving to map file)
	virtual PropertySet exportObject() { return PropertySet(Properties::Null); }

	// Getters
	inline void *getParent() const						{ return m_parent;		}
	inline bool isInitialized() const					{ return m_initialized; }
	const inline std::string &getName() const			{ return m_name;		}
	inline SystemScene *getSystemScene() const			{ return m_systemScene; }
	inline Properties::PropertyID getObjectType() const { return m_objectType;	}
	const virtual std::string &getString(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Generic::Name:
			return m_name;
		}

		return ObservedSubject::getString(p_observer, p_changedBits);
	}

	// Setters
	inline void setParent(void *p_parent)								 { m_parent = p_parent;				}
	inline void setName(std::string p_name)								 { m_name = p_name;					}
	inline void setSystemScene(SystemScene *p_systemScene)				 { m_systemScene = p_systemScene;	}
	inline void setObjectType(const Properties::PropertyID p_objectType) { m_objectType = p_objectType;		}

	// Bool operator; returns true if the system object is not null (i.e. valid derived object)
	// Implemented to allow requested objects to be checked if valid and ability to return 
	// null (empty) objects on initialization failure, instead of raw nullptr pointers
	virtual operator bool() const { return false; }

protected:
	Properties::PropertyID m_objectType;

protected:
	void *m_parent;
	bool m_initialized;

	std::string m_name;
	SystemScene *m_systemScene;
};

class SystemTask
{
	friend SystemBase;

public:
	SystemTask(SystemScene *p_systemScene) : m_systemScene(p_systemScene) { }

	SystemScene *getSystemScene() { return m_systemScene; }

	virtual Systems::TypeID getSystemType() = 0;

	virtual void update(const float p_deltaTime) = 0;

	virtual bool isPrimaryThreadOnly() = 0;

protected:
	SystemScene *m_systemScene;
};