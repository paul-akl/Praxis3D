
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

EntityID WorldScene::createEntity(const PropertySet &p_properties)
{
	return EntityID();
}

SystemObject *WorldScene::createObject(const PropertySet &p_properties)
{
	GameObjectComponent *newGameObject = nullptr;
		
	// Get the object name and ID
	std::string name = p_properties.getPropertyByID(Properties::Name).getString();
	EntityID desiredEntityID = (EntityID)p_properties.getPropertyByID(Properties::ID).getInt();

	// Add entity to the registry and assign its entity ID
	EntityID newEntity = addEntity(desiredEntityID);

	// Log an error if the desired ID couldn't be assigned
	if(desiredEntityID != newEntity)
		ErrHandlerLoc::get().log(ErrorCode::Duplicate_object_id, ErrorSource::Source_WorldScene, name + " - Entity ID \'" + Utilities::toString(desiredEntityID) + "\' is already taken. Replaced with: \'" + Utilities::toString(newEntity) + "\'");

	// If the name property is missing, generate a unique name based on the entity ID
	if(name.empty())
		name = GetString(Properties::GameObject) + Utilities::toString(newEntity);

	// Construct the new GameObject
	newGameObject = &addComponent<GameObjectComponent>(newEntity, this, name, newEntity);

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Parent:
		{
			// Get the entity ID if the parent object
			EntityID parentID = (EntityID)p_properties[i].getInt();

			if(m_entityRegistry.all_of<GameObjectComponent>(parentID))
			{
				GameObjectComponent &parentGameObject = m_entityRegistry.get<GameObjectComponent>(parentID);

				// Add the game object component (child) to the observer list of the parent
				m_sceneLoader->getChangeController()->createObjectLink(&parentGameObject, newGameObject);

				newGameObject->setParent(parentID);
				parentGameObject.addChild(newEntity);

			}
		}
		break;
		}
	}

	SpatialComponent *spatialComponent = nullptr;

	// Add WORLD components
	auto &worldProperty = p_properties.getPropertySetByID(Properties::World);
	if(worldProperty)
	{
		// Add SPATIAL COMPONENT
		auto &spatialComponentProperty = worldProperty.getPropertySetByID(Properties::SpatialComponent);
		if(spatialComponentProperty)
		{
			//auto spatial = m_entityRegistry.emplace<SpatialComponent>(newEntity, this, name);
			SpatialComponent &newSpatialComponent = addComponent<SpatialComponent>(newEntity, this, name + Config::componentVar().component_name_separator + GetString(Properties::SpatialComponent), newEntity);

			// Load property data
			for(decltype(spatialComponentProperty.getNumProperties()) i = 0, size = spatialComponentProperty.getNumProperties(); i < size; i++)
			{
				switch(spatialComponentProperty[i].getPropertyID())
				{
				case Properties::LocalPosition:
					newSpatialComponent.m_spatialData.setLocalPosition(spatialComponentProperty[i].getVec3f());
					break;
				case Properties::LocalRotation:
					newSpatialComponent.m_spatialData.setLocalRotation(spatialComponentProperty[i].getVec3f());
					break;
				case Properties::LocalRotationQuaternion:
					newSpatialComponent.m_spatialData.setLocalRotation(glm::quat(spatialComponentProperty[i].getVec4f()));
					break;
				case Properties::LocalScale:
					newSpatialComponent.m_spatialData.setLocalScale(spatialComponentProperty[i].getVec3f());
					break;
				}
			}

			// Perform a spatial data update, so that all the transform matrices are calculated
			newSpatialComponent.m_spatialData.update();

			spatialComponent = &newSpatialComponent;
		}
	}

	// Add RENDERING components
	std::vector<SystemObject*> renderingComponents;
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::Rendering);
		if(sceneProperty)
		{
			SystemScene *scene = m_sceneLoader->getSystemScene(Systems::Graphics);
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				renderingComponents.push_back(scene->createComponent(newEntity, name, sceneProperty.getPropertySet(i)));
			}
		}
	}

	// Add GUI components
	std::vector<SystemObject*> guiComponents;
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::GUI);
		if(sceneProperty)
		{
			SystemScene *scene = m_sceneLoader->getSystemScene(Systems::GUI);
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				guiComponents.push_back(scene->createComponent(newEntity, name, sceneProperty.getPropertySet(i)));
			}
		}
	}	
	
	// Add PHYSICS components
	std::vector<SystemObject*> physicsComponents;
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::Physics);
		if(sceneProperty)
		{
			SystemScene *scene = m_sceneLoader->getSystemScene(Systems::Physics);
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				physicsComponents.push_back(scene->createComponent(newEntity, name, sceneProperty.getPropertySet(i)));
			}
		}
	}

	// Add SCRIPTING components
	std::vector<SystemObject*> scriptingComponents;
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::Script);
		if(sceneProperty)
		{
			SystemScene *scene = m_sceneLoader->getSystemScene(Systems::Script);
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				auto *component = scene->createComponent(newEntity, name, sceneProperty.getPropertySet(i));

				//auto *spatialComponent = m_entityRegistry.try_get<SpatialComponent>(newEntity);
				//if(spatialComponent != nullptr)
				//	m_sceneLoader->getChangeController()->createObjectLink(component, spatialComponent);

				// Create subject-observer object links between scripting components (subject) and GUI components (observers)
				//for(decltype(guiComponents.size()) size = guiComponents.size(), i = 0; i < size; i++)
				//	m_sceneLoader->getChangeController()->createObjectLink(component, guiComponents[i]);

				scriptingComponents.push_back(component);
			}
		}
	}

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


	return newGameObject;





	ErrorCode objPoolError = ErrorCode::Failure;
	GameObject *newGameObject2 = nullptr;

	// Find a place for the new object in the pool
	auto gameObjectFromPool = m_gameObjects.newObject();

	// Check if the pool wasn't full
	if(gameObjectFromPool != nullptr)
	{
		auto &nameProperty = p_properties.getPropertyByID(Properties::Name);

		std::string name;

		// If the name property is missing, generate a unique name based on the object's index in the pool
		if(nameProperty)
			name = nameProperty.getString();
		else
			name = GetString(Properties::GameObject) + Utilities::toString(gameObjectFromPool->getIndex());

		// Construct the new GameObject
		gameObjectFromPool->construct(this, name, *m_sceneLoader);
		newGameObject2 = gameObjectFromPool->getObject();
						
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::ID:
				{
					// Get the game object ID
					auto objectIDdesired = static_cast<EntityID>(p_properties.getPropertyByID(Properties::ID).getInt());

					// Create an entity
					auto entityID = m_entityRegistry.create(objectIDdesired);

					// If the enity wasn't registered with the given game object ID, log an error
					if(objectIDdesired != entityID)
					{
						// If an enity already exists at the given game object ID, log a duplicate ID error, otherwise log an invalid ID error
						if(m_entityRegistry.valid(objectIDdesired))
							ErrHandlerLoc::get().log(ErrorCode::Duplicate_object_id, ErrorSource::Source_World, newGameObject2->getName());
						else
							ErrHandlerLoc::get().log(ErrorCode::Invalid_object_id, ErrorSource::Source_World, newGameObject2->getName());
					}

					// Set the game object ID to the created entity ID in the registry
					newGameObject2->m_GameObjectID = entityID;

					// Try to register the object with the given game object ID
					auto objectIDactual = m_objectRegister.registerObject(newGameObject2, objectIDdesired);

					// If the object wasn't registered with the given game object ID, log an error
					if(objectIDdesired != objectIDactual)
					{
						// If an object already exists at the given game object ID, log a duplicate ID error, otherwise log an invalid ID error
						if(m_objectRegister.getObject(objectIDdesired) != nullptr)
							ErrHandlerLoc::get().log(ErrorCode::Duplicate_object_id, ErrorSource::Source_World, newGameObject2->getName());
						else
							ErrHandlerLoc::get().log(ErrorCode::Invalid_object_id, ErrorSource::Source_World, newGameObject2->getName());
					}
				}
				break;
			case Properties::Parent:
				{
					// Get the ID if the parent object
					decltype(GameObject::m_GameObjectID) parentID = p_properties[i].getInt();

					// Try to get the parent game object, from the object register, by its ID
					auto parentObject = m_objectRegister.getObject(parentID);

					// Check if the retrieved parent game object is valid
					if(parentObject != nullptr)
					{
						// If it is valid, set it as the parent
						newGameObject2->setParent(*parentObject);
					}
					else
					{
						// If it is not valid, add it to the unassigned parent list, to be set later
						// The game object with the parent ID probably hasn't been loaded yet
						m_unassignedParents.emplace_back(newGameObject2, parentID);
					}
				}
				break;
			case Properties::LocalPosition:
				newGameObject2->m_spatialData.setLocalPosition(p_properties[i].getVec3f());
				break;
			case Properties::LocalRotation:
				newGameObject2->m_spatialData.setLocalRotation(p_properties[i].getVec3f());
				break;
			case Properties::LocalRotationQuaternion:
				newGameObject2->m_spatialData.setLocalRotation(glm::quat(p_properties[i].getVec4f()));
				break;
			case Properties::LocalScale:
				newGameObject2->m_spatialData.setLocalScale(p_properties[i].getVec3f());
				break;
			}
		}

		// Spatial data manager should be updated after setting all the spatial data
		newGameObject2->m_spatialData.update();
		newGameObject2->m_spatialData.resetChanges();

		// Declare data struct for children whose IDs haven't been registered yet
		GameObjectAndChildren unassignedChildren(newGameObject2);

		// Get the children property set array
		auto &children = p_properties.getPropertySetByID(Properties::Children);

		// Iterate over every child entry
		for(decltype(children.getNumPropertySets()) i = 0, size = children.getNumPropertySets(); i < size; i++)
		{
			decltype(GameObject::m_GameObjectID) childID = children.getPropertySet(i).getPropertyByID(Properties::ID).getInt();
								
			// Try to get the child game object, from the object register, by its ID
			auto childObject = m_objectRegister.getObject(childID);

			// Check if the retrieved child game object is valid
			if(childObject != nullptr)
			{
				// If it is valid, set it as the parent
				newGameObject2->addChild(**childObject);
			}
			else
			{
				// If it is not valid, add it to the unassigned children list, to be set later
				// The game object with the child ID probably hasn't been loaded yet
				unassignedChildren.m_children.push_back(childID);
			}
		}

		// If there were any children added to the unassigned children list, add the struct to the main unassigned children list
		if(!unassignedChildren.m_children.empty())
			m_unassignedChildren.emplace_back(unassignedChildren);

		std::vector<std::pair<const EntityID, const PropertySet&>> componentProperties[Systems::NumberOfSystems];

		// Get component entries
		auto &components = p_properties.getPropertySetByID(Properties::Components);
		if(components)
		{
			// Loop over each model entry in the node
			for(decltype(components.getNumPropertySets()) i = 0, size = p_properties.getNumPropertySets(); i < size; i++)
			{
				auto &singleComponent = components.getPropertySet(i);
				switch(singleComponent.getPropertyID())
				{
				case Properties::CameraComponent:
				{
					//componentProperties[Systems::Graphics].push_back()
				}
				break;

				case Properties::ModelComponent:
				{

				}
				break;

				case Properties::ShaderComponent:
				{

				}
				break;

				default:
					break;
				}

				// Get component type
				auto &componentType = p_properties.getPropertySet(i).getPropertyByID(Properties::Filename).getString();
			}
		}


		// Add a GraphicsObject as a component if the Rendering property is present
		auto &rendering = p_properties.getPropertySetByID(Properties::Rendering);
		if(rendering)
		{
			auto graphicsObject = m_sceneLoader->getSystemScene(Systems::Graphics)->createObject(p_properties);
			if(graphicsObject != nullptr && graphicsObject->getSystemType() != Systems::Null)
				newGameObject2->addComponent(static_cast<GraphicsObject *>(graphicsObject));
		}

		// Add a GUIObject as a component if the Rendering property is present
		auto &guiProperty = p_properties.getPropertySetByID(Properties::GUI);
		if(guiProperty)
		{
			auto guiProperty = m_sceneLoader->getSystemScene(Systems::GUI)->createObject(p_properties);
			if(guiProperty != nullptr && guiProperty->getSystemType() != Systems::Null)
				newGameObject2->addComponent(static_cast<GUIObject *>(guiProperty));
		}

		// Add a PhysicsObject as a component if the Script property is present
		auto &physicsProperty = p_properties.getPropertySetByID(Properties::Physics);
		if(physicsProperty)
		{
			auto physicsObject = m_sceneLoader->getSystemScene(Systems::Physics)->createObject(p_properties);
			if(physicsObject != nullptr && physicsObject->getSystemType() != Systems::Null)
				newGameObject2->addComponent(static_cast<PhysicsObject *>(physicsObject));
		}

		// Add a ScriptObject as a component if the Script property is present
		auto &scripting = p_properties.getPropertySetByID(Properties::Script);
		if(scripting)
		{
			auto scriptObject = m_sceneLoader->getSystemScene(Systems::Script)->createObject(p_properties);
			if(scriptObject != nullptr && scriptObject->getSystemType() != Systems::Null)
				newGameObject2->addComponent(static_cast<ScriptObject *>(scriptObject));
		}

	}
	else
	{
		// If valid type was not specified, or object creation failed, assign a null object instead
		//newGameObject2 = g_nullSystemBase.getScene()->createObject(p_properties);
		//ErrHandlerLoc::get().log(ErrorCode::ObjectPool_full, ErrorSource::Source_WorldScene, "Failed to add GameObject - \'" + nameProperty.getString() + "\'");
	}

    return newGameObject2;
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
