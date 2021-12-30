#pragma once

#include "CameraComponent.h"
#include "Containers.h"
#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"
#include "LightComponent.h"
#include "Loaders.h"
#include "Math.h"
#include "ModelComponent.h"
#include "NullSystemObjects.h"
#include "ShaderComponent.h"
#include "SpatialDataManager.h"
#include "System.h"

enum GraphicsComponentType : std::size_t
{
	GraphicsComponentType_Camera = 0,
	GraphicsComponentType_Light,
	GraphicsComponentType_Model,
	GraphicsComponentType_Shader,
	GraphicsComponentType_NumOfComponents
};

class GraphicsObject : public SystemObject, public SpatialDataManagerObject, public LoadableGraphicsObject
{
public:
	GraphicsObject(SystemScene *p_systemScene, const std::string &p_name)
		: SystemObject(p_systemScene, p_name, Properties::GraphicsObject)
	{
		m_cameraComponent = nullptr;
		m_lightComponent = nullptr;
		m_modelComponent = nullptr;
		m_shaderComponent = nullptr;
		m_updateQuaternion = false;
		m_componentsFlag = 0;

	}
	~GraphicsObject() 
	{
		// Iterate over all component types and delete components if they have been created
		for(std::size_t i = 0; i < GraphicsComponentType::GraphicsComponentType_NumOfComponents; i++)
			removeComponent(static_cast<GraphicsComponentType>(i));
	}
	
	ErrorCode init() { return ErrorCode::Success; }

	ErrorCode importObject(const PropertySet &p_properties)
	{
		ErrorCode returnError = ErrorCode::Success;

		// Check if the property set is valid and the graphics object hasn't been loaded already
		if(p_properties)
		{
			if(!isLoadedToMemory())
			{
				// Check if there is a property set for camera and load the camera component if there is
				auto const &camera = p_properties.getPropertySetByID(Properties::Camera);
				if(camera)
				{
					// Create the camera component
					addComponent(new CameraComponent(m_systemScene, m_name + Config::componentVar().camera_component_name));

					// Try to initialize the camera component
					auto componentInitError = m_cameraComponent->init();
					if(componentInitError == ErrorCode::Success)
					{
						// Try to import the component
						auto const &componentImportError = m_cameraComponent->importObject(camera);

						// Remove the component if it failed to import
						if(componentImportError != ErrorCode::Success)
						{
							removeComponent(GraphicsComponentType::GraphicsComponentType_Camera);
							ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_CameraComponent, m_name);
						}
					}
					else // Remove the component if it failed to initialize
					{
						removeComponent(GraphicsComponentType::GraphicsComponentType_Camera);
						ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_CameraComponent, m_name);
					}
				}

				// Check if there is a property set for lighting and load the light component if there is
				auto const &lighting = p_properties.getPropertySetByID(Properties::Lighting);
				if(lighting)
				{
					// Create the light component
					addComponent(new LightComponent(m_systemScene, m_name + Config::componentVar().light_component_name));

					// Try to initialize the light component
					auto componentInitError = m_lightComponent->init();
					if(componentInitError == ErrorCode::Success)
					{
						// Try to import the component
						auto const &componentImportError = m_lightComponent->importObject(lighting);

						// Remove the component if it failed to import
						if(componentImportError != ErrorCode::Success)
						{
							removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
							ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_LightComponent, m_name);
						}
					}
					else // Remove the component if it failed to initialize
					{
						removeComponent(GraphicsComponentType::GraphicsComponentType_Camera);
						ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_LightComponent, m_name);
					}
				}

				// Check if there is a property set for models and load the model component if there is
				auto const &models = p_properties.getPropertySetByID(Properties::Models);
				if(models)
				{
					// Create the model component
					addComponent(new ModelComponent(m_systemScene, m_name + Config::componentVar().model_component_name));

					// Try to initialize the model component
					auto componentInitError = m_modelComponent->init();
					if(componentInitError == ErrorCode::Success)
					{
						// Try to import the component
						auto const &componentImportError = m_modelComponent->importObject(models);

						// Remove the component if it failed to import
						if(componentImportError != ErrorCode::Success)
						{
							removeComponent(GraphicsComponentType::GraphicsComponentType_Model);
							ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_ModelComponent, m_name);
						}
					}
					else // Remove the component if it failed to initialize
					{
						removeComponent(GraphicsComponentType::GraphicsComponentType_Camera);
						ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ModelComponent, m_name);
					}
				}

				// Check if there is a property set for shaders and load the shader component if there is
				auto const &shaders = p_properties.getPropertySetByID(Properties::Shaders);
				if(shaders)
				{
					// Create the shader component
					addComponent(new ShaderComponent(m_systemScene, m_name + Config::componentVar().shader_component_name));

					// Try to initialize the shader component
					auto componentInitError = m_shaderComponent->init();
					if(componentInitError == ErrorCode::Success)
					{
						// Try to import the component
						auto const &componentImportError = m_shaderComponent->importObject(shaders);

						// Remove the component if it failed to import
						if(componentImportError != ErrorCode::Success)
						{
							removeComponent(GraphicsComponentType::GraphicsComponentType_Shader);
							ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_ShaderComponent, m_name);
						}
					}
					else // Remove the component if it failed to initialize
					{
						removeComponent(GraphicsComponentType::GraphicsComponentType_Camera);
						ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_ShaderComponent, m_name);
					}
				}
			}
		}
		else
		{
			returnError = ErrorCode::Failure;
		}

		return returnError;
	}

	PropertySet exportObject()
	{
		// If there are components present, export each one; if there are no components, return an empty propertySet
		if(containsComponents())
		{
			PropertySet exportPropertySet(Properties::Rendering);

			if(cameraComponentPresent())
				exportPropertySet.addPropertySet(m_cameraComponent->exportObject());
			if(lightComponentPresent())
				exportPropertySet.addPropertySet(m_lightComponent->exportObject());
			if(modelComponentPresent())
				exportPropertySet.addPropertySet(m_modelComponent->exportObject());
			if(shaderComponentPresent())
				exportPropertySet.addPropertySet(m_shaderComponent->exportObject());

			return exportPropertySet;
		}
		else
			return PropertySet();
	}
		
	void update(const float p_deltaTime)
	{
		if(!isLoadedToVideoMemory())
		{
			performCheckIsLoadedToVideoMemory();

			if(!isLoadedToVideoMemory())
				return;
		}

		if(hasSpatialDataUpdated())
		{

		}

		if(isUpdateNeeded())
		{
			if(m_updateQuaternion)
			{
				//TODO: calculate rotation quaternion
				m_updateQuaternion = false;
			}

			// Calculate model matrix
			//m_worldSpace.m_transformMat = Math::createTransformMat(m_worldSpace.m_spatialData.m_position, m_worldSpace.m_spatialData.m_rotationEuler, m_worldSpace.m_spatialData.m_scale);

			// Update components
			if(modelComponentPresent())
				m_modelComponent->update(p_deltaTime);
			if(shaderComponentPresent())
				m_shaderComponent->update(p_deltaTime);
			if(lightComponentPresent())
				m_lightComponent->update(p_deltaTime);

			// Mark as updated
			updatePerformed();
		}
	}

	// Assign a pointer to a const SpatialDataChangeManager, so the object can use it for its spatial data
	// Also assigns the pointer to every component that needs it
	virtual void setSpatialDataManagerReference(const SpatialDataManager &p_spatialData) 
	{
		SpatialDataManagerObject::setSpatialDataManagerReference(p_spatialData);

		if(lightComponentPresent())
			m_lightComponent->setSpatialDataManagerReference(*m_spatialData);
	}

	// System type is Graphics
	BitMask getSystemType() { return Systems::Graphics; }

	virtual BitMask getDesiredSystemChanges()	{ return Systems::Changes::Graphics::All; }
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::Graphics::All; }

	inline CameraComponent *getCameraComponent()	{ return m_cameraComponent; }
	inline LightComponent *getLightComponent()		{ return m_lightComponent;	}
	inline ModelComponent *getModelComponent()		{ return m_modelComponent;	}
	inline ShaderComponent *getShaderComponent()	{ return m_shaderComponent; }

	inline const bool cameraComponentPresent()	const { return (m_cameraComponent	== nullptr) ? false : true;	}
	inline const bool lightComponentPresent()	const { return (m_lightComponent	== nullptr) ? false : true;	}
	inline const bool modelComponentPresent()	const { return (m_modelComponent	== nullptr) ? false : true;	}
	inline const bool shaderComponentPresent()	const { return (m_shaderComponent	== nullptr) ? false : true;	}

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;
		


		// If any data has been updated, post the changes to listeners
		if(newChanges != Systems::Changes::None)
		{
			postChanges(newChanges);
		}
	}
			
	/*/ Get the world spatial data (without transform matrix)
	const inline SpatialData &getWorldSpatialData()	const					{ return m_worldSpace.m_spatialData;					}
	// Get the world spatial transform data
	const inline SpatialTransformData &getWorldSpatialTransformData() const	{ return m_worldSpace;									}
	// Get the world position
	const inline Math::Vec3f &getWorldPosition() const						{ return m_worldSpace.m_spatialData.m_position;			}
	// Get the world rotation in Euler
	const inline Math::Vec3f getWorldRotationEuler() const					{ return m_worldSpace.m_spatialData.m_rotationEuler;	}
	// Get the world rotation as a quaternion
	const inline Math::Quaternion getWorldRotationQuat() const				{ return m_worldSpace.m_spatialData.m_rotationQuat;		}
	// Get the world scale
	const inline Math::Vec3f &getWorldScale() const							{ return m_worldSpace.m_spatialData.m_scale;			}

	// Set the world spatial data (without transform matrix)
	inline void setWorldSpatialData(const SpatialData &p_spatialData)					{ setUpdateNeeded(true); m_worldSpace.m_spatialData = p_spatialData;												}
	// Set the world spatial transform data
	inline void setWorldSpatialTransformData(const SpatialTransformData &p_spatialData)	{ m_worldSpace = p_spatialData;																						}
	// Set the world position
	inline void setWorldPosition(const Math::Vec3f &p_position)							{ setUpdateNeeded(true); m_worldSpace.m_spatialData.m_position = p_position;										}
	// Set the world rotation in Euler
	inline void setWorldRotation(const Math::Vec3f &p_rotationEuler)					{ setUpdateNeeded(true); m_updateQuaternion - true; m_worldSpace.m_spatialData.m_rotationEuler = p_rotationEuler;	}
	// Set the world rotation as a quaternion
	inline void setWorldRotation(const Math::Quaternion &p_rotationQuat)				{ setUpdateNeeded(true); m_worldSpace.m_spatialData.m_rotationQuat = p_rotationQuat;								}
	// Set the world scale
	inline void setWorldScale(const Math::Vec3f &p_scale)								{ setUpdateNeeded(true); m_worldSpace.m_spatialData.m_scale = p_scale;												}
	*/
	// Component functions
	/*void addComponent(DirectionalLightDataSet &p_lightDataSet)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		//m_lightComponent = new LightComponent(p_lightDataSet);
	}
	void addComponent(PointLightDataSet &p_lightDataSet)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		//m_lightComponent = new LightComponent(p_lightDataSet);
	}
	void addComponent(SpotLightDataSet &p_lightDataSet)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		//m_lightComponent = new LightComponent(p_lightDataSet);
	}*/	
	void addComponent(CameraComponent *p_component)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Camera);
		m_cameraComponent = p_component;

		// Share the GraphicsObjects spatial data with the component
		m_cameraComponent->setSpatialDataManagerReference(*m_spatialData);

		// Set the flag for the camera component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::GraphicsObjectComponents::Camera;
	}
	void addComponent(LightComponent *p_component)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		m_lightComponent = p_component;

		// Share the GraphicsObjects spatial data with the component
		m_lightComponent->setSpatialDataManagerReference(*m_spatialData);

		// Set the flag for the lighting component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::GraphicsObjectComponents::Lighting;
	}
	void addComponent(ModelComponent *p_component)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Model);
		m_modelComponent = p_component;

		// Set the flag for the model component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::GraphicsObjectComponents::Model;
	}
	void addComponent(ShaderComponent *p_component)
	{ 
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Shader);
		m_shaderComponent = p_component;

		// Set the flag for the shader component, so it is known from the flag that there is one currently present
		m_componentsFlag |= Systems::GraphicsObjectComponents::Shader;
	}
	void removeComponent(const GraphicsComponentType p_compType)
	{
		switch(p_compType)
		{
		case GraphicsComponentType::GraphicsComponentType_Camera:
		{
			if(m_cameraComponent != nullptr)
			{
				// Delete the actual component
				delete m_cameraComponent;

				// Assign the component pointer as nullptr to denote that it has been removed
				m_cameraComponent = nullptr;

				// Remove the bit corresponding to camera component from the componentsFlag bitmask
				m_componentsFlag &= ~Systems::GraphicsObjectComponents::Camera;
			}
			break;
		}
		case GraphicsComponentType::GraphicsComponentType_Light:
		{
			if(m_lightComponent != nullptr)
			{
				// Delete the actual component
				delete m_lightComponent;

				// Assign the component pointer as nullptr to denote that it has been removed
				m_lightComponent = nullptr;

				// Remove the bit corresponding to lighting component from the componentsFlag bitmask
				m_componentsFlag &= ~Systems::GraphicsObjectComponents::Lighting;
			}
			break;
		}
		case GraphicsComponentType::GraphicsComponentType_Model:
		{
			if(m_modelComponent != nullptr)
			{
				// Delete the actual component
				delete m_modelComponent;

				// Assign the component pointer as nullptr to denote that it has been removed
				m_modelComponent = nullptr;

				// Remove the bit corresponding to model component from the componentsFlag bitmask
				m_componentsFlag &= ~Systems::GraphicsObjectComponents::Model;
			}
			break;
		}
		case GraphicsComponentType::GraphicsComponentType_Shader:
		{
			if(m_shaderComponent != nullptr)
			{
				// Delete the actual component
				delete m_shaderComponent;

				// Assign the component pointer as nullptr to denote that it has been removed
				m_shaderComponent = nullptr;

				// Remove the bit corresponding to shader component from the componentsFlag bitmask
				m_componentsFlag &= ~Systems::GraphicsObjectComponents::Shader;
			}
			break;
		}
		}
	}
	
	// Returns true if the graphics object contains any components
	inline const bool containsComponents()
	{
		if(modelComponentPresent())
			return true;
		if(lightComponentPresent())
			return true;
		if(cameraComponentPresent())
			return true;
		if(shaderComponentPresent())
			return true;

		return false;
	}

	std::vector<LoadableObjectsContainer> getLoadableObjects() 
	{ 
		std::vector<LoadableObjectsContainer> returnLoadableObjects;

		if(cameraComponentPresent())
		{
			auto loadableObjects = getCameraComponent()->getLoadableObjects();
			//returnLoadableObjects.insert(returnLoadableObjects.end(), loadableObjects.begin(), loadableObjects.end());
			returnLoadableObjects.insert(returnLoadableObjects.end(), std::make_move_iterator(loadableObjects.begin()), std::make_move_iterator(loadableObjects.end()));
		}

		if(modelComponentPresent())
		{
			auto loadableObjects = getModelComponent()->getLoadableObjects();
			returnLoadableObjects.insert(returnLoadableObjects.end(), std::make_move_iterator(loadableObjects.begin()), std::make_move_iterator(loadableObjects.end()));
		}

		if(shaderComponentPresent())
		{
			auto loadableObjects = getShaderComponent()->getLoadableObjects();
			returnLoadableObjects.insert(returnLoadableObjects.end(), std::make_move_iterator(loadableObjects.begin()), std::make_move_iterator(loadableObjects.end()));
		}

		if(lightComponentPresent())
		{
			auto loadableObjects = getLightComponent()->getLoadableObjects();
			returnLoadableObjects.insert(returnLoadableObjects.end(), std::make_move_iterator(loadableObjects.begin()), std::make_move_iterator(loadableObjects.end()));
		}

		return returnLoadableObjects;
	}

	void performCheckIsLoadedToMemory()
	{
		bool componentsAreLoaded = true;

		if(cameraComponentPresent() && !getCameraComponent()->isLoadedToMemory())
			componentsAreLoaded = false;

		if(modelComponentPresent() && !getModelComponent()->isLoadedToMemory())
		{
			getModelComponent()->performCheckIsLoadedToMemory();
			if(!getModelComponent()->isLoadedToMemory())
				componentsAreLoaded = false;
		}

		if(shaderComponentPresent() && !getShaderComponent()->isLoadedToMemory())
		{
			getShaderComponent()->performCheckIsLoadedToMemory();
			if(!getShaderComponent()->isLoadedToMemory())
				componentsAreLoaded = false;
		}

		if(lightComponentPresent() && !getLightComponent()->isLoadedToMemory())
			componentsAreLoaded = false;

		if(componentsAreLoaded)
			setLoadedToMemory(true);
	}
	void performCheckIsLoadedToVideoMemory() 
	{
		bool componentsAreLoaded = true;

		if(cameraComponentPresent() && !getCameraComponent()->isLoadedToVideoMemory())
			componentsAreLoaded = false;

		if(modelComponentPresent() && !getModelComponent()->isLoadedToVideoMemory())
		{
			getModelComponent()->performCheckIsLoadedToVideoMemory();
			if(!getModelComponent()->isLoadedToVideoMemory())
				componentsAreLoaded = false;
		}

		if(shaderComponentPresent() && !getShaderComponent()->isLoadedToVideoMemory())
		{
			getShaderComponent()->performCheckIsLoadedToVideoMemory();
			if(!getShaderComponent()->isLoadedToVideoMemory())
				componentsAreLoaded = false;
		}

		if(lightComponentPresent() && !getLightComponent()->isLoadedToVideoMemory())
			componentsAreLoaded = false;

		if(componentsAreLoaded)
			setLoadedToVideoMemory(true);
	}

private:
	//enum 
	/*struct
	{
		union
		{

		} m_component;
	} m_graphicsComponents;*/

	/*struct
	{
		enum LightComponentType
		{

		};
		union
		{

		} m_light;
	} m_lightComponent;*/

	// Pointers to multiple variables, intended as a way to modify component values directly (like a simplified observer pattern).
	// If a pointer to a value is not nullptr, then that variable is acting as a listener its value should be updated
	struct ProxyValues
	{
		ProxyValues()
		{
			m_color = nullptr;
		}

		Math::Vec3f *m_color;
	};

	ProxyValues m_proxyValues;
	//std::vector<BaseGraphicsComponent&> m_components;

	//std::vector<MeshData> m_meshData;
	//SpatialTransformData m_worldSpace;

	// Components
	CameraComponent *m_cameraComponent;
	LightComponent	*m_lightComponent;
	ModelComponent	*m_modelComponent;
	ShaderComponent *m_shaderComponent;

	// Stores a separate flag for each component currently present
	BitMask m_componentsFlag;

	// Flag data that needs to be updated
	bool m_updateQuaternion;
};

/*class GraphicsObject : public SystemObject
{
public:
	GraphicsObject(SystemScene *p_systemScene, const std::string &p_name)
		: SystemObject(p_systemScene, p_name, Properties::Graphics), m_needsUpdate(true), m_affectedByLighting(true) { }
	virtual ~GraphicsObject() { }
		
	BitMask getSystemType() { return Systems::Graphics; }

	virtual BitMask getDesiredSystemChanges()	{ return Systems::Changes::Spatial::All;	}
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::None;			}

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spatial::Position)
		{
			m_baseObjectData.m_position = 
				p_subject->getVec3(this, Systems::Changes::Spatial::Position) + m_baseObjectData.m_offsetPosition;
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spatial::Rotation)
		{
			m_baseObjectData.m_rotation = 
				p_subject->getVec3(this, Systems::Changes::Spatial::Rotation) + m_baseObjectData.m_offsetRotation;
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spatial::Scale)
		{
			m_baseObjectData.m_scale = p_subject->getVec3(this, Systems::Changes::Spatial::Scale);
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spatial::ModelMatrix)
		{
			m_baseObjectData.m_modelMat = p_subject->getMat4(this, Systems::Changes::Spatial::ModelMatrix);
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Graphics::Lighting)
		{
			m_affectedByLighting = p_subject->getBool(this, Systems::Changes::Graphics::Lighting);
		}
	}

	// Has the object been already loaded to memory (RAM)?
	const inline bool isLoadedToMemory() const { return m_loadedToMemory; }

	// Has the object been already loaded to video memory (GPU VRAM)
	const inline bool isLoadedToVideoMemory() const { return m_loadedToVideoMemory; }

	// Getters
	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::Position:
			return m_baseObjectData.m_position;
			break;
		case Systems::Changes::Spatial::Rotation:
			return m_baseObjectData.m_rotation;
			break;
		case Systems::Changes::Spatial::Scale:
			return m_baseObjectData.m_scale;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}
	const virtual bool getBool(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Graphics::Lighting:
			return m_affectedByLighting;
			break;
		}

		return ObservedSubject::getBool(p_observer, p_changedBits);
	}
	const inline GraphicsData &getBaseObjectData() const { return m_baseObjectData; }

	// Setters for spacial data
	inline void setScale(const Math::Vec3f &p_scale)				{ m_baseObjectData.m_scale = p_scale;				}
	inline void setPosition(const Math::Vec3f &p_position)			{ m_baseObjectData.m_position = p_position;			}
	inline void setRotation(const Math::Vec3f &p_rotation)			{ m_baseObjectData.m_rotation = p_rotation;			}
	inline void setOffsetPosition(const Math::Vec3f &p_position)	{ m_baseObjectData.m_offsetPosition = p_position;	}
	inline void setOffsetRotation(const Math::Vec3f &p_rotation)	{ m_baseObjectData.m_offsetRotation = p_rotation;	}

	// Setters for misc data
	inline void setAffectedByLighting(const bool p_flag)		{ m_affectedByLighting = p_flag;					}
	inline void setAlphaThreshold(const float p_value)			{ m_baseObjectData.m_alphaThreshold = p_value;		}
	inline void setEmissiveThreshold(const float p_value)		{ m_baseObjectData.m_emissiveThreshold = p_value;	}
	inline void setHeightScale(const float p_value)				{ m_baseObjectData.m_heightScale = p_value;			}
	inline void setTextureTilingFactor(const float p_value)		{ m_baseObjectData.m_textureTilingFactor = p_value; }

protected:
	// A flag telling if this object should be rendered during geometry pass or as a post-process (i.e. after lighting)
	bool m_affectedByLighting;

	// Does the object need to be updated after any of its data has been changed
	bool m_needsUpdate;

	// Spatial and misc data of an object
	GraphicsData m_baseObjectData;
};*/