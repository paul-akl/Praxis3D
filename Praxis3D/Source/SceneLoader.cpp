
#include <utility>
#include <vector>

#include "PropertyLoader.h"
#include "SceneLoader.h"

ErrorCode SceneLoader::loadFromFile(const std::string &p_filename)
{
	// Load properties from file
	PropertyLoader loadedProperties(Config::filepathVar().map_path + p_filename);
	const ErrorCode loaderError = loadedProperties.loadFromFile();

	// Check if loading was successful, return an error, if not
	if(loaderError != ErrorCode::Success)
		return loaderError;

	std::vector<std::pair<const std::string&, SystemObject*>> createdObjects;

	// Get systems property set
	auto &systemProperties = loadedProperties.getPropertySet().getPropertySetByID(Properties::Systems);

	// Iterate over all systems scenes
	for(int sysIndex = 0; sysIndex < Systems::NumberOfSystems; sysIndex++)
	{
		// Create an empty property set, in case there is none in the loaded file, because a system scene setup must be called either way
		PropertySet scenePropertySet;

		// Iterate over each system property set
		for(decltype(systemProperties.getNumPropertySets()) propIndex = 0, propSize = systemProperties.getNumPropertySets(); propIndex < propSize; propIndex++)
		{
			// If the system scene property matches in the loaded file, retrieve it so it can be passed to the corresponding scene
			if(Systems::SystemNames[m_systemScenes[sysIndex]->getSystemType()] == GetString(systemProperties.getPropertySetUnsafe(propIndex).getPropertyID()))
				scenePropertySet = systemProperties.getPropertySetUnsafe(propIndex).getPropertySetByID(Properties::Scene);
		}

		// Pass the scene propertySet parameters
		m_systemScenes[sysIndex]->setup(scenePropertySet);
	}

	// Get Game Objects
	auto &gameObjects = loadedProperties.getPropertySet().getPropertySetByID(Properties::GameObject);

	// Iterate over all game object
	for(decltype(gameObjects.getNumPropertySets()) objIndex = 0, objSize = gameObjects.getNumPropertySets(); objIndex < objSize; objIndex++)
	{
		// Create each game object by passing its PropertySet
		m_systemScenes[Systems::World]->createObject(gameObjects.getPropertySetUnsafe(objIndex));
	}

	/*/ Get the object link property sets
	auto &objLinkProperties = loadedProperties.getPropertySet().getPropertySetByID(Properties::ObjectLinks);

	// Iterate over all object link property sets
	for(decltype(objLinkProperties.getNumPropertySets()) linkIndex = 0, linkSize = objLinkProperties.getNumPropertySets(); linkIndex < linkSize; linkIndex++)
	{
		// Get subject name
		const auto &subjectName = objLinkProperties.getPropertySetUnsafe(linkIndex).getPropertyByID(Properties::Subject).getString();
		if(subjectName.empty()) // Make sure subject's name is not empty
			continue;

		// Get observer name
		const auto &observerName = objLinkProperties.getPropertySetUnsafe(linkIndex).getPropertyByID(Properties::Observer).getString();
		if(observerName.empty()) // Make sure observer's name is not empty
			continue;

		// Iterate over created objects and match subject's name
		for(decltype(createdObjects.size()) subjIndex = 0, subjSize = createdObjects.size(); subjIndex < subjSize; subjIndex++)
		{
			// Compare subject name
			if(createdObjects[subjIndex].first == subjectName)
			{
				// Iterate over created objects and match observer's name
				for(decltype(createdObjects.size()) observIndex = 0, observSize = createdObjects.size(); observIndex < observSize; observIndex++)
				{
					// Compare observer name
					if(createdObjects[observIndex].first == observerName)
					{
						// Create object link between subject and observer in the change controller
						m_changeController->createObjectLink(createdObjects[subjIndex].second, createdObjects[observIndex].second);
						
						// Exit the inner loop
						break;
					}
				}

				// Exit the outer loop
				break;
			}
		}
	}*/

	// Check if the scene should be loaded in background
	if(loadedProperties.getPropertySet().getPropertyByID(Properties::LoadInBackground).getBool())
	{
		// Start loading in background threads in all scenes
		for(int i = 0; i < Systems::NumberOfSystems; i++)
			m_systemScenes[i]->loadInBackground();
	}
	else
	{
		// Preload all scenes sequentially
		for(int i = 0; i < Systems::NumberOfSystems; i++)
			m_systemScenes[i]->preload();
	}

	return ErrorCode::Success;
}

ErrorCode SceneLoader::saveToFile(const std::string p_filename)
{
	std::string filename;

	if(!p_filename.empty())
		filename = p_filename;
	else
		filename = m_filename;

	if(!filename.empty())
	{
		PropertySet propertySet(Properties::Default);

		// Add scene loaded properties
		propertySet.addProperty(Properties::LoadInBackground, m_loadInBackground);
		
		// Add root property set for systems
		auto &systems = propertySet.addPropertySet(Properties::Systems);

		// Add each system's properties
		for(size_t i = 0; i < Systems::NumberOfSystems; i++)
			systems.addPropertySet(m_systemScenes[i]->exportObject());

		// Add root property set for object links
		auto &objLinksPropertySet = propertySet.addPropertySet(Properties::ObjectLinks);

		// Get object links from the change controller
		/*auto &objLinkList = m_changeController->getObjectLinksList();

		for(UniversalScene::ObjectLinkList::const_iterator it = objLinkList.begin(); it != objLinkList.end(); it++)
		{
			auto &objectLinkEntry = objLinksPropertySet.addPropertySet(Properties::ArrayEntry);
			objectLinkEntry.addProperty(Properties::Subject, it->m_observerName);
			objectLinkEntry.addProperty(Properties::Observer, it->m_subjectName);
		}*/

		// Save properties to a file
		PropertyLoader savedProperties(Config::filepathVar().map_path + filename);
		ErrorCode loaderError = savedProperties.saveToFile(propertySet);

		// Check if loading was successful, return an error, if not
		if(loaderError != ErrorCode::Success)
			return loaderError;

		ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_PropertyLoader, "File has been exported: " + filename);
	}

	// TODO ERROR empty filename
	return ErrorCode::Failure;
}
