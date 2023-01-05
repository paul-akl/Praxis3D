
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
	
	if(!entt::basic_component_traits::in_place_delete)
		ErrHandlerLoc().get().log(ErrorType::Error, ErrorSource::Source_WorldScene, "entt::basic_component_traits::in_place_delete is switched off, disabling pointer stability upon component deletion");
		
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
	// Get default object pool size
	decltype(m_graphicsObjPool.getPoolSize()) objectPoolSize = Config::objectPoolVar().object_pool_size;

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
	m_graphicsObjPool.init(objectPoolSize);
	m_objectsLoadingToMemory.reserve(objectPoolSize);
	m_objectsToDestroy.reserve(objectPoolSize);

	return ErrorCode::Success;
}

ErrorCode RendererScene::preload()
{


	// Implementation note: use number of allocated objects as an early bail - this method is most
	// likely called after populating pools, which means objects are lined at the start of the pools)

	/*/ Start loading Model Objects 
	for(decltype(m_modelObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_modelObjPool.getNumAllocated(),
		size = m_modelObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// If current object is allocated and is not loaded to memory already
		if(m_modelObjPool[i].allocated() && !(m_modelObjPool[i].getObject()->isLoadedToMemory()))
		{
			// Add the object to be loaded later
			//m_objectsBeingLoaded.push_back(LoadableGraphicsObjectAndIndex(m_modelObjPool[i].getObject(), m_modelObjPool[i].getIndex()));
			m_objectsBeingLoaded.emplace_back(m_modelObjPool[i].getObject(), m_modelObjPool[i].getIndex());

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;
		}
	}

	// Start loading ShaderModel Objects 
	for(decltype(m_shaderObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_shaderObjPool.getNumAllocated(),
		size = m_shaderObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// If current object is allocated and is not loaded to memory already
		if(m_shaderObjPool[i].allocated() && !(m_shaderObjPool[i].getObject()->isLoadedToMemory()))
		{
			// Add the object to be loaded later
			//m_objectsBeingLoaded.push_back(LoadableGraphicsObjectAndIndex(m_shaderObjPool[i].getObject(), m_shaderObjPool[i].getIndex()));
			m_objectsBeingLoaded.emplace_back(m_shaderObjPool[i].getObject(), m_shaderObjPool[i].getIndex());

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;
		}
	}
	
	// Start loading Environment Map Objects 
	for(decltype(m_envMapPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_envMapPool.getNumAllocated(),
		size = m_envMapPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// If current object is allocated and is not loaded to memory already
		if (m_envMapPool[i].allocated() && !(m_envMapPool[i].getObject()->isLoadedToMemory()))
		{
			// Add the object to be loaded later
			//m_objectsBeingLoaded.push_back(LoadableGraphicsObjectAndIndex(m_envMapPool[i].getObject(), m_shaderObjPool[i].getIndex()));
			m_objectsBeingLoaded.emplace_back(m_envMapPool[i].getObject(), m_shaderObjPool[i].getIndex());

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;
		}
	}

	// Load every object to memory. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), m_objectsBeingLoaded.size(), size_t(1), [=](size_t i)
	{
		m_objectsBeingLoaded[i].LoadToMemory();
	});*/

	// Go over each graphics object and add it to the loading array
	for(decltype(m_graphicsObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_graphicsObjPool.getNumAllocated(),
		size = m_graphicsObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// Check if the graphics object is allocated inside the pool container
		if(m_graphicsObjPool[i].allocated())
		{
			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;

			auto graphicsObject = m_graphicsObjPool[i].getObject();

			m_objectsLoadingToMemory.push_back(graphicsObject);
		}
	}

	// Load every object to memory. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), m_objectsLoadingToMemory.size(), size_t(1), [=](size_t i)
		{
			m_objectsLoadingToMemory[i]->loadToMemory();
		});

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
	// Implementation note: use number of allocated objects as an early bail - this method is most
	// likely called after populating pools, which means objects are lined up at the start of the pools)

	// Go over each graphics object
	for(decltype(m_graphicsObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_graphicsObjPool.getNumAllocated(),
		size = m_graphicsObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// Check if the graphics object is allocated inside the pool container
		if(m_graphicsObjPool[i].allocated())
		{
			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;

			auto graphicsObject = m_graphicsObjPool[i].getObject();

			// Add the graphics object to the loading array and start loading it in the background
			m_objectsLoadingToMemory.push_back(graphicsObject);
			TaskManagerLocator::get().startBackgroundThread(std::bind(&GraphicsObject::loadToMemory, graphicsObject));
		}
	}
}

PropertySet RendererScene::exportObject()
{
	// Create the root property set
	PropertySet propertySet(Properties::Graphics);
	/*
	// Add root property set for scene values
	auto &scene = propertySet.addPropertySet(Properties::Scene);

	// Add individual scene values
	scene.addProperty(Properties::ModelPoolSize, (int)m_modelObjPool.getPoolSize());
	scene.addProperty(Properties::PointLightPoolSize, (int)m_pointLightPool.getPoolSize());
	scene.addProperty(Properties::SpotLightPoolSize, (int)m_spotLightPool.getPoolSize());

	// Add root property set for all the objects
	auto &objects = propertySet.addPropertySet(Properties::Objects);

	// Add camera object property set
	objects.addPropertySet(m_camera->exportObject());

	// Add directional light property set
	objects.addPropertySet(m_directionalLight->exportObject());

	// Add static environment map property set
	objects.addPropertySet(m_skybox->exportObject());

	// Add model object property sets
	for(decltype(m_modelObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_modelObjPool.getNumAllocated(),
		size = m_modelObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_modelObjPool[i].allocated())
			objects.addPropertySet(m_modelObjPool[i].getObject()->exportObject());
	}

	// Add custom shader object property sets
	for(decltype(m_shaderObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_shaderObjPool.getNumAllocated(),
		size = m_shaderObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_shaderObjPool[i].allocated())
			objects.addPropertySet(m_shaderObjPool[i].getObject()->exportObject());
	}

	// Add point light property sets
	for(decltype(m_pointLightPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_pointLightPool.getNumAllocated(),
		size = m_pointLightPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_pointLightPool[i].allocated())
			objects.addPropertySet(m_pointLightPool[i].getObject()->exportObject());
	}

	// Add spot light property sets
	for(decltype(m_spotLightPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_spotLightPool.getNumAllocated(),
		size = m_spotLightPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_spotLightPool[i].allocated())
			objects.addPropertySet(m_spotLightPool[i].getObject()->exportObject());
	}
	*/
	return propertySet;
}

void RendererScene::update(const float p_deltaTime)
{
	// Clear variables from previous frame
	m_sceneObjects.m_directionalLight = &m_directionalLight->getLightDataSet();

	// Clear arrays from previous frame
	m_sceneObjects.m_pointLights.clear();
	m_sceneObjects.m_spotLights.clear();
	m_sceneObjects.m_loadToVideoMemory.clear();

	// Get the world scene required for attaching components to the entity
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
					modelComponent.setActive(true);

					// Get all loadable objects from the model component
					auto loadableObjectsFromModel = modelComponent.getLoadableObjects();

					// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
					for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
						if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
							m_sceneObjects.m_loadToVideoMemory.emplace_back(loadableObjectsFromModel[i]);
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
					shaderComponent.setActive(true);

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

	return;

	//m_sceneObjects.m_camera.m_spatialData.m_transformMat.initCamera(m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position, targetVector + m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position, upVector);



	// Clear variables
	//m_sceneObjects.m_staticSkybox = nullptr;
	//m_sceneObjects.m_directionalLight = nullptr;
	m_sceneObjects.m_directionalLight = &m_directionalLight->getLightDataSet();

	// Clear arrays from previous frame
	m_sceneObjects.m_pointLights.clear();
	m_sceneObjects.m_spotLights.clear();
	m_sceneObjects.m_modelData.clear();
	m_sceneObjects.m_modelDataWithShaders.clear();
	m_sceneObjects.m_loadToVideoMemory.clear();

	//	 _______________________________
	//	|								|
	//	|	 Objects to be destroyed	|
	//	|_______________________________|
	//
	// Iterate over objects that are queued to be destroyed
	if(!m_objectsToDestroy.empty())
		for(decltype(m_objectsToDestroy.size()) i = 0, size = m_objectsToDestroy.size(); i < size;)
		{
			// Check if the object isn't being currently loaded
			if(getCurrentlyLoadingObject(*m_objectsToDestroy[i]) == nullptr)
			{
				// Delete the object as it's not being loaded = not in use
				delete m_objectsToDestroy[i];

				// Remove the object from the destroy list
				m_objectsToDestroy.erase(m_objectsToDestroy.begin() + i);
			}
			// If current object is still loading, advance the index
			else
				i++;
		}

	//	 _______________________________
	//	|								|
	//	|	Currently Loading Objects	|
	//	|_______________________________|
	//
	// Iterate over currently loading objects
	for(decltype(m_objectsLoadingToMemory.size()) i = 0, size = m_objectsLoadingToMemory.size(),
		maxObjects = Config::rendererVar().objects_loaded_per_frame; i < size;)
	{
		// Perform a check that marks an object if it is loaded to memory
		m_objectsLoadingToMemory[i]->performCheckIsLoadedToMemory();

		// If the object has loaded to memory already, add to load queue
		if(m_objectsLoadingToMemory[i]->isLoadedToMemory())
		{
			// If object should be activated after loading (for example wasn't set to be deleted while loading)
			//if(m_objectsBeingLoaded[i].isActivatedAfterLoading())
			//{
				// Make object active, so it is passed to the renderer for drawing
			m_objectsLoadingToMemory[i]->setActive(true);

			if(m_objectsLoadingToMemory[i]->modelComponentPresent())
			{
				// Get all loadable objects from the model component
				auto loadableObjectsFromModel = m_objectsLoadingToMemory[i]->getModelComponent()->getLoadableObjects();

				// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
				for(decltype(loadableObjectsFromModel.size()) size = loadableObjectsFromModel.size(), i = 0; i < size; i++)
					if(!loadableObjectsFromModel[i].isLoadedToVideoMemory())
						m_sceneObjects.m_loadToVideoMemory.emplace_back(loadableObjectsFromModel[i]);
			}

			if(m_objectsLoadingToMemory[i]->shaderComponentPresent())
			{
				// Get all loadable objects from the model component
				auto loadableObjectsFromShader = m_objectsLoadingToMemory[i]->getShaderComponent()->getLoadableObjects();

				// Iterate over all loadable objects from the model component, and if any of them are not loaded to video memory already, add them to the to-load list
				for(decltype(loadableObjectsFromShader.size()) size = loadableObjectsFromShader.size(), i = 0; i < size; i++)
					if(!loadableObjectsFromShader[i].isLoadedToVideoMemory())
						m_sceneObjects.m_loadToVideoMemory.emplace_back(loadableObjectsFromShader[i]);
			}

			// Remove the object from the current list
			m_objectsLoadingToMemory.erase(m_objectsLoadingToMemory.begin() + i);
			
			// If the max number of object to be processed per frame has been reached, break from the loop
			if(--maxObjects == 0)
				break;
		}
		// If current object is still loading, advance the index
		else
			i++;
	}

	//	 ___________________________
	//	|							|
	//	|	  Graphics Objects		|
	//	|___________________________|
	//
	// Iterate over all graphics object and process them to be rendered
	for(decltype(m_graphicsObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_graphicsObjPool.getNumAllocated(),
		size = m_graphicsObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// Check if the graphics object is allocated inside the pool container
		if(m_graphicsObjPool[i].allocated())
		{
			auto *graphicsObject = m_graphicsObjPool[i].getObject();

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;

			// Check if the graphics object is enabled
			if(graphicsObject->isObjectActive())
			{
				// Update the object
				graphicsObject->update(p_deltaTime);

				// Check if the graphics object is already loaded to video memory (GPU)
				if(graphicsObject->isLoadedToVideoMemory())
				{
					// Check if the graphics object contains a model component
					if(graphicsObject->modelComponentPresent())
					{
						auto modelComponent = graphicsObject->getModelComponent();

						// Check if the graphics object contains a shader component
						if(graphicsObject->shaderComponentPresent())
						{
							// Loop over each model and add it to the render list of models with custom shaders
							for(decltype(modelComponent->m_modelData.size()) size = modelComponent->m_modelData.size(), i = 0; i < size; i++)
								m_sceneObjects.m_modelDataWithShaders.emplace_back(modelComponent->m_modelData[i], graphicsObject->getShaderComponent()->m_shaderData->m_shader, graphicsObject->getSpatialDataManagerReference().getWorldTransform());
						}
						else
						{
							// Loop over each model and add it to the render list of models with default shaders
							for(decltype(modelComponent->m_modelData.size()) size = modelComponent->m_modelData.size(), i = 0; i < size; i++)
								m_sceneObjects.m_modelData.emplace_back(modelComponent->m_modelData[i], graphicsObject->getSpatialDataManagerReference().getWorldTransform());
						}
					}

					// Check if the graphics object contains a light component
					if(graphicsObject->lightComponentPresent())
					{
						auto lightComponent = graphicsObject->getLightComponent();

						// Check if the light is enabled
						if(lightComponent->isObjectActive())
						{
							// Add the light data to the corresponding array, based on the light type
							switch(lightComponent->getLightType())
							{
							case LightComponent::LightComponentType_point:
								m_sceneObjects.m_pointLights.push_back(*lightComponent->getPointLight());
								break;
							case LightComponent::LightComponentType_spot:
								m_sceneObjects.m_spotLights.push_back(*lightComponent->getSpotLight());
								break;
							case LightComponent::LightComponentType_directional:
								m_sceneObjects.m_directionalLight = lightComponent->getDirectionalLight();
								break;
							}
						}
					}

					// Check if the graphics object contains a camera component
					if(graphicsObject->cameraComponentPresent())
					{
						m_sceneObjects.m_camera.m_spatialData.m_transformMat = graphicsObject->getSpatialDataManagerReference().getWorldTransform();

						/*glm::vec3 m_positionVec(0.0f, 0.0f, 0.0f);
						glm::vec3 m_targetVec(0.0f, 0.0f, 0.0f);
						glm::vec3 m_upVector(0.0f, 1.0f, 0.0f);
						glm::vec3 m_horizontalVec(0.0f, 0.0f, 0.0f);

						float m_verticalAngle = 0.5f;
						float m_horizontalAngle = 3.14f;

						// Calculate camera's rotation
						m_targetVec.target(m_verticalAngle, m_horizontalAngle);
						m_horizontalVec.horizontal(m_horizontalAngle);

						// Calculate camera's position based on the pressed movement keys
						m_upVector = Math::cross(m_horizontalVec, m_targetVec);
						m_sceneObjects.m_camera.m_spatialData.m_transformMat.initCamera(m_positionVec, m_targetVec + m_positionVec, m_upVector);

						// Set the target vector variable, so it can be retrieved later by listeners
						m_targetVec = glm::vec3(0.0f);
						m_targetVec.y = m_verticalAngle;
						m_targetVec.z = m_horizontalAngle;

						m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position = m_positionVec;
						m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler = m_targetVec;*/

						//m_sceneObjects.m_camera.m_spatialData.m_transformMat.initCamera(m_positionVec, m_targetVec + m_positionVec, m_upVector);
						//m_sceneObjects.m_camera.m_spatialData.m_transformMat.initCamera(m_positionVec, m_targetVec + m_positionVec, m_upVector);
					}
				}
			}
			else
			{

			}
		}
	}

	/*/	 ___________________________
	//	|							|
	//	|		Model Objects		|
	//	|___________________________|
	//
	// Update objects and put them into scene object list (use number of allocated objects as early bail)
	for(decltype(m_modelObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_modelObjPool.getNumAllocated(),
		size = m_modelObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_modelObjPool[i].allocated())
		{
			auto *object = m_modelObjPool[i].getObject();
			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;

			// Check if object is active, if so, update it and assign it to 'to-be-rendered' array
			if(object->isObjectActive())
			{
				// Update object
				object->update(p_deltaTime);

				// Place the object to the appropriate array
				if(object->isAffectedByLighting())
					m_sceneObjects.m_modelObjects.push_back(&object->getRenderableObjectData());
				else
					m_sceneObjects.m_postLightingObjects.push_back(&object->getRenderableObjectData());

				// Put the object in the appropriate array
				//m_sceneObjects.m_modelObjects.push_back(&object->getRenderableObjectData());
			}
		}
	}

	//	 ___________________________
	//	|							|
	//	|	   Shader Objects		|
	//	|___________________________|
	//
	// Update custom shader objects and put them into scene object list (use number of allocated objects as early bail)
	for(decltype(m_shaderObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_shaderObjPool.getNumAllocated(),
		size = m_shaderObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_shaderObjPool[i].allocated())
		{
			auto *object = m_shaderObjPool[i].getObject();
			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;

			// Check if object is active, if so, update it and assign it to 'to-be-rendered' array
			if(object->isObjectActive())
			{
				// Update object
				object->update(p_deltaTime);

				// Place the object to the appropriate array
				if(object->isAffectedByLighting())
					m_sceneObjects.m_customShaderObjects.push_back(&object->getRenderableObjectData());
				else
					m_sceneObjects.m_postLightingObjects.push_back(&object->getRenderableObjectData());
			}
		}
	}

	// TODO make isActive a part of system object
	if(m_directionalLight->active())
		m_directionalLight->update(p_deltaTime);

	//	 ___________________________
	//	|							|
	//	|		Point Lights		|
	//	|___________________________|
	//
	// Put all the active point lights into scene object list
	for(decltype(m_pointLightPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_pointLightPool.getNumAllocated(),
		size = m_pointLightPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_pointLightPool[i].allocated())
		{
			auto *light = m_pointLightPool[i].getObject();

			// Increment the number of allocated lights (early bail mechanism)
			numAllocObjecs++;

			if(light->active())
			{
				m_sceneObjects.m_pointLights.push_back(light->getLightDataSet());
			}
		}
	}

	//	 ___________________________
	//	|							|
	//	|		 Spot Lights		|
	//	|___________________________|
	//
	// Put all the active spot lights into scene object list
	for(decltype(m_spotLightPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_spotLightPool.getNumAllocated(),
		size = m_spotLightPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		if(m_spotLightPool[i].allocated())
		{
			auto *light = m_spotLightPool[i].getObject();

			// Increment the number of allocated lights (early bail mechanism)
			numAllocObjecs++;

			if(light->active())
			{
				m_sceneObjects.m_spotLights.push_back(light->getLightDataSet());
			}
		}
	}*/



	/*/	 ___________________________
	//	|							|
	//	|	  Light Components		|
	//	|___________________________|
	//
	// Put all the active lights into scene object lists, separate by light-type
	for(decltype(m_lightComponents.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_lightComponents.getNumAllocated(),
		size = m_lightComponents.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// Check if the light component object is allocated inside the pool container
		if(m_lightComponents[i].allocated())
		{
			auto* lightComponent = m_lightComponents[i].getObject();

			// Increment the number of allocated lights (early bail mechanism)
			numAllocObjecs++;

			// Check if the light is enabled
			if(lightComponent->isObjectActive())
			{
				// Add the light data to the corresponding array, based on the light type
				switch(lightComponent->getLightType())
				{
				case LightComponent::LightComponentType_point:
					m_sceneObjects.m_pointLights.push_back(*lightComponent->getPointLight());
					break;
				case LightComponent::LightComponentType_spot:
					m_sceneObjects.m_spotLights.push_back(*lightComponent->getSpotLight());
					break;
				case LightComponent::LightComponentType_directional:
					m_sceneObjects.m_directionalLight = lightComponent->getDirectionalLight();
					break;
				}
			}

		}
	}*/


	// Update camera spatial data
	calculateCamera(m_sceneObjects.m_camera.m_spatialData);

	//std::cout << GetString(static_cast<Properties::PropertyID>(10)) << std::endl;
	

	//m_sceneObjects.m_camera.m_spatialData.m_rotationEuler = Math::toRadian(p_viewData.m_spatialData.m_rotationEuler);
	/*m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.y = 0.5f;
	m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.z = 3.14f;

	//const glm::vec3 upVector = Math::cross(m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.z, m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.y);
	//const glm::vec3 targetVector(0.0f, m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.y, m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler.z);

	//m_sceneObjects.m_camera.m_spatialData.m_transformMat.initCamera(m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position, targetVector + m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position, upVector);

	glm::vec3 m_positionVec(0.0f, 0.0f, 0.0f);
	glm::vec3 m_targetVec(0.0f, 0.0f, 0.0f);
	glm::vec3 m_upVector(0.0f, 1.0f, 0.0f);
	glm::vec3 m_horizontalVec(0.0f, 0.0f, 0.0f);

	float m_verticalAngle = 0.0f;
	float m_horizontalAngle = 3.14f;

	// Calculate camera's rotation
	m_targetVec.target(m_verticalAngle, m_horizontalAngle);
	m_horizontalVec.horizontal(m_horizontalAngle);

	// Calculate camera's position based on the pressed movement keys
	m_upVector = Math::cross(m_horizontalVec, m_targetVec);
	m_sceneObjects.m_camera.m_spatialData.m_transformMat.initCamera(m_positionVec, m_targetVec + m_positionVec, m_upVector);

	// Set the target vector variable, so it can be retrieved later by listeners
	m_targetVec = glm::vec3(0.0f);
	m_targetVec.y = m_verticalAngle;
	m_targetVec.z = m_horizontalAngle;

	m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_position = m_positionVec;
	m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler = m_targetVec;

	m_sceneObjects.m_camera.m_spatialData.m_spatialData.m_rotationEuler = glm::vec3(0.0f);

	m_sceneObjects.m_camera.m_spatialData.m_transformMat = Math::createTransformMat(
		glm::vec3(0.0f),
		glm::vec3(0.0f, 0.0f, 45.0f),
		glm::vec3(1.0f));

	glm::vec3 rotation(30.0f, 30.0f, 0.0f);
	rotation = Math::toRadian(rotation);

	float cosY = cosf(rotation.y);     // Yaw
	float sinY = sinf(rotation.y);

	float cosP = cosf(rotation.x);     // Pitch
	float sinP = sinf(rotation.x);

	float cosR = cosf(rotation.z);     // Roll
	float sinR = sinf(rotation.z);

	glm::mat4 mat;
	mat.identity();
	mat.m[0] = cosY * cosR + sinY * sinP * sinR;
	mat.m[1] = cosR * sinY * sinP - sinR * cosY;
	mat.m[2] = cosP * sinY;

	mat.m[4] = cosP * sinR;
	mat.m[5] = cosR * cosP;
	mat.m[6] = -sinP;

	mat.m[8] = sinR * cosY * sinP - sinY * cosR;
	mat.m[9] = sinY * sinR + cosR * cosY * sinP;
	mat.m[10] = cosP * cosY;

	m_sceneObjects.m_camera.m_spatialData.m_transformMat = mat * glm::mat4();*/

	//m_sceneObjects.m_staticSkybox = m_skybox;
}

SystemObject *RendererScene::createComponent(const EntityID &p_entityID, const std::string &p_entityName, const PropertySet &p_properties)
{
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->createObject(p_properties);

	// Check if property set node is present
	if(p_properties)
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

		switch(p_properties.getPropertyID())
		{
			case Properties::PropertyID::CameraComponent:
			{
				//auto &component = worldScene->addComponent<CameraComponent>(p_entityID, this, p_entityName + Config::componentVar().camera_component_name);
				auto &component = worldScene->addComponent<CameraComponent>(p_entityID, this, p_entityName + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::CameraComponent), p_entityID);

				// Try to initialize the camera component
				auto componentInitError = component.init();
				if(componentInitError == ErrorCode::Success)
				{
					// Try to import the component
					auto const &componentImportError = component.importObject(p_properties);

					// Remove the component if it failed to import
					if(componentImportError != ErrorCode::Success)
					{
						worldScene->removeComponent<CameraComponent>(p_entityID);
						ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_CameraComponent, p_entityName);
					}
					else
						returnObject = &component;
				}
				else // Remove the component if it failed to initialize
				{
					worldScene->removeComponent<CameraComponent>(p_entityID);
					ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_CameraComponent, p_entityName);
				}

			}
			break;

			case Properties::PropertyID::LightComponent:
			{
				auto &component = worldScene->addComponent<LightComponent>(p_entityID, this, p_entityName + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::LightComponent), p_entityID);

				// Try to initialize the light component
				auto componentInitError = component.init();
				if(componentInitError == ErrorCode::Success)
				{
					// Try to import the component
					auto const &componentImportError = component.importObject(p_properties);

					// Remove the component if it failed to import
					if(componentImportError != ErrorCode::Success)
					{
						worldScene->removeComponent<LightComponent>(p_entityID);
						ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_LightComponent, p_entityName);
					}
					else
						returnObject = &component;
				}
				else // Remove the component if it failed to initialize
				{
					worldScene->removeComponent<LightComponent>(p_entityID);
					ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_LightComponent, p_entityName);
				}
			}
			break;

			case Properties::PropertyID::ModelComponent:
			{
				// Create the model component
				auto &component = worldScene->addComponent<ModelComponent>(p_entityID, this, p_entityName + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::ModelComponent), p_entityID);

				// Try to initialize the model component
				auto componentInitError = component.init();
				if(componentInitError == ErrorCode::Success)
				{
					// Try to import the component
					auto const &componentImportError = component.importObject(p_properties);

					// Remove the component if it failed to import
					if(componentImportError != ErrorCode::Success)
					{
						worldScene->removeComponent<ModelComponent>(p_entityID);
						ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_ModelComponent, p_entityName);
					}
					else
					{
						returnObject = &component;

						// Add the component to an array signifying that it is currently being loaded to memory
						m_componentsLoadingToMemory.emplace_back(component);
					}
				}
				else // Remove the component if it failed to initialize
				{
					worldScene->removeComponent<ModelComponent>(p_entityID);
					ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ModelComponent, p_entityName);
				}
			}
			break;

			case Properties::PropertyID::ShaderComponent:
			{
				// Check if there is a property set for shaders and load the shader component if there is
				auto const &shaders = p_properties.getPropertySetByID(Properties::Shaders);
				if(shaders)
				{
					// Create the shader component
					auto &component = worldScene->addComponent<ShaderComponent>(p_entityID, this, p_entityName + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::ShaderComponent), p_entityID);

					// Try to initialize the shader component
					auto componentInitError = component.init();
					if(componentInitError == ErrorCode::Success)
					{
						// Try to import the component
						auto const &componentImportError = component.importObject(shaders);

						// Remove the component if it failed to import
						if(componentImportError != ErrorCode::Success)
						{
							worldScene->removeComponent<ShaderComponent>(p_entityID);
							ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_ShaderComponent, p_entityName);
						}
						else
						{
							returnObject = &component;

							// Add the component to an array signifying that it is currently being loaded to memory
							m_componentsLoadingToMemory.emplace_back(component);
						}
					}
					else // Remove the component if it failed to initialize
					{
						worldScene->removeComponent<ShaderComponent>(p_entityID);
						ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ShaderComponent, p_entityName);
					}
				}
			}
			break;
		}
	}
	
	return returnObject;
}

SystemObject *RendererScene::createObject(const PropertySet &p_properties)
{
	// Check if property set node is present
	if(p_properties)
	{
		// Check if the rendering property is present
		auto &renderingProperty = p_properties.getPropertySetByID(Properties::Rendering);
		if(renderingProperty)
		{
			// Get the object name
			auto &nameProperty = p_properties.getPropertyByID(Properties::Name);

			// Find a place for the new object in the pool
			auto graphicsObjectFromPool = m_graphicsObjPool.newObject();

			// Check if the pool wasn't full
			if(graphicsObjectFromPool != nullptr)
			{
				std::string name;

				// If the name property is missing, generate a unique name based on the object's index in the pool
				if(nameProperty)
					name = nameProperty.getString() + " (" + GetString(Properties::GraphicsObject) + ")";
				else
					name = GetString(Properties::GraphicsObject) + Utilities::toString(graphicsObjectFromPool->getIndex());
				
				// Construct the GraphicsObject
				graphicsObjectFromPool->construct(this, name);
				auto graphicsObject = graphicsObjectFromPool->getObject();

				//graphicsObject->importObject(renderingProperty);

				// Start importing the newly created object in a background thread
				//TaskManagerLocator::get().startBackgroundThread(std::bind(&GraphicsObject::importObject, graphicsObject, renderingProperty));
				graphicsObject->importObject(renderingProperty);
				graphicsObject->loadToMemory();
			
				return graphicsObject;
			}
			else
			{
				ErrHandlerLoc::get().log(ErrorCode::ObjectPool_full, ErrorSource::Source_RendererScene, "Failed to add GraphicsObject - \'" + nameProperty.getString() + "\'");
			}
		}
	}

	// If valid type was not specified, or object creation failed, return a null object instead
	return g_nullSystemBase.getScene()->createObject(p_properties);
}
ErrorCode RendererScene::destroyObject(SystemObject *p_systemObject)
{
	// Check if object is valid and belongs to graphics system
	if(p_systemObject != nullptr && p_systemObject->getSystemType() == Systems::Graphics)
	{
		// Cast the system object to graphics object, as it belongs to the renderer scene
		GraphicsObject *objectToDestroy = static_cast<GraphicsObject *>(p_systemObject);

		// Check if the object is being currently loaded
		auto loadingObject = getCurrentlyLoadingObject(*p_systemObject);

		if(loadingObject != nullptr)
		{
			// If it is currently being loaded, add it to the destroy list, as it cannot be deleted now while it is being used
			m_objectsToDestroy.push_back(objectToDestroy);
			return ErrorCode::Success;
		}
		else
		{
			// Try to destroy the object; return success if it succeeds
			if(removeObjectFromPool(*objectToDestroy))
				return ErrorCode::Success;
		}
	}

	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

void RendererScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
	//std::cout << "change occurred" << std::endl;
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

ModelComponent *RendererScene::loadModelComponent(const PropertySet &p_properties)
{
	ModelComponent *newComponent = nullptr;
		
	// Check if models node is present
	if(p_properties)
	{
		// Create the model component
		newComponent = new ModelComponent(this, "", 0);

		// Loop over each model entry in the node
		for(decltype(p_properties.getNumProperties()) iModel = 0, numModels = p_properties.getNumProperties(); iModel < numModels; iModel++)
		{
			// Get model filename
			auto modelName = p_properties.getPropertySet(iModel).getPropertyByID(Properties::Filename).getString();

			// Add a new model data entry, and get a reference to it
			newComponent->m_modelData.push_back(ModelData(Loaders::model().load(modelName, false)));
			auto &newModelData = newComponent->m_modelData.back();

			// Load the model to memory, to be able to access all of its meshes
			newModelData.m_model.loadToMemory();

			// Get the meshes array
			auto meshesInModelArray = newModelData.m_model.getMeshArray();

			// Get the meshes array
			auto meshesProperty = p_properties.getPropertySet(iModel).getPropertySetByID(Properties::Meshes);

			// Check if the meshes array node is present;
			// If it is present, only add the meshes included in the meshes node
			// If it is not present, add all the meshes included in the model
			if(meshesProperty)
			{
				// Loop over each mesh entry in the model node
				for(decltype(meshesProperty.getNumProperties()) iMesh = 0, numMeshes = meshesProperty.getNumProperties(); iMesh < numMeshes; iMesh++)
				{
					// Try to get the mesh index property node and check if it is present
					auto meshIndexProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::Index);
					if(meshIndexProperty)
					{
						// Get the mesh index, check if it is valid and within the range of mesh array that was loaded from the model
						const int meshDataIndex = meshIndexProperty.getInt();
						if(meshDataIndex > 0 && meshDataIndex < meshesInModelArray.size())
						{
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
									// Load the material data
									materials[iMatType] = loadMaterialData(materialProperties[iMatType], newModelData.m_model.getMaterialArrays(), static_cast<MaterialType>(iMatType), meshDataIndex);
								}
							}

							/*if(materialsProperty)
							{
								auto diffuseMatProperty = materialsProperty.getPropertySetByID(Properties::Diffuse);
								materials[MaterialType_Diffuse] = loadMaterialData(diffuseMatProperty, newModelData.m_model.getMaterialArrays(), MaterialType_Diffuse, meshDataIndex);

								//newModelData.m_model.getMaterialArrays()
								//materials[MaterialType_Diffuse] = &materialProperty.getPropertySetByID(Properties::Diffuse);
								//materials[MaterialType_Normal] = &materialProperty.getPropertySetByID(Properties::Normal);
								//materials[MaterialType_Emissive] = &materialProperty.getPropertySetByID(Properties::Emissive);
								//materials[MaterialType_Combined] = &materialProperty.getPropertySetByID(Properties::RMHAO);
							}*/
							
							//newModelData.m_meshes.push_back(MeshData(meshesInModelArray[iMesh], materials));
						}
					}
				}
			}
			else
			{
				// Get the material arrays that were loaded from the model file
				auto &materialArrayFromModel = newModelData.m_model.getMaterialArrays();

				// Iterate over every mesh in the model
				for(decltype(meshesInModelArray.size()) iMesh = 0, numMeshes = meshesInModelArray.size(); iMesh < numMeshes; iMesh++)
				{			
					// Define material data and material properties
					MaterialData materials[MaterialType::MaterialType_NumOfTypes];

					// Go over each mesh in the model
					if(iMesh > materialArrayFromModel.m_numMaterials)
					{
						// Go over each material type
						for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
						{
							// Get the texture filename and load it to memory
							auto textureFromModel = Loaders::texture2D().load(materialArrayFromModel.m_materials[iMatType][iMesh].m_filename, static_cast<MaterialType>(iMatType), false);
							auto materialLoadError = textureFromModel.loadToMemory();
														
							// Check if the texture was loaded successfully
							if(materialLoadError == ErrorCode::Success)
							{
								materials[MaterialType::MaterialType_Diffuse].m_texture = textureFromModel;
							}
							else
							{
								ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
							}
						}
						
						// Add the data for this mesh. Include materials loaded from the model itself, if they were present, otherwise, include default textures instead
						//newModelData.m_meshes.push_back(MeshData(meshesInModelArray[iMesh], materials));
					}
				}
			}
		}
	}

	return newComponent;
}
ShaderComponent *RendererScene::loadShaderComponent(const PropertySet &p_properties)
{
	ShaderComponent *newComponent = nullptr;

	// Check if shaders node is valid
	if(p_properties)
	{
		// Get nodes for different shader types
		auto fragmentShaderNode = p_properties.getPropertyByID(Properties::FragmentShader);
		auto vertexShaderNode = p_properties.getPropertyByID(Properties::VertexShader);

		// Check if any of the shader nodes are present
		if(fragmentShaderNode || vertexShaderNode)
		{
			// Load shader program
			auto shaderProgram = Loaders::shader().load(p_properties);

			// If shader is not default (i.e. at least one of the shader types was loaded)
			if(!shaderProgram->isDefaultProgram())
			{
				// Load the shader to memory and assign it to the new shader component
				shaderProgram->loadToMemory();
				newComponent = new ShaderComponent(this, "", *shaderProgram, 0);
			}
		}
	}

	return newComponent;
}
LightComponent *RendererScene::loadLightComponent(const PropertySet &p_properties)
{
	LightComponent *newComponent = nullptr;

	// Check if the property node is valid
	if(p_properties)
	{
		glm::vec3 color;
		float	intensity = 0.0f,
				cutoffAngle = 0.0f;
		Properties::PropertyID type = Properties::PropertyID::Null;

		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::Color:
				color = p_properties[i].getVec3f();
				break;

			case Properties::CutoffAngle:
				// Convert to radians
				cutoffAngle = cosf(glm::radians(p_properties[i].getFloat()));
				break;

			case Properties::Intensity:
				intensity = p_properties[i].getFloat();
				break;

			case Properties::Type:
				type = p_properties[i].getID();
				break;
			}
		}

		// Create light component based on light type
		switch(type)
		{
		case Properties::DirectionalLight:
			{
				// Create and setup the directional light data set
				DirectionalLightDataSet dirLightDataSet;
				dirLightDataSet.m_color = color;
				dirLightDataSet.m_intensity = intensity;

				// Create the component of the directional light type
				newComponent = new LightComponent(this, "", dirLightDataSet, 0);
			}
			break;
		case Properties::PointLight:
			{
				// Create and setup the point light data set
				PointLightDataSet pointLightDataSet;
				pointLightDataSet.m_color = color;
				pointLightDataSet.m_intensity = intensity;

				// Create the component of the point light type
				newComponent = new LightComponent(this, "",pointLightDataSet, 0);
			}
			break;
		case Properties::SpotLight:
			{
				// Create and setup the spot light data set
				SpotLightDataSet spotLightDataSet;
				spotLightDataSet.m_color = color;
				spotLightDataSet.m_cutoffAngle = cutoffAngle;
				spotLightDataSet.m_intensity = intensity;

				// Create the component of the spot light type
				newComponent = new LightComponent(this, "", spotLightDataSet, 0);
			}
			break;
		}
	}

	return newComponent;
}

ModelObject *RendererScene::loadModelObject(const PropertySet &p_properties)
{
	// Get model properties
	auto &models = p_properties.getPropertySetByID(Properties::Models);
	auto &shaderProperty = p_properties.getPropertySetByID(Properties::Shaders);

	ErrorCode objPoolError = ErrorCode::Failure;
	ModelObject *newObject = nullptr;
	/*
	// If shaders are present
	if(shaderProperty)
	{
		// Try to add a new object to the pool
		objPoolError = m_shaderObjPool.add(
			this, p_properties.getPropertyByID(Properties::Name).getString(),
			Loaders::model().load(models.getPropertyByID(Properties::Filename).getString(), false),
			Loaders::shader().load(shaderProperty));

		// The newly added object in the pool
		newObject = m_shaderObjPool.getLastRawObject();
		newObject->getRenderableObjectData().m_shader->loadToMemory();
	}
	else
	{
		// Try to add a new object to the pool
		objPoolError = m_modelObjPool.add(
			this, p_properties.getPropertyByID(Properties::Name).getString(),
			Loaders::model().load(models.getPropertyByID(Properties::Filename).getString(), false));

		// The newly added object in the pool
		newObject = m_modelObjPool.getLastRawObject();
	}

	// If adding a new object was successful, continue to load data
	if(objPoolError == ErrorCode::Success)
	{
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::OffsetPosition:
				newObject->setOffsetPosition(p_properties[i].getVec3f());
				break;
			case Properties::OffsetRotation:
				newObject->setOffsetRotation(p_properties[i].getVec3f());
				break;
			case Properties::LocalPosition:
				newObject->setPosition(p_properties[i].getVec3f());
				break;
			case Properties::LocalRotation:
				newObject->setRotation(p_properties[i].getVec3f());
				break;
			case Properties::LocalScale:
				newObject->setScale(p_properties[i].getVec3f());
				break;
			case Properties::Lighting:
				newObject->setLighting(p_properties[i].getBool());
				break;
			case Properties::AlphaThreshold:
				newObject->setAlphaThreshold(p_properties[i].getFloat());
				break;
			case Properties::HeightScale:
				newObject->setHeightScale(p_properties[i].getFloat());
				break;
			case Properties::TextureTilingFactor:
				newObject->setTextureTilingFactor(p_properties[i].getFloat());
				break;
			}
		}

		// Adjust the position and rotation by offset
		newObject->setPosition(newObject->getBaseObjectData().m_position + newObject->getBaseObjectData().m_offsetPosition);
		newObject->setRotation(newObject->getBaseObjectData().m_rotation + newObject->getBaseObjectData().m_offsetRotation);

		// Get material parent property
		auto &materialProperty = p_properties.getPropertySetByID(Properties::Materials);

		// Get material properties
		const PropertySet *materials[MaterialType_NumOfTypes];
		materials[MaterialType_Diffuse] = &materialProperty.getPropertySetByID(Properties::Diffuse);
		materials[MaterialType_Normal] = &materialProperty.getPropertySetByID(Properties::Normal);
		materials[MaterialType_Emissive] = &materialProperty.getPropertySetByID(Properties::Emissive);
		materials[MaterialType_Combined] = &materialProperty.getPropertySetByID(Properties::RMHAO);
		
		// Process all materials
		// For every type of material
		for(unsigned int matType = 0; matType < MaterialType_NumOfTypes; matType++)
			// Check if material property is valid
			if(*materials[matType])
				// For every property set in the material property
				for(decltype(materials[matType]->getNumPropertySets()) i = 0, size = materials[matType]->getNumPropertySets(); i < size; i++)
					// Add the material to the new object
					newObject->addMaterial(static_cast<MaterialType>(matType),
										   materials[matType]->getPropertySet(i).getPropertyByID(Properties::Filename).getString(),
										   materials[matType]->getPropertySet(i).getPropertyByID(Properties::Index).getInt());
	
		// Get default material properties
		// Default materials replace any missing materials from the model file
		auto &defaultMaterials = materialProperty.getPropertySetByID(Properties::Default);
		if(defaultMaterials)
		{
			// Get individual default materials
			const PropertySet *defaulMaterials[MaterialType_NumOfTypes];
			defaulMaterials[MaterialType_Diffuse] = &defaultMaterials.getPropertySetByID(Properties::Diffuse);
			defaulMaterials[MaterialType_Normal] = &defaultMaterials.getPropertySetByID(Properties::Normal);
			defaulMaterials[MaterialType_Emissive] = &defaultMaterials.getPropertySetByID(Properties::Emissive);
			defaulMaterials[MaterialType_Combined] = &defaultMaterials.getPropertySetByID(Properties::RMHAO);

			// Process default materials by assigning them to the model object
			for(unsigned int matType = 0; matType < MaterialType_NumOfTypes; matType++)
				if(*defaulMaterials[matType])
					newObject->setDefaultMaterial(matType, defaulMaterials[matType]->getPropertyByID(Properties::Filename).getString());
		}

		return newObject;
	}
	// If adding a new object failed, log an error and return a nullptr
	else
	{
		ErrHandlerLoc::get().log(objPoolError, ErrorSource::Source_SceneLoader);
		return nullptr;
	}*/
	return newObject;
}
CameraObject *RendererScene::loadCameraObject(const PropertySet & p_properties)
{
	// Delete an existing camera
	if(m_camera != nullptr)
		delete m_camera;

	// Create a new camera
	m_camera = new CameraObject(this, p_properties.getPropertyByID(Properties::Name).getString());
	
	return m_camera;
}
EnvironmentMapObject *RendererScene::loadEnvironmentMap(const PropertySet &p_properties)
{
	EnvironmentMapObject *newObject = nullptr;
	/*ErrorCode objPoolError = ErrorCode::Failure;
	bool staticEnvMap = false;

	auto &materials = p_properties.getPropertySetByID(Properties::Materials);

	std::string filenames[CubemapFace_NumOfFaces];

	if(materials.getNumPropertySets() >= CubemapFace_NumOfFaces)
		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
			filenames[face] = materials.getPropertySet(face).getPropertyByID(Properties::Filename).getString();

	// Check if the environment map should be a static one
	staticEnvMap = p_properties.getPropertyByID(Properties::Static).getBool();

	if(staticEnvMap)
	{
		//newObject = new EnvironmentMapObject(this,
		//	p_properties.getPropertyByID(Properties::Name).getString(),
		//	Loaders::textureCubemap().load(filenames, false));

		m_skybox = newObject;
	}
	else
	{
		objPoolError = m_envMapPool.add(this,
			p_properties.getPropertyByID(Properties::Name).getString(),
			Loaders::textureCubemap().load(filenames, false));

		// If adding a new object failed, log an error and return a nullptr
		if (objPoolError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(objPoolError, ErrorSource::Source_SceneLoader);
			return nullptr;
		}
	}

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		//switch(p_properties[i].getPropertyID())
		//{
		//case Properties::Position:
		//	newObject->setPosition(p_properties[i].getVec3f());
		//	break;
		//}
	}*/

	return newObject;
}
DirectionalLightObject *RendererScene::loadDirectionalLight(const PropertySet &p_properties)
{
	// Since only one directional light is supported at a time, delete the old one first
	delete m_directionalLight;

	m_directionalLight = new DirectionalLightObject(this, 
													p_properties.getPropertyByID(Properties::Name).getString(), 
													DirectionalLightDataSet());
	
	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Color:
			m_directionalLight->setColor(p_properties[i].getVec3f());
			break;

		case Properties::Direction:
			// Need to normalize the light direction
			m_directionalLight->setDirection(glm::normalize(p_properties[i].getVec3f()));
			break;

		case Properties::Intensity:
			m_directionalLight->setIntensity(p_properties[i].getFloat());
			break;
		}
	}

	m_directionalLight->init();

	return m_directionalLight;
}
PointLightObject *RendererScene::loadPointLight(const PropertySet &p_properties)
{
	ErrorCode objPoolError = ErrorCode::Failure;
	PointLightObject *newObject = nullptr;
	/*
	// Try to add a new object to the pool
	objPoolError = m_pointLightPool.add(
		this, p_properties.getPropertyByID(Properties::Name).getString(), PointLightDataSet());

	// Get the newly added object in the pool
	newObject = m_pointLightPool.getLastRawObject();

	// If adding a new object was successful, continue to load data
	if(objPoolError == ErrorCode::Success)
	{
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::Attenuation:
				newObject->setAttenuation(p_properties[i].getVec3f());
				break;

			case Properties::Color:
				newObject->setColor(p_properties[i].getVec3f());
				break;

			case Properties::Intensity:
				newObject->setIntensity(p_properties[i].getFloat());
				break;

			case Properties::OffsetPosition:
				newObject->setOffsetPosition(p_properties[i].getVec3f());
				break;

			case Properties::Position:
				newObject->setPosition(p_properties[i].getVec3f());
				break;
			}
		}

		// Adjust the position and rotation by offset
		newObject->setPosition(newObject->getLightDataSet().m_position + newObject->getOffsetPosition());
	}
	// If adding a new object failed, log an error and return a nullptr
	else
	{
		ErrHandlerLoc::get().log(objPoolError, ErrorSource::Source_SceneLoader);
		return nullptr;
	}
	*/
	return newObject;
}
SpotLightObject *RendererScene::loadSpotLight(const PropertySet &p_properties)
{
	ErrorCode objPoolError = ErrorCode::Failure;
	SpotLightObject *newObject = nullptr;
	/*
	// Try to add a new object to the pool
	objPoolError = m_spotLightPool.add(
		this, p_properties.getPropertyByID(Properties::Name).getString(), SpotLightDataSet());

	// Get the newly added object in the pool
	newObject = m_spotLightPool.getLastRawObject();

	// If adding a new object was successful, continue to load data
	if(objPoolError == ErrorCode::Success)
	{
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::Attenuation:
				newObject->setAttenuation(p_properties[i].getVec3f());
				break;

			case Properties::CutoffAngle:
				// Convert to radians
				newObject->setCutoffAngle(cosf(Math::toRadian(p_properties[i].getFloat())));
				break;

			case Properties::Color:
				newObject->setColor(p_properties[i].getVec3f());
				break;

			case Properties::Direction:
				newObject->setDirection(p_properties[i].getVec3f());
				break;

			case Properties::Intensity:
				newObject->setIntensity(p_properties[i].getFloat());
				break;

			case Properties::OffsetPosition:
				newObject->setOffsetPosition(p_properties[i].getVec3f());
				break;

			case Properties::OffsetRotation:
				newObject->setOffsetRotation(p_properties[i].getVec3f());
				break;

			case Properties::Position:
				newObject->setPosition(p_properties[i].getVec3f());
				break;
			}
		}

		// Adjust the position and rotation by offset (and normalize the direction)

		newObject->setPosition(newObject->getLightDataSet().m_position + newObject->getOffsetPosition());
		newObject->setDirection(Math::normalize(newObject->getLightDataSet().m_direction + newObject->getOffsetRotation()));
	}
	// If adding a new object failed, log an error and return a nullptr
	else
	{
		ErrHandlerLoc::get().log(objPoolError, ErrorSource::Source_SceneLoader);
		return nullptr;
	}
	*/
	return newObject;
}
