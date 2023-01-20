#pragma once

#include "CommonDefinitions.h"
#include "GUIScene.h"
#include "RendererScene.h"
#include "PhysicsScene.h"
#include "ScriptScene.h"
#include "WorldScene.h"

struct ComponentsConstructionInfo
{
	ComponentsConstructionInfo()
	{
		m_name = "null";
		m_id = NULL_ENTITY_ID;
		m_parent = NULL_ENTITY_ID;
	}

	void deleteConstructionInfo()
	{
		m_graphicsComponents.deleteConstructionInfo();
		m_guiComponents.deleteConstructionInfo();
		m_physicsComponents.deleteConstructionInfo();
		m_scriptComponents.deleteConstructionInfo();
		m_worldComponents.deleteConstructionInfo();
	}

	GraphicsComponentsConstructionInfo m_graphicsComponents;
	GUIComponentsConstructionInfo m_guiComponents;
	PhysicsComponentsConstructionInfo m_physicsComponents;
	ScriptComponentsConstructionInfo m_scriptComponents;
	WorldComponentsConstructionInfo m_worldComponents;

	std::string m_name;
	EntityID m_id;
	EntityID m_parent;
};

typedef std::vector<ComponentsConstructionInfo> EntitiesConstructionInfo;