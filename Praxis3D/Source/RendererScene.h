#pragma once

// Renderer scene stores all the objects in the scene.
// Upon updating, processes all the objects (for example, frustum culling, matrix updates, etc),
// and creates arrays of objects to be rendered. From here, they should be sent to the renderer.

#include <list>

#include "EntityViewDefinitions.h"
#include "GraphicsDataSets.h"
#include "GraphicsLoadComponents.h"
#include "Loaders.h"
#include "ObjectPool.h"
#include "RenderTask.h"
#include "System.h"

class RendererSystem;

struct GraphicsComponentsConstructionInfo
{
	GraphicsComponentsConstructionInfo()
	{
		m_cameraConstructionInfo = nullptr;
		m_lightConstructionInfo = nullptr;
		m_modelConstructionInfo = nullptr;
		m_shaderConstructionInfo = nullptr;
	}

	// Perform a complete copy, instantiating (with new) every member variable pointer, instead of just assigning the pointer to the same memory
	void completeCopy(const GraphicsComponentsConstructionInfo &p_other)
	{
		Utilities::performCopy<CameraComponent::CameraComponentConstructionInfo>(&m_cameraConstructionInfo, &p_other.m_cameraConstructionInfo);
		Utilities::performCopy<LightComponent::LightComponentConstructionInfo>(&m_lightConstructionInfo, &p_other.m_lightConstructionInfo);
		Utilities::performCopy<ModelComponent::ModelComponentConstructionInfo>(&m_modelConstructionInfo, &p_other.m_modelConstructionInfo);
		Utilities::performCopy<ShaderComponent::ShaderComponentConstructionInfo>(&m_shaderConstructionInfo, &p_other.m_shaderConstructionInfo);
	}

	void deleteConstructionInfo()
	{
		if(m_cameraConstructionInfo != nullptr)
			delete m_cameraConstructionInfo;

		if(m_lightConstructionInfo != nullptr)
			delete m_lightConstructionInfo;

		if(m_modelConstructionInfo != nullptr)
			delete m_modelConstructionInfo;

		if(m_shaderConstructionInfo != nullptr)
			delete m_shaderConstructionInfo;

		//GraphicsComponentsConstructionInfo();
	}

	CameraComponent::CameraComponentConstructionInfo *m_cameraConstructionInfo;
	LightComponent::LightComponentConstructionInfo *m_lightConstructionInfo;
	ModelComponent::ModelComponentConstructionInfo *m_modelConstructionInfo;
	ShaderComponent::ShaderComponentConstructionInfo *m_shaderConstructionInfo;
};

struct LoadableComponentContainer
{
	LoadableComponentContainer(ModelComponent &p_model)
	{
		m_componentType = ComponentType::ComponentType_Model;
		m_entityID = p_model.getEntityID();
	}
	LoadableComponentContainer(ShaderComponent &p_shader)
	{
		m_componentType = ComponentType::ComponentType_Shader;
		m_entityID = p_shader.getEntityID();
	}

	enum ComponentType
	{
		ComponentType_Model,
		ComponentType_Shader
	};

	ComponentType m_componentType;
	EntityID m_entityID;
};

// Used to store processed objects, so they can be sent to the renderer
struct SceneObjects
{
	SceneObjects() { }

	// ECS registry views
	ModelSpatialView m_models;
	ModelShaderSpatialView m_modelsWithShaders;
	LightSpatialView m_lights;
	LoadToVideoMemoryView m_objectsToLoadToVideoMemory;

	// Camera
	glm::mat4 m_cameraViewMatrix;

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

	void exportSetup(PropertySet &p_propertySet);

	void activate();

	// Preloads all the resources in the scene (as opposed to loading them while rendering, in background threads)
	ErrorCode preload();

	// Starts loading all the created objects in background threads
	void loadInBackground();

	// Processes all the objects and puts them in the separate vectors
	void update(const float p_deltaTime);

	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	std::vector<SystemObject*> createComponents(const EntityID p_entityID, const GraphicsComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading = true)
	{
		std::vector<SystemObject *> components;

		if(p_constructionInfo.m_cameraConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_cameraConstructionInfo, p_startLoading));

		if(p_constructionInfo.m_lightConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_lightConstructionInfo, p_startLoading));

		if(p_constructionInfo.m_modelConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_modelConstructionInfo, p_startLoading));

		if(p_constructionInfo.m_shaderConstructionInfo != nullptr)
			components.push_back(createComponent(p_entityID, *p_constructionInfo.m_shaderConstructionInfo, p_startLoading));

		return components;
	}

	void exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo);
	void exportComponents(const EntityID p_entityID, GraphicsComponentsConstructionInfo &p_constructionInfo);

	SystemObject *createComponent(const EntityID &p_entityID, const CameraComponent::CameraComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	SystemObject *createComponent(const EntityID &p_entityID, const LightComponent::LightComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	SystemObject *createComponent(const EntityID &p_entityID, const ModelComponent::ModelComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);
	SystemObject *createComponent(const EntityID &p_entityID, const ShaderComponent::ShaderComponentConstructionInfo &p_constructionInfo, const bool p_startLoading = true);

	void exportComponent(CameraComponent::CameraComponentConstructionInfo &p_constructionInfo, const CameraComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();
		p_constructionInfo.m_fov = p_component.m_fov;
	}
	void exportComponent(LightComponent::LightComponentConstructionInfo &p_constructionInfo, const LightComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_constructionInfo.m_lightComponentType = p_component.getLightType();

		switch(p_component.getLightType())
		{
			case LightComponent::LightComponentType_directional:
				p_constructionInfo.m_color = p_component.m_lightComponent.m_directional.m_color;
				p_constructionInfo.m_intensity = p_component.m_lightComponent.m_directional.m_intensity;
				p_constructionInfo.m_direction = p_component.m_lightComponent.m_directional.m_direction;
				break;
			case LightComponent::LightComponentType_point:
				p_constructionInfo.m_color = p_component.m_lightComponent.m_point.m_color;
				p_constructionInfo.m_intensity = p_component.m_lightComponent.m_point.m_intensity;
				break;
			case LightComponent::LightComponentType_spot:
				p_constructionInfo.m_color = p_component.m_lightComponent.m_spot.m_color;
				p_constructionInfo.m_intensity = p_component.m_lightComponent.m_spot.m_intensity;
				p_constructionInfo.m_cutoffAngle = p_component.m_lightComponent.m_spot.m_cutoffAngle;
				break;
		}
	}
	void exportComponent(ModelComponent::ModelComponentConstructionInfo &p_constructionInfo, const ModelComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_component.getMeshMaterialsProperties(p_constructionInfo.m_materialsFromProperties);
		p_component.getModelsProperties(p_constructionInfo.m_modelsProperties);
	}
	void exportComponent(ShaderComponent::ShaderComponentConstructionInfo &p_constructionInfo, const ShaderComponent &p_component)
	{
		p_constructionInfo.m_active = p_component.isObjectActive();
		p_constructionInfo.m_name = p_component.getName();

		p_constructionInfo.m_fragmentShaderFilename = p_component.getShaderData()->m_shader.getShaderFilename(ShaderType::ShaderType_Fragment);
		p_constructionInfo.m_geometryShaderFilename = p_component.getShaderData()->m_shader.getShaderFilename(ShaderType::ShaderType_Geometry);
		p_constructionInfo.m_vetexShaderFilename = p_component.getShaderData()->m_shader.getShaderFilename(ShaderType::ShaderType_Vertex);
	}

	ErrorCode destroyObject(SystemObject *p_systemObject);

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType);

	void receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving);

	BitMask getDesiredSystemChanges() { return Systems::Changes::Generic::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	// Getters
	const unsigned int getUnsignedInt(const Observer *p_observer, BitMask p_changedBits) const;
	SystemTask *getSystemTask() { return m_renderTask; }
	Systems::TypeID getSystemType() { return Systems::Graphics; }
	inline SceneObjects &getSceneObjects() { return m_sceneObjects; }
	
private:
	MaterialData loadMaterialData(PropertySet &p_materialProperty, Model::MaterialArrays &p_materialArraysFromModel, MaterialType p_materialType, std::size_t p_meshIndex);

	// Only one directional light present at a time
	DirectionalLightDataSet m_directionalLight;

	// Used to store processed objects
	SceneObjects m_sceneObjects;

	// Task responsible for initiating rendering each frame
	RenderTask *m_renderTask;

	// Contains a list of rendering passes that gets set in the renderer system
	RenderingPasses m_renderingPasses;

	// Render-to-texture data
	bool m_renderToTexture;
	glm::ivec2 m_renderToTextureResolution;
};