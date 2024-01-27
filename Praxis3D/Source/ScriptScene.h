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
struct ComponentsConstructionInfo;

struct ScriptComponentsConstructionInfo
{
	ScriptComponentsConstructionInfo()
	{
		m_luaConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const ScriptComponentsConstructionInfo &p_other)
	{
		Utilities::performCopy<LuaComponent::LuaComponentConstructionInfo>(&m_luaConstructionInfo, &p_other.m_luaConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		if(m_luaConstructionInfo != nullptr)
			delete m_luaConstructionInfo;
	}

	LuaComponent::LuaComponentConstructionInfo *m_luaConstructionInfo;
};

class ScriptScene : public SystemScene
{
public:
	ScriptScene(ScriptSystem *p_system, SceneLoader *p_sceneLoader);
	~ScriptScene();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties); 
	
	void exportSetup(PropertySet &p_propertySet);

	void update(const float p_deltaTime);

	void loadInBackground();

	ErrorCode preload();

	// Get all the created components of the given entity that belong to this scene
	std::vector<SystemObject *> getComponents(const EntityID p_entityID);

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ScriptComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		std::vector<SystemObject*> components;

		if(p_constructionInfo.m_luaConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_luaConstructionInfo, p_startLoading));

		return components;
	}

	void exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo);
	void exportComponents(const EntityID p_entityID, ScriptComponentsConstructionInfo &p_constructionInfo);

	SystemObject *createComponent(const EntityID &p_entityID, const LuaComponent::LuaComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);

	void exportComponent(LuaComponent::LuaComponentConstructionInfo &p_constructionInfo, const LuaComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();
		p_constructionInfo.m_pauseInEditor = p_component.pauseInEditor();

		if(p_component.getLuaScript() != nullptr)
		{
			p_constructionInfo.m_luaScriptFilename = Utilities::stripFilename(p_component.getLuaScript()->getLuaScriptFilename());
			p_constructionInfo.m_variables = p_component.getLuaScript()->getLuaVariables();
		}
	}

	void releaseObject(SystemObject *p_systemObject);

	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType);

	void receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving);

	BitMask getDesiredSystemChanges() { return 0; }
	BitMask getPotentialSystemChanges() { return 0; }

	// Getters
	SystemTask *getSystemTask()
	{ 
		if(m_scriptingTask != nullptr)
			return m_scriptingTask;

		return g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getSystemTask();
	}
	Systems::TypeID getSystemType() { return Systems::Script; }

private:
	FreeCamera *loadFreeCamera(const PropertySet &p_properties);
	DebugUIScript *loadDebugUI(const PropertySet &p_properties);
	DebugMoveScript *loadDebugMove(const PropertySet &p_properties);
	DebugRotateScript *loadDebugRotate(const PropertySet &p_properties);
	SolarTimeScript *loadSolarTime(const PropertySet &p_properties);
	SunScript *loadSun(const PropertySet &p_properties);
	WorldEditScript *loadWorldEdit(const PropertySet &p_properties);

	ScriptTask *m_scriptingTask;

	bool m_luaScriptsEnabled;
};