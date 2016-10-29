#pragma once

// Renderer scene stores all the objects in the scene.
// Upon updating, processes all the objects (for example, frustum culling, matrix updates, etc),
// and creates arrays of objects to be rendered. From here, they should be sent to the renderer.

#include "CameraGraphicsObject.h"
#include "GraphicsDataSets.h"
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
	SceneObjects() : m_camera(nullptr) { }

	const CameraObject *m_camera;
	const DirectionalLightDataSet *m_directionalLight;

	std::vector<RenderableObjectData*> m_modelObjects;
	std::vector<RenderableObjectData*> m_tessellatedObjects;
	std::vector<RenderableObjectData*> m_customShaderObjects;
	std::vector<RenderableObjectData*> m_postLightingObjects;

	std::vector<RenderableObjectData*> m_objectsToLoad;

	std::vector<PointLightDataSet> m_pointLights;
	std::vector<SpotLightDataSet> m_spotLights;
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

	BitMask getDesiredSystemChanges();
	BitMask getPotentialSystemChanges();

	// Inline getters
	SystemTask *getSystemTask() { return m_renderTask; }
	Systems::TypeID getSystemType() { return Systems::Graphics; }
	const inline SceneObjects &getSceneObjects() const { return m_sceneObjects; }
	
private:
	// Used to hold info on objects that are being loaded in a background thread
	struct LoadableObjectAndIndex
	{
		LoadableObjectAndIndex() : m_loadableObject(nullptr), m_index(0), m_activateAfterLoading(true) { }
		LoadableObjectAndIndex(LoadableGraphicsObject *p_loadableObject, size_t p_index)
			: m_loadableObject(p_loadableObject), m_index(p_index), m_activateAfterLoading(true) { }

		// The actual object that's being loaded
		LoadableGraphicsObject *m_loadableObject;
		
		// This should always be true, unless object was set to be removed before loading completed
		bool m_activateAfterLoading;

		// Unique index of the object in corresponding pool (used for fast access)
		size_t m_index;
	};

	// Removes an object from a pool, by iterating checking each pool for matched index and name
	inline void removeObjectFromPool(LoadableObjectAndIndex *p_objectAndIndex)
	{
		// Remove object from the pool determined by object's type
		switch(p_objectAndIndex->m_loadableObject->getObjectType())
		{
		case Properties::ModelObject:

			// Get a pool object by it's index
			auto *object = m_modelObjPool.getObject(p_objectAndIndex->m_index);

			// If object is valid, is allocated, and it's name matches, remove it from the pool and return from the function
			if(object != nullptr && object->allocated() && object->getObject()->getName() == p_objectAndIndex->m_loadableObject->getName())
			{
				m_modelObjPool.remove(p_objectAndIndex->m_index);
				return;
			}

			break;
		}
	}
	
	// Checks if an object is allocated in an object pool
	inline bool checkIfAllocated(const LoadableObjectAndIndex &p_objectAndIndex)
	{
		// Remove object from the pool determined by object's type
		switch(p_objectAndIndex.m_loadableObject->getObjectType())
		{
		case Properties::ModelObject:
			
			// Get a pool object by it's index and check if it's allocated
			return m_modelObjPool.getObject(p_objectAndIndex.m_index)->allocated();
			
			break;
		}
	}

	// Finds a match in the currently being loaded object array; returns null pointer if no match is found
	inline LoadableObjectAndIndex *getCurrentlyLoadingObject(LoadableGraphicsObject *p_loadableObject)
	{
		// Iterate over currently loading objects
		for(decltype(m_objectsBeingLoaded.size()) i = 0, size = m_objectsBeingLoaded.size(); i < size;)
		{
			// Compare pointers, if match is found, return the object
			if(m_objectsBeingLoaded[i].m_loadableObject == p_loadableObject)
				return &m_objectsBeingLoaded[i];
		}

		// If this point is reached - object wasn't found, return a null pointer
		return nullptr;
	}

	// Object creators (factories)
	ModelObject *loadModelObject(const PropertySet &p_properties);
	CameraObject *loadCameraObject(const PropertySet &p_properties);
	ShaderModelGraphicsObject *loadShaderModelObject(const PropertySet &p_properties);	// Obsolete (ModelObjects handle custom shaders)
	DirectionalLightObject *loadDirectionalLight(const PropertySet &p_properties);
	PointLightObject *loadPointLight(const PropertySet &p_properties);
	SpotLightObject *loadSpotLight(const PropertySet &p_properties);
		
	// Object pools
	ObjectPool<ModelObject> m_modelObjPool;
	ObjectPool<ModelObject> m_shaderObjPool;
	ObjectPool<PointLightObject> m_pointLightPool;
	ObjectPool<SpotLightObject> m_spotLightPool;
	
	// Stores objects that are currently being loaded to memory in background thread
	std::vector<LoadableObjectAndIndex> m_objectsBeingLoaded;

	// Only one camera present at a time
	CameraObject *m_camera;

	// Only one directional light present at a time
	DirectionalLightObject *m_directionalLight;

	// Used to store processed objects
	SceneObjects m_sceneObjects;

	// Task responsible for initiating rendering each frame
	RenderTask *m_renderTask;
};