#pragma once

#include <queue>

#include "Definitions/Include/CommonDefinitions.hpp"
#include "Systems/RendererSystem/Include/GraphicsDataSets.hpp"

struct GraphicsLoadToMemoryComponent
{
	GraphicsLoadToMemoryComponent(const EntityID p_entityID) : m_entityID(p_entityID)
	{
		m_loaded = false;
	}
	~GraphicsLoadToMemoryComponent() { }

	bool m_loaded;
	EntityID m_entityID;
};

struct GraphicsLoadToVideoMemoryComponent
{
	GraphicsLoadToVideoMemoryComponent(const EntityID p_entityID) : m_entityID(p_entityID)
	{
		m_loaded = false;
	}
	~GraphicsLoadToVideoMemoryComponent() { }

	bool m_loaded;
	EntityID m_entityID;

	// Objects that need to be loaded to VRAM
	std::queue<LoadableObjectsContainer> m_objectsToLoad;
};