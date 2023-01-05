#pragma once

// uncomment to disable assert()
// #define NDEBUG
#include <assert.h>
#include <string>

#include "Config.h"
#include "ObserverBase.h"
#include "PropertySet.h"

class SceneLoader;
class SystemObject;
class SystemScene;
class SystemTask;

class ImportExportObject
{
public:
	// Imports all the data of the object from a PropertySet
	virtual ErrorCode importObject(const PropertySet &p_properties) { return ErrorCode::Success; }

	// Exports all the data of the object as a PropertySet (for example, used for saving to map file)
	virtual PropertySet exportObject() { return PropertySet(Properties::Null); }
};

class SystemBase
{
public:
	SystemBase() : m_initialized(false) { }
	//~SystemBase();

	virtual ErrorCode init() = 0;
	//virtual ErrorCode destroyScene(SystemScene *p_systemScene) = 0;

	virtual ErrorCode setup(const PropertySet &p_properties) = 0;

	virtual Systems::TypeID getSystemType() = 0;
	virtual std::string getName() { return m_systemName; }

	// Passes scene loader as argument so that scenes could have access to other system scenes 
	// that are register with this scene loader
	virtual SystemScene *createScene(SceneLoader *p_sceneLoader) = 0;

	virtual SystemScene *getScene() = 0;

	virtual void loadInBackground() = 0;
	
protected:
	std::string m_systemName;
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

	virtual SystemObject *createComponent(const EntityID &p_entityID, const std::string &p_entityName, const PropertySet &p_properties);
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

class SystemObject : public ObservedSubject, public Observer, public ImportExportObject
{
	friend SystemBase;
	friend SystemScene;
public:
	SystemObject();
	SystemObject(SystemScene *p_systemScene, const std::string &p_name, Properties::PropertyID p_objectType, EntityID p_entityID = NULL_ENTITY_ID);
	~SystemObject();

	virtual ErrorCode init() { return ErrorCode::Success; };

	virtual void loadToMemory() { };

	virtual BitMask getSystemType() = 0;

	virtual void update(const float p_deltaTime) { };

	virtual BitMask getDesiredSystemChanges() = 0;

	// Is the object active (i.e. should be drawn, updated, etc...)
	const inline bool isObjectActive() const { return m_active; }

	// Getters
	inline EntityID getEntityID() const					{ return m_entityID;					 }
	const inline bool isEntityIDValid() const			{ return (m_entityID != NULL_ENTITY_ID); }
	inline std::size_t getObjectID() const				{ return m_objectID;					 }
	inline void *getParent() const						{ return m_parent;						 }
	inline bool isInitialized() const					{ return m_initialized;					 }
	inline bool isUpdateNeeded() const					{ return m_updateNeeded;				 }
	const inline std::string &getName() const			{ return m_name;						 }
	inline SystemScene *getSystemScene() const			{ return m_systemScene;					 }
	inline Properties::PropertyID getObjectType() const { return m_objectType;					 }
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
	inline void setEntityID(const EntityID p_entityID)					 { m_entityID = p_entityID;			}
	inline void setParent(void *p_parent)								 { m_parent = p_parent;				}
	inline void setName(std::string p_name)								 { m_name = p_name;					}
	inline void setSystemScene(SystemScene *p_systemScene)				 { m_systemScene = p_systemScene;	}
	inline void setObjectType(const Properties::PropertyID p_objectType) { m_objectType = p_objectType;		}
	inline void setActive(const bool p_isActive)						 { m_active = p_isActive;			}

	// Bool operator; returns true if the system object is not null (i.e. valid derived object)
	// Implemented to allow requested objects to be checked if valid and ability to return 
	// null (empty) objects on initialization failure, instead of raw nullptr pointers
	virtual operator bool() const { return false; }

	// Comparator operator; uses object ID to determine if the object is the same
	bool operator==(const SystemObject &p_systemObject) const { return m_objectID == p_systemObject.m_objectID ? true : false; }

protected:
	inline void setUpdateNeeded(bool p_updateNeeded) { m_updateNeeded = p_updateNeeded; }

	// Sets the 'update needed' flag to false
	inline void updatePerformed() { setUpdateNeeded(false); }

	Properties::PropertyID m_objectType;

	void *m_parent;
	bool m_initialized;
	bool m_active;
	bool m_updateNeeded;

	std::string m_name;
	SystemScene *m_systemScene;

private:
	std::size_t m_objectID;
	EntityID m_entityID;
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