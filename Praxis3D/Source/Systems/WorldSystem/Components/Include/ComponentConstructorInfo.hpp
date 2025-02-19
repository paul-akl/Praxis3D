#pragma once

#include "Systems/AudioSystem/Include/AudioScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"
#include "Systems/GUISystem/Include/GUIScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Systems/PhysicsSystem/Include/PhysicsScene.hpp"
#include "Systems/ScriptingSystem/Include/ScriptScene.hpp"
#include "Systems/WorldSystem/Include/WorldScene.hpp"

struct ComponentsConstructionInfo
{
	ComponentsConstructionInfo()
	{
		m_name = "null";
		m_id = NULL_ENTITY_ID;
		m_parent = 0;
	}

	ComponentsConstructionInfo(const ComponentsConstructionInfo &p_other)
	{
		completeCopy(p_other);
	}

	ComponentsConstructionInfo &operator=(const ComponentsConstructionInfo &p_other)
	{
		m_name = p_other.m_name;
		m_id = p_other.m_id;
		m_parent = p_other.m_parent;

		m_audioComponents = p_other.m_audioComponents;
		m_graphicsComponents = p_other.m_graphicsComponents;
		m_guiComponents = p_other.m_guiComponents;
		m_physicsComponents = p_other.m_physicsComponents;
		m_scriptComponents = p_other.m_scriptComponents;
		m_worldComponents = p_other.m_worldComponents;

		return *this;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const ComponentsConstructionInfo &p_other)
	{
		m_name = p_other.m_name;
		m_id = p_other.m_id;
		m_parent = p_other.m_parent;

		m_audioComponents.completeCopy(p_other.m_audioComponents);
		m_graphicsComponents.completeCopy(p_other.m_graphicsComponents);
		m_guiComponents.completeCopy(p_other.m_guiComponents);
		m_physicsComponents.completeCopy(p_other.m_physicsComponents);
		m_scriptComponents.completeCopy(p_other.m_scriptComponents);
		m_worldComponents.completeCopy(p_other.m_worldComponents);
	}

	void deleteConstructionInfo()
	{
		m_audioComponents.deleteConstructionInfo();
		m_graphicsComponents.deleteConstructionInfo();
		m_guiComponents.deleteConstructionInfo();
		m_physicsComponents.deleteConstructionInfo();
		m_scriptComponents.deleteConstructionInfo();
		m_worldComponents.deleteConstructionInfo();
	}

	AudioComponentsConstructionInfo m_audioComponents;
	GraphicsComponentsConstructionInfo m_graphicsComponents;
	GUIComponentsConstructionInfo m_guiComponents;
	PhysicsComponentsConstructionInfo m_physicsComponents;
	ScriptComponentsConstructionInfo m_scriptComponents;
	WorldComponentsConstructionInfo m_worldComponents;

	std::string m_name;
	EntityID m_id;
	EntityID m_parent;

	std::string m_prefab;
};

typedef std::vector<ComponentsConstructionInfo> EntitiesConstructionInfo;