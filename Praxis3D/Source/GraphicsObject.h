#pragma once

#include "Containers.h"
#include "GraphicsDataSets.h"
#include "LightComponent.h"
#include "Loaders.h"
#include "Math.h"
#include "NullSystemObjects.h"
#include "System.h"

enum GraphicsComponentType : std::size_t
{
	GraphicsComponentType_Model = 0,
	GraphicsComponentType_Shader,
	GraphicsComponentType_Light,
	GraphicsComponentType_NumOfComponents
};

class GraphicsObject : public SystemObject
{
public:
	GraphicsObject(SystemScene *p_systemScene, const std::string &p_name)
		: SystemObject(p_systemScene, p_name, Properties::Graphics) 
	{
		m_modelComponent = nullptr;
		m_shaderComponent = nullptr;
		m_lightComponent = nullptr;
		m_updateQuaternion = false;
	}
	virtual ~GraphicsObject() 
	{
		// Iterate over all component types and delete components if they have been created
		for(std::size_t i = 0; i < GraphicsComponentType::GraphicsComponentType_NumOfComponents; i++)
			removeComponent(static_cast<GraphicsComponentType>(i));
	}
	
	// System type is Graphics
	BitMask getSystemType() { return Systems::Graphics; }
		
	void update(const float p_deltaTime)
	{
		if(isUpdateNeeded())
		{
			if(m_updateQuaternion)
			{
				//TODO: calculate rotation quaternion
				m_updateQuaternion = false;
			}

			// Calculate model matrix
			m_worldSpace.m_transformMat = Math::createTransformMat(m_worldSpace.m_spatialData.m_position, m_worldSpace.m_spatialData.m_rotationEuler, m_worldSpace.m_spatialData.m_scale);

			// Mark as updated
			updatePerformed();
		}
	}

	virtual BitMask getDesiredSystemChanges()	{ return Systems::Changes::Spatial::AllWorld | Systems::Changes::Graphics::All;		}
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::WorldTransform;	}
	
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;
		
		// Get all of the world spatial data, include the transform matrix; add up the bit-mask of changed data;
		if(p_changeType & Systems::Changes::Spatial::AllWorld)
		{
			m_worldSpace.m_spatialData = p_subject->getSpatialData(this, Systems::Changes::Spatial::AllWorldNoTransform);
			m_worldSpace.m_transformMat = p_subject->getMat4(this, Systems::Changes::Spatial::WorldTransform);

			newChanges = newChanges | Systems::Changes::Spatial::AllWorld;
		}
		else
		{
			// Get world spatial data without transform matrix; add up the bit-mask of changed data; flag object to need updating
			if(p_changeType & Systems::Changes::Spatial::AllWorldNoTransform)
			{
				m_worldSpace.m_spatialData = p_subject->getSpatialData(this, Systems::Changes::Spatial::AllWorldNoTransform);

				newChanges = newChanges | Systems::Changes::Spatial::AllWorldNoTransform;

				setUpdateNeeded(true);
			}
			else
			{
				// Get world position vector; add up the bit-mask of changed data; flag object to need updating
				if(p_changeType & Systems::Changes::Spatial::WorldPosition)
				{
					m_worldSpace.m_spatialData.m_position = p_subject->getVec3(this, Systems::Changes::Spatial::WorldPosition);

					newChanges = newChanges | Systems::Changes::Spatial::WorldPosition;
				
					setUpdateNeeded(true);
				}

				// Get world rotation vector; add up the bit-mask of changed data; flag object to need updating; flag rotation quaternion to need updating
				if(p_changeType & Systems::Changes::Spatial::WorldRotation)
				{
					m_worldSpace.m_spatialData.m_rotationEuler = p_subject->getVec3(this, Systems::Changes::Spatial::WorldRotation);

					newChanges = newChanges | Systems::Changes::Spatial::WorldRotation;
				
					setUpdateNeeded(true);
					m_updateQuaternion = true;
				}

				// Get world scale vector; add up the bit-mask of changed data; flag object to need updating
				if(p_changeType & Systems::Changes::Spatial::WorldScale)
				{
					m_worldSpace.m_spatialData.m_scale = p_subject->getVec3(this, Systems::Changes::Spatial::WorldScale);

					newChanges = newChanges | Systems::Changes::Spatial::WorldScale;

					setUpdateNeeded(true);
				}
			}
		}

		// If any new data has been left, pass it to the components
		if(newChanges != Systems::Changes::None)
		{

		}
			//postChanges(newChanges);
	}
		
	const Math::Mat4f &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{ 		
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::WorldTransform:
			return m_worldSpace.m_transformMat;
			break;
		}

		return ObservedSubject::getMat4(p_observer, p_changedBits);
	}
	const SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits) const
	{ 		
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::AllWorld:
			return m_worldSpace.m_spatialData;
			break;
		}

		return ObservedSubject::getSpatialData(p_observer, p_changedBits);
	}
		
	// Get the world spatial data (without transform matrix)
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
	
	// Component functions
	void addComponent(DirectionalLightDataSet &p_lightDataSet)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		m_lightComponent = new LightComponent(p_lightDataSet);
	}
	void addComponent(PointLightDataSet &p_lightDataSet)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		m_lightComponent = new LightComponent(p_lightDataSet);
	}
	void addComponent(SpotLightDataSet &p_lightDataSet)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		m_lightComponent = new LightComponent(p_lightDataSet);
	}	
	void addComponent(LightComponent *p_component)
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Light);
		m_lightComponent = p_component;
	}
	void addComponent(ModelComponentData *p_component) 
	{
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Model);
		m_modelComponent = p_component; 
	}
	void addComponent(ShaderData *p_component) 
	{ 
		// Make sure that this component isn't assigned already
		removeComponent(GraphicsComponentType::GraphicsComponentType_Shader);
		m_shaderComponent = p_component; 
	}
	void removeComponent(const GraphicsComponentType p_compType)
	{
		switch(p_compType)
		{
		case GraphicsComponentType_Light:
		{
			if(m_lightComponent != nullptr)
				delete m_lightComponent;
			break;
		}
		case GraphicsComponentType_Model:
		{
			if(m_modelComponent != nullptr)
				delete m_modelComponent;
			break;
		}
		case GraphicsComponentType_Shader:
		{
			if(m_shaderComponent != nullptr)
				delete m_shaderComponent;
			break;
		}
		}
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

	std::vector<MeshData> m_meshData;
	SpatialTransformData m_worldSpace;

	// Components
	ModelComponentData *m_modelComponent;
	ShaderData *m_shaderComponent;
	LightComponent *m_lightComponent;
	
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