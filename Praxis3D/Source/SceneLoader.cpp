
#include <utility>
#include <vector>

#include "PropertyLoader.h"
#include "SceneLoader.h"

ErrorCode SceneLoader::loadFromFile(const std::string &p_filename)
{
	if(!p_filename.empty())
	{
		m_filename = p_filename;

		// Load properties from file
		PropertyLoader loadedProperties(Config::filepathVar().map_path + m_filename);
		ErrorCode loaderError = loadedProperties.loadFromFile();

		// Check if loading was successful, return an error, if not
		if(loaderError != ErrorCode::Success)
			return loaderError;

		// Get systems property set
		auto &systemProperties = loadedProperties.getPropertySet().getPropertySetByID(Properties::Systems);

		// Iterate over each system property set
		for(decltype(systemProperties.getNumPropertySets()) propIndex = 0, propSize = systemProperties.getNumPropertySets(); propIndex < propSize; propIndex++)
		{
			// Iterate over all systems scenes
			for(int sysIndex = 0; sysIndex < Systems::NumberOfSystems; sysIndex++)
			{
				// If system scene name and property set name match
				if(Systems::SystemNames[m_systemScenes[sysIndex]->getSystemType()]
				   == GetString(systemProperties.getPropertySetUnsafe(propIndex).getPropertyID()))
				{
					// Pass the 'scene' property set to the matched system scene
					m_systemScenes[sysIndex]->setup(systemProperties.getPropertySetUnsafe(propIndex).getPropertySetByID(Properties::Scene));

					// Get 'objects' property set
					auto &objectProperties = systemProperties.getPropertySetUnsafe(propIndex).getPropertySetByID(Properties::Objects);

					// Iterate over all object property sets
					for(decltype(objectProperties.getNumPropertySets()) objIndex = 0, objSize = objectProperties.getNumPropertySets(); objIndex < objSize; objIndex++)
					{
						// Create a new system object (by pasting the object property set)
						auto *newObject = m_systemScenes[sysIndex]->createObject(objectProperties.getPropertySetUnsafe(objIndex));

						// Save the newly created object for later linking
						m_createdObjects[sysIndex].push_back(std::pair<const std::string&, SystemObject*>(newObject->getName(), newObject));
					}
				}
			}
		}

		// Get the object link property sets
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
			for(int subjSysIndex = 0; subjSysIndex < Systems::NumberOfSystems; subjSysIndex++)
			{
				for(decltype(m_createdObjects[subjSysIndex].size()) subjIndex = 0, subjSize = m_createdObjects[subjSysIndex].size(); subjIndex < subjSize; subjIndex++)
				{
					// Compare subject name
					if(m_createdObjects[subjSysIndex][subjIndex].first == subjectName)
					{
						// Iterate over created objects and match observer's name
						for(int observSysIndex = 0; observSysIndex < Systems::NumberOfSystems; observSysIndex++)
						{
							for(decltype(m_createdObjects[observSysIndex].size()) observIndex = 0, observSize = m_createdObjects[observSysIndex].size(); observIndex < observSize; observIndex++)
							{
								// Compare observer name
								if(m_createdObjects[observSysIndex][observIndex].first == observerName)
								{
									// Create object link between subject and observer in the change controller
									m_changeController->createObjectLink(m_createdObjects[subjSysIndex][subjIndex].second, m_createdObjects[observSysIndex][observIndex].second);

									// Exit the inner loop
									break;
								}
							}
						}

						// Exit the outer loop
						break;
					}
				}
			}
		}

		// Get flag telling if the objects should be loaded in parallel
		m_loadInBackground = loadedProperties.getPropertySet().getPropertyByID(Properties::LoadInBackground).getBool();

		// If the objects should be loaded in background threads
		if(m_loadInBackground)
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

		ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_PropertyLoader, "Map file has been loaded: " + m_filename);

		return ErrorCode::Success;
	}

	// TODO ERROR empty filename
	return ErrorCode::Failure;
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
		auto &objLinkList = m_changeController->getObjectLinksList();

		for(UniversalScene::ObjectLinkList::const_iterator it = objLinkList.begin(); it != objLinkList.end(); it++)
		{
			auto &objectLinkEntry = objLinksPropertySet.addPropertySet(Properties::ArrayEntry);
			objectLinkEntry.addProperty(Properties::Subject, it->m_observerName);
			objectLinkEntry.addProperty(Properties::Observer, it->m_subjectName);
		}

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
