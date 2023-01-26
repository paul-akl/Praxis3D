
#include "ComponentConstructorInfo.h"
#include "GameObjectComponent.h"
#include "NullSystemObjects.h"
#include "SceneLoader.h"
#include "SpatialComponent.h"
#include "WorldScene.h"

WorldScene::WorldScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_worldTask = new WorldTask(this);
}

ErrorCode WorldScene::setup(const PropertySet &p_properties)
{
	// Get default object pool size
	decltype(m_gameObjects.getPoolSize()) objectPoolSize = Config::objectPoolVar().object_pool_size;

	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ObjectPoolSize:
			objectPoolSize = p_properties[i].getInt();
			break;
		}
	}

	// Initialize object pools
	m_gameObjects.init(objectPoolSize);

	return ErrorCode::Success;
}

void WorldScene::update(const float p_deltaTime)
{
	//std::cout << "World update:" << std::endl;

	//std::vector<EntityID> entities;

	//m_entityRegistry.each([&entities](auto entity)
	//	{
	//		entities.push_back(entity);
	//	});

	//for(size_t i = 0; i < entities.size(); i++)
	//{
	//	std::cout << entities[i];
	//	if(m_entityRegistry.all_of<CameraComponent>(entities[i]))
	//		std::cout << " cam ";
	//	if(m_entityRegistry.all_of<LightComponent>(entities[i]))
	//		std::cout << " lht ";
	//	if(m_entityRegistry.all_of<ModelComponent>(entities[i]))
	//		std::cout << " mdl ";
	//	std::cout << std::endl;
	//}

	//std::cout << std::endl;

	// Go over each game object
	//for(decltype(m_gameObjects.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_gameObjects.getNumAllocated(),
	//	size = m_gameObjects.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	//{
	//	// Check if the game object is allocated inside the pool container
	//	if(m_gameObjects[i].allocated())
	//	{
	//		// Increment the number of allocated objects (early bail mechanism)
	//		numAllocObjecs++;

	//		// Update the game object
	//		m_gameObjects[i].getObject()->update(p_deltaTime);
	//	}
	//}
	

	//	 ___________________________
	//	|							|
	//	|	  SPATIAL COMPONENT		|
	//	|___________________________|
	//
	auto spatialView = m_entityRegistry.view<SpatialComponent>();
	for(auto entity : spatialView)
	{
		auto &component = spatialView.get<SpatialComponent>(entity);

		component.update(p_deltaTime);
	}

}

EntityID WorldScene::createEntity(const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	EntityID newEntity = 0;

	if(p_constructionInfo.m_id != NULL_ENTITY_ID)
	{
		// Add entity to the registry and assign its entity ID
		newEntity = addEntity(p_constructionInfo.m_id);

		// Log an error if the desired ID couldn't be assigned
		if(p_constructionInfo.m_id != newEntity)
			ErrHandlerLoc::get().log(ErrorCode::Duplicate_object_id, ErrorSource::Source_WorldScene, p_constructionInfo.m_name + " - Entity ID \'" + Utilities::toString(p_constructionInfo.m_id) + "\' is already taken. Replaced with: \'" + Utilities::toString(newEntity) + "\'");
	}
	else // Do not request a specific entity ID if the requested ID is null
		newEntity = addEntity();

	// Add WORLD components
	std::vector<SystemObject*> worldComponents = createComponents(newEntity, p_constructionInfo.m_worldComponents, p_startLoading);

	SystemObject *spatialComponent = nullptr;

	// Find a spatial component within the world components array
	for(decltype(worldComponents.size()) i = 0, size = worldComponents.size(); i < size; i++)
		if(worldComponents[i]->getObjectType() == Properties::SpatialComponent)
		{
			spatialComponent = worldComponents[i];
			break;
		}

	// Add RENDERING components
	std::vector<SystemObject*> renderingComponents = m_sceneLoader->getSystemScene(Systems::Graphics)->createComponents(newEntity, p_constructionInfo, p_startLoading);

	// Add GUI components
	std::vector<SystemObject*> guiComponents = m_sceneLoader->getSystemScene(Systems::GUI)->createComponents(newEntity, p_constructionInfo, p_startLoading);

	// Add PHYSICS components
	std::vector<SystemObject*> physicsComponents = m_sceneLoader->getSystemScene(Systems::Physics)->createComponents(newEntity, p_constructionInfo, p_startLoading);

	// Add SCRIPTING components
	std::vector<SystemObject*> scriptingComponents = m_sceneLoader->getSystemScene(Systems::Script)->createComponents(newEntity, p_constructionInfo, p_startLoading);

	// Link subjects and observers of different components

	if(spatialComponent != nullptr)
	{
		// Link PHYSICS -> SPATIAL
		for(decltype(physicsComponents.size()) i = 0, size = physicsComponents.size(); i < size; i++)
		{
			m_sceneLoader->getChangeController()->createObjectLink(physicsComponents[i], spatialComponent);
		}
	}

	// Link SCRIPTING
	for(decltype(scriptingComponents.size()) scriptingIndex = 0, scriptingSize = scriptingComponents.size(); scriptingIndex < scriptingSize; scriptingIndex++)
	{
		// If there are no physics components, link to spatial directly. If there are physics components, link to physics components instead
		if(physicsComponents.empty())
		{
			// Link SCRIPTING -> SPATIAL
			if(spatialComponent != nullptr)
				m_sceneLoader->getChangeController()->createObjectLink(scriptingComponents[scriptingIndex], spatialComponent);
		}
		else
		{
			// Link SCRIPTING -> PHYSICS
			for(decltype(physicsComponents.size()) physicsIndex = 0, physicsSize = physicsComponents.size(); physicsIndex < physicsSize; physicsIndex++)
				m_sceneLoader->getChangeController()->createObjectLink(scriptingComponents[scriptingIndex], physicsComponents[physicsIndex]);
		}

		// Link SCRIPTING -> GUI
		for(decltype(guiComponents.size()) guiIndex = 0, guiSize = guiComponents.size(); guiIndex < guiSize; guiIndex++)
		{
			m_sceneLoader->getChangeController()->createObjectLink(scriptingComponents[scriptingIndex], guiComponents[guiIndex]);
		}
	}

	return newEntity;
}

std::vector<SystemObject*> WorldScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_worldComponents, p_startLoading);
}

std::vector<SystemObject*> WorldScene::createComponents(const EntityID p_entityID, const WorldComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	std::vector<SystemObject*> components;

	if(p_constructionInfo.m_spatialConstructionInfo != nullptr)
		components.push_back(createComponent(p_entityID, *p_constructionInfo.m_spatialConstructionInfo, p_startLoading));

	return components;
}

ErrorCode WorldScene::destroyObject(SystemObject *p_systemObject)
{
	// Check if object is valid and belongs to world system
	if(p_systemObject != nullptr && p_systemObject->getSystemType() == Systems::World)
	{
		// Cast the system object to game object, as it belongs to the renderer scene
		GameObject *objectToDestroy = static_cast<GameObject*>(p_systemObject);

		// Try to destroy the object; return success if it succeeds
		if(removeObjectFromPool(*objectToDestroy))
			return ErrorCode::Success;
	}

	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}
