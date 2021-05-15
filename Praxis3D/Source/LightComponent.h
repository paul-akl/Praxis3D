#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "ObserverBase.h"
#include "System.h"

class LightComponent
{
public:
	enum LightComponentType
	{
		LightComponentType_directional,
		LightComponentType_point,
		LightComponentType_spot
	};

	LightComponent(DirectionalLightDataSet &p_lightDataSet)
	{
		m_lightComponentType = LightComponentType::LightComponentType_directional;
		m_lightComponent.m_directional = p_lightDataSet;
	}
	LightComponent(PointLightDataSet &p_lightDataSet)
	{
		m_lightComponentType = LightComponentType::LightComponentType_point;
		m_lightComponent.m_point = p_lightDataSet;
	}
	LightComponent(SpotLightDataSet &p_lightDataSet)
	{
		m_lightComponentType = LightComponentType::LightComponentType_spot;
		m_lightComponent.m_spot = p_lightDataSet;
	}

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

	void updateSpatialData(SpatialTransformData &p_data)
	{
		// Update each type of light individually, as their spatial data is made up of different attributes
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			m_lightComponent.m_directional.m_direction = p_data.m_spatialData.m_rotationEuler;
			break;
		case LightComponent::LightComponentType_point:
			m_lightComponent.m_point.m_position = p_data.m_spatialData.m_position;
			break;
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_position = p_data.m_spatialData.m_position;
			m_lightComponent.m_spot.m_direction = p_data.m_spatialData.m_rotationEuler;
			break;
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