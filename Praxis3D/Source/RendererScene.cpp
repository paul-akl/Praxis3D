
#include "ComponentConstructorInfo.h"
#include "WorldScene.h"
#include "RendererScene.h"
#include "RendererSystem.h"
#include "SceneLoader.h"
#include "SpatialComponent.h"
#include "WorldScene.h"

RendererScene::RendererScene(RendererSystem *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_renderTask = new RenderTask(this, p_system->getRenderer());
	m_camera = nullptr;
	m_skybox = nullptr;
}

RendererScene::~RendererScene()
{
	delete m_camera;
}

ErrorCode RendererScene::init()
{
	ErrorCode returnError = ErrorCode::Success;
			
	// Create a default camera, in case it is not created upon loading a scene
	m_camera = new CameraObject(this, "Default Camera");

	// Create a default directional light, in case it is not created upon loading a scene
	m_directionalLight = new DirectionalLightObject(this, "Default Directional Light", DirectionalLightDataSet());
	
	// Create a default static environment map, so it can be used while a real one hasn't been loaded yet
	//m_sceneObjects.m_staticSkybox = new EnvironmentMapObject(this, "default", Loaders::textureCubemap().load());

	return returnError;
}

ErrorCode RendererScene::setup(const PropertySet &p_properties)
{
	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));
	
	// Reserve every component type that belongs to this scene
	worldScene->reserve<CameraComponent>(Config::objectPoolVar().camera_component_default_pool_size);
	worldScene->reserve<LightComponent>(Config::objectPoolVar().light_component_default_pool_size);
	worldScene->reserve<ModelComponent>(Config::objectPoolVar().model_component_default_pool_size);
	worldScene->reserve<ShaderComponent>(Config::objectPoolVar().shader_component_default_pool_size);

	return ErrorCode::Success;
}

ErrorCode RendererScene::preload()
{
	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	std::vector<SystemObject*> componentsToLoad;
	for(auto &component : m_componentsLoadingToMemory)
	{
		switch(component.m_componentType)
		{
			case LoadableComponentContainer::ComponentType_Model:
			{
				componentsToLoad.push_back(&entityRegistry.get<ModelComponent>(component.m_entityID));
			}
			break;

			case LoadableComponentContainer::ComponentType_Shader:
			{
				componentsToLoad.push_back(&entityRegistry.get<ShaderComponent>(component.m_entityID));
			}
			break;
		}
	}	
	
	// Load every object to memory. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), componentsToLoad.size(), size_t(1), [=](size_t i)
		{
			componentsToLoad[i]->loadToMemory();
		});

	return ErrorCode::Success;
}

void RendererScene::loadInBackground()
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	auto modelView = worldScene->getEntityRegistry().view<ModelComponent>();
	for(auto entity : modelView)
	{
		auto &component = modelView.get<ModelComponent>(entity);

		TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelComponent::loadToMemory, &component));
	}

	auto shaderView = worldScene->getEntityRegistry().view<ShaderComponent>();
	for(auto entity : shaderView)
	{
		auto &component = shaderView.get<ShaderComponent>(entity);

		TaskManagerLocator::get().startBackgroundThread(std::bind(&ShaderComponent::loadToMemory, &component));
	}
}

void RendererScene::update(const float p_deltaTime)
{
	// Clear variables from previous frame
	m_sceneObjects.m_directionalLight = &m_directionalLight->getLightDataSet();

	// Clear arrays from previous frame
	m_sceneObjects.m_pointLights.clear();
	m_sceneObjects.m_spotLights.clear();

	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	//	 _______________________________
	//	|							    |
	//	| CURRENTLY LOADING COMPONENTS	|
	//	|_______________________________|
	//
	bool componentHasBeenLoaded = false;
	decltype(Config::rendererVar().objects_loaded_per_frame) objectsThatCanBeLoadedThisFrame = Config::rendererVar().objects_loaded_per_frame;
	auto it = m_componentsLoadingToMemory.begin();
	while(it != m_componentsLoadingToMemory.end() && objectsThatCanBeLoadedThisFrame != 0)
	{
		switch(it->m_componentType)
		{
			case LoadableComponentContainer::ComponentType_Model:
			{
				ModelComponent &modelComponent = entityRegistry.get<ModelComponent>(it->m_entityID);

				// Perform a check that marks an object if it is loaded to memory
				modelComponent.performCheckIsLoadedToMemory();

				// If the object has loaded to memory already, add to load queue
				if(modelComponent.isLoadedToMemory())
				{
					componentHasBeenLoaded = true;

					// Make the component active, so it is processed in the renderer
					modelComponent.setActive(modelComponent.m_setActiveAfterLoading);

					//if(!modelComponent.isLoadedToVideoMemory())
					modelComponent.performCheckIsLoadedToVideoMemory();

					if(!modelComponent.isLoadedToVideoMemory())
					{
						// Get all loadable objects from the model component
						auto loadableObjectsFromModel = modelComponent.getLoadableObjects();

						// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
						for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
							if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
								m_sceneObjects.m_loadToVideoMemory.emplace_back(loadableObjectsFromModel[i]);
					}
				}
			}
			break;

			case LoadableComponentContainer::ComponentType_Shader:
			{
				ShaderComponent &shaderComponent = entityRegistry.get<ShaderComponent>(it->m_entityID);

				// Perform a check that marks an object if it is loaded to memory
				shaderComponent.performCheckIsLoadedToMemory();

				// If the object has loaded to memory already, add to load queue
				if(shaderComponent.isLoadedToMemory())
				{
					componentHasBeenLoaded = true;

					// Make the component active, so it is processed in the renderer
					shaderComponent.setActive(shaderComponent.m_setActiveAfterLoading);

					// Get all loadable objects from the shader component
					auto loadableObjectsFromShader = shaderComponent.getLoadableObjects();

					// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
					for(decltype(loadableObjectsFromShader.size()) size = loadableObjectsFromShader.size(), i = 0; i < size; i++)
						if(!loadableObjectsFromShader[i].isLoadedToVideoMemory())
							m_sceneObjects.m_loadToVideoMemory.emplace_back(loadableObjectsFromShader[i]);
				}
			}
			break;
		}

		// If the component has been loaded, remove it from the list; otherwise just increment the iterator to continue to the next element
		if(componentHasBeenLoaded)
		{
			// Remove the element from the list since it has been loaded to memory
			it = m_componentsLoadingToMemory.erase(it);

			// Decrement the count of the allowed number of objects that can be loaded per frame
			objectsThatCanBeLoadedThisFrame--;

			componentHasBeenLoaded = false;
		}
		else
		{
			// Increment iterator since no elements have been removed
			++it;
		}
	}

	//	 ___________________________
	//	|							|
	//	|	  COMPONENT UPDATES		|
	//	|___________________________|
	//
	auto modelView = worldScene->getEntityRegistry().view<ModelComponent>();
	for(auto entity : modelView)
	{
		auto &component = modelView.get<ModelComponent>(entity);

		if(!component.isLoadedToVideoMemory())
			component.performCheckIsLoadedToVideoMemory();
	}

	auto shaderView = worldScene->getEntityRegistry().view<ShaderComponent>();
	for(auto entity : shaderView)
	{
		auto &component = shaderView.get<ShaderComponent>(entity);

		if(!component.isLoadedToVideoMemory())
			component.performCheckIsLoadedToVideoMemory();
	}

	//	 ___________________________
	//	|							|
	//	|	  MODEL COMPONENTS		|
	//	|___________________________|
	//
	m_sceneObjects.m_models = worldScene->getEntityRegistry().view<ModelComponent, SpatialComponent>(entt::exclude<ShaderComponent>);
	m_sceneObjects.m_modelsWithShaders = worldScene->getEntityRegistry().view<ModelComponent, ShaderComponent, SpatialComponent>();

	//	 ___________________________
	//	|							|
	//	|	  LIGHT COMPONENTS		|
	//	|___________________________|
	//
	auto lightsView = worldScene->getEntityRegistry().view<LightComponent, SpatialComponent>();
	for(auto entity : lightsView)
	{
		auto &lightComponent = lightsView.get<LightComponent>(entity);
		auto &spatialComponent = lightsView.get<SpatialComponent>(entity);

		// Check if the light is enabled
		if(lightComponent.isObjectActive())
		{
			// Add the light data to the corresponding array, based on the light type
			switch(lightComponent.getLightType())
			{
			case LightComponent::LightComponentType_point:
			{
				// Update position of the light data set
				PointLightDataSet *lightDataSet = lightComponent.getPointLight();
				lightDataSet->m_position = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[3];

				m_sceneObjects.m_pointLights.push_back(*lightDataSet);
			}
				break;
			case LightComponent::LightComponentType_spot:
			{
				// Update position and rotation of the light data set
				SpotLightDataSet *lightDataSet = lightComponent.getSpotLight();
				lightDataSet->m_position = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[3];
				lightDataSet->m_direction = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[2];

				m_sceneObjects.m_spotLights.push_back(*lightDataSet);
			}
				break;
			case LightComponent::LightComponentType_directional:
			{

				DirectionalLightDataSet *lightDataSet = lightComponent.getDirectionalLight();
				lightDataSet->m_direction = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[2];

				m_sceneObjects.m_directionalLight = lightComponent.getDirectionalLight();
			}
				break;
			}
		}
	}

	//	 ___________________________
	//	|							|
	//	|	  CAMERA COMPONENTS		|
	//	|___________________________|
	//
	auto cameraView = worldScene->getEntityRegistry().view<CameraComponent, SpatialComponent>();
	for(auto entity : cameraView)
	{
		auto &cameraComponent = cameraView.get<CameraComponent>(entity);
		auto &spatialComponent = cameraView.get<SpatialComponent>(entity);

		m_sceneObjects.m_camera.m_spatialData.m_transformMat = spatialComponent.getSpatialDataChangeManager().getWorldTransform();

		break;
	}

	// Update camera spatial data
	calculateCamera(m_sceneObjects.m_camera.m_spatialData);
}

std::vector<SystemObject*> RendererScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_graphicsComponents, p_startLoading);
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const CameraComponent::CameraComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	auto &component = worldScene->addComponent<CameraComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	// Try to initialize the camera component
	auto componentInitError = component.init();
	if(componentInitError == ErrorCode::Success)
	{
		component.setActive(p_constructionInfo.m_active);
		component.setLoadedToMemory(true);
		component.setLoadedToVideoMemory(true);

		returnObject = &component;
	}
	else // Remove the component if it failed to initialize
	{
		worldScene->removeComponent<CameraComponent>(p_entityID);
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_CameraComponent, p_constructionInfo.m_name);
	}

	return returnObject;
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const LightComponent::LightComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

	// Proceed only if the light type is not null
	if(p_constructionInfo.m_lightComponentType != LightComponent::LightComponentType::LightComponentType_null)
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

		auto &component = worldScene->addComponent<LightComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		// Try to initialize the camera component
		auto componentInitError = component.init();
		if(componentInitError == ErrorCode::Success)
		{
			component.m_lightComponentType = p_constructionInfo.m_lightComponentType;
			component.setActive(p_constructionInfo.m_active);

			// Load values based on the type of light
			switch(component.m_lightComponentType)
			{
			case LightComponent::LightComponentType::LightComponentType_directional:
				component.m_objectType = Properties::PropertyID::DirectionalLight;

				component.m_lightComponent.m_directional.m_color = p_constructionInfo.m_color;
				component.m_lightComponent.m_directional.m_intensity = p_constructionInfo.m_intensity;
				component.setLoadedToMemory(true);

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, p_constructionInfo.m_name + " - Directional light loaded");
				break;

			case LightComponent::LightComponentType::LightComponentType_point:
				component.m_objectType = Properties::PropertyID::PointLight;

				component.m_lightComponent.m_point.m_color = p_constructionInfo.m_color;
				component.m_lightComponent.m_point.m_intensity = p_constructionInfo.m_intensity;
				component.setLoadedToMemory(true);

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, p_constructionInfo.m_name + " - Point light loaded");
				break;

			case LightComponent::LightComponentType::LightComponentType_spot:
				component.m_objectType = Properties::PropertyID::SpotLight;

				component.m_lightComponent.m_spot.m_color = p_constructionInfo.m_color;
				component.m_lightComponent.m_spot.m_cutoffAngle = p_constructionInfo.m_cutoffAngle;
				component.m_lightComponent.m_spot.m_intensity = p_constructionInfo.m_intensity;
				component.setLoadedToMemory(true);

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, p_constructionInfo.m_name + " - Spot light loaded");
				break;

			case LightComponent::LightComponentType::LightComponentType_null:
			default:
				ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LightComponent, p_constructionInfo.m_name + " - missing \'Type\' identifier");
				worldScene->removeComponent<LightComponent>(p_entityID);
				break;
			}

			returnObject = &component;
		}
		else // Remove the component if it failed to initialize
		{
			worldScene->removeComponent<CameraComponent>(p_entityID);
			ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_LightComponent, p_constructionInfo.m_name);
		}
	}

	return returnObject;
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const ModelComponent::ModelComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

	// Make sure there are models present
	if(!p_constructionInfo.m_modelsProperties.m_modelNames.empty())
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

		auto &component = worldScene->addComponent<ModelComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		// Try to initialize the camera component
		auto componentInitError = component.init();
		if(componentInitError == ErrorCode::Success)
		{
			component.m_setActiveAfterLoading = p_constructionInfo.m_active;
			component.setActive(false);

			component.m_modelsProperties = new ModelComponent::ModelsProperties(p_constructionInfo.m_modelsProperties);
			component.m_materialsFromProperties = new ModelComponent::MeshMaterialsProperties(p_constructionInfo.m_materialsFromProperties);

			// Add the component to an array signifying that it is currently being loaded to memory
			m_componentsLoadingToMemory.emplace_back(component);

			// Start loading the component to memory in the background if the flag is set to do so
			if(p_startLoading)
				TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelComponent::loadToMemory, &component));

			returnObject = &component;
		}
		else // Remove the component if it failed to initialize
		{
			worldScene->removeComponent<ModelComponent>(p_entityID);
			ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ModelComponent, p_constructionInfo.m_name);
		}
	}
	else
		ErrHandlerLoc().get().log(ErrorCode::Initialize_failure, ErrorSource::Source_ModelComponent, p_constructionInfo.m_name);

	return returnObject;
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const ShaderComponent::ShaderComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

	// Check if any of the shader nodes are present
	if(!p_constructionInfo.m_vetexShaderFilename.empty() && !p_constructionInfo.m_fragmentShaderFilename.empty())
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

		auto &component = worldScene->addComponent<ShaderComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

		// Try to initialize the camera component
		auto componentInitError = component.init();
		if(componentInitError == ErrorCode::Success)
		{
			component.m_setActiveAfterLoading = p_constructionInfo.m_active;
			component.setActive(false);

			// Create a property-set used to load the shaders
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::VertexShader, p_constructionInfo.m_vetexShaderFilename);
			shaderProperties.addProperty(Properties::FragmentShader, p_constructionInfo.m_fragmentShaderFilename);
			if(!p_constructionInfo.m_geometryShaderFilename.empty())
				shaderProperties.addProperty(Properties::GeometryShader, p_constructionInfo.m_geometryShaderFilename);

			// Load shader program
			auto shaderProgram = Loaders::shader().load(shaderProperties);

			// If shader is not default (i.e. at least one of the shader types was loaded)
			if(!shaderProgram->isDefaultProgram())
			{
				// Load the shader to memory and assign it to the new shader component
				shaderProgram->loadToMemory();
				component.m_shaderData = new ShaderData(*shaderProgram);

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ShaderComponent, p_constructionInfo.m_name + " - \'" + p_constructionInfo.m_fragmentShaderFilename + "\' and \'" + p_constructionInfo.m_vetexShaderFilename + "\' shaders imported.");
			}

			// Set the component as loaded, because the load function was called
			//component.setLoadedToMemory(true);

			// Start loading the component to memory in the background if the flag is set to do so
			if(p_startLoading)
				TaskManagerLocator::get().startBackgroundThread(std::bind(&ShaderComponent::loadToMemory, &component));

			returnObject = &component;
		}
		else // Remove the component if it failed to initialize
		{
			worldScene->removeComponent<ShaderComponent>(p_entityID);
			ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ShaderComponent, p_constructionInfo.m_name);
		}
	}
	else
		ErrHandlerLoc().get().log(ErrorCode::Property_no_filename, p_constructionInfo.m_name, ErrorSource::Source_ShaderComponent);

	return returnObject;
}

ErrorCode RendererScene::destroyObject(SystemObject *p_systemObject)
{
	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

void RendererScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
	//std::cout << "change occurred" << std::endl;
}

void RendererScene::receiveData(const DataType p_dataType, void *p_data)
{
	switch(p_dataType)
	{
	case DataType_Texture2D:
		{
			TextureLoader2D::Texture2DHandle *textureHandle = static_cast<TextureLoader2D::Texture2DHandle *>(p_data);
			if(textureHandle->isLoadedToMemory())
				m_sceneObjects.m_loadToVideoMemory.emplace_back(*textureHandle);
		}
		break;

	case DataType_Texture3D:

		break;
	}
}

MaterialData RendererScene::loadMaterialData(PropertySet &p_materialProperty, Model::MaterialArrays &p_materialArraysFromModel, MaterialType p_materialType, std::size_t p_meshIndex)
{
	// Declare the material data that is to be returned and a flag showing whether the material data was loaded successfully
	MaterialData newMaterialData;
	bool materialWasLoaded = false;

	/*/ Try to load the material from the filename retrieved from properties
	if (p_materialProperty)
	{
		// Get texture filename property, check if it is valid
		auto filenameProperty = p_materialProperty.getPropertyByID(Properties::Filename);
		if (filenameProperty.isVariableTypeString())
		{
			// Get texture filename string, check if it is valid
			auto filename = filenameProperty.getString();
			if (!filename.empty())
			{
				// Get the texture and load it to memory
				auto materialHandle = Loaders::texture2D().load(filename, p_materialType, false);
				auto materialLoadError = materialHandle.loadToMemory();

				// Check if the texture was loaded successfully
				if (materialLoadError == ErrorCode::Success)
				{
					newMaterialData.m_texture = materialHandle;
					materialWasLoaded = true;
				}
				else
				{
					ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
				}
			}
		}
	}

	// Try to load the material from the filename retrieved from the model
	if (!materialWasLoaded)
	{
		// Check if there are enough materials, and if the material isn't default
		if (p_materialArraysFromModel.m_numMaterials > p_meshIndex
			&& !p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].isEmpty()
			&& !p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].isDefaultMaterial())
		{
			// Get the texture and load it to memory
			auto materialHandle = Loaders::texture2D().load(p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].m_filename, p_materialType, false);
			auto materialLoadError = materialHandle.loadToMemory();

			// Check if the texture was loaded successfully
			if (materialLoadError == ErrorCode::Success)
			{
				newMaterialData.m_texture = materialHandle;
				materialWasLoaded = true;
			}
			else
			{
				ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
			}
		}
	}

	// All attempts to load the material were unsuccessful, so load a default material
	if (!materialWasLoaded)
	{
		newMaterialData.m_texture = Loaders::texture2D().getDefaultTexture(p_materialType);
	}
	*/
	// Return the newly loaded material data
	return newMaterialData;
}
