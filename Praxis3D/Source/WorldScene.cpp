
#include "ComponentConstructorInfo.h"
#include "GameObjectComponent.h"
#include "NullSystemObjects.h"
#include "SceneLoader.h"
#include "SpatialComponent.h"
#include "WorldScene.h"

WorldScene::WorldScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::World)
{
	m_worldTask = new WorldTask(this);
}

ErrorCode WorldScene::init() 
{
	// Since this flag is modified by hand in the Entt code, check to see if it is still correct, because updating the Entt library will revert the flag back to default (false)
	if(!entt::basic_component_traits::in_place_delete)
		ErrHandlerLoc().get().log(ErrorType::Error, ErrorSource::Source_WorldScene, "entt::basic_component_traits::in_place_delete is switched off, disabling pointer stability upon component deletion");

	return ErrorCode::Success; 
}

ErrorCode WorldScene::setup(const PropertySet &p_properties)
{
	// Get the property set containing object pool size
	auto &objectPoolSizeProperty = p_properties.getPropertySetByID(Properties::ObjectPoolSize);

	// Reserve every component type that belongs to this scene (and set the minimum number of objects based on default config)
	reserve<MetadataComponent>(std::max(Config::objectPoolVar().spatial_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::MetadataComponent).getInt()));
	reserve<ObjectMaterialComponent>(std::max(Config::objectPoolVar().spatial_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::ObjectMaterialComponent).getInt()));
	reserve<SpatialComponent>(std::max(Config::objectPoolVar().spatial_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::SpatialComponent).getInt()));

	return ErrorCode::Success;
}

void WorldScene::exportSetup(PropertySet &p_propertySet)
{
	// Add object pool sizes
	auto &objectPoolSizePropertySet = p_propertySet.addPropertySet(Properties::ObjectPoolSize);
	objectPoolSizePropertySet.addProperty(Properties::MetadataComponent, (int)getPoolSize<MetadataComponent>());
	objectPoolSizePropertySet.addProperty(Properties::ObjectMaterialComponent, (int)getPoolSize<ObjectMaterialComponent>());
	objectPoolSizePropertySet.addProperty(Properties::SpatialComponent, (int)getPoolSize<SpatialComponent>());
}

void WorldScene::update(const float p_deltaTime)
{
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

	// Create the metadata component by passing all of the construction info
	createComponent(newEntity, p_constructionInfo, p_startLoading);

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

	// Add AUDIO components
	std::vector<SystemObject*> audioComponents = m_sceneLoader->getSystemScene(Systems::Audio)->createComponents(newEntity, p_constructionInfo, p_startLoading);

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

		// Link SCRIPTING -> AUDIO
		for(decltype(audioComponents.size()) audioIndex = 0, audioSize = audioComponents.size(); audioIndex < audioSize; audioIndex++)
		{
			m_sceneLoader->getChangeController()->createObjectLink(scriptingComponents[scriptingIndex], audioComponents[audioIndex]);
		}

		// Link SCRIPTING -> GUI
		for(decltype(guiComponents.size()) guiIndex = 0, guiSize = guiComponents.size(); guiIndex < guiSize; guiIndex++)
		{
			m_sceneLoader->getChangeController()->createObjectLink(scriptingComponents[scriptingIndex], guiComponents[guiIndex]);
		}
	}

	return newEntity;
}

void WorldScene::exportEntity(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	if(m_entityRegistry.valid(p_entityID))
	{
		// Export AUDIO components
		m_sceneLoader->getSystemScene(Systems::Audio)->exportComponents(p_entityID, p_constructionInfo);

		// Export RENDERING components
		m_sceneLoader->getSystemScene(Systems::Graphics)->exportComponents(p_entityID, p_constructionInfo);

		// Export GUI components
		m_sceneLoader->getSystemScene(Systems::GUI)->exportComponents(p_entityID, p_constructionInfo);

		// Export PHYSICS components
		m_sceneLoader->getSystemScene(Systems::Physics)->exportComponents(p_entityID, p_constructionInfo);

		// Export SCRIPTING components
		m_sceneLoader->getSystemScene(Systems::Script)->exportComponents(p_entityID, p_constructionInfo);

		// Export WORLD components
		exportComponents(p_entityID, p_constructionInfo);
	}
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

	if(p_constructionInfo.m_objectMaterialConstructionInfo != nullptr)
		components.push_back(createComponent(p_entityID, *p_constructionInfo.m_objectMaterialConstructionInfo, p_startLoading));

	return components;
}

void WorldScene::exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	p_constructionInfo.m_id = p_entityID;

	// Export MetadataComponent
	auto *metadataComponent = m_entityRegistry.try_get<MetadataComponent>(p_entityID);
	if(metadataComponent != nullptr)
	{
		p_constructionInfo.m_name = metadataComponent->getName();
		p_constructionInfo.m_parent = metadataComponent->getParentEntityID();

		if(metadataComponent->getUsesPrefab())
			p_constructionInfo.m_prefab = metadataComponent->getPrefabName();
	}
	else
	{
		p_constructionInfo.m_name = GetString(Properties::GameObject) + Utilities::toString(p_entityID);
		p_constructionInfo.m_parent = 0;
	}

	// Export SpatialComponent
	auto *spatialComponent = m_entityRegistry.try_get<SpatialComponent>(p_entityID);
	if(spatialComponent != nullptr)
	{
		if(p_constructionInfo.m_worldComponents.m_spatialConstructionInfo == nullptr)
			p_constructionInfo.m_worldComponents.m_spatialConstructionInfo = new SpatialComponent::SpatialComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_worldComponents.m_spatialConstructionInfo, *spatialComponent);
	}

	// Export ObjectMaterialComponent
	auto *objectMaterialComponent = m_entityRegistry.try_get<ObjectMaterialComponent>(p_entityID);
	if(objectMaterialComponent != nullptr)
	{
		if(p_constructionInfo.m_worldComponents.m_objectMaterialConstructionInfo == nullptr)
			p_constructionInfo.m_worldComponents.m_objectMaterialConstructionInfo = new ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_worldComponents.m_objectMaterialConstructionInfo, *objectMaterialComponent);
	}
}

ErrorCode WorldScene::destroyObject(SystemObject *p_systemObject)
{
	ErrorCode returnError = ErrorCode::Success;

	switch(p_systemObject->getObjectType())
	{
	case Properties::PropertyID::ObjectMaterialComponent:
		//m_sceneLoader->getChangeController()->removeObjectLink(p_systemObject);
		removeComponent<ObjectMaterialComponent>(p_systemObject->getEntityID());
		break;

	case Properties::PropertyID::SpatialComponent:
		//m_sceneLoader->getChangeController()->removeObjectLink(p_systemObject);
		removeComponent<SpatialComponent>(p_systemObject->getEntityID());
		break;

	default:
		// No object was found, return an appropriate error
		returnError = ErrorCode::Destroy_obj_not_found;
		break;
	}

	// If this point is reached, 
	return returnError;
}

SystemObject *WorldScene::createComponent(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	MetadataComponent *metadataComponent = nullptr;

	metadataComponent = &addComponent<MetadataComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	metadataComponent->setParent(p_constructionInfo.m_parent);

	if(!p_constructionInfo.m_prefab.empty())
		metadataComponent->setPrefabName(p_constructionInfo.m_prefab);
	
	metadataComponent->init();

	return metadataComponent;
}
