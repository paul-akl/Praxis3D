#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"
#include "ObserverBase.h"
#include "System.h"

class LightComponent : public SystemObject, public SpatialDataManagerObject
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
		// If the spatial data has changed, update the spatial data in light datasets
		if(hasSpatialDataUpdated())
		{
			updatePosition(m_spatialData->getWorldSpaceData().m_spatialData.m_position);
			updateRotation(m_spatialData->getWorldSpaceData().m_spatialData.m_rotationEuler);
		}

		if(isUpdateNeeded())
		{
			// Mark as updated
			updatePerformed();
		}
	}

	BitMask getDesiredSystemChanges()	{ return Systems::Changes::Graphics::AllLighting; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

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

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		BitMask newChanges = Systems::Changes::None;
		BitMask processedChange = 0;

		// Remove the processed change mask from the bit mask value, and reset the processed change value
		p_changeType &= ~processedChange;
		processedChange = 0;
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

	void updateColor(const Math::Vec3f &p_color)
	{
		// Update each type of light individually
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			m_lightComponent.m_directional.m_color = p_color;
			break;
		case LightComponent::LightComponentType_point:
			m_lightComponent.m_point.m_color = p_color;
			break;
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_color = p_color;
			break;
		}
	}
	void updateCutoffAngle(const float p_cutoffAngle)
	{
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_cutoffAngle = p_cutoffAngle;
			break;
		}
	}
	void updateIntensity(const float p_intensity)
	{
		// Update each type of light individually
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			m_lightComponent.m_directional.m_intensity = p_intensity;
			break;
		case LightComponent::LightComponentType_point:
			m_lightComponent.m_point.m_intensity = p_intensity;
			break;
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_intensity = p_intensity;
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

	LightComponentType m_lightComponentType;
};