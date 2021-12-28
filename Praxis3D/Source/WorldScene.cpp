
#include "NullSystemObjects.h"
#include "SceneLoader.h"
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

SystemObject *WorldScene::createObject(const PropertySet &p_properties)
{
	ErrorCode objPoolError = ErrorCode::Failure;
	GameObject *newGameObject = nullptr;
		
	// Get the object name
	auto &nameProperty = p_properties.getPropertyByID(Properties::Name);

	// Find a place for the new object in the pool
	auto gameObjectFromPool = m_gameObjects.newObject();

	// Check if the pool wasn't full
	if(gameObjectFromPool != nullptr)
	{
		std::string name;

		// If the name property is missing, generate a unique name based on the object's index in the pool
		if(nameProperty)
			name = nameProperty.getString();
		else
			name = GetString(Properties::GameObject) + Utilities::toString(gameObjectFromPool->getIndex());

		// Construct the new GameObject
		gameObjectFromPool->construct(this, name, *m_sceneLoader);
		newGameObject = gameObjectFromPool->getObject();
						
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::ID:
				{
					// Get the game object ID
					auto objectIDdesired = static_cast<std::size_t>(p_properties.getPropertyByID(Properties::ID).getInt());

					// Try to register the object with the given game object ID
					auto objectIDactual = m_objectRegister.registerObject(newGameObject, objectIDdesired);

					// If the object wasn't registered with the given game object ID, log an error
					if(objectIDdesired != objectIDactual)
					{
						// If an object already exists at the given game object ID, log a duplicate ID error, otherwise log an invalid ID error
						if(m_objectRegister.getObject(objectIDdesired) != nullptr)
							ErrHandlerLoc::get().log(ErrorCode::Duplicate_object_id, ErrorSource::Source_World, newGameObject->getName());
						else
							ErrHandlerLoc::get().log(ErrorCode::Invalid_object_id, ErrorSource::Source_World, newGameObject->getName());
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
						newGameObject->setParent(*parentObject);
					}
					else
					{
						// If it is not valid, add it to the unassigned parent list, to be set later
						// The game object with the parent ID probably hasn't been loaded yet
						m_unassignedParents.emplace_back(newGameObject, parentID);
					}
				}
				break;
			case Properties::LocalPosition:
				newGameObject->m_spatialData.setLocalPosition(p_properties[i].getVec3f());
				break;
			case Properties::LocalRotation:
				newGameObject->m_spatialData.setLocalRotation(p_properties[i].getVec3f());
				break;
			case Properties::LocalRotationQuaternion:
				newGameObject->m_spatialData.setLocalRotation(Math::Quaternion(p_properties[i].getVec4f()));
				break;
			case Properties::LocalScale:
				newGameObject->m_spatialData.setLocalScale(p_properties[i].getVec3f());
				break;
			}
		}

		// Spatial data manager should be updated after setting all the spatial data
		newGameObject->m_spatialData.update();

		// Declare data struct for children whose IDs haven't been registered yet
		GameObjectAndChildren unassignedChildren(newGameObject);

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
				newGameObject->addChild(**childObject);
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

		auto &rendering = p_properties.getPropertySetByID(Properties::Rendering);
		if(rendering)
		{
			auto graphicsObject = m_sceneLoader->getSystemScene(Systems::Graphics)->createObject(p_properties);
			if(graphicsObject != nullptr)
				newGameObject->addComponent(static_cast<GraphicsObject*>(graphicsObject));
		}
		
		auto &scripting = p_properties.getPropertySetByID(Properties::Scripting);
		if(scripting)
		{
			//auto scriptingObject = m_sceneLoader->getSystemScene(Systems::Scripting)->createObject(p_properties);
			//if(scriptingObject != nullptr)
			//	newGameObject->addComponent(static_cast<ScriptingObject *>(scriptingObject));
		}

	}
	else
	{
		// If valid type was not specified, or object creation failed, assign a null object instead
		//newGameObject = g_nullSystemBase.getScene()->createObject(p_properties);
		ErrHandlerLoc::get().log(ErrorCode::ObjectPool_full, ErrorSource::Source_WorldScene, "Failed to add GameObject - \'" + nameProperty.getString() + "\'");
	}

    return newGameObject;
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
