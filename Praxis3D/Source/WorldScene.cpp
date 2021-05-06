
#include "NullSystemObjects.h"
#include "SceneLoader.h"
#include "WorldScene.h"

SystemObject *WorldScene::createObject(const PropertySet &p_properties)
{
	ErrorCode objPoolError = ErrorCode::Failure;
	SystemObject *returnObject = nullptr;

	// Try to add a new object to the pool
	objPoolError = m_gameObjects.add(this, p_properties.getPropertyByID(Properties::Name).getString(), &m_sceneLoader);
	
	// If adding a new object was successful, continue to load data
	if(objPoolError == ErrorCode::Success)
	{
		// The newly added object in the pool
		auto newGameObject = m_gameObjects.getLastRawObject();
				
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
					decltype(GameObject::m_id) parentID = p_properties[i].getInt();

					// Try to get the parent game object, from the object register, by its ID
					auto parentObject = m_objectRegister.getObject(parentID);

					// Check if the retrieved parent game object is valid
					if(parentObject != nullptr)
					{
						// If it is valid, set it as the parent
						newGameObject->setParent(parentObject);
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
				newGameObject->m_localSpace.m_position = p_properties[i].getVec3f();
				break;
			case Properties::LocalRotation:
				newGameObject->m_localSpace.m_rotationEuler = p_properties[i].getVec3f();
				break;
			case Properties::LocalRotationQuaternion:
				newGameObject->m_localSpace.m_rotationQuat = p_properties[i].getVec4f();
				break;
			case Properties::LocalScale:
				newGameObject->m_localSpace.m_scale = p_properties[i].getVec3f();
				break;
			}
		}

		// Declare data struct for children whose IDs haven't been registered yet
		GameObjectAndChildren unassignedChildren(newGameObject);

		// Get the children property set array
		auto &children = p_properties.getPropertySetByID(Properties::Children);

		// Iterate over every child entry
		for(decltype(children.getNumPropertySets()) i = 0, size = children.getNumPropertySets(); i < size; i++)
		{
			decltype(GameObject::m_id) childID = children.getPropertySet(i).getPropertyByID(Properties::ID).getInt();
								
			// Try to get the child game object, from the object register, by its ID
			auto childObject = m_objectRegister.getObject(childID);

			// Check if the retrieved child game object is valid
			if(childObject != nullptr)
			{
				// If it is valid, set it as the parent
				newGameObject->addChild(*childObject);
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
			m_sceneLoader->getSystemScene(Systems::Graphics);
		}
		
		auto &scripting = p_properties.getPropertySetByID(Properties::Scripting);
		if(scripting)
		{

		}

		returnObject = newGameObject;
	}
	else
	{
		// If valid type was not specified, or object creation failed, assign a null object instead
		returnObject = g_nullSystemBase.getScene()->createObject(p_properties);
	}

    return returnObject;
}
