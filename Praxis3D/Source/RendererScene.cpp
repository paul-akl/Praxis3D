
#include "RendererScene.h"
#include "RendererSystem.h"

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
	
	return returnError;
}

ErrorCode RendererScene::setup(const PropertySet &p_properties)
{
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ModelPoolSize:
			m_modelObjPool.init(p_properties[i].getInt());
			break;
		case Properties::PointLightPoolSize:
			m_pointLightPool.init(p_properties[i].getInt());
			break;
		case Properties::ShaderPoolSize:
			m_shaderObjPool.init(p_properties[i].getInt());
			break;
		case Properties::SpotLightPoolSize:
			m_spotLightPool.init(p_properties[i].getInt());
			break;
		}
	}

	// If a pool hasn't been initialized, initialize it to the default size
	if(m_modelObjPool.getPoolSize() <= 1)
		m_modelObjPool.init(Config::objectPoolVar().model_object_pool_size);
	if(m_pointLightPool.getPoolSize() <= 1)
		m_pointLightPool.init(Config::objectPoolVar().point_light_pool_size);
	if(m_shaderObjPool.getPoolSize() <= 1)
		m_shaderObjPool.init(Config::objectPoolVar().shader_object_pool_size);
	if(m_spotLightPool.getPoolSize() <= 1)
		m_spotLightPool.init(Config::objectPoolVar().spot_light_pool_size);

	return ErrorCode::Success;
}

ErrorCode RendererScene::preload()
{
	// Implementation note: use number of allocated objects as an early bail - this method is most
	// likely called after populating pools, which means objects are lined at the start of the pools)

	// Start loading Model Objects 
	for(decltype(m_modelObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_modelObjPool.getNumAllocated(),
		size = m_modelObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// If current object is allocated and is not loaded to memory already
		if(m_modelObjPool[i].allocated() && !(m_modelObjPool[i].getObject()->loadedToMemory()))
		{
			// Add the object to be loaded later
			m_objectsBeingLoaded.push_back(LoadableGraphicsObjectAndIndex(m_modelObjPool[i].getObject(), m_modelObjPool[i].getIndex()));

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;
		}
	}

	// Start loading ShaderModel Objects 
	for(decltype(m_shaderObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_shaderObjPool.getNumAllocated(),
		size = m_shaderObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// If current object is allocated and is not loaded to memory already
		if(m_shaderObjPool[i].allocated() && !(m_shaderObjPool[i].getObject()->loadedToMemory()))
		{
			// Add the object to be loaded later
			m_objectsBeingLoaded.push_back(LoadableGraphicsObjectAndIndex(m_shaderObjPool[i].getObject(), m_shaderObjPool[i].getIndex()));

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;
		}
	}
	
	// Load every object to memory. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), m_objectsBeingLoaded.size(), size_t(1), [=](size_t i)
	{
		m_objectsBeingLoaded[i].m_loadableObject->loadToMemory();
	});

	return ErrorCode::Success;
}

void RendererScene::loadInBackground()
{
	// Implementation note: use number of allocated objects as an early bail - this method is most
	// likely called after populating pools, which means objects are lined up at the start of the pools)

	// Start loading Model Objects 
	for(decltype(m_modelObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_modelObjPool.getNumAllocated(),
		size = m_modelObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// If current object is allocated and is not loaded to memory already
		if(m_modelObjPool[i].allocated() && !(m_modelObjPool[i].getObject()->loadedToMemory()))
		{
			// Add object to 'being loaded' list and start loading it in a background thread
			m_objectsBeingLoaded.push_back(LoadableGraphicsObjectAndIndex(m_modelObjPool[i].getObject(), m_modelObjPool[i].getIndex()));
			TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelObject::loadToMemory, m_modelObjPool[i].getObject()));

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;
		}
	}

	// Start loading ShaderModel Objects 
	for(decltype(m_shaderObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_shaderObjPool.getNumAllocated(),
		size = m_shaderObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
	{
		// If current object is allocated and is not loaded to memory already
		if(m_shaderObjPool[i].allocated() && !(m_shaderObjPool[i].getObject()->loadedToMemory()))
		{
			// Add object to 'being loaded' list and start loading it in a background thread
			m_objectsBeingLoaded.push_back(LoadableGraphicsObjectAndIndex(m_shaderObjPool[i].getObject(), m_shaderObjPool[i].getIndex()));
			TaskManagerLocator::get().startBackgroundThread(std::bind(&ModelObject::loadToMemory, m_shaderObjPool[i].getObject()));

			// Increment the number of allocated objects (early bail mechanism)
			numAllocObjecs++;
		}
	}
}

PropertySet RendererScene::exportObject()
{
	// Create the root property set
	PropertySet propertySet(Properties::Graphics);

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

	return propertySet;
}

void RendererScene::update(const float p_deltaTime)
{
	// Clear variables
	m_sceneObjects.m_camera = nullptr;
	m_sceneObjects.m_directionalLight = nullptr;

	// Clear arrays from previous frame
	m_sceneObjects.m_modelObjects.clear();
	m_sceneObjects.m_objectsToLoad.clear();
	m_sceneObjects.m_postLightingObjects.clear();
	m_sceneObjects.m_customShaderObjects.clear();
	m_sceneObjects.m_pointLights.clear();
	m_sceneObjects.m_spotLights.clear();
		
	//	 _______________________________
	//	|								|
	//	|	Currently Loaded Objects	|
	//	|_______________________________|
	//
	// Iterate over currently loading objects
	for(decltype(m_objectsBeingLoaded.size()) i = 0, size = m_objectsBeingLoaded.size(), 
		maxObjects = Config::rendererVar().objects_loaded_per_frame; i < size;)
	{
		// If the object has loaded to memory already, add to load queue
		if(m_objectsBeingLoaded[i].m_loadableObject->loadedToMemory())
		{
			// If object should be activated after loading (for example wasn't set to be deleted while loading)
			if(m_objectsBeingLoaded[i].m_activateAfterLoading)
			{
				// Make object active, so it is passed to the renderer for drawing
				m_objectsBeingLoaded[i].m_loadableObject->setActive(true);

				// Add the object to objects-to-load list, that will be sent to the renderer to process
				m_sceneObjects.m_objectsToLoad.push_back(&m_objectsBeingLoaded[i].m_loadableObject->getRenderableObjectData());
			}
			else
			{
				// Remove the object from pool
				removeObjectFromPool(&m_objectsBeingLoaded[i]);
			}

			// Remove the object from the current list
			m_objectsBeingLoaded.erase(m_objectsBeingLoaded.begin() + i);
			
			// If the max number of object to be processed per frame has been reached, break from the loop
			if(--maxObjects == 0)
				break;
		}
		// If current object is still loading, advance the index
		else
			i++;
	}

	//	 _______________________________
	//	|								|
	//	|		Misc Loaded Objects		|
	//	|_______________________________|
	//
	// Iterate over currently loading objects
	/*for (decltype(m_miscObjectsBeingLoaded.size()) i = 0, size = m_miscObjectsBeingLoaded.size(),
		maxObjects = Config::rendererVar().objects_loaded_per_frame; i < size;)
	{
		// If the object has loaded to memory already, add to load queue
		if (m_miscObjectsBeingLoaded[i].m_loadableObject->loadedToMemory())
		{
			// If object should be activated after loading (for example wasn't set to be deleted while loading)
			if (m_miscObjectsBeingLoaded[i].m_activateAfterLoading)
			{
				// Make object active, so it is passed to the renderer for drawing
				//m_miscObjectsBeingLoaded[i].m_loadableObject->setActive(true);

				// Add the object to objects-to-load list, that will be sent to the renderer to process
				m_sceneObjects.m_objectsToLoad.push_back(&m_miscObjectsBeingLoaded[i].m_loadableObject->getRenderableObjectData());
			}
			else
			{
				// Remove the object from pool
				removeObjectFromPool(&m_miscObjectsBeingLoaded[i]);
			}

			// Remove the object from the current list
			m_miscObjectsBeingLoaded.erase(m_miscObjectsBeingLoaded.begin() + i);

			// If the max number of object to be processed per frame has been reached, break from the loop
			if (--maxObjects == 0)
				break;
		}
		// If current object is still loading, advance the index
		else
			i++;
	}*/

	//	 ___________________________
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
			if(object->active())
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
			if(object->active())
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
	}

	// Update camera and put it in scene object list
	m_camera->update(p_deltaTime);
	m_sceneObjects.m_camera = m_camera;

	m_sceneObjects.m_staticSkybox = m_skybox;
	m_sceneObjects.m_directionalLight = &m_directionalLight->getLightDataSet();
}

SystemObject *RendererScene::createObject(const PropertySet &p_properties)
{
	//  Get object's type
	auto const &type = p_properties.getPropertyByID(Properties::Type).getID();

	SystemObject *newObject = nullptr;

	// Create the object by it's type
	switch(type)
	{
	case Properties::ModelObject:
		newObject = loadModelObject(p_properties);
		break;
	case Properties::Camera:
		newObject = loadCameraObject(p_properties);
		break;
	case Properties::DirectionalLight:
		newObject = loadDirectionalLight(p_properties);
		break;
	case Properties::EnvironmentMapStatic:
		newObject = loadEnvMapStatic(p_properties);
		break;
	case Properties::PointLight:
		newObject = loadPointLight(p_properties);
		break;
	case Properties::SpotLight:
		newObject = loadSpotLight(p_properties);
		break;
	}

	// If the object creation was successful, return the new object
	if(newObject != nullptr)
		return newObject;

	// If valid type was not specified, or object creation failed, return a null object instead
	return g_nullSystemBase.getScene()->createObject(p_properties);
}
ErrorCode RendererScene::destroyObject(SystemObject *p_systemObject)
{
	if(p_systemObject != nullptr)
	{
		switch(p_systemObject->getObjectType())
		{
		case Properties::ModelObject:

			// Iterate over all elements and match the pointer address
			for(decltype(m_modelObjPool.getPoolSize()) i = 0, size = m_modelObjPool.getPoolSize(); i < size; i++)
			{
				// If object is allocated and the pointer addresses match
				if(m_modelObjPool[i].allocated() && m_modelObjPool[i].getObject() == p_systemObject)
				{
					auto *loadableObject = getCurrentlyLoadingObject(m_modelObjPool[i].getObject());

					// If the object is currently being loaded, instead of removing it, mark it for removal after
					// loading (so a new object cannot be added to it's place before it's done loading)
					if(loadableObject != nullptr)
					{
						loadableObject->m_activateAfterLoading = false;
					}
					// If object is not currently being loaded, just remove it from the list
					else
					{
						m_modelObjPool.remove(i);
					}

					return ErrorCode::Success;
				}
			}

			break;
		}

	}

	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

void RendererScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
	//std::cout << "change occurred" << std::endl;
}

BitMask RendererScene::getDesiredSystemChanges()
{
	return Systems::Changes::Spacial::All;
}
BitMask RendererScene::getPotentialSystemChanges()
{
	return Systems::Changes::Spacial::All;
}

ModelObject *RendererScene::loadModelObject(const PropertySet &p_properties)
{
	// Get model properties
	auto &models = p_properties.getPropertySetByID(Properties::Models);
	auto &shaderProperty = p_properties.getPropertySetByID(Properties::Shaders);

	ErrorCode objPoolError = ErrorCode::Failure;
	ModelObject *newObject = nullptr;

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
			case Properties::Position:
				newObject->setPosition(p_properties[i].getVec3f());
				break;
			case Properties::Rotation:
				newObject->setRotation(p_properties[i].getVec3f());
				break;
			case Properties::Scale:
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
	}
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
EnvironmentMapStatic *RendererScene::loadEnvMapStatic(const PropertySet &p_properties)
{
	EnvironmentMapStatic *newObject = nullptr;

	auto &materials = p_properties.getPropertySetByID(Properties::Materials);

	std::string filenames[CubemapFace_NumOfFaces];

	if(materials.getNumPropertySets() >= CubemapFace_NumOfFaces)
		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
			filenames[face] = materials.getPropertySet(face).getPropertyByID(Properties::Filename).getString();

	newObject = new EnvironmentMapStatic(this,
										 p_properties.getPropertyByID(Properties::Name).getString(),
										 Loaders::textureCubemap().load(filenames, false));

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Position:
			newObject->setPosition(p_properties[i].getVec3f());
			break;
		}
	}

	newObject->loadToMemory();
	newObject->loadToVideoMemory();

	m_skybox = newObject;

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
			m_directionalLight->setDirection(Math::normalize(p_properties[i].getVec3f()));
			break;

		case Properties::Intensity:
			m_directionalLight->setIntensity(p_properties[i].getFloat());
			break;
		}
	}

	return m_directionalLight;
}
PointLightObject *RendererScene::loadPointLight(const PropertySet &p_properties)
{
	ErrorCode objPoolError = ErrorCode::Failure;
	PointLightObject *newObject = nullptr;

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

	return newObject;
}
SpotLightObject *RendererScene::loadSpotLight(const PropertySet &p_properties)
{
	ErrorCode objPoolError = ErrorCode::Failure;
	SpotLightObject *newObject = nullptr;

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

	return newObject;
}
