
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
		m_systemScenes[i] = g_nullSystemBase.createScene(this);
}

SceneLoader::~SceneLoader()
{
}

ErrorCode SceneLoader::loadFromFile(const std::string &p_filename)
{
	ErrorCode returnError = ErrorCode::Success;

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
			m_systemScenes[sysIndex]->setup(scenePropertySet);
			m_systemScenes[sysIndex]->getSystem()->setup(systemropertySet);
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
		PropertySet propertySet(Properties::Default);

		// Add scene loaded properties
		propertySet.addProperty(Properties::LoadInBackground, m_loadInBackground);
		
		// Add root property set for systems
		auto &systems = propertySet.addPropertySet(Properties::Systems);

		// Add each system's properties
		//for(size_t i = 0; i < Systems::NumberOfSystems; i++)
		//	systems.addPropertySet(m_systemScenes[i]->exportObject());

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
				importPrefab(p_constructionInfo, p_properties[i].getString());
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
		case Properties::PropertyID::Audio:
		{
			
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

					p_constructionInfo.m_lightConstructionInfo->m_color = p_properties.getPropertyByID(Properties::Color).getVec3f();
					p_constructionInfo.m_lightConstructionInfo->m_intensity = p_properties.getPropertyByID(Properties::Intensity).getFloat();

					break;

				case Properties::PointLight:

					p_constructionInfo.m_lightConstructionInfo->m_lightComponentType = LightComponent::LightComponentType::LightComponentType_point;

					p_constructionInfo.m_lightConstructionInfo->m_color = p_properties.getPropertyByID(Properties::Color).getVec3f();
					p_constructionInfo.m_lightConstructionInfo->m_intensity = p_properties.getPropertyByID(Properties::Intensity).getFloat();

					break;

				case Properties::SpotLight:

					p_constructionInfo.m_lightConstructionInfo->m_lightComponentType = LightComponent::LightComponentType::LightComponentType_spot;

					p_constructionInfo.m_lightConstructionInfo->m_color = p_properties.getPropertyByID(Properties::Color).getVec3f();
					p_constructionInfo.m_lightConstructionInfo->m_cutoffAngle = p_properties.getPropertyByID(Properties::CutoffAngle).getFloat();
					p_constructionInfo.m_lightConstructionInfo->m_intensity = p_properties.getPropertyByID(Properties::Intensity).getFloat();

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
