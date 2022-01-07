#pragma once

// Renderer scene stores all the objects in the scene.
// Upon updating, processes all the objects (for example, frustum culling, matrix updates, etc),
// and creates arrays of objects to be rendered. From here, they should be sent to the renderer.

#include "CameraGraphicsObject.h"
#include "EnvironmentMapObjects.h"
#include "GraphicsDataSets.h"
#include "GraphicsObject.h"
#include "LightComponent.h"
#include "LightingGraphicsObjects.h"
#include "Loaders.h"
#include "ObjectPool.h"
#include "ModelGraphicsObjects.h"
#include "RenderTask.h"
#include "ShaderGraphicsObjects.h"
#include "System.h"

class RendererSystem;

// Used to store processed objects, so they can be sent to the renderer
struct SceneObjects
{
	SceneObjects() : /*m_staticSkybox(nullptr),*/ m_directionalLight(nullptr) { }

	// Camera
	CameraData m_camera;
	//EnvironmentMapObject *m_staticSkybox;

	// Lights
	const DirectionalLightDataSet *m_directionalLight;
	std::vector<PointLightDataSet> m_pointLights;
	std::vector<SpotLightDataSet> m_spotLights;

	// Models
	std::vector<ModelAndSpatialData> m_modelData;
	std::vector<ModelShaderSpatialData> m_modelDataWithShaders;

	// Objects that need to be loaded to VRAM
	std::vector<LoadableObjectsContainer> m_loadToVideoMemory;
};

class RendererScene : public SystemScene
{
public:
	RendererScene(RendererSystem *p_system, SceneLoader *p_sceneLoader);
	~RendererScene();

	ErrorCode init();

	// Sets up various scene-specific values (should be called before creating objects / updating)
	ErrorCode setup(const PropertySet &p_properties);

	// Preloads all the resources in the scene (as opposed to loading them while rendering, in background threads)
	ErrorCode preload();

	// Starts loading all the created objects in background threads
	void loadInBackground();

	// Exports all the data of the scene (including all objects within) as a PropertySet
	virtual PropertySet exportObject();

	// Processes all the objects and puts them in the separate vectors
	void update(const float p_deltaTime);
	
	SystemObject *createObject(const PropertySet &p_properties);
	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType);

	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	// Getters
	SystemTask *getSystemTask() { return m_renderTask; }
	Systems::TypeID getSystemType() { return Systems::Graphics; }
	inline SceneObjects &getSceneObjects() { return m_sceneObjects; }
	
private:
	// Removes an object from a pool, by iterating checking each pool for matched index; returns true if the object was found and removed
	inline bool removeObjectFromPool(GraphicsObject &p_object)
	{
		// Go over each graphics object
		for(decltype(m_graphicsObjPool.getPoolSize()) i = 0, numAllocObjecs = 0, totalNumAllocObjs = m_graphicsObjPool.getNumAllocated(),
			size = m_graphicsObjPool.getPoolSize(); i < size && numAllocObjecs < totalNumAllocObjs; i++)
		{
			// Check if the graphics object is allocated inside the pool container
			if(m_graphicsObjPool[i].allocated())
			{
				// Increment the number of allocated objects (early bail mechanism)
				numAllocObjecs++;

				// If the object matches with the one we are looking for, remove it from the graphics object pool
				if(*m_graphicsObjPool[i].getObject() == p_object)
				{
					m_graphicsObjPool.remove(m_graphicsObjPool[i].getIndex());
					return true;
				}
			}
		}

		return false;
	}
	
	/*/ Checks if an object is allocated in an object pool
	inline bool checkIfAllocated(const LoadableGraphicsObject &p_object)
	{
		// Remove object from the pool determined by object's type
		switch(p_object.getObjectType())
		{
		case Properties::ModelObject:
			
			// Get a pool object by it's index and check if it's allocated
			return m_modelObjPool.getObject(p_object.getIndex())->allocated();
			
			break;
		}
	}*/

	// Finds a match in the currently being loaded object array; returns null pointer if no match is found
	inline LoadableGraphicsObject *getCurrentlyLoadingObject(SystemObject &p_loadableObject)
	{
		// Iterate over currently loading objects
		for(decltype(m_objectsLoadingToMemory.size()) i = 0, size = m_objectsLoadingToMemory.size(); i < size;)
		{
			// Compare pointers, if match is found, return the object
			if(*m_objectsLoadingToMemory[i] == p_loadableObject)
				return m_objectsLoadingToMemory[i];
		}

		// If this point is reached - object wasn't found, return a null pointer
		return nullptr;
	}

	inline void calculateCamera(SpatialTransformData &p_viewData)
	{
		/*p_viewData.m_spatialData.m_rotationEuler = Math::toRadian(p_viewData.m_spatialData.m_rotationEuler);
		p_viewData.m_spatialData.m_rotationEuler.y = 0.5f;
		p_viewData.m_spatialData.m_rotationEuler.z = 3.14f;

		const glm::vec3 upVector = Math::cross(p_viewData.m_spatialData.m_rotationEuler.z, p_viewData.m_spatialData.m_rotationEuler.y);
		const glm::vec3 targetVector(0.0f, p_viewData.m_spatialData.m_rotationEuler.y, p_viewData.m_spatialData.m_rotationEuler.z);

		p_viewData.m_transformMat.initCamera(p_viewData.m_spatialData.m_position, targetVector + p_viewData.m_spatialData.m_position, upVector);*/

		//p_viewData.m_transformMat.identity();
		//p_viewData.m_transformMat.translate(-p_viewData.m_spatialData.m_position);
		//p_viewData.m_transformMat.rotate(p_viewData.m_spatialData.m_rotationEuler);

		/*/p_viewData.m_transformMat

		p_viewData.m_transformMat = glm::translate(p_viewData.m_transformMat, p_viewData.m_spatialData.m_position);

		//glm::quat quaternion(glm::radians(p_rotation));
		//returnMatrix *= glm::toMat4(quaternion);

		glm::quat yawQ = glm::quat(glm::vec3(0.0f, glm::radians(p_viewData.m_spatialData.m_rotationEuler.y), 0.0f));
		yawQ = glm::normalize(yawQ);
		glm::mat4 yawMat = glm::mat4_cast(yawQ);

		glm::quat pitchQ = glm::quat(glm::vec3(glm::radians(p_viewData.m_spatialData.m_rotationEuler.z), 0.0f, 0.0f));
		pitchQ = glm::normalize(pitchQ);
		glm::mat4 pitchMat = glm::mat4_cast(pitchQ);

		glm::quat rollQ = glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(p_viewData.m_spatialData.m_rotationEuler.z)));
		rollQ = glm::normalize(rollQ);
		glm::mat4 rollMat = glm::mat4_cast(rollQ);

		p_viewData.m_transformMat *= pitchMat * yawMat * rollMat;

		p_viewData.m_transformMat = glm::scale(p_viewData.m_transformMat, p_viewData.m_spatialData.m_scale);*/

		//p_viewData.m_transformMat = Math::createTransformMat(-p_viewData.m_spatialData.m_position, p_viewData.m_spatialData.m_rotationEuler, p_viewData.m_spatialData.m_scale);

	}

	MaterialData loadMaterialData(PropertySet &p_materialProperty, Model::MaterialArrays &p_materialArraysFromModel, MaterialType p_materialType, std::size_t p_meshIndex);

	// Object component creators (factories)
	ModelComponent *loadModelComponent(const PropertySet &p_properties);
	ShaderComponent *loadShaderComponent(const PropertySet &p_properties);
	LightComponent *loadLightComponent(const PropertySet &p_properties);

	ModelObject *loadModelObject(const PropertySet &p_properties);
	CameraObject *loadCameraObject(const PropertySet &p_properties);
	EnvironmentMapObject *loadEnvironmentMap(const PropertySet &p_properties);
	DirectionalLightObject *loadDirectionalLight(const PropertySet &p_properties);
	PointLightObject *loadPointLight(const PropertySet &p_properties);
	SpotLightObject *loadSpotLight(const PropertySet &p_properties);
		
	// Object pools
	// OLD
	//ObjectPool<ModelObject> m_modelObjPool;
	//ObjectPool<ModelObject> m_shaderObjPool;
	//ObjectPool<PointLightObject> m_pointLightPool;
	//ObjectPool<SpotLightObject> m_spotLightPool;
	//ObjectPool<EnvironmentMapObject> m_envMapPool;
	
	//NEW
	ObjectPool<GraphicsObject> m_graphicsObjPool;
	std::vector<GraphicsObject*> m_objectsLoadingToMemory;
	std::vector<GraphicsObject*> m_objectsToDestroy;

	ObjectPool<LightComponent> m_lightComponents;
	std::vector<ModelComponentData> m_modelComponents;

	// Stores objects that are currently being loaded to memory in background thread
	//std::vector<LoadableGraphicsObject> m_objectsBeingLoaded;
	
	EnvironmentMapObject *m_skybox;

	// Only one camera present at a time
	CameraObject *m_camera;

	// Only one directional light present at a time
	DirectionalLightObject *m_directionalLight;

	// Used to store processed objects
	SceneObjects m_sceneObjects;

	// Task responsible for initiating rendering each frame
	RenderTask *m_renderTask;
};