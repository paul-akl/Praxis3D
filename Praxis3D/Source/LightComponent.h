#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "ObserverBase.h"
#include "System.h"

class LightComponent : public SystemObject
{
public:
	enum LightComponentType
	{
		LightComponentType_directional,
		LightComponentType_point,
		LightComponentType_spot
	};

	LightComponent(SystemScene *p_systemScene, std::string p_name, SceneLoader &p_sceneLoader, DirectionalLightDataSet &p_lightDataSet, std::size_t p_id = 0)
	{
		m_lightComponentType = LightComponentType::LightComponentType_directional;
		m_lightComponent.m_directional = p_lightDataSet;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, SceneLoader &p_sceneLoader, PointLightDataSet &p_lightDataSet, std::size_t p_id = 0)
	{
		m_lightComponentType = LightComponentType::LightComponentType_point;
		m_lightComponent.m_point = p_lightDataSet;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, SceneLoader &p_sceneLoader, SpotLightDataSet &p_lightDataSet, std::size_t p_id = 0)
	{
		m_lightComponentType = LightComponentType::LightComponentType_spot;
		m_lightComponent.m_spot = p_lightDataSet;
	}

	ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void loadToMemory() { }

	// System type is Graphics
	BitMask getSystemType() { return Systems::Graphics; }

	void update(const float p_deltaTime)
	{
		if(isUpdateNeeded())
		{
			// Mark as updated
			updatePerformed();
		}
	}

	BitMask getDesiredSystemChanges() { return Systems::Changes::Spatial::AllWorld | Systems::Changes::Graphics::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::WorldTransform; }

	~LightComponent() 
	{
		// Nothing to delete so far
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			break;
		case LightComponent::LightComponentType_point:
			break;
		case LightComponent::LightComponentType_spot:
			break;
		}
	}

	void updateSpatialData(const SpatialData &p_data)
	{
		// Update each type of light individually, as their spatial data is made up of different attributes
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			m_lightComponent.m_directional.m_direction = p_data.m_rotationEuler;
			break;
		case LightComponent::LightComponentType_point:
			m_lightComponent.m_point.m_position = p_data.m_position;
			break;
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_position = p_data.m_position;
			m_lightComponent.m_spot.m_direction = p_data.m_rotationEuler;
			break;
		}
	}
	void updatePosition(const Math::Vec3f &p_position)
	{
		// Update each type of light individually, as their spatial data is made up of different attributes
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_point:
			m_lightComponent.m_point.m_position = p_position;
			break;
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_position = p_position;
			break;
		}
	}
	void updateRotation(const Math::Vec3f &p_rotation)
	{
		// Update each type of light individually, as their spatial data is made up of different attributes
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			m_lightComponent.m_directional.m_direction = p_rotation;
			break;
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_direction = p_rotation;
			break;
		}
	}

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;

		// Get all of the world spatial data, include the transform matrix; add up the bit-mask of changed data;
		if(p_changeType & Systems::Changes::Spatial::AllWorld)
		{
			updateSpatialData(p_subject->getSpatialData(this, Systems::Changes::Spatial::AllWorldNoTransform));
			newChanges = newChanges | Systems::Changes::Spatial::AllWorld;
		}
		else
		{
			// Get world spatial data without transform matrix; add up the bit-mask of changed data; flag object to need updating
			if(p_changeType & Systems::Changes::Spatial::AllWorldNoTransform)
			{
				setUpdateNeeded(true);
				newChanges = newChanges | Systems::Changes::Spatial::AllWorldNoTransform;
			}
			else
			{
				// Get world position vector; add up the bit-mask of changed data; flag object to need updating
				if(p_changeType & Systems::Changes::Spatial::WorldPosition)
				{
					updatePosition(p_subject->getVec3(this, Systems::Changes::Spatial::WorldPosition));
					newChanges = newChanges | Systems::Changes::Spatial::WorldPosition;

					setUpdateNeeded(true);
				}

				// Get world rotation vector; add up the bit-mask of changed data; flag object to need updating; flag rotation quaternion to need updating
				if(p_changeType & Systems::Changes::Spatial::WorldRotation)
				{
					updateRotation(p_subject->getVec3(this, Systems::Changes::Spatial::WorldRotation));
					newChanges = newChanges | Systems::Changes::Spatial::WorldRotation;

					setUpdateNeeded(true);
				}
			}
		}
	}

	LightComponentType getLightType() { return m_lightComponentType; }

	// Get the directional light data set. If the type of light is not directional, returns a null pointer
	DirectionalLightDataSet *getDirectionalLight() { return (m_lightComponentType == LightComponentType::LightComponentType_directional) ? &m_lightComponent.m_directional : nullptr; }
	// Get the point light data set. If the type of light is not point, returns a null pointer
	PointLightDataSet *getSpotLight() { return (m_lightComponentType == LightComponentType::LightComponentType_point) ? &m_lightComponent.m_point : nullptr; }
	// Get the spot light data set. If the type of light is not spot, returns a null pointer
	SpotLightDataSet *getPointLight() { return (m_lightComponentType == LightComponentType::LightComponentType_spot) ? &m_lightComponent.m_spot : nullptr; }

private:
	union m_lightComponent
	{
		m_lightComponent() { }
		~m_lightComponent() { }

		DirectionalLightDataSet m_directional;
		PointLightDataSet m_point;
		SpotLightDataSet m_spot;
	} m_lightComponent;

	LightComponentType m_lightComponentType;
};