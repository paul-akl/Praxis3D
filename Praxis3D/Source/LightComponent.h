#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"
#include "ObserverBase.h"
#include "System.h"

class LightComponent : public SystemObject, public SpatialDataManagerObject, public LoadableGraphicsObject
{
	friend class RendererScene;
public:
	enum LightComponentType
	{
		LightComponentType_null,
		LightComponentType_directional,
		LightComponentType_point,
		LightComponentType_spot
	};

	LightComponent(SystemScene *p_systemScene, std::string p_name, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::Lighting)
	{
		m_lightComponentType = LightComponentType::LightComponentType_null;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, DirectionalLightDataSet &p_lightDataSet, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::DirectionalLight)
	{
		m_lightComponentType = LightComponentType::LightComponentType_directional;
		m_lightComponent.m_directional = p_lightDataSet;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, PointLightDataSet &p_lightDataSet, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::PointLight)
	{
		m_lightComponentType = LightComponentType::LightComponentType_point;
		m_lightComponent.m_point = p_lightDataSet;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, SpotLightDataSet &p_lightDataSet, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::SpotLight)
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

	ErrorCode init()
	{
		// Mark the object as loaded, because there is nothing to be specifically loaded, at least for now
		//setLoadedToMemory(true);
		//setLoadedToVideoMemory(true);

		return ErrorCode::Success;
	}

	ErrorCode importObject(const PropertySet &p_properties)
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if light node is present and the component hasn't been loaded already
		if(p_properties && !isLoadedToMemory())
		{	
			// Get the light type
			auto const &type = p_properties.getPropertyByID(Properties::Type).getID();

			// Load values based on the type of light
			switch(type)
			{
			case Properties::DirectionalLight:
				m_lightComponentType = LightComponentType::LightComponentType_directional;
				m_objectType = Properties::PropertyID::DirectionalLight;

				m_lightComponent.m_directional.m_color = p_properties.getPropertyByID(Properties::Color).getVec3f();
				m_lightComponent.m_directional.m_intensity = p_properties.getPropertyByID(Properties::Intensity).getFloat();
				m_lightComponent.m_directional.m_direction = glm::vec3(m_spatialData->getWorldTransform()[2]);
				setLoadedToMemory(true);
				importError = ErrorCode::Success;

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, m_name + " - Directional light loaded");
				break;

			case Properties::PointLight:
				m_lightComponentType = LightComponentType::LightComponentType_point;
				m_objectType = Properties::PropertyID::PointLight;

				m_lightComponent.m_point.m_color = p_properties.getPropertyByID(Properties::Color).getVec3f();
				m_lightComponent.m_point.m_intensity = p_properties.getPropertyByID(Properties::Intensity).getFloat();
				m_lightComponent.m_point.m_position = glm::vec3(m_spatialData->getWorldTransform()[3]);
				setLoadedToMemory(true);
				importError = ErrorCode::Success;

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, m_name + " - Point light loaded");
				break;

			case Properties::SpotLight:
				m_lightComponentType = LightComponentType::LightComponentType_spot;
				m_objectType = Properties::PropertyID::SpotLight;

				m_lightComponent.m_spot.m_color = p_properties.getPropertyByID(Properties::Color).getVec3f();
				m_lightComponent.m_spot.m_cutoffAngle = p_properties.getPropertyByID(Properties::CutoffAngle).getFloat();
				m_lightComponent.m_spot.m_intensity = p_properties.getPropertyByID(Properties::Intensity).getFloat();
				m_lightComponent.m_spot.m_direction = glm::vec3(m_spatialData->getWorldTransform()[2]);
				m_lightComponent.m_spot.m_position = glm::vec3(m_spatialData->getWorldTransform()[3]);
				setLoadedToMemory(true);
				importError = ErrorCode::Success;

				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, m_name + " - Spot light loaded");
				break;

			default:
				ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LightComponent, m_name + " - missing \'Type\' identifier");
				break;
			}

			if(importError == ErrorCode::Success)
			{
				setLoadedToMemory(true);
				setLoadedToVideoMemory(true);
				setActive(true);
			}
		}
		return importError;
	}

	PropertySet exportObject()
	{
		// Create the root Lighting property set
		PropertySet propertySet(Properties::Lighting);

		// Add the properties based on the type of light
		switch(getLightType())
		{
		case LightComponent::LightComponentType_directional:
			propertySet.addProperty(Properties::Type, Properties::DirectionalLight);
			propertySet.addProperty(Properties::Color, m_lightComponent.m_directional.m_color);
			propertySet.addProperty(Properties::Intensity, m_lightComponent.m_directional.m_intensity);

			ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, m_name + " - Directional light exported.");
			break;

		case LightComponent::LightComponentType_point:
			propertySet.addProperty(Properties::Type, Properties::PointLight);
			propertySet.addProperty(Properties::Color, m_lightComponent.m_point.m_color);
			propertySet.addProperty(Properties::Intensity, m_lightComponent.m_point.m_intensity);

			ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, m_name + " - Point light exported.");
			break;

		case LightComponent::LightComponentType_spot:
			propertySet.addProperty(Properties::Type, Properties::SpotLight);
			propertySet.addProperty(Properties::Color, m_lightComponent.m_spot.m_color);
			propertySet.addProperty(Properties::CutoffAngle, m_lightComponent.m_spot.m_cutoffAngle);
			propertySet.addProperty(Properties::Intensity, m_lightComponent.m_spot.m_intensity);

			ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LightComponent, m_name + " - Spot light exported.");
			break;

		default:
			break;
		}

		return propertySet;
	}

	void loadToMemory() 
	{
		updatePosition(glm::vec3(m_spatialData->getWorldTransform()[3]));
		updateRotation(glm::vec3(m_spatialData->getWorldTransform()[2]));
	}

	// System type is Graphics
	BitMask getSystemType() { return Systems::Graphics; }

	void update(const float p_deltaTime)
	{
		// If the spatial data has changed, update the spatial data in light datasets
		if(hasSpatialDataUpdated())
		{
			updatePosition(glm::vec3(m_spatialData->getWorldTransform()[3]));
			updateRotation(glm::vec3(m_spatialData->getWorldTransform()[2]));
		}

		if(isUpdateNeeded())
		{
			// Mark as updated
			updatePerformed();
		}
	}

	BitMask getDesiredSystemChanges()	{ return Systems::Changes::Graphics::AllLighting; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Track what data has been modified
		//BitMask newChanges = Systems::Changes::None;
		//BitMask processedChange = 0;

		// Check if the light should be enabled/disabled
		if(CheckBitmask(p_changeType, Systems::Changes::Graphics::LightEnabled))
			m_active = p_subject->getBool(this, Systems::Changes::Graphics::LightEnabled);

		switch(getLightType())
		{
			case LightComponent::LightComponentType_point:
			{
				// Check if the light color should be changed
				if(CheckBitmask(p_changeType, Systems::Changes::Graphics::Color))
					m_lightComponent.m_point.m_color = p_subject->getVec3(this, Systems::Changes::Graphics::Color);

				// Check if the light intensity should be changed
				if(CheckBitmask(p_changeType, Systems::Changes::Graphics::Intensity))
					m_lightComponent.m_point.m_intensity = p_subject->getFloat(this, Systems::Changes::Graphics::Intensity);
			}
			break;
			case LightComponent::LightComponentType_spot:
			{
				// Check if the light color should be changed
				if(CheckBitmask(p_changeType, Systems::Changes::Graphics::Color))
					m_lightComponent.m_spot.m_color = p_subject->getVec3(this, Systems::Changes::Graphics::Color);

				// Check if the spot lights cutoff angle (cone size) should be changed
				if(CheckBitmask(p_changeType, Systems::Changes::Graphics::CutoffAngle))
					m_lightComponent.m_spot.m_cutoffAngle = p_subject->getFloat(this, Systems::Changes::Graphics::CutoffAngle);

				// Check if the light intensity should be changed
				if(CheckBitmask(p_changeType, Systems::Changes::Graphics::Intensity))
					m_lightComponent.m_spot.m_intensity = p_subject->getFloat(this, Systems::Changes::Graphics::Intensity);
			}
			break;
			case LightComponent::LightComponentType_directional:
			{
				// Check if the light color should be changed
				if(CheckBitmask(p_changeType, Systems::Changes::Graphics::Color))
					m_lightComponent.m_directional.m_color = p_subject->getVec3(this, Systems::Changes::Graphics::Color);

				// Check if the light intensity should be changed
				if(CheckBitmask(p_changeType, Systems::Changes::Graphics::Intensity))
					m_lightComponent.m_directional.m_intensity = p_subject->getFloat(this, Systems::Changes::Graphics::Intensity);
			}
			break;
		}

		// Remove the processed change mask from the bit mask value, and reset the processed change value
		//p_changeType &= ~processedChange;
		//processedChange = 0;
	}

	inline const LightComponentType getLightType() const { return m_lightComponentType; }

	// Get the directional light data set. If the type of light is not directional, returns a null pointer
	DirectionalLightDataSet *getDirectionalLight() { return (m_lightComponentType == LightComponentType::LightComponentType_directional) ? &m_lightComponent.m_directional : nullptr; }
	// Get the point light data set. If the type of light is not point, returns a null pointer
	PointLightDataSet *getPointLight() { return (m_lightComponentType == LightComponentType::LightComponentType_point) ? &m_lightComponent.m_point : nullptr; }
	// Get the spot light data set. If the type of light is not spot, returns a null pointer
	SpotLightDataSet *getSpotLight() { return (m_lightComponentType == LightComponentType::LightComponentType_spot) ? &m_lightComponent.m_spot : nullptr; }

private:
	// Union object that hold any one of the three light types
	union m_lightComponent
	{
		m_lightComponent() { }
		~m_lightComponent() { }

		DirectionalLightDataSet m_directional;
		PointLightDataSet m_point;
		SpotLightDataSet m_spot;
	} m_lightComponent;

	inline void updateColor(const glm::vec3 &p_color)
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
	inline void updateCutoffAngle(const float p_cutoffAngle)
	{
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_cutoffAngle = p_cutoffAngle;
			break;
		}
	}
	inline void updateIntensity(const float p_intensity)
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
	inline void updateSpatialData(const SpatialData &p_data)
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
	inline void updatePosition(const glm::vec3 &p_position)
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
	inline void updateRotation(const glm::vec3 &p_rotation)
	{
		// Update each type of light individually, as their spatial data is made up of different attributes
		switch(m_lightComponentType)
		{
		case LightComponent::LightComponentType_directional:
			m_lightComponent.m_directional.m_direction = glm::normalize(p_rotation);
			break;
		case LightComponent::LightComponentType_spot:
			m_lightComponent.m_spot.m_direction = p_rotation;
			break;
		}
	}

	LightComponentType m_lightComponentType;
};