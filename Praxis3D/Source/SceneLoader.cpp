
#include <utility>
#include <vector>

#include "ComponentConstructorInfo.h"
#include "PropertyLoader.h"
#include "SceneLoader.h"

SceneLoader::SceneLoader()
{
	m_changeController = nullptr;
	m_loadInBackground = false;

	for(int i = 0; i < Systems::NumberOfSystems; i++)
		m_systemScenes[i] = g_nullSystemBase.createScene(this, EngineStateType::EngineStateType_Default);
}

SceneLoader::~SceneLoader()
{
}

ErrorCode SceneLoader::loadFromFile(const std::string &p_filename)
{
	ErrorCode returnError = ErrorCode::Success;
	m_filename = p_filename;

	EntitiesConstructionInfo constructionInfo;

	// Load properties from file
	PropertyLoader loadedProperties(Config::filepathVar().map_path + p_filename);
	returnError = loadedProperties.loadFromFile();

	// Check if loading was successful, return an error, if not
	if(returnError != ErrorCode::Success)
	{
		ErrHandlerLoc().get().log(returnError, ErrorSource::Source_SceneLoader);
	}
	else
	{
		// Get systems property set
		auto &systemProperties = loadedProperties.getPropertySet().getPropertySetByID(Properties::Systems);

		// Iterate over all systems scenes
		for(int sysIndex = 0; sysIndex < Systems::NumberOfSystems; sysIndex++)
		{
			// Create an empty property set, in case there is none in the loaded file, because a system scene setup must be called either way
			PropertySet scenePropertySet;
			PropertySet systemropertySet;

			// Iterate over each system property set
			for(decltype(systemProperties.getNumPropertySets()) propIndex = 0, propSize = systemProperties.getNumPropertySets(); propIndex < propSize; propIndex++)
			{
				// If the system scene property matches in the loaded file, retrieve it so it can be passed to the corresponding scene
				if(Systems::SystemNames[m_systemScenes[sysIndex]->getSystemType()] == GetString(systemProperties.getPropertySetUnsafe(propIndex).getPropertyID()))
				{
					scenePropertySet = systemProperties.getPropertySetUnsafe(propIndex).getPropertySetByID(Properties::Scene);
					systemropertySet = systemProperties.getPropertySetUnsafe(propIndex).getPropertySetByID(Properties::System);
				}
			}

			// Pass the scene and system propertySet parameters
			m_systemScenes[sysIndex]->getSystem()->setup(systemropertySet);
			m_systemScenes[sysIndex]->setup(scenePropertySet);
		}

		// Get Game Objects
		auto &gameObjects = loadedProperties.getPropertySet().getPropertySetByID(Properties::GameObject);
		if(gameObjects)
		{
			// Reserve enough room for all the game objects
			constructionInfo.resize(gameObjects.getNumPropertySets());

			// Iterate over all game objects
			for(decltype(gameObjects.getNumPropertySets()) objIndex = 0, objSize = gameObjects.getNumPropertySets(); objIndex < objSize; objIndex++)
			{
				// Import the game object data from PropertySets to EntitiesConstructionInfo
				importFromProperties(constructionInfo[objIndex], gameObjects.getPropertySetUnsafe(objIndex));
			}

			// Get the world scene required for creating entities
			WorldScene *worldScene = static_cast<WorldScene*>(m_systemScenes[Systems::World]);

			// Go over each entity and create it
			for(decltype(constructionInfo.size()) i = 0, size = constructionInfo.size(); i < size; i++)
			{
				worldScene->createEntity(constructionInfo[i], false);
			}
		}
		else
		{
			// GameObject property set is missing
			returnError = ErrorCode::GameObjects_missing;
			ErrHandlerLoc().get().log(ErrorCode::GameObjects_missing, ErrorSource::Source_SceneLoader);
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
	}

	// Make sure to clear the memory of contructionInfo
	for(decltype(constructionInfo.size()) i = 0, size = constructionInfo.size(); i < size; i++)
		constructionInfo[i].deleteConstructionInfo();

	return returnError;
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
		// Get the world scene required for getting the entity registry
		WorldScene *worldScene = static_cast<WorldScene *>(m_systemScenes[Systems::World]);

		// Get the entity registry 
		auto &entityRegistry = worldScene->getEntityRegistry();

		// Add root property set for the whole file
		PropertySet rootPropertySet(Properties::Default);

		// Add root property set game objects
		auto &gameObjects = rootPropertySet.addPropertySet(Properties::GameObject);

		// An array holding all of the entities
		std::vector<EntityID> allEntities;

		// Add all entities to the array
		entityRegistry.each([&](auto entity)
			{
				allEntities.push_back(entity);
			});

		// Sort the entities so they are written to file in order
		std::sort(allEntities.begin(), allEntities.end());

		// Iterate every entity
		for(auto &entity : allEntities)
		{
			// Create an array entry for the entity
			auto &gameObjectEntry = gameObjects.addPropertySet(Properties::ArrayEntry);

			// Export the entity to the Construction Info
			ComponentsConstructionInfo constructionInfo;
			worldScene->exportEntity(entity, constructionInfo);

			// Export the Construction Info to the Property Set
			exportToProperties(constructionInfo, gameObjectEntry);
		}

		// Add scene loaded properties
		rootPropertySet.addProperty(Properties::LoadInBackground, m_loadInBackground);

		// Add root property set for systems
		auto &rootSystemsPropertySet = rootPropertySet.addPropertySet(Properties::Systems);

		// Add each system's properties
		for(size_t systemType = 0; systemType < Systems::NumberOfSystems; systemType++)
		{
			// Convert TypeID to PropertyID
			Properties::PropertyID systemTypeProperty = Properties::Null;
			switch(systemType)
			{
				case Systems::TypeID::Audio:
					systemTypeProperty = Properties::Audio;
					break;
				case Systems::TypeID::Graphics:
					systemTypeProperty = Properties::Graphics;
					break;
				case Systems::TypeID::GUI:
					systemTypeProperty = Properties::GUI;
					break;
				case Systems::TypeID::Physics:
					systemTypeProperty = Properties::Physics;
					break;
				case Systems::TypeID::Script:
					systemTypeProperty = Properties::Script;
					break;
				case Systems::TypeID::World:
					systemTypeProperty = Properties::World;
					break;
			}

			// Add system type entry
			auto &systemPropertyIDentry = rootSystemsPropertySet.addPropertySet(systemTypeProperty);

			// Add scene and system settings
			m_systemScenes[systemType]->exportSetup(systemPropertyIDentry.addPropertySet(Properties::Scene));
			m_systemScenes[systemType]->getSystem()->exportSetup(systemPropertyIDentry.addPropertySet(Properties::System));
		}

		// Save properties to a file
		PropertyLoader savedProperties(filename);
		ErrorCode loaderError = savedProperties.saveToFile(rootPropertySet);

		// Check if loading was successful, return an error, if not
		if(loaderError != ErrorCode::Success)
			return loaderError;

		ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_PropertyLoader, "File has been exported: " + filename);
	}

	// TODO ERROR empty filename
	return ErrorCode::Failure;
}

ErrorCode SceneLoader::importPrefab(ComponentsConstructionInfo &p_constructionInfo, const std::string &p_filename, const bool p_forceReload)
{
	ErrorCode returnError = ErrorCode::Success;

	// Check if the given filename isn't empty
	if(!p_filename.empty())
	{
		// Search for the given prefab (it might have been loaded before, already)
		auto prefabIterator = m_prefabs.find(p_filename);

		// If the prefab was already imported and exists in the map, use it
		if(prefabIterator != m_prefabs.end())
		{
			if(p_forceReload)
				returnError = importFromFile(p_constructionInfo, Config::filepathVar().prefab_path + p_filename);
			else
				p_constructionInfo.completeCopy(prefabIterator->second);
		}
		else // If the prefab doesn't exist in the map, import it
		{
			// Make sure calls from other threads are locked, while current call is in progress
			// This is needed as the prefab that is being requested might be currently being imported
			// Mutex prevents duplicate prefabs being loaded, and same data being changed.
			SpinWait::Lock lock(m_mutex);

			// Search for the prefab again, as it might have been imported from another thread call before mutex lock was released
			auto prefabIteratorNew = m_prefabs.find(p_filename);
			if(prefabIteratorNew != m_prefabs.end())
			{
				if(p_forceReload)
					returnError = importFromFile(p_constructionInfo, Config::filepathVar().prefab_path + p_filename);
				else
					p_constructionInfo.completeCopy(prefabIterator->second);
			}
			else
			{
				// Load properties from file
				PropertyLoader loadedProperties(Config::filepathVar().prefab_path + p_filename);
				returnError = loadedProperties.loadFromFile();

				if(returnError == ErrorCode::Success)
				{
					// Insert a new prefab into the map
					ComponentsConstructionInfo &constructionInfo = m_prefabs.try_emplace(p_filename).first->second;

					// Populate the newly imported prefab
					importFromProperties(constructionInfo, loadedProperties.getPropertySet());

					p_constructionInfo.completeCopy(constructionInfo);
				}
			}
		}
	}
	else
		returnError = ErrorCode::Filename_empty;

	return returnError;
}

ErrorCode SceneLoader::importFromFile(ComponentsConstructionInfo &p_constructionInfo, const std::string &p_filename)
{
	ErrorCode returnError = ErrorCode::Success;

	// Load properties from file
	PropertyLoader loadedProperties(p_filename);
	returnError = loadedProperties.loadFromFile();

	if(returnError == ErrorCode::Success)
		importFromProperties(p_constructionInfo, loadedProperties.getPropertySet());

	return returnError;
}

void SceneLoader::importFromProperties(ComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties)
{
	// Get the object name and ID
	std::string name = p_properties.getPropertyByID(Properties::Name).getString();
	EntityID desiredEntityID = (EntityID)p_properties.getPropertyByID(Properties::ID).getInt();

	// If the name property is missing, generate a unique name based on the entity ID
	if(name.empty())
		name = GetString(Properties::GameObject) + Utilities::toString(desiredEntityID);

	p_constructionInfo.m_name = name;
	p_constructionInfo.m_id = desiredEntityID;

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
			case Properties::Parent:
				{
					// Get the entity ID if the parent object
					p_constructionInfo.m_parent = (EntityID)p_properties[i].getInt();
				}
				break;

			case Properties::Prefab:
				{
					std::string prefabName = p_properties[i].getString();
					importPrefab(p_constructionInfo, prefabName);
					p_constructionInfo.m_prefab = prefabName;
				}
				break;
		}
	}

	// Load audio components
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::Audio);
		if(sceneProperty)
		{
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				importFromProperties(p_constructionInfo.m_audioComponents, sceneProperty.getPropertySet(i), name);
			}
		}
	}

	// Load graphics components
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::Rendering);
		if(sceneProperty)
		{
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				importFromProperties(p_constructionInfo.m_graphicsComponents, sceneProperty.getPropertySet(i), name);
			}
		}
	}

	// Load GUI components
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::GUI);
		if(sceneProperty)
		{
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				importFromProperties(p_constructionInfo.m_guiComponents, sceneProperty.getPropertySet(i), name);
			}
		}
	}

	// Load physics components
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::Physics);
		if(sceneProperty)
		{
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				importFromProperties(p_constructionInfo.m_physicsComponents, sceneProperty.getPropertySet(i), name);
			}
		}
	}

	// Load script components
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::Script);
		if(sceneProperty)
		{
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				importFromProperties(p_constructionInfo.m_scriptComponents, sceneProperty.getPropertySet(i), name);
			}
		}
	}

	// Load world components
	{
		auto &sceneProperty = p_properties.getPropertySetByID(Properties::World);
		if(sceneProperty)
		{
			for(decltype(sceneProperty.getNumPropertySets()) i = 0, size = sceneProperty.getNumPropertySets(); i < size; i++)
			{
				importFromProperties(p_constructionInfo.m_worldComponents, sceneProperty.getPropertySet(i), name);
			}
		}
	}
}

void SceneLoader::importFromProperties(AudioComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name)
{
	// Check if property set node is present
	if(p_properties)
	{
		switch(p_properties.getPropertyID())
		{
			case Properties::PropertyID::SoundComponent:
			{
				if(p_constructionInfo.m_soundConstructionInfo == nullptr)
					p_constructionInfo.m_soundConstructionInfo = new SoundComponent::SoundComponentConstructionInfo();

				p_constructionInfo.m_soundConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::SoundComponent);

				// Get the sound filename
				auto const &filename = p_properties.getPropertyByID(Properties::Filename).getString();

				if(!filename.empty())
				{
					// Get the sound type
					auto const &type = p_properties.getPropertyByID(Properties::Type).getID();

					// Load values based on the type of sound
					switch(type)
					{
					case Properties::Ambient:

						p_constructionInfo.m_soundConstructionInfo->m_soundType = SoundComponent::SoundType::SoundType_Ambient;

						p_properties.getValueByID(Properties::Loop, p_constructionInfo.m_soundConstructionInfo->m_loop);
						p_properties.getValueByID(Properties::Spatialized, p_constructionInfo.m_soundConstructionInfo->m_spatialized);
						p_properties.getValueByID(Properties::StartPlaying, p_constructionInfo.m_soundConstructionInfo->m_startPlaying);
						p_properties.getValueByID(Properties::Volume, p_constructionInfo.m_soundConstructionInfo->m_volume);
						p_constructionInfo.m_soundConstructionInfo->m_soundFilename = filename;

						break;

					case Properties::Music:

						p_constructionInfo.m_soundConstructionInfo->m_soundType = SoundComponent::SoundType::SoundType_Music;

						p_properties.getValueByID(Properties::Loop, p_constructionInfo.m_soundConstructionInfo->m_loop);
						p_properties.getValueByID(Properties::Spatialized, p_constructionInfo.m_soundConstructionInfo->m_spatialized);
						p_properties.getValueByID(Properties::StartPlaying, p_constructionInfo.m_soundConstructionInfo->m_startPlaying);
						p_properties.getValueByID(Properties::Volume, p_constructionInfo.m_soundConstructionInfo->m_volume);
						p_constructionInfo.m_soundConstructionInfo->m_soundFilename = filename;

						break;

					case Properties::SoundEffect:

						p_constructionInfo.m_soundConstructionInfo->m_soundType = SoundComponent::SoundType::SoundType_SoundEffect;

						p_properties.getValueByID(Properties::Loop, p_constructionInfo.m_soundConstructionInfo->m_loop);
						p_properties.getValueByID(Properties::Spatialized, p_constructionInfo.m_soundConstructionInfo->m_spatialized);
						p_properties.getValueByID(Properties::StartPlaying, p_constructionInfo.m_soundConstructionInfo->m_startPlaying);
						p_properties.getValueByID(Properties::Volume, p_constructionInfo.m_soundConstructionInfo->m_volume);
						p_constructionInfo.m_soundConstructionInfo->m_soundFilename = filename;

						break;

					default:

						ErrHandlerLoc().get().log(ErrorCode::Property_missing_type, p_name, ErrorSource::Source_SoundComponent);
						delete p_constructionInfo.m_soundConstructionInfo;
						p_constructionInfo.m_soundConstructionInfo = nullptr;

						break;
					}
				}
				else
				{
					ErrHandlerLoc().get().log(ErrorCode::Property_no_filename, p_name, ErrorSource::Source_SoundComponent);
					delete p_constructionInfo.m_soundConstructionInfo;
					p_constructionInfo.m_soundConstructionInfo = nullptr;
				}
			}
			break;

			case Properties::PropertyID::SoundListenerComponent:
			{
				if(p_constructionInfo.m_soundListenerConstructionInfo == nullptr)
					p_constructionInfo.m_soundListenerConstructionInfo = new SoundListenerComponent::SoundListenerComponentConstructionInfo();

				p_constructionInfo.m_soundListenerConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::SoundListenerComponent);

				p_properties.getValueByID(Properties::PropertyID::ID, p_constructionInfo.m_soundListenerConstructionInfo->m_listenerID);
				p_properties.getValueByID(Properties::PropertyID::Active, p_constructionInfo.m_soundListenerConstructionInfo->m_active);
			}
			break;
		}
	}
}

void SceneLoader::importFromProperties(GraphicsComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name)
{
	// Check if property set node is present
	if(p_properties)
	{
		switch(p_properties.getPropertyID())
		{
			case Properties::PropertyID::CameraComponent:
			{
				if(p_constructionInfo.m_cameraConstructionInfo == nullptr)
					p_constructionInfo.m_cameraConstructionInfo = new CameraComponent::CameraComponentConstructionInfo();

				p_constructionInfo.m_cameraConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::CameraComponent);
			}
			break;

			case Properties::PropertyID::LightComponent:
			{
				if(p_constructionInfo.m_lightConstructionInfo == nullptr)
					p_constructionInfo.m_lightConstructionInfo = new LightComponent::LightComponentConstructionInfo();

				p_constructionInfo.m_lightConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::LightComponent);

				// Get the light type
				auto const &type = p_properties.getPropertyByID(Properties::Type).getID();

				// Load values based on the type of light
				switch(type)
				{
				case Properties::DirectionalLight:

					p_constructionInfo.m_lightConstructionInfo->m_lightComponentType = LightComponent::LightComponentType::LightComponentType_directional;

					p_properties.getValueByID(Properties::Color, p_constructionInfo.m_lightConstructionInfo->m_color);
					p_properties.getValueByID(Properties::Intensity, p_constructionInfo.m_lightConstructionInfo->m_intensity);

					break;

				case Properties::PointLight:

					p_constructionInfo.m_lightConstructionInfo->m_lightComponentType = LightComponent::LightComponentType::LightComponentType_point;

					p_properties.getValueByID(Properties::Color, p_constructionInfo.m_lightConstructionInfo->m_color);
					p_properties.getValueByID(Properties::Intensity, p_constructionInfo.m_lightConstructionInfo->m_intensity);

					break;

				case Properties::SpotLight:

					p_constructionInfo.m_lightConstructionInfo->m_lightComponentType = LightComponent::LightComponentType::LightComponentType_spot;

					p_properties.getValueByID(Properties::Color, p_constructionInfo.m_lightConstructionInfo->m_color);
					p_properties.getValueByID(Properties::CutoffAngle, p_constructionInfo.m_lightConstructionInfo->m_cutoffAngle);
					p_properties.getValueByID(Properties::Intensity, p_constructionInfo.m_lightConstructionInfo->m_intensity);

					break;

				default:

					ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LightComponent, p_name + " - missing \'Type\' identifier");
					delete p_constructionInfo.m_lightConstructionInfo;
					p_constructionInfo.m_lightConstructionInfo = nullptr;

					break;
				}
			}
			break;

			case Properties::PropertyID::ModelComponent:
			{
				if(p_constructionInfo.m_modelConstructionInfo == nullptr)
					p_constructionInfo.m_modelConstructionInfo = new ModelComponent::ModelComponentConstructionInfo();

				p_constructionInfo.m_modelConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::ModelComponent);

				auto &modelsProperty = p_properties.getPropertySetByID(Properties::Models);

				if(modelsProperty)
				{
					// Loop over each model entry in the node
					for(decltype(modelsProperty.getNumPropertySets()) iModel = 0, numModels = modelsProperty.getNumPropertySets(); iModel < numModels; iModel++)
					{
						// Get model filename
						auto modelName = modelsProperty.getPropertySet(iModel).getPropertyByID(Properties::Filename).getString();

						// Add a new model data entry, and get a reference to it
						p_constructionInfo.m_modelConstructionInfo->m_modelsProperties.m_modelNames.push_back(modelName);

						auto &meshesProperty = modelsProperty.getPropertySet(iModel).getPropertySetByID(Properties::Meshes);

						// Check if the meshes array node is present;
						// If it is present, only add the meshes included in the meshes node
						// If it is not present, add all the meshes included in the model
						if(meshesProperty)
						{
							if(meshesProperty.getNumPropertySets() > 0)
							{
								// Loop over each mesh entry in the model node
								for(decltype(meshesProperty.getNumPropertySets()) iMesh = 0, numMeshes = meshesProperty.getNumPropertySets(); iMesh < numMeshes; iMesh++)
								{
									// Try to get the mesh index property node and check if it is present
									auto &meshIndexProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::Index);
									if(meshIndexProperty)
									{
										// Get the mesh index, check if it is valid and within the range of mesh array that was loaded from the model
										const int meshDataIndex = meshIndexProperty.getInt();

										// Make sure the meshMaterials vector can fit the given mesh index
										if(meshDataIndex >= p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_meshMaterials.size())
										{
											p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.resize(meshDataIndex + 1);
											p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_present[meshDataIndex] = true;
										}

										// Get material alpha threshold value, if it is present
										auto alphaThresholdProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::AlphaThreshold);
										if(alphaThresholdProperty)
											p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_alphaThreshold[meshDataIndex] = alphaThresholdProperty.getFloat();

										// Get material height scale value, if it is present
										auto heightScaleProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::HeightScale);
										if(heightScaleProperty)
											p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_heightScale[meshDataIndex] = heightScaleProperty.getFloat();

										// Get material properties
										auto materialsProperty = meshesProperty.getPropertySet(iMesh).getPropertySetByID(Properties::Materials);

										// Define material data and material properties
										MaterialData materials[MaterialType::MaterialType_NumOfTypes];
										PropertySet materialProperties[MaterialType::MaterialType_NumOfTypes] =
										{
											materialsProperty.getPropertySetByID(Properties::Diffuse),
											materialsProperty.getPropertySetByID(Properties::Normal),
											materialsProperty.getPropertySetByID(Properties::Emissive),
											materialsProperty.getPropertySetByID(Properties::RMHAO)
										};

										// Go over each material type
										for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
										{
											// Check if an entry for the current material type was present within the properties
											if(materialProperties[iMatType])
											{
												// Get texture filename property, check if it is valid
												auto filenameProperty = materialProperties[iMatType].getPropertyByID(Properties::Filename);
												if(filenameProperty.isVariableTypeString())
												{
													// Get texture filename string, check if it is valid
													p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_meshMaterials[meshDataIndex][iMatType] = filenameProperty.getString();
												}

												// Get texture scale property, check if it is valid
												auto scaleProperty = materialProperties[iMatType].getPropertyByID(Properties::TextureScale);
												if(scaleProperty)
													p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_meshMaterialsScale[meshDataIndex][iMatType] = scaleProperty.getVec2f();
											}
										}
									}
								}
							}
						}
					}
				}

				if(p_properties.getNumPropertySets() == 0)
					ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ModelComponent, p_name + " - missing model data");
			}
			break;

			case Properties::PropertyID::ShaderComponent:
			{
				if(p_constructionInfo.m_shaderConstructionInfo == nullptr)
					p_constructionInfo.m_shaderConstructionInfo = new ShaderComponent::ShaderComponentConstructionInfo();

				p_constructionInfo.m_shaderConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::ShaderComponent);

				// Get nodes for different shader types
				auto geometryShaderNode = p_properties.getPropertyByID(Properties::GeometryShader);
				auto vertexShaderNode = p_properties.getPropertyByID(Properties::VertexShader);
				auto fragmentShaderNode = p_properties.getPropertyByID(Properties::FragmentShader);

				if(geometryShaderNode)
					p_constructionInfo.m_shaderConstructionInfo->m_geometryShaderFilename = geometryShaderNode.getString();

				if(vertexShaderNode)
					p_constructionInfo.m_shaderConstructionInfo->m_vetexShaderFilename = vertexShaderNode.getString();

				if(fragmentShaderNode)
					p_constructionInfo.m_shaderConstructionInfo->m_fragmentShaderFilename = fragmentShaderNode.getString();
			}
			break;
		}
	}
}

void SceneLoader::importFromProperties(GUIComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name)
{
	// Check if property set node is present
	if(p_properties)
	{
		switch(p_properties.getPropertyID())
		{
			case Properties::PropertyID::Sequence:
			{
				if(p_constructionInfo.m_guiSequenceConstructionInfo == nullptr)
					p_constructionInfo.m_guiSequenceConstructionInfo = new GUISequenceComponent::GUISequenceComponentConstructionInfo();

				p_constructionInfo.m_guiSequenceConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::GUISequenceComponent);

			}
			break;
		}
	}
}

void SceneLoader::importFromProperties(PhysicsComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name)
{
	// Check if property set node is present
	if(p_properties)
	{
		switch(p_properties.getPropertyID())
		{
		case Properties::PropertyID::RigidBodyComponent:
		{
			if(p_constructionInfo.m_rigidBodyConstructionInfo == nullptr)
				p_constructionInfo.m_rigidBodyConstructionInfo = new RigidBodyComponent::RigidBodyComponentConstructionInfo();

			p_constructionInfo.m_rigidBodyConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::RigidBodyComponent);

			// --------------------
			// Load collision shape
			// --------------------
			auto const &collisionShapeProperty = p_properties.getPropertySetByID(Properties::CollisionShape);
			if(collisionShapeProperty)
			{
				// Get the type of the collision shape and load the data based on it
				auto const &typeProperty = collisionShapeProperty.getPropertyByID(Properties::Type);
				if(typeProperty)
				{
					switch(typeProperty.getID())
					{
					case Properties::Box:
					{
						// Get the size property
						auto const &sizeProperty = collisionShapeProperty.getPropertyByID(Properties::Size);

						// If the size was not given, leave it to a default 0.5f all around (so it gets to the final 1.0/1.0/1.0 dimension)
						if(sizeProperty)
							p_constructionInfo.m_rigidBodyConstructionInfo->m_collisionShapeSize = sizeProperty.getVec3f();
						else
							ErrHandlerLoc().get().log(ErrorCode::Property_missing_size, p_name, ErrorSource::Source_RigidBodyComponent);

						p_constructionInfo.m_rigidBodyConstructionInfo->m_collisionShapeType = RigidBodyComponent::CollisionShapeType::CollisionShapeType_Box;
					}
					break;

					case Properties::Sphere:
					{
						// Get the size property
						auto const &radiusProperty = collisionShapeProperty.getPropertyByID(Properties::Radius);

						// If the size was not given, leave it to a default radius of 0.5f (which makes the sphere diameter equal to 1.0)
						if(radiusProperty)
							p_constructionInfo.m_rigidBodyConstructionInfo->m_collisionShapeSize.x = radiusProperty.getFloat();
						else
							ErrHandlerLoc().get().log(ErrorCode::Property_missing_radius, p_name, ErrorSource::Source_RigidBodyComponent);

						p_constructionInfo.m_rigidBodyConstructionInfo->m_collisionShapeType = RigidBodyComponent::CollisionShapeType::CollisionShapeType_Sphere;
					}
					break;

					default:

						// If this is reached, the collision shape type was not valid
						ErrHandlerLoc().get().log(ErrorCode::Collision_invalid, p_name, ErrorSource::Source_RigidBodyComponent);
						break;
					}
				}
				else
				{
					// Missing the Type property entirely
					ErrHandlerLoc().get().log(ErrorCode::Property_missing_type, p_name, ErrorSource::Source_RigidBodyComponent);
				}
			}

			// -----------------------------
			// Load individual property data
			// -----------------------------
			for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
			{
				switch(p_properties[i].getPropertyID())
				{
				case Properties::Friction:
					p_constructionInfo.m_rigidBodyConstructionInfo->m_friction = p_properties[i].getFloat();
					break;
				case Properties::Kinematic:
					p_constructionInfo.m_rigidBodyConstructionInfo->m_kinematic = p_properties[i].getBool();
					break;
				case Properties::Mass:
					p_constructionInfo.m_rigidBodyConstructionInfo->m_mass = p_properties[i].getFloat();
					break;
				case Properties::Restitution:
					p_constructionInfo.m_rigidBodyConstructionInfo->m_restitution = p_properties[i].getFloat();
					break;
				case Properties::Velocity:
					p_constructionInfo.m_rigidBodyConstructionInfo->m_linearVelocity = p_properties[i].getVec3f();
					break;
				}
			}
		}

		break;
		}
	}
}

void SceneLoader::importFromProperties(ScriptComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name)
{
	// Check if property set node is present
	if(p_properties)
	{
		switch(p_properties.getPropertyID())
		{
			case Properties::PropertyID::LuaComponent:
			{
				if(p_constructionInfo.m_luaConstructionInfo == nullptr)
					p_constructionInfo.m_luaConstructionInfo = new LuaComponent::LuaComponentConstructionInfo();

				p_constructionInfo.m_luaConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::LuaComponent);

				auto const &luaFilenameProperty = p_properties.getPropertyByID(Properties::Filename);
				auto const &luaVariablesProperty = p_properties.getPropertySetByID(Properties::Variables);

				if(luaFilenameProperty)
				{
					std::string luaFilename = luaFilenameProperty.getString();
					if(!luaFilename.empty())
					{
						p_constructionInfo.m_luaConstructionInfo->m_luaScriptFilename = luaFilename;

						if(luaVariablesProperty)
						{
							// Loop over each variable entry in the node
							for(decltype(luaVariablesProperty.getNumPropertySets()) iVariable = 0, numVariables = luaVariablesProperty.getNumPropertySets(); iVariable < numVariables; iVariable++)
							{
								// Add the variable
								p_constructionInfo.m_luaConstructionInfo->m_variables.emplace_back(
									luaVariablesProperty.getPropertySet(iVariable).getPropertyByID(Properties::Name).getString(),
									luaVariablesProperty.getPropertySet(iVariable).getPropertyByID(Properties::Value));
							}
						}
					}
					else
					{
						ErrHandlerLoc().get().log(ErrorCode::Property_no_filename, p_name, ErrorSource::Source_SceneLoader);
					}
				}
				else
				{
					ErrHandlerLoc().get().log(ErrorCode::Property_no_filename, p_name, ErrorSource::Source_SceneLoader);
				}
			}
			break;
		}
	}
}

void SceneLoader::importFromProperties(WorldComponentsConstructionInfo &p_constructionInfo, const PropertySet &p_properties, const std::string &p_name)
{
	// Check if property set node is present
	if(p_properties)
	{
		switch(p_properties.getPropertyID())
		{
		case Properties::PropertyID::ObjectMaterialComponent:
			{
				if(p_constructionInfo.m_objectMaterialConstructionInfo == nullptr)
					p_constructionInfo.m_objectMaterialConstructionInfo = new ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo();

				p_constructionInfo.m_objectMaterialConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::ObjectMaterialComponent);

				// Get the type property representing the object material type
				auto const &typeProperty = p_properties.getPropertyByID(Properties::Type);

				// Check if the property exists
				if(typeProperty)
				{
					// Define the type based on the imported property
					switch(typeProperty.getID())
					{
					case Properties::Concrete:
					default:
						p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType = ObjectMaterialType::Concrete;
						break;

					case Properties::Glass:
						p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType = ObjectMaterialType::Glass;
					break;

					case Properties::Metal:
						p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType = ObjectMaterialType::Metal;
						break;

					case Properties::Plastic:
						p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType = ObjectMaterialType::Plastic;
						break;

					case Properties::Rock:
						p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType = ObjectMaterialType::Rock;
						break;

					case Properties::Rubber:
						p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType = ObjectMaterialType::Rubber;
						break;

					case Properties::Wood:
						p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType = ObjectMaterialType::Wood;
						break;

						// If this is reached, the object material type was not valid
						ErrHandlerLoc().get().log(ErrorCode::Property_missing_type, p_name, ErrorSource::Source_ObjectMaterialComponent);
						break;
					}
				}
				else
				{
					// Missing the Type property entirely
					ErrHandlerLoc().get().log(ErrorCode::Property_missing_type, p_name, ErrorSource::Source_ObjectMaterialComponent);
				}
			}
			break;

			case Properties::PropertyID::SpatialComponent:
			{
				if(p_constructionInfo.m_spatialConstructionInfo == nullptr)
					p_constructionInfo.m_spatialConstructionInfo = new SpatialComponent::SpatialComponentConstructionInfo();

				p_constructionInfo.m_spatialConstructionInfo->m_name = p_name + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::SpatialComponent);

				// Load property data
				for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
				{
					switch(p_properties[i].getPropertyID())
					{
					case Properties::LocalPosition:
						p_constructionInfo.m_spatialConstructionInfo->m_localPosition = p_properties[i].getVec3f();
						break;
					case Properties::LocalRotation:
						p_constructionInfo.m_spatialConstructionInfo->m_localRotationEuler = p_properties[i].getVec3f();
						break;
					case Properties::LocalRotationQuaternion:
						p_constructionInfo.m_spatialConstructionInfo->m_localRotationQuaternion = glm::quat(p_properties[i].getVec4f());
						break;
					case Properties::LocalScale:
						p_constructionInfo.m_spatialConstructionInfo->m_localScale = p_properties[i].getVec3f();
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneLoader::exportToProperties(const ComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties)
{
	p_properties.addProperty(Properties::PropertyID::Name, p_constructionInfo.m_name);
	p_properties.addProperty(Properties::PropertyID::ID, (int)p_constructionInfo.m_id);
	p_properties.addProperty(Properties::PropertyID::Parent, (int)p_constructionInfo.m_parent);

	if(!p_constructionInfo.m_prefab.empty())
		p_properties.addProperty(Properties::PropertyID::Prefab, p_constructionInfo.m_prefab);

	// Export audio components
	exportToProperties(p_constructionInfo.m_audioComponents, p_properties);

	// Export graphics components
	exportToProperties(p_constructionInfo.m_graphicsComponents, p_properties);

	// Export GUI components
	exportToProperties(p_constructionInfo.m_guiComponents, p_properties);

	// Export physics components
	exportToProperties(p_constructionInfo.m_physicsComponents, p_properties);

	// Export script components
	exportToProperties(p_constructionInfo.m_scriptComponents, p_properties);

	// Export world components
	exportToProperties(p_constructionInfo.m_worldComponents, p_properties);
}

void SceneLoader::exportToProperties(const AudioComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties)
{
	if(	p_constructionInfo.m_soundConstructionInfo != nullptr ||
		p_constructionInfo.m_soundListenerConstructionInfo != nullptr)
	{
		auto &propertySet = p_properties.addPropertySet(Properties::PropertyID::Audio);

		// Export SoundComponent
		if(p_constructionInfo.m_soundConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::SoundComponent);

			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_soundConstructionInfo->m_active);
			componentPropertySet.addProperty(Properties::PropertyID::Filename, p_constructionInfo.m_soundConstructionInfo->m_soundFilename);
			switch(p_constructionInfo.m_soundConstructionInfo->m_soundType)
			{
				case SoundComponent::SoundType::SoundType_Ambient:
					componentPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::Ambient);
					break;
				case SoundComponent::SoundType::SoundType_Music:
					componentPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::Music);
					break;
				case SoundComponent::SoundType::SoundType_SoundEffect:
					componentPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::SoundEffect);
					break;
			}
			componentPropertySet.addProperty(Properties::PropertyID::Loop, p_constructionInfo.m_soundConstructionInfo->m_loop);
			componentPropertySet.addProperty(Properties::PropertyID::Spatialized, p_constructionInfo.m_soundConstructionInfo->m_spatialized);
			componentPropertySet.addProperty(Properties::PropertyID::StartPlaying, p_constructionInfo.m_soundConstructionInfo->m_startPlaying);
			componentPropertySet.addProperty(Properties::PropertyID::Volume, p_constructionInfo.m_soundConstructionInfo->m_volume);
		}

		// Export SoundListenerComponent
		if(p_constructionInfo.m_soundListenerConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::SoundListenerComponent);

			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_soundListenerConstructionInfo->m_active);
			//componentPropertySet.addProperty(Properties::PropertyID::ListenerID, p_constructionInfo.m_soundListenerConstructionInfo->m_listenerID);
		}
	}
}

void SceneLoader::exportToProperties(const GraphicsComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties)
{
	if(p_constructionInfo.m_cameraConstructionInfo != nullptr ||
		p_constructionInfo.m_lightConstructionInfo != nullptr ||
		p_constructionInfo.m_modelConstructionInfo != nullptr ||
		p_constructionInfo.m_shaderConstructionInfo != nullptr)
	{
		auto &propertySet = p_properties.addPropertySet(Properties::PropertyID::Graphics);

		// Export CameraComponent
		if(p_constructionInfo.m_cameraConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::CameraComponent);

			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_cameraConstructionInfo->m_active);
			//componentPropertySet.addProperty(Properties::PropertyID::FOV, p_constructionInfo.m_cameraConstructionInfo->m_fov);
		}

		// Export LightComponent
		if(p_constructionInfo.m_lightConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::LightComponent);

			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_lightConstructionInfo->m_active);
			switch(p_constructionInfo.m_lightConstructionInfo->m_lightComponentType)
			{
				case LightComponent::LightComponentType::LightComponentType_directional:
					componentPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::DirectionalLight);
					componentPropertySet.addProperty(Properties::PropertyID::Color, p_constructionInfo.m_lightConstructionInfo->m_color);
					componentPropertySet.addProperty(Properties::PropertyID::Intensity, p_constructionInfo.m_lightConstructionInfo->m_intensity);
					break;
				case LightComponent::LightComponentType::LightComponentType_point:
					componentPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::PointLight);
					componentPropertySet.addProperty(Properties::PropertyID::Color, p_constructionInfo.m_lightConstructionInfo->m_color);
					componentPropertySet.addProperty(Properties::PropertyID::Intensity, p_constructionInfo.m_lightConstructionInfo->m_intensity);
					break;
				case LightComponent::LightComponentType::LightComponentType_spot:
					componentPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::SpotLight);
					componentPropertySet.addProperty(Properties::PropertyID::Color, p_constructionInfo.m_lightConstructionInfo->m_color);
					componentPropertySet.addProperty(Properties::PropertyID::Intensity, p_constructionInfo.m_lightConstructionInfo->m_intensity);
					componentPropertySet.addProperty(Properties::PropertyID::CutoffAngle, p_constructionInfo.m_lightConstructionInfo->m_cutoffAngle);
					break;
			}

		}

		// Export ModelComponent
		if(p_constructionInfo.m_modelConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::ModelComponent);

			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_modelConstructionInfo->m_active);

			auto &modelsPropertySet = componentPropertySet.addPropertySet(Properties::PropertyID::Models);

			// Go over each model
			for(auto &model : p_constructionInfo.m_modelConstructionInfo->m_modelsProperties.m_modelNames)
			{
				auto &modelPropertyArrayEntry = modelsPropertySet.addPropertySet(Properties::ArrayEntry);

				// Add model data
				modelPropertyArrayEntry.addProperty(Properties::PropertyID::Filename, model);

				auto &meshesPropertySet = modelPropertyArrayEntry.addPropertySet(Properties::PropertyID::Meshes);

				// Go over each mesh
				for(decltype(p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_numOfMeshes) i = 0; i < p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_numOfMeshes; i++)
				{
					// Make sure the mesh data is present
					if(p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_present[i])
					{
						auto &meshPropertyArrayEntry = meshesPropertySet.addPropertySet(Properties::ArrayEntry);

						// Add mesh data
						meshPropertyArrayEntry.addProperty(Properties::PropertyID::Index, (int)i);
						meshPropertyArrayEntry.addProperty(Properties::PropertyID::AlphaThreshold, p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_alphaThreshold[i]);
						meshPropertyArrayEntry.addProperty(Properties::PropertyID::HeightScale, p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_heightScale[i]);

						auto &materialsPropertySet = meshPropertyArrayEntry.addPropertySet(Properties::Materials);

						// Go over each material
						for(unsigned int materialType = 0; materialType < MaterialType::MaterialType_NumOfTypes; materialType++)
						{
							// Make sure the material filename is not empty
							if(!p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_meshMaterials[i][materialType].empty())
							{
								Properties::PropertyID materialPropertyID = Properties::Null;

								// Convert MaterialType to PropertyID
								switch(materialType)
								{
									case MaterialType_Diffuse:
										materialPropertyID = Properties::Diffuse;
										break;
									case MaterialType_Normal:
										materialPropertyID = Properties::Normal;
										break;
									case MaterialType_Emissive:
										materialPropertyID = Properties::Emissive;
										break;
									case MaterialType_Combined:
										materialPropertyID = Properties::RMHAO;
										break;
								}
								auto &materialPropertySet = materialsPropertySet.addPropertySet(materialPropertyID);

								// Add material data
								materialPropertySet.addProperty(Properties::PropertyID::Filename, p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_meshMaterials[i][materialType]);
								materialPropertySet.addProperty(Properties::PropertyID::TextureScale, p_constructionInfo.m_modelConstructionInfo->m_materialsFromProperties.m_meshMaterialsScale[i][materialType]);
							}
						}
					}
				}



			}

			modelsPropertySet.addPropertySet(Properties::ArrayEntry);
		}

		// Export ShaderComponent
		if(p_constructionInfo.m_shaderConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::ShaderComponent);

			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_shaderConstructionInfo->m_active);

			// Add shader data, making sure isn't not empty before adding it
			if(!p_constructionInfo.m_shaderConstructionInfo->m_fragmentShaderFilename.empty())
				componentPropertySet.addProperty(Properties::PropertyID::FragmentShader, p_constructionInfo.m_shaderConstructionInfo->m_fragmentShaderFilename);
			if(!p_constructionInfo.m_shaderConstructionInfo->m_vetexShaderFilename.empty())
				componentPropertySet.addProperty(Properties::PropertyID::VertexShader, p_constructionInfo.m_shaderConstructionInfo->m_vetexShaderFilename);
			if(!p_constructionInfo.m_shaderConstructionInfo->m_geometryShaderFilename.empty())
				componentPropertySet.addProperty(Properties::PropertyID::GeometryShader, p_constructionInfo.m_shaderConstructionInfo->m_geometryShaderFilename);
		}
	}
}

void SceneLoader::exportToProperties(const GUIComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties)
{
	if(p_constructionInfo.m_guiSequenceConstructionInfo != nullptr)
	{
		auto &propertySet = p_properties.addPropertySet(Properties::PropertyID::GUI);

		// Export GUISequenceComponent
		if(p_constructionInfo.m_guiSequenceConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::GUISequenceComponent);

			// Add GUI sequence data
			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_guiSequenceConstructionInfo->m_active);
			componentPropertySet.addProperty(Properties::PropertyID::Static, p_constructionInfo.m_guiSequenceConstructionInfo->m_staticSequence);
		}
	}
}

void SceneLoader::exportToProperties(const PhysicsComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties)
{
	if(p_constructionInfo.m_rigidBodyConstructionInfo != nullptr)
	{
		auto &propertySet = p_properties.addPropertySet(Properties::PropertyID::Physics);

		// Export RigidBodyComponent
		if(p_constructionInfo.m_rigidBodyConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::RigidBodyComponent);

			// Add rigid body data
			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_rigidBodyConstructionInfo->m_active);
			componentPropertySet.addProperty(Properties::PropertyID::Friction, p_constructionInfo.m_rigidBodyConstructionInfo->m_friction);
			componentPropertySet.addProperty(Properties::PropertyID::Mass, p_constructionInfo.m_rigidBodyConstructionInfo->m_mass);
			componentPropertySet.addProperty(Properties::PropertyID::Restitution, p_constructionInfo.m_rigidBodyConstructionInfo->m_restitution);
			componentPropertySet.addProperty(Properties::PropertyID::Kinematic, p_constructionInfo.m_rigidBodyConstructionInfo->m_kinematic);

			// Convert CollisionShapeType to PropertyID
			Properties::PropertyID collisionShapeType = Properties::Null;
			switch(p_constructionInfo.m_rigidBodyConstructionInfo->m_collisionShapeType)
			{
				case RigidBodyComponent::CollisionShapeType_Box:
					collisionShapeType = Properties::Box;
					break;
				case RigidBodyComponent::CollisionShapeType_Capsule:
					collisionShapeType = Properties::Capsule;
					break;
				case RigidBodyComponent::CollisionShapeType_Cone:
					collisionShapeType = Properties::Cone;
					break;
				case RigidBodyComponent::CollisionShapeType_ConvexHull:
					collisionShapeType = Properties::ConvexHull;
					break;
				case RigidBodyComponent::CollisionShapeType_Cylinder:
					collisionShapeType = Properties::Cylinder;
					break;
				case RigidBodyComponent::CollisionShapeType_Sphere:
					collisionShapeType = Properties::Sphere;
					break;
			}

			// Add collision shape data, if it's valid
			if(collisionShapeType != Properties::Null)
			{
				auto &collisionShapePropertySet = componentPropertySet.addPropertySet(Properties::PropertyID::CollisionShape);
				collisionShapePropertySet.addProperty(Properties::PropertyID::Type, collisionShapeType);
				collisionShapePropertySet.addProperty(Properties::PropertyID::Size, p_constructionInfo.m_rigidBodyConstructionInfo->m_collisionShapeSize);
			}
		}
	}
}

void SceneLoader::exportToProperties(const ScriptComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties)
{
	if(p_constructionInfo.m_luaConstructionInfo != nullptr)
	{
		auto &propertySet = p_properties.addPropertySet(Properties::PropertyID::Script);

		// Export LuaComponent
		if(p_constructionInfo.m_luaConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::LuaComponent);

			// Add LUA component data
			if(!p_constructionInfo.m_luaConstructionInfo->m_luaScriptFilename.empty())
			{
				componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_luaConstructionInfo->m_active);
				componentPropertySet.addProperty(Properties::PropertyID::Filename, p_constructionInfo.m_luaConstructionInfo->m_luaScriptFilename);

				if(!p_constructionInfo.m_luaConstructionInfo->m_variables.empty())
				{
					auto &variablesPropertySet = componentPropertySet.addPropertySet(Properties::PropertyID::Variables);

					// Go over each variable
					for(auto &variable : p_constructionInfo.m_luaConstructionInfo->m_variables)
					{
						// Make sure variable type is valid
						if(variable.second.getVariableType() != Property::PropertyVariableType::Type_null)
						{
							auto &variableArrayEntry = variablesPropertySet.addPropertySet(Properties::PropertyID::ArrayEntry);

							// Add variable data
							variableArrayEntry.addProperty(Properties::PropertyID::Name, variable.first);

							// Add the appropriate data based on the variable's data type
							switch(variable.second.getVariableType())
							{
								case Property::PropertyVariableType::Type_bool:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getBool());
									break;
								case Property::PropertyVariableType::Type_int:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getInt());
									break;
								case Property::PropertyVariableType::Type_float:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getFloat());
									break;
								case Property::PropertyVariableType::Type_double:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getDouble());
									break;
								case Property::PropertyVariableType::Type_vec2i:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getVec2i());
									break;
								case Property::PropertyVariableType::Type_vec2f:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getVec2f());
									break;
								case Property::PropertyVariableType::Type_vec3f:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getVec3f());
									break;
								case Property::PropertyVariableType::Type_vec4f:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getVec4f());
									break;
								case Property::PropertyVariableType::Type_string:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getString());
									break;
								case Property::PropertyVariableType::Type_propertyID:
									variableArrayEntry.addProperty(Properties::PropertyID::Value, variable.second.getID());
									break;
							}
						}
					}

				}
			}
		}
	}

	std::string m_luaScriptFilename;
	std::vector<std::pair<std::string, Property>> m_variables;
}

void SceneLoader::exportToProperties(const WorldComponentsConstructionInfo &p_constructionInfo, PropertySet &p_properties)
{
	if(	p_constructionInfo.m_objectMaterialConstructionInfo != nullptr ||
		p_constructionInfo.m_spatialConstructionInfo != nullptr)
	{
		auto &propertySet = p_properties.addPropertySet(Properties::PropertyID::World);

		// Export ObjectMaterialComponent
		if(p_constructionInfo.m_objectMaterialConstructionInfo != nullptr)
		{
			// Convert ObjectMaterialType to PropertyID
			Properties::PropertyID objectMaterialType = Properties::Null;
			switch(p_constructionInfo.m_objectMaterialConstructionInfo->m_materialType)
			{
				case Concrete:
					objectMaterialType = Properties::Concrete;
					break;
				case Glass:
					objectMaterialType = Properties::Glass;
					break;
				case Metal:
					objectMaterialType = Properties::Metal;
					break;
				case Plastic:
					objectMaterialType = Properties::Plastic;
					break;
				case Rock:
					objectMaterialType = Properties::Rock;
					break;
				case Rubber:
					objectMaterialType = Properties::Rubber;
					break;
				case Wood:
					objectMaterialType = Properties::Wood;
					break;
			}

			// Add object material data, if it's valid
			if(objectMaterialType != Properties::Null)
			{
				auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::ObjectMaterialComponent);

				componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_objectMaterialConstructionInfo->m_active);
				componentPropertySet.addProperty(Properties::PropertyID::Type, objectMaterialType);
			}
		}

		// Export SpatialComponent
		if(p_constructionInfo.m_spatialConstructionInfo != nullptr)
		{
			auto &componentPropertySet = propertySet.addPropertySet(Properties::PropertyID::SpatialComponent);

			// Add spatial data
			componentPropertySet.addProperty(Properties::PropertyID::Active, p_constructionInfo.m_spatialConstructionInfo->m_active);
			componentPropertySet.addProperty(Properties::PropertyID::LocalPosition, p_constructionInfo.m_spatialConstructionInfo->m_localPosition);
			componentPropertySet.addProperty(Properties::PropertyID::LocalRotation, p_constructionInfo.m_spatialConstructionInfo->m_localRotationEuler);
			componentPropertySet.addProperty(Properties::PropertyID::LocalRotationQuaternion, Math::toGlmVec4(p_constructionInfo.m_spatialConstructionInfo->m_localRotationQuaternion));
			componentPropertySet.addProperty(Properties::PropertyID::LocalScale, p_constructionInfo.m_spatialConstructionInfo->m_localScale);
		}
	}
}
