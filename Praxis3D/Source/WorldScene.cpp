
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

std::vector<SystemObject *> WorldScene::getComponents(const EntityID p_entityID)
{
	std::vector<SystemObject *> returnVector;

	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	auto *metadataComponent = m_entityRegistry.try_get<MetadataComponent>(p_entityID);
	if(metadataComponent != nullptr)
		returnVector.push_back(metadataComponent);

	auto *spatialComponent = m_entityRegistry.try_get<SpatialComponent>(p_entityID);
	if(spatialComponent != nullptr)
		returnVector.push_back(spatialComponent);

	auto *objectMaterialComponent = m_entityRegistry.try_get<ObjectMaterialComponent>(p_entityID);
	if(objectMaterialComponent != nullptr)
		returnVector.push_back(objectMaterialComponent);

	return returnVector;
}

ErrorCode WorldScene::addComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	ErrorCode returnError = ErrorCode::Success;

	if(m_entityRegistry.valid(p_entityID))
	{
		// Add WORLD components
		std::vector<SystemObject *> worldComponents = createComponents(p_entityID, p_constructionInfo.m_worldComponents, p_startLoading);

		SystemObject *newSpatialComponent = nullptr;
		SystemObject *oldSpatialComponent = nullptr;

		// Find a spatial component within the world components array
		for(decltype(worldComponents.size()) i = 0, size = worldComponents.size(); i < size; i++)
			if(worldComponents[i]->getObjectType() == Properties::SpatialComponent)
			{
				newSpatialComponent = worldComponents[i];
				break;
			}

		if(newSpatialComponent != nullptr)
			oldSpatialComponent = m_entityRegistry.try_get<MetadataComponent>(p_entityID);

		// Add AUDIO components
		std::vector<SystemObject *> newAudioComponents = m_sceneLoader->getSystemScene(Systems::Audio)->createComponents(p_entityID, p_constructionInfo, p_startLoading);
		std::vector<SystemObject *> oldAudioComponents = m_sceneLoader->getSystemScene(Systems::Audio)->getComponents(p_entityID);

		// Add RENDERING components
		std::vector<SystemObject *> newRenderingComponents = m_sceneLoader->getSystemScene(Systems::Graphics)->createComponents(p_entityID, p_constructionInfo, p_startLoading);
		std::vector<SystemObject *> oldRenderingComponents = m_sceneLoader->getSystemScene(Systems::Graphics)->getComponents(p_entityID);

		// Add GUI components
		std::vector<SystemObject *> newGuiComponents = m_sceneLoader->getSystemScene(Systems::GUI)->createComponents(p_entityID, p_constructionInfo, p_startLoading);
		std::vector<SystemObject *> oldGuiComponents = m_sceneLoader->getSystemScene(Systems::GUI)->getComponents(p_entityID);

		// Add PHYSICS components
		std::vector<SystemObject *> newPhysicsComponents = m_sceneLoader->getSystemScene(Systems::Physics)->createComponents(p_entityID, p_constructionInfo, p_startLoading);
		std::vector<SystemObject *> oldPhysicsComponents = m_sceneLoader->getSystemScene(Systems::Physics)->getComponents(p_entityID);

		// Add SCRIPTING components
		std::vector<SystemObject *> newScriptingComponents = m_sceneLoader->getSystemScene(Systems::Script)->createComponents(p_entityID, p_constructionInfo, p_startLoading);
		std::vector<SystemObject *> oldScriptingComponents = m_sceneLoader->getSystemScene(Systems::Script)->getComponents(p_entityID);

		// Link subjects and observers of different components
		// Link NEW components to both NEW and OLD components
		// Link OLD components to NEW components only (to avoid duplicate linking)

		if(newSpatialComponent != nullptr)
		{
			// Link PHYSICS -> SPATIAL
			// NEW components -> NEW and OLD
			for(decltype(newPhysicsComponents.size()) i = 0, size = newPhysicsComponents.size(); i < size; i++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(newPhysicsComponents[i], newSpatialComponent);
			}
			for(decltype(oldScriptingComponents.size()) i = 0, size = oldScriptingComponents.size(); i < size; i++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(oldScriptingComponents[i], newSpatialComponent);
			}
		}
		else
			if(oldSpatialComponent != nullptr)
			{
				// Link PHYSICS -> SPATIAL
				// OLD components -> NEW
				for(decltype(newPhysicsComponents.size()) i = 0, size = newPhysicsComponents.size(); i < size; i++)
				{
					m_sceneLoader->getChangeController()->createObjectLink(newPhysicsComponents[i], oldSpatialComponent);
				}
			}


		// Link SCRIPTING
		// NEW components -> NEW and OLD
		for(decltype(newScriptingComponents.size()) scriptingIndex = 0, scriptingSize = newScriptingComponents.size(); scriptingIndex < scriptingSize; scriptingIndex++)
		{
			// If there are no physics components, link to spatial directly. If there are physics components, link to physics components instead
			if(newPhysicsComponents.empty())
			{
				// Link SCRIPTING -> SPATIAL
				// NEW -> NEW
				if(newSpatialComponent != nullptr)
					m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], newSpatialComponent);
				else
					// NEW -> OLD
					if(oldSpatialComponent != nullptr)
						m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], oldSpatialComponent);
			}
			else
			{
				// Link SCRIPTING -> PHYSICS
				// NEW -> NEW
				for(decltype(newPhysicsComponents.size()) physicsIndex = 0, physicsSize = newPhysicsComponents.size(); physicsIndex < physicsSize; physicsIndex++)
					m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], newPhysicsComponents[physicsIndex]);
				// NEW -> OLD
				for(decltype(oldPhysicsComponents.size()) physicsIndex = 0, physicsSize = oldPhysicsComponents.size(); physicsIndex < physicsSize; physicsIndex++)
					m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], oldPhysicsComponents[physicsIndex]);
			}

			// Link SCRIPTING -> AUDIO
			// NEW -> NEW
			for(decltype(newAudioComponents.size()) audioIndex = 0, audioSize = newAudioComponents.size(); audioIndex < audioSize; audioIndex++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], newAudioComponents[audioIndex]);
			}
			// NEW -> OLD
			for(decltype(oldAudioComponents.size()) audioIndex = 0, audioSize = oldAudioComponents.size(); audioIndex < audioSize; audioIndex++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], oldAudioComponents[audioIndex]);
			}

			// Link SCRIPTING -> GUI
			// NEW -> NEW
			for(decltype(newGuiComponents.size()) guiIndex = 0, guiSize = newGuiComponents.size(); guiIndex < guiSize; guiIndex++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], newGuiComponents[guiIndex]);
			}
			// NEW -> OLD
			for(decltype(oldGuiComponents.size()) guiIndex = 0, guiSize = oldGuiComponents.size(); guiIndex < guiSize; guiIndex++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(newScriptingComponents[scriptingIndex], oldGuiComponents[guiIndex]);
			}
		}

		// Link SCRIPTING
		// OLD components -> NEW
		for(decltype(oldScriptingComponents.size()) scriptingIndex = 0, scriptingSize = oldScriptingComponents.size(); scriptingIndex < scriptingSize; scriptingIndex++)
		{
			// If there are no physics components, link to spatial directly. If there are physics components, link to physics components instead
			if(newPhysicsComponents.empty())
			{
				// Link SCRIPTING -> SPATIAL
				if(newSpatialComponent != nullptr)
					m_sceneLoader->getChangeController()->createObjectLink(oldScriptingComponents[scriptingIndex], newSpatialComponent);
			}
			else
			{
				// Link SCRIPTING -> PHYSICS
				for(decltype(newPhysicsComponents.size()) physicsIndex = 0, physicsSize = newPhysicsComponents.size(); physicsIndex < physicsSize; physicsIndex++)
					m_sceneLoader->getChangeController()->createObjectLink(oldScriptingComponents[scriptingIndex], newPhysicsComponents[physicsIndex]);
			}

			// Link SCRIPTING -> AUDIO
			for(decltype(newAudioComponents.size()) audioIndex = 0, audioSize = newAudioComponents.size(); audioIndex < audioSize; audioIndex++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(oldScriptingComponents[scriptingIndex], newAudioComponents[audioIndex]);
			}

			// Link SCRIPTING -> GUI
			for(decltype(newGuiComponents.size()) guiIndex = 0, guiSize = newGuiComponents.size(); guiIndex < guiSize; guiIndex++)
			{
				m_sceneLoader->getChangeController()->createObjectLink(oldScriptingComponents[scriptingIndex], newGuiComponents[guiIndex]);
			}
		}
	}
	else
		returnError = ErrorCode::Nonexistent_object_id;

	return returnError;
}

EntityID WorldScene::createEntity(const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	EntityID newEntity = 0;

	if(p_constructionInfo.m_id != NULL_ENTITY_ID)
	{
		// Add entity to the registry and assign its entity ID
		newEntity = addEntity(p_constructionInfo.m_id);

		// Log an error if the desired ID couldn't be assigned, unless desired ID was 0
		if(p_constructionInfo.m_id != 0 && p_constructionInfo.m_id != newEntity)
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

void WorldScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
}

void WorldScene::receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
	switch(p_dataType)
	{
		case DataType::DataType_CreateComponent:
			{
				// Get entity and component data
				auto const *componentConstructionInfo = static_cast<ComponentsConstructionInfo *>(p_data);

				auto addComponentError = addComponents(componentConstructionInfo->m_id, *componentConstructionInfo);

				if(addComponentError == ErrorCode::Success)
					ErrHandlerLoc::get().log(addComponentError, "EntityID: " + Utilities::toString(componentConstructionInfo->m_id), ErrorSource::Source_WorldScene);

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentConstructionInfo;
			}
			break;

		case DataType::DataType_DeleteComponent:
			{
				// Get entity and component data
				auto const *componentData = static_cast<EntityAndComponent *>(p_data);

				// Delete the component based on its type
				switch(componentData->m_componentType)
				{
					case ComponentType::ComponentType_ObjectMaterialComponent:
						{
							// Check if the component exists
							auto *component = m_entityRegistry.try_get<ObjectMaterialComponent>(componentData->m_entityID);
							if(component != nullptr)
							{
								// Delete component
								removeComponent<ObjectMaterialComponent>(componentData->m_entityID);
							}
						}
						break;

					case ComponentType::ComponentType_SpatialComponent:
						{
							// Check if the component exists
							auto *component = m_entityRegistry.try_get<SpatialComponent>(componentData->m_entityID);
							if(component != nullptr)
							{
								// Delete component
								removeComponent<SpatialComponent>(componentData->m_entityID);
							}
						}
						break;
				}

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentData;
			}
			break;

		case DataType::DataType_CreateEntity:
			{
				// Get entity and component data
				auto const *componentConstructionInfo = static_cast<ComponentsConstructionInfo *>(p_data);

				auto entityID = createEntity(*componentConstructionInfo);

				//if(addComponentError == ErrorCode::Success)
				//	ErrHandlerLoc::get().log(addComponentError, "EntityID: " + Utilities::toString(componentConstructionInfo->m_id), ErrorSource::Source_WorldScene);

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentConstructionInfo;
			}
			break;

		case DataType::DataType_DeleteEntity:
			{
				// Get entity and component data
				auto const *componentData = static_cast<EntityAndComponent *>(p_data);

				// Delete the component based on its type
				switch(componentData->m_componentType)
				{
					case ComponentType::ComponentType_ObjectMaterialComponent:
						{
							// Check if the component exists
							auto *component = m_entityRegistry.try_get<ObjectMaterialComponent>(componentData->m_entityID);
							if(component != nullptr)
							{
								// Delete component
								removeComponent<ObjectMaterialComponent>(componentData->m_entityID);
							}
						}
						break;

					case ComponentType::ComponentType_SpatialComponent:
						{
							// Check if the component exists
							auto *component = m_entityRegistry.try_get<SpatialComponent>(componentData->m_entityID);
							if(component != nullptr)
							{
								// Delete component
								removeComponent<SpatialComponent>(componentData->m_entityID);
							}
						}
						break;
				}

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentData;
			}
			break;

		default:
			assert(p_deleteAfterReceiving == true && "Memory leak - unhandled orphaned void data pointer in receiveData");
			break;
	}
}
