
#include "Universal.h"

UniversalScene::UniversalScene(ChangeController *p_sceneChangeController, ChangeController *p_objectChangeController) : 
								Observer(Properties::PropertyID::UniversalScene),
								m_sceneChangeController(p_sceneChangeController), 
								m_objectChangeController(p_objectChangeController)
{

}
UniversalScene::~UniversalScene()
{
	/*for(SystemScenes::const_iterator it = m_systemScenes.begin(); it != m_systemScenes.end(); it++)
	{
		// TODO set status changes to "pre destroying"
	}*/

	// Iterate over the links and unregister them (before clearing the object link list)
	for(ObjectLinkList::iterator it = m_objectLinks.begin(); it != m_objectLinks.end(); it++)
	{
		const ObjectLinkData &linkData = *it;
		m_objectChangeController->unregisterSubject(linkData.m_subject, linkData.m_observer);
	}
	m_objectLinks.clear();

	// Destroy objects before clearing the list
	for(UniversalObjectList::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		destroyObject(*it);
	}
	m_objects.clear();

	// Make a "snapshot" of current list, so we can properly iterate over it (since calling "unextend" removes entries)
	SystemSceneMap systemScenesOld = m_systemScenes;
	for(SystemSceneMap::iterator it = systemScenesOld.begin(); it != systemScenesOld.end(); it++)
	{
		SystemScene *currentSystemScene = it->second;

		// TODO set status changes to "pre destroying"

		unextend(it->second);
	}
	m_systemScenes.clear();
}

SystemScene *UniversalScene::extend(SystemBase *p_system)
{
	// TODO ASSERT ERROR
	_ASSERT(p_system != nullptr);

	// Get the system type
	BitMask systemType = p_system->getSystemType();

	// Check for duplicates
	if(m_systemScenes.find(systemType) == m_systemScenes.end())
	{
		// TODO ERROR
		// System already exists.
		_ASSERT(true);
	}

	// Get the new scene
	SystemScene *newScene = p_system->getScene();

	// TODO ASSERT ERROR
	_ASSERT(newScene != nullptr);

	// Check if the newly created scene is valid
	if(newScene != nullptr)
	{
		// Add the new scene to the list
		m_systemScenes[systemType] = newScene;

		// Register the new scene to receive all changes
		//m_sceneChangeController->registerSubject(newScene, Systems::Changes::Generic::All, this);
		m_sceneChangeController->registerSubject(newScene, newScene->getDesiredSystemChanges(), this);
	}
	else
	{
		// TODO ERROR
	}

	return newScene;
}
ErrorCode UniversalScene::unextend(SystemScene *p_scene)
{
	ErrorCode returnError = ErrorCode::Failure;

	// TODO ASSERT ERROR
	_ASSERT(p_scene != nullptr);

	// Get the new system
	SystemBase *system = p_scene->getSystem();

	// TODO ASSERT ERROR
	_ASSERT(system != nullptr);

	// Get the passed system type
	BitMask systemType = system->getSystemType();

	// Find the scene in the internal list
	SystemSceneMap::iterator it = m_systemScenes.find(systemType);

	// TODO ASSERT ERROR
	_ASSERT(it != m_systemScenes.end());

	// Check if the passed scene was found in the list
	if(it != m_systemScenes.end())
	{
		// If the scene is found, remove it from the list
		m_systemScenes.erase(it);

		// Unregister the subject from the listeners
		// TODO ERROR returned from unregister
		m_sceneChangeController->unregisterSubject(p_scene, this);

		// Destroy the scene and return the error as success
		//system->destroyScene(p_scene);

		returnError = ErrorCode::Success;
	}
	else
	{
		// TODO ERROR
	}

	return returnError;
}

UniversalObject *UniversalScene::createObject(std::string p_name)
{
	// Create the new object
	UniversalObject *newObject = new UniversalObject(this, p_name);

	// TODO ASSERT ERROR
	_ASSERT(newObject != nullptr); 

	// Assign the change controller to the new object
	newObject->m_objectChangeController = m_objectChangeController;

	// Add the new object to the internal list
	m_objects.push_back(newObject);

	// Register the new object with the scene's control manager
	m_sceneChangeController->registerSubject(newObject, Systems::Changes::All, this);

	return newObject;
}
ErrorCode UniversalScene::destroyObject(UniversalObject *p_object)
{
	// Check if the passed object is valid
	// TODO ASSERT ERROR
	_ASSERT(p_object != nullptr);

	// TODO returned error
	m_sceneChangeController->unregisterSubject(p_object, this);
	m_objects.remove(p_object);
	delete p_object;

	return ErrorCode::Success;
}

UniversalObject *UniversalScene::getObject(std::string p_name)
{
	UniversalObject *returnObject = nullptr;

	// Iterate over the internal object list and compare the name
	for(UniversalObjectList::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		// If the name matches, assign the object that was found and break from the loop
		if((*it)->getName() == p_name)
		{
			returnObject = *it;
			break;
		}
	}

	return returnObject;
}

void UniversalScene::createObjectLink(ObservedSubject *p_subject, SystemObject *p_observer)
{
	// Get the correlating potential and desired changes
	BitMask changes = p_subject->getPotentialSystemChanges() & p_observer->getDesiredSystemChanges();

	// If any changes happened
	if(changes)
	{
		m_objectChangeController->registerSubject(p_subject, changes, p_observer);
		
		// Inform the subject about established link
		p_subject->postChanges(Systems::Changes::Generic::Link);
	}
}

void UniversalScene::removeObjectLink(ObservedSubject *p_subject, SystemObject *p_observer)
{
	m_objectChangeController->unregisterSubject(p_subject, p_observer);
}

void UniversalScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changes)
{
	switch(p_changes)
	{
		case Systems::Changes::Generic::CreateObject:
		{
			break;
		}

		case Systems::Changes::Generic::DeleteObject:
		{
			break;
		}

		case Systems::Changes::Generic::ExtendObject:
		{
			break;
		}

		case Systems::Changes::Generic::UnextendObject:
		{
			break;
		}
	}
}

UniversalObject::UniversalObject(UniversalScene *p_universalScene, std::string p_name) : Observer(Properties::PropertyID::UniversalObject), m_scene(p_universalScene)
{
	setName(p_name);
}
UniversalObject::~UniversalObject()
{
	// Iterate over all objects and unextend them before clearing the list
	for(SystemObjectMap::iterator it = m_objectExtensions.begin(); it != m_objectExtensions.end(); it++)
	{
		unextend(it->second->getSystemScene());
	}
	m_objectExtensions.clear();
}

SystemObject *UniversalObject::extend(SystemScene *p_systemScene, PropertySet &p_properties)
{
	/*/ TODO ASSERT ERROR
	_ASSERT(p_systemScene != nullptr);
	_ASSERT(m_objectExtensions.find(p_systemScene->getSystemType()) == m_objectExtensions.end());

	SystemObject *systemObject = p_systemScene->createObject(p_properties);
	_ASSERT(systemObject != nullptr);

	extend(systemObject);

	return systemObject;*/
	return nullptr;
}

bool UniversalObject::extend(SystemObject *p_systemObject)
{
	bool returnSuccession = false;

	// Make sure the passes system object isn't already in the object extension list
	if(m_objectExtensions.find(p_systemObject->getSystemType()) == m_objectExtensions.end())
	{
		p_systemObject->setParent(this);

		BitMask systemObjectPotentialChanges = p_systemObject->getPotentialSystemChanges();
		BitMask systemObjectDesiredChanges = p_systemObject->getDesiredSystemChanges();

		// Register each object with the scenes that care about object's changes
		UniversalScene::SystemSceneMap systemScenes = m_scene->getSystemScenes();
		for(UniversalScene::SystemSceneMap::iterator it = systemScenes.begin(); it != systemScenes.end(); it++)
		{
			SystemScene *currentScene = it->second;
			if(p_systemObject->getPotentialSystemChanges() & currentScene->getDesiredSystemChanges())
			{
				m_objectChangeController->registerSubject(p_systemObject, currentScene->getDesiredSystemChanges(), currentScene);
			}
		}

		// Register each of the systems with each other
		BitMask desiredChanges = p_systemObject->getDesiredSystemChanges();
		for(SystemObjectMap::iterator it = m_objectExtensions.begin(); it != m_objectExtensions.end(); it++)
		{
			SystemObject *currentObject = it->second;

			if(currentObject->getPotentialSystemChanges() & systemObjectDesiredChanges)
				m_objectChangeController->registerSubject(currentObject, desiredChanges, p_systemObject);

			if(systemObjectPotentialChanges & currentObject->getDesiredSystemChanges())
				m_objectChangeController->registerSubject(p_systemObject, currentObject->getDesiredSystemChanges(), currentObject);
		}

		// Add the passed system to the internal extension list
		BitMask systemObjectType = p_systemObject->getSystemType();
		m_objectExtensions[systemObjectType] = p_systemObject;

		// TODO "Set up the speed path for the geometry and graphics objects."

		returnSuccession = true;
	}

	return returnSuccession;
}
void UniversalObject::unextend(SystemScene *p_systemScene)
{
	// TODO ASSERT ERROR
	_ASSERT(p_systemScene != nullptr);

	// Get the system type (required for finding the system in internal list)
	BitMask systemSceneType = p_systemScene->getSystem()->getSystemType();

	// Find the object within extension list
	SystemObjectMap::iterator systemObjectIterator = m_objectExtensions.find(systemSceneType);
	_ASSERT(systemObjectIterator != m_objectExtensions.end());
	// "The object to delete doesn't exist in the scene."
	// TODO ERROR return from the function if the scene doesn't exist
	if(systemObjectIterator == m_objectExtensions.end())
	{
		return;
	}

	SystemObject *systemObject = systemObjectIterator->second;

	// Iterate over all the systems (from extension list) and unregister them with the passed system
	// as both observer and subject. THe change controller will check (internally) if the system was
	// registered and will ignore the "unregisterSubject" call if not.
	for(SystemObjectMap::iterator it = m_objectExtensions.begin(); it != m_objectExtensions.end(); it++)
	{
		SystemObject *currentObject = it->second;

		// Make sure that the systems object isn't unregistering as an observer of itself
		if(systemObject != currentObject)
		{
			m_objectChangeController->unregisterSubject(currentObject, systemObject);
			m_objectChangeController->unregisterSubject(systemObject, currentObject);
		}
	}

	// Unregister each object with the scenes that were interested about the object's changes
	UniversalScene::SystemSceneMap systemScenes = m_scene->getSystemScenes();
	for(UniversalScene::SystemSceneMap::iterator it = systemScenes.begin(); it != systemScenes.end(); it++)
	{
		SystemScene *currentScene = it->second;

		if(systemObject->getPotentialSystemChanges() & currentScene->getDesiredSystemChanges())
		{
			m_objectChangeController->unregisterSubject(systemObject, currentScene);
		}
	}

	// Unregister the system object as an observer of this system
	m_objectChangeController->unregisterSubject(systemObject, this);

	// Destroy the system object before erasing it from the internal list
	p_systemScene->destroyObject(systemObject);

	// Remove the deleted object from the map
	m_objectExtensions.erase(systemObjectIterator);
}

SystemObject *UniversalObject::getExtension(BitMask p_system)
{
	SystemObject *returnSystemObject = nullptr;

	// Try to find the object in the extension list
	SystemObjectMap::iterator objectExtensionIterator = m_objectExtensions.find(p_system);

	// Check if the object is indeed contained in the list
	if(objectExtensionIterator != m_objectExtensions.end())
		returnSystemObject = objectExtensionIterator->second;
	// TODO ERROR WARNING DEBUG

	return returnSystemObject;
}

void UniversalObject::changeOccurred(ObservedSubject *p_subject, BitMask p_changes)
{
	//UNREFERENCED_PARAMETER(p_subject);

	if(p_changes & Systems::Changes::Generic::All)
	{
		p_changes &= Systems::Changes::Generic::All;

		postChanges(p_changes);
	}
}

BitMask UniversalObject::getPotentialSystemChanges()
{
	return Systems::Changes::Generic::All;
}