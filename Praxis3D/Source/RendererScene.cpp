
#include "ComponentConstructorInfo.h"
#include "WorldScene.h"
#include "RendererScene.h"
#include "RendererSystem.h"
#include "SceneLoader.h"
#include "SpatialComponent.h"
#include "WorldScene.h"

RendererScene::RendererScene(RendererSystem *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::Renderer)
{
	m_renderTask = new RenderTask(this, p_system->getRenderer());
	//m_camera = nullptr;
	//m_skybox = nullptr;
}

RendererScene::~RendererScene()
{
	//delete m_camera;
}

ErrorCode RendererScene::init()
{
	ErrorCode returnError = ErrorCode::Success;
			
	// Create a default camera, in case it is not created upon loading a scene
	//m_camera = new CameraObject(this, "Default Camera");

	// Create a default directional light, in case it is not created upon loading a scene
	//m_directionalLight = new DirectionalLightObject(this, "Default Directional Light", DirectionalLightDataSet());
	
	// Create a default static environment map, so it can be used while a real one hasn't been loaded yet
	//m_sceneObjects.m_staticSkybox = new EnvironmentMapObject(this, "default", Loaders::textureCubemap().load());

	return returnError;
}

ErrorCode RendererScene::setup(const PropertySet &p_properties)
{
	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the property set containing object pool size
	auto &objectPoolSizeProperty = p_properties.getPropertySetByID(Properties::ObjectPoolSize);

	// Get model and shader components pool sizes
	int modelComponentPoolSize = std::max(Config::objectPoolVar().model_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::ModelComponent).getInt());
	int shaderComponentPoolSize = std::max(Config::objectPoolVar().shader_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::ShaderComponent).getInt());

	// Reserve every component type that belongs to this scene (and set the minimum number of objects based on default config)
	worldScene->reserve<CameraComponent>(std::max(Config::objectPoolVar().camera_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::CameraComponent).getInt()));
	worldScene->reserve<LightComponent>(std::max(Config::objectPoolVar().light_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::LightComponent).getInt()));
	worldScene->reserve<ModelComponent>(modelComponentPoolSize);
	worldScene->reserve<ShaderComponent>(shaderComponentPoolSize);
	worldScene->reserve<GraphicsLoadToMemoryComponent>(modelComponentPoolSize + shaderComponentPoolSize);
	worldScene->reserve<GraphicsLoadToVideoMemoryComponent>(modelComponentPoolSize + shaderComponentPoolSize);

	return ErrorCode::Success;
}

void RendererScene::exportSetup(PropertySet &p_propertySet)
{
	// Get the world scene required for getting the pool sizes
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Add object pool sizes
	auto &objectPoolSizePropertySet = p_propertySet.addPropertySet(Properties::ObjectPoolSize);
	objectPoolSizePropertySet.addProperty(Properties::CameraComponent, (int)worldScene->getPoolSize<CameraComponent>());
	objectPoolSizePropertySet.addProperty(Properties::LightComponent, (int)worldScene->getPoolSize<LightComponent>());
	objectPoolSizePropertySet.addProperty(Properties::ModelComponent, (int)worldScene->getPoolSize<ModelComponent>());
	objectPoolSizePropertySet.addProperty(Properties::ShaderComponent, (int)worldScene->getPoolSize<ShaderComponent>());
}

ErrorCode RendererScene::preload()
{
	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	std::vector<SystemObject *> componentsToLoad;

	auto modelAndLoadView = entityRegistry.view<ModelComponent, GraphicsLoadToMemoryComponent>();
	for(auto entity : modelAndLoadView)
		componentsToLoad.push_back(&modelAndLoadView.get<ModelComponent>(entity));

	auto shaderAndLoadView = entityRegistry.view<ShaderComponent, GraphicsLoadToMemoryComponent>();
	for(auto entity : shaderAndLoadView)
		componentsToLoad.push_back(&shaderAndLoadView.get<ShaderComponent>(entity));
		
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

	auto modelView = entityRegistry.view<ModelComponent>();
	for(auto entity : modelView)
	{
		auto &component = modelView.get<ModelComponent>(entity);

		TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelComponent::loadToMemory, &component));
	}

	auto shaderView = entityRegistry.view<ShaderComponent>();
	for(auto entity : shaderView)
	{
		auto &component = shaderView.get<ShaderComponent>(entity);

		TaskManagerLocator::get().startBackgroundThread(std::bind(&ShaderComponent::loadToMemory, &component));
	}
}

void RendererScene::update(const float p_deltaTime)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	//	 _______________________________
	//	|							    |
	//	| CURRENTLY LOADING COMPONENTS	|
	//	|		TO VIDEO MEMORY			|
	//	|_______________________________|
	//
	// Remove the load-to-video-memory component for entities that have already been loaded during the previous frame
	auto loadToVideoMemoryView = entityRegistry.view<GraphicsLoadToVideoMemoryComponent>(entt::exclude<GraphicsLoadToMemoryComponent>);
	for(auto entity : loadToVideoMemoryView)
	{
		auto &loadToVideoMemoryComponent = loadToVideoMemoryView.get<GraphicsLoadToVideoMemoryComponent>(entity);
		if(loadToVideoMemoryComponent.m_loaded)
		{
			// Set model component as loaded to video memory, if it is present
			auto modelComponent = entityRegistry.try_get<ModelComponent>(entity);
			if(modelComponent != nullptr)
				modelComponent->setLoadedToVideoMemory(true);

			// Set shader component as loaded to video memory, if it is present
			auto shaderComponent = entityRegistry.try_get<ShaderComponent>(entity);
			if(shaderComponent != nullptr)
				shaderComponent->setLoadedToVideoMemory(true);

			// Remove load-to-video-memory component to mark the entity as loaded to GPU
			worldScene->removeComponent<GraphicsLoadToVideoMemoryComponent>(entity);
		}
	}

	//	 _______________________________
	//	|							    |
	//	| CURRENTLY LOADING COMPONENTS	|
	//	|		   TO MEMORY			|
	//	|_______________________________|
	//
	// Check the entities that are being loaded to memory
	// If they have already been loaded, remove the load-to-memory component and add the load-to-video-memory component, so they can be loaded to GPU in the renderer
	/*auto modelAndLoadView = entityRegistry.view<ModelComponent, GraphicsLoadToMemoryComponent>(entt::exclude<ShaderComponent>);
	for(auto entity : modelAndLoadView)
	{
		auto &modelComponent = modelAndLoadView.get<ModelComponent>(entity);

		// Perform a check that marks an object if it is loaded to memory
		modelComponent.performCheckIsLoadedToMemory();

		// If the object has loaded to memory already, add to load queue
		if(modelComponent.isLoadedToMemory())
		{
			// Remove the load-to-memory component, signifying that it has already been loaded to memory
			worldScene->removeComponent<GraphicsLoadToMemoryComponent>(entity);

			// Make the component active, so it is processed in the renderer
			modelComponent.setActive(modelComponent.m_setActiveAfterLoading);

			// Do not add the load-to-video-memory component if the object is already loaded to GPU
			modelComponent.performCheckIsLoadedToVideoMemory();
			if(!modelComponent.isLoadedToVideoMemory())
			{
				auto &loadToVideoMemoryComponent = worldScene->addComponent<GraphicsLoadToVideoMemoryComponent>(entity, entity);

				// Get all loadable objects from the model component
				auto loadableObjectsFromModel = modelComponent.getLoadableObjects();

				// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
				for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
					if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
						loadToVideoMemoryComponent.m_objectsToLoad.emplace(loadableObjectsFromModel[i]);
			}
		}
	}*/

	// Check the entities that are being loaded to memory
	// If they have already been loaded, remove the load-to-memory component and add the load-to-video-memory component, so they can be loaded to GPU in the renderer
	auto modelAndLoadView = entityRegistry.view<ModelComponent, GraphicsLoadToMemoryComponent>();
	for(auto entity : modelAndLoadView)
	{
		auto &modelComponent = modelAndLoadView.get<ModelComponent>(entity);
		auto shaderComponent = entityRegistry.try_get<ShaderComponent>(entity);

		// In addition to model component, the entity might contain a shader component too
		// In that case, process both model and shader components
		if(shaderComponent == nullptr)
		{
			// Perform a check that marks an object if it is loaded to memory
			if(!modelComponent.isLoadedToMemory())
				modelComponent.performCheckIsLoadedToMemory();

			// If the object has loaded to memory already, add to load queue
			if(modelComponent.isLoadedToMemory())
			{
				// Remove the load-to-memory component, signifying that it has already been loaded to memory
				worldScene->removeComponent<GraphicsLoadToMemoryComponent>(entity);

				// Make the component active, so it is processed in the renderer
				modelComponent.setActive(modelComponent.m_setActiveAfterLoading);

				// Do not add the load-to-video-memory component if the object is already loaded to GPU
				modelComponent.performCheckIsLoadedToVideoMemory();
				if(!modelComponent.isLoadedToVideoMemory())
				{
					auto &loadToVideoMemoryComponent = worldScene->addComponent<GraphicsLoadToVideoMemoryComponent>(entity, entity);

					// Get all loadable objects from the model component
					auto loadableObjectsFromModel = modelComponent.getLoadableObjects();

					// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
					for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
						if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
							loadToVideoMemoryComponent.m_objectsToLoad.emplace(loadableObjectsFromModel[i]);
				}
			}
		}
		else
		{
			// Perform a check on model component that marks an object if it is loaded to memory
			if(!modelComponent.isLoadedToMemory())
				modelComponent.performCheckIsLoadedToMemory();

			// If the model component has loaded to memory already, add to load queue
			if(shaderComponent->isLoadedToMemory())
			{
				// Perform a check on shader component that marks an object if it is loaded to memory
				if(!shaderComponent->isLoadedToMemory())
					shaderComponent->performCheckIsLoadedToMemory();

				// If the shader component has loaded to memory already, add to load queue
				if(shaderComponent->isLoadedToMemory())
				{
					// Remove the load-to-memory component, signifying that it has already been loaded to memory
					worldScene->removeComponent<GraphicsLoadToMemoryComponent>(entity);

					// Make the components active, so it is processed in the renderer
					modelComponent.setActive(modelComponent.m_setActiveAfterLoading);
					shaderComponent->setActive(shaderComponent->m_setActiveAfterLoading);

					// Do not add the load-to-video-memory component if the object is already loaded to GPU
					modelComponent.performCheckIsLoadedToVideoMemory();
					shaderComponent->performCheckIsLoadedToVideoMemory();
					if(!modelComponent.isLoadedToVideoMemory() || !shaderComponent->isLoadedToVideoMemory())
					{
						auto &loadToVideoMemoryComponent = worldScene->addComponent<GraphicsLoadToVideoMemoryComponent>(entity, entity);

						if(!modelComponent.isLoadedToVideoMemory())
						{
							// Get all loadable objects from the model component
							auto loadableObjectsFromModel = modelComponent.getLoadableObjects();

								// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
								for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
									if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
										loadToVideoMemoryComponent.m_objectsToLoad.emplace(loadableObjectsFromModel[i]);
						}

						if(!shaderComponent->isLoadedToVideoMemory())
						{
							// Get all loadable objects from the shader component
							auto loadableObjectsFromShader = shaderComponent->getLoadableObjects();

							// Iterate over all loadable objects from the shader component, and if any of them are not loaded to video memory already, add them to the to-load list
							for(decltype(loadableObjectsFromShader.size()) size = loadableObjectsFromShader.size(), i = 0; i < size; i++)
								if(!loadableObjectsFromShader[i].isLoadedToVideoMemory())
									loadToVideoMemoryComponent.m_objectsToLoad.emplace(loadableObjectsFromShader[i]);
						}
					}
				}
			}
		}
	}

	//	 ___________________________
	//	|							|
	//	|	  COMPONENT UPDATES		|
	//	|___________________________|
	//
	/*auto modelView = entityRegistry.view<ModelComponent>();
	for(auto entity : modelView)
	{
		auto &component = modelView.get<ModelComponent>(entity);

		if(!component.isLoadedToVideoMemory())
			component.performCheckIsLoadedToVideoMemory();
	}

	auto shaderView = entityRegistry.view<ShaderComponent>();
	for(auto entity : shaderView)
	{
		auto &component = shaderView.get<ShaderComponent>(entity);

		if(!component.isLoadedToVideoMemory())
			component.performCheckIsLoadedToVideoMemory();
	}*/

	//	 ___________________________
	//	|							|
	//	|	  MODEL COMPONENTS		|
	//	|___________________________|
	//
	m_sceneObjects.m_models = entityRegistry.view<ModelComponent, SpatialComponent>(entt::exclude<ShaderComponent, GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>);
	m_sceneObjects.m_modelsWithShaders = entityRegistry.view<ModelComponent, ShaderComponent, SpatialComponent>(entt::exclude<GraphicsLoadToMemoryComponent, GraphicsLoadToVideoMemoryComponent>);
	m_sceneObjects.m_objectsToLoadToVideoMemory = entityRegistry.view<GraphicsLoadToVideoMemoryComponent>(entt::exclude<GraphicsLoadToMemoryComponent>);

	//	 ___________________________
	//	|							|
	//	|	  LIGHT COMPONENTS		|
	//	|___________________________|
	//
	m_sceneObjects.m_lights = entityRegistry.view<LightComponent, SpatialComponent>();

	//	 ___________________________
	//	|							|
	//	|	  CAMERA COMPONENTS		|
	//	|___________________________|
	//
	auto cameraView = entityRegistry.view<CameraComponent, SpatialComponent>();
	for(auto entity : cameraView)
	{
		auto &cameraComponent = cameraView.get<CameraComponent>(entity);
		auto &spatialComponent = cameraView.get<SpatialComponent>(entity);

		m_sceneObjects.m_cameraViewMatrix = spatialComponent.getSpatialDataChangeManager().getWorldTransform();

		break;
	}
}

std::vector<SystemObject*> RendererScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_graphicsComponents, p_startLoading);
}

void RendererScene::exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	exportComponents(p_entityID, p_constructionInfo.m_graphicsComponents);
}

void RendererScene::exportComponents(const EntityID p_entityID, GraphicsComponentsConstructionInfo &p_constructionInfo)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Export CameraComponent
	auto *cameraComponent = entityRegistry.try_get<CameraComponent>(p_entityID);
	if(cameraComponent != nullptr)
	{
		if(p_constructionInfo.m_cameraConstructionInfo == nullptr)
			p_constructionInfo.m_cameraConstructionInfo = new CameraComponent::CameraComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_cameraConstructionInfo, *cameraComponent);
	}

	// Export LightComponent
	auto *lightComponent = entityRegistry.try_get<LightComponent>(p_entityID);
	if(lightComponent != nullptr)
	{
		if(p_constructionInfo.m_lightConstructionInfo == nullptr)
			p_constructionInfo.m_lightConstructionInfo = new LightComponent::LightComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_lightConstructionInfo, *lightComponent);
	}

	// Export ModelComponent
	auto *modelComponent = entityRegistry.try_get<ModelComponent>(p_entityID);
	if(modelComponent != nullptr)
	{
		if(p_constructionInfo.m_modelConstructionInfo == nullptr)
			p_constructionInfo.m_modelConstructionInfo = new ModelComponent::ModelComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_modelConstructionInfo, *modelComponent);
	}

	// Export ShaderComponent
	auto *shaderComponent = entityRegistry.try_get<ShaderComponent>(p_entityID);
	if(shaderComponent != nullptr)
	{
		if(p_constructionInfo.m_shaderConstructionInfo == nullptr)
			p_constructionInfo.m_shaderConstructionInfo = new ShaderComponent::ShaderComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_shaderConstructionInfo, *shaderComponent);
	}
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
		component.m_fov = p_constructionInfo.m_fov;

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

		// Try to initialize the light component
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

		// Try to initialize the model component
		auto componentInitError = component.init();
		if(componentInitError == ErrorCode::Success)
		{
			component.m_setActiveAfterLoading = p_constructionInfo.m_active;
			component.setActive(false);

			component.m_modelsProperties = new ModelComponent::ModelsProperties(p_constructionInfo.m_modelsProperties);
			component.m_materialsFromProperties = new ModelComponent::MeshMaterialsProperties(p_constructionInfo.m_materialsFromProperties);

			// Add the load-to-memory component signifying that it is currently being loaded to memory
			//m_componentsLoadingToMemory.emplace_back(component);
			worldScene->addComponent<GraphicsLoadToMemoryComponent>(p_entityID, p_entityID);

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

		// Try to initialize the shader component
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
			// 
			// Add the load-to-memory component signifying that it is currently being loaded to memory
			worldScene->addComponent<GraphicsLoadToMemoryComponent>(p_entityID, p_entityID);

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

void RendererScene::receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
	switch(p_dataType)
	{
	case DataType_GUIPassFunctors:
		m_renderTask->m_renderer.setGUIPassFunctorSequence(static_cast<FunctorSequence *>(p_data));
		break;

	case DataType_RenderToTexture:
		m_renderTask->m_renderer.setRenderFinalToTexture(static_cast<bool>(p_data));
		break;

	case DataType_RenderToTextureResolution:
		{
			auto renderToTextureResolution = static_cast<glm::ivec2 *>(p_data);
			m_renderTask->m_renderer.setRenderToTextureResolution(*renderToTextureResolution);

			// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
			if(p_deleteAfterReceiving)
				delete renderToTextureResolution;
		}
		break;

	case DataType_Texture2D:
		{
			TextureLoader2D::Texture2DHandle *textureHandle = static_cast<TextureLoader2D::Texture2DHandle *>(p_data);
			if(textureHandle->isLoadedToMemory())
				m_sceneObjects.m_loadToVideoMemory.emplace_back(*textureHandle);

			// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
			if(p_deleteAfterReceiving)
				delete textureHandle;
		}
		break;

	case DataType_Texture3D:

		break;
	}
}

const unsigned int RendererScene::getUnsignedInt(const Observer *p_observer, BitMask p_changedBits) const
{
	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::RenderToTextureBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(static_cast<GBufferTextureType>(Config::rendererVar().render_to_texture_buffer));

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::PositionBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferPosition);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::DiffuseBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferDiffuse);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::NormalBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferNormal);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::EmissiveBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferEmissive);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::MatPropertiesBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferMatProperties);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::IntermediateBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferIntermediate);

	if(CheckBitmask(p_changedBits, Systems::Changes::Graphics::FinalBuffer))
		return m_renderTask->m_renderer.getFramebufferTextureHandle(GBufferTextureType::GBufferFinal);

	return NullObjects::NullUnsignedInt;
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
