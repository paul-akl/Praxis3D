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
struct ComponentsConstructionInfo;

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

	virtual ErrorCode init() = 0;
	//virtual ErrorCode destroyScene(SystemScene *p_systemScene) = 0;

	// Internal system data-driven setup based on passed properties
	virtual ErrorCode setup(const PropertySet &p_properties) = 0;

	// Exports all the system settings
	virtual void exportSetup(PropertySet &p_propertySet) { }

	virtual Systems::TypeID getSystemType() = 0;
	virtual std::string getName() { return m_systemName; }

	// Passes scene loader as argument so that scenes could have access to other system scenes that are register with this scene loader
	// Also takes in an engine state type, as a way to identify created scenes, since scenes can only be created for each engine state
	virtual SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState) = 0;

	virtual SystemScene *getScene(EngineStateType p_engineState) = 0;

	virtual void deleteScene(EngineStateType p_engineState) = 0;

	virtual void loadInBackground() = 0;
	
protected:
	std::string m_systemName;
	bool m_initialized;
};

class SystemScene : public ObservedSubject, public Observer
{
	friend SystemBase;
public:
	SystemScene(SystemBase *p_system, SceneLoader *p_sceneLoader, Properties::PropertyID p_objectType)
		: Observer(p_objectType), m_initialized(false), m_system(p_system), m_sceneLoader(p_sceneLoader) { }
	//~SystemScene();

	// Gets the parent system
	SystemBase *getSystem() { return m_system; }

	// Initializes the scene
	virtual ErrorCode init() = 0;

	// Internal scene data-driven setup based on passed properties
	virtual ErrorCode setup(const PropertySet &p_properties) = 0;

	// Exports all the scene settings
	virtual void exportSetup(PropertySet &p_propertySet) { }
	
	// Activation is called when the engine play state containing this scene is made current (activated)
	virtual void activate() { }

	// Deactivation is called when the engine play state changes and this scene isn't current anymore
	virtual void deactivate() { }

	// Called every frame with the last frame's delta time
	virtual void update(const float p_deltaTime) = 0;

	// Load all the scene objects at once
	virtual ErrorCode preload() = 0;

	// Start loading all the scene objects in the background threads and return (without waiting)
	virtual void loadInBackground() = 0;

	// Get all the created components of the given entity that belong to this scene
	virtual std::vector<SystemObject*> getComponents(const EntityID p_entityID);

	// Create all the components that belong to this scene, that are contained in ComponentsConstructionInfo; return a vector of all created components
	virtual std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);

	// Exports all the components that belong to this scene into ComponentsConstructionInfo
	virtual void exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo) { }

	// Destroy an object that belongs to this system scene
	virtual ErrorCode destroyObject(SystemObject *p_systemObject) = 0;

	// Returns a null system object that is used as a fall-back when an object creation fails
	virtual SystemObject *getNullObject();

	// Called whenever there are changes pending for this scene; always called at the end of frame
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	// Return the parent system task
	virtual SystemTask *getSystemTask() = 0;

	// Return the system type that this scene belongs to
	virtual Systems::TypeID getSystemType() = 0;

	// Return the data changes this scene is subscribed to
	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::None; };

	// Return the parent scene loader
	inline SceneLoader *getSceneLoader() { return m_sceneLoader; }

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
	struct SystemObjectConstructionInfo
	{
		SystemObjectConstructionInfo()
		{
			m_active = true;
			m_name = "null";
		}

		bool m_active;
		std::string m_name;
	};

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

	virtual void activate() { m_systemScene->activate(); }

	virtual void deactivate() { m_systemScene->deactivate(); }

protected:
	SystemScene *m_systemScene;
};