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

	void update(const float p_deltaTime);

	void loadInBackground();

	ErrorCode preload();

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ScriptComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		std::vector<SystemObject*> components;

		if(p_constructionInfo.m_luaConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_luaConstructionInfo, p_startLoading));

		return components;
	}

	SystemObject *createComponent(const EntityID &p_entityID, const LuaComponent::LuaComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
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
};