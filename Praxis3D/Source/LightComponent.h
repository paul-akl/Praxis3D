#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "ObserverBase.h"
#include "System.h"

class DirectionalLightComponent : public BaseGraphicsComponent
{
	friend class LightComponent;
public:
	void load(PropertySet &p_properties) final override
	{ 
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::Color:
				m_lightDataSet.m_color = p_properties[i].getVec3f();
				break;

			case Properties::Direction:
				// Need to normalize the light direction
				m_lightDataSet.m_direction = Math::normalize(p_properties[i].getVec3f());
				break;

			case Properties::Intensity:
				m_lightDataSet.m_intensity = p_properties[i].getFloat();
				break;
				
			case Properties::WorldRotation:
				m_lightDataSet.m_direction = p_properties[i].getVec3f();
				break;
			}
		}
	}

	PropertySet export() final override
	{ 
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);
		
		// Add variables
		propertySet.addProperty(Properties::Type, Properties::DirectionalLight);
		propertySet.addProperty(Properties::Color, m_lightDataSet.m_color);
		propertySet.addProperty(Properties::Direction, m_lightDataSet.m_direction);
		propertySet.addProperty(Properties::Intensity, m_lightDataSet.m_intensity);
		
		return propertySet;
	}

	void update(GraphicsObject &p_parentObject);
	
	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Graphics::Color)
		{
			m_lightDataSet.m_color = p_subject->getVec3(nullptr, Systems::Changes::Graphics::Color);
		}

		if(p_changeType & Systems::Changes::Graphics::Intensity)
		{
			m_lightDataSet.m_intensity = p_subject->getFloat(nullptr, Systems::Changes::Graphics::Intensity);
		}
	}
	
	BitMask getDesiredSystemChanges() { return Systems::Changes::Graphics::Color | Systems::Changes::Graphics::Intensity; }

private:
	//DirectionalLightComponent() { }
	DirectionalLightComponent(DirectionalLightDataSet &p_lightDataSet) : m_lightDataSet(p_lightDataSet) { }
	~DirectionalLightComponent() { }

	DirectionalLightDataSet m_lightDataSet;
};

class PointLightComponent : public BaseGraphicsComponent
{
	friend class LightComponent;
public:
	void load(PropertySet &p_properties) final override
	{ 
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::Color:
				m_lightDataSet.m_color = p_properties[i].getVec3f();
				break;

			case Properties::Intensity:
				m_lightDataSet.m_intensity = p_properties[i].getFloat();
				break;
				
			case Properties::WorldPosition:
				m_lightDataSet.m_position = p_properties[i].getVec3f();
				break;
			}
		}
	}

	PropertySet export() final override
	{ 		
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);
		
		// Add variables
		propertySet.addProperty(Properties::Type, Properties::PointLight);
		propertySet.addProperty(Properties::Color, m_lightDataSet.m_color);
		propertySet.addProperty(Properties::Intensity, m_lightDataSet.m_intensity);

		return propertySet;
	}
	
	void update(GraphicsObject &p_parentObject);

	void changeOccurred(ObservedSubject* p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Graphics::Color)
		{
			m_lightDataSet.m_color = p_subject->getVec3(nullptr, Systems::Changes::Graphics::Color);
		}

		if(p_changeType & Systems::Changes::Graphics::Intensity)
		{
			m_lightDataSet.m_intensity = p_subject->getFloat(nullptr, Systems::Changes::Graphics::Intensity);
		}
	}

	BitMask getDesiredSystemChanges() { return Systems::Changes::Graphics::Color | Systems::Changes::Graphics::Intensity; }

private:
	//PointLightComponent() { }
	PointLightComponent(PointLightDataSet &p_lightDataSet) : m_lightDataSet(p_lightDataSet) { }
	~PointLightComponent() { }

	PointLightDataSet m_lightDataSet;
};

class SpotLightComponent : public BaseGraphicsComponent
{
	friend class LightComponent;
public:
	void load(PropertySet &p_properties) final override
	{ 
		// Load property data
		for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
		{
			switch(p_properties[i].getPropertyID())
			{
			case Properties::CutoffAngle:
				// Convert to radians
				m_lightDataSet.m_cutoffAngle = cosf(Math::toRadian(p_properties[i].getFloat()));
				break;

			case Properties::Color:
				m_lightDataSet.m_color = p_properties[i].getVec3f();
				break;

			case Properties::Intensity:
				m_lightDataSet.m_intensity = p_properties[i].getFloat();
				break;
				
			case Properties::WorldPosition:
				m_lightDataSet.m_position = p_properties[i].getVec3f();
				break;

			case Properties::WorldRotation:
				m_lightDataSet.m_direction = p_properties[i].getVec3f();
				break;
			}
		}
	}

	PropertySet export() final override
	{ 		
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::SpotLight);
		// Make sure to revert the cutoff angle back to degrees
		propertySet.addProperty(Properties::CutoffAngle, Math::toDegree(acosf(m_lightDataSet.m_cutoffAngle)));
		propertySet.addProperty(Properties::Color, m_lightDataSet.m_color);
		propertySet.addProperty(Properties::Direction, m_lightDataSet.m_direction);
		propertySet.addProperty(Properties::Intensity, m_lightDataSet.m_intensity);

		return propertySet;
	}
	
	void update(GraphicsObject &p_parentObject);

	void changeOccurred(ObservedSubject* p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Graphics::Color)
		{
			m_lightDataSet.m_color = p_subject->getVec3(nullptr, Systems::Changes::Graphics::Color);
		}

		if(p_changeType & Systems::Changes::Graphics::Intensity)
		{
			m_lightDataSet.m_intensity = p_subject->getFloat(nullptr, Systems::Changes::Graphics::Intensity);
		}

		if(p_changeType & Systems::Changes::Graphics::CutoffAngle)
		{
			m_lightDataSet.m_cutoffAngle = p_subject->getFloat(nullptr, Systems::Changes::Graphics::CutoffAngle);
		}
	}

	BitMask getDesiredSystemChanges() { return Systems::Changes::Graphics::Color | Systems::Changes::Graphics::Intensity | Systems::Changes::Graphics::CutoffAngle; }

private:
	//SpotLightComponent() { }
	SpotLightComponent(SpotLightDataSet &p_lightDataSet) : m_lightDataSet(p_lightDataSet) { }
	~SpotLightComponent() { }

	SpotLightDataSet m_lightDataSet;
};

class LightComponent : public BaseGraphicsComponent
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
		m_currentLightComponent = &m_lightComponent.m_directional;
	}
	LightComponent(PointLightDataSet &p_lightDataSet)
	{
		m_lightComponentType = LightComponentType::LightComponentType_point;
		m_lightComponent.m_point = p_lightDataSet;
		m_currentLightComponent = &m_lightComponent.m_point;
	}
	LightComponent(SpotLightDataSet &p_lightDataSet)
	{
		m_lightComponentType = LightComponentType::LightComponentType_spot;
		m_lightComponent.m_spot = p_lightDataSet;
		m_currentLightComponent = &m_lightComponent.m_spot;
	}

	~LightComponent() 
	{
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			//delete m_lightComponent.m_directional;
			break;
		case LightComponent::LightComponentType_point:
			//delete m_lightComponent.m_point;
			break;
		case LightComponent::LightComponentType_spot:
			//delete m_lightComponent.m_spot;
			break;
		}
	}

	void load(PropertySet& p_properties) final override { m_currentLightComponent->load(p_properties); }

	PropertySet export() final override { return m_currentLightComponent->export(); }

	void update(GraphicsObject &p_parentObject) final override { m_currentLightComponent->update(p_parentObject); }

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

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) final override { m_currentLightComponent->changeOccurred(p_subject, p_changeType); }

	BitMask getDesiredSystemChanges() final override { return m_currentLightComponent->getDesiredSystemChanges(); }

	LightComponentType getLightType() { return m_lightComponentType; }

	DirectionalLightDataSet *getDirectionalLight() { return (m_lightComponentType == LightComponentType::LightComponentType_directional) ? &m_lightComponent.m_directional : nullptr; }
	PointLightDataSet *getSpotLight() { return (m_lightComponentType == LightComponentType::LightComponentType_point) ? &m_lightComponent.m_point : nullptr; }
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
	BaseGraphicsComponent *m_currentLightComponent;
};