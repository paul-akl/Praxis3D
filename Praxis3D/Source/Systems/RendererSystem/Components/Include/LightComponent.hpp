#pragma once

#include "Systems/RendererSystem/Components/Include/BaseGraphicsComponent.hpp"
#include "Systems/RendererSystem/Include/GraphicsDataSets.hpp"
#include "Definitions/Include/InheritanceObjects.hpp"
#include "Communication/Include/ObserverBase.hpp"
#include "Systems/Base/Include/System.hpp"

class LightComponent : public SystemObject, public LoadableGraphicsObject
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

	struct LightComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		LightComponentConstructionInfo()
		{
			m_color = glm::vec3(1.0f);
			m_intensity = 1.0f;
			m_cutoffAngle = 1.0f;
			m_lightComponentType = LightComponentType::LightComponentType_null;
		}

		glm::vec3 m_color;
		glm::vec3 m_direction;
		glm::vec3 m_position;

		float m_intensity;
		float m_cutoffAngle;

		LightComponentType m_lightComponentType;
	};

	LightComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::Lighting, p_entityID)
	{
		m_lightComponentType = LightComponentType::LightComponentType_null;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, DirectionalLightDataSet &p_lightDataSet, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::DirectionalLight, p_entityID)
	{
		m_lightComponentType = LightComponentType::LightComponentType_directional;
		m_lightComponent.m_directional = p_lightDataSet;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, PointLightDataSet &p_lightDataSet, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::PointLight, p_entityID)
	{
		m_lightComponentType = LightComponentType::LightComponentType_point;
		m_lightComponent.m_point = p_lightDataSet;
	}
	LightComponent(SystemScene *p_systemScene, std::string p_name, SpotLightDataSet &p_lightDataSet, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::SpotLight, p_entityID)
	{
		m_lightComponentType = LightComponentType::LightComponentType_spot;
		m_lightComponent.m_spot = p_lightDataSet;
	}
	LightComponent(const LightComponent &p_other)
	{
		m_lightComponentType = p_other.m_lightComponentType;

		switch(m_lightComponentType)
		{
			case LightComponent::LightComponentType_directional:
				m_lightComponent.m_directional = p_other.m_lightComponent.m_directional;
				break;
			case LightComponent::LightComponentType_point:
				m_lightComponent.m_point = p_other.m_lightComponent.m_point;
				break;
			case LightComponent::LightComponentType_spot:
				m_lightComponent.m_spot = p_other.m_lightComponent.m_spot;
				break;
		}
	}
	LightComponent(LightComponent &&p_other) noexcept
	{
		m_lightComponentType = p_other.m_lightComponentType;

		switch(m_lightComponentType)
		{
			case LightComponent::LightComponentType_directional:
				m_lightComponent.m_directional = p_other.m_lightComponent.m_directional;
				break;
			case LightComponent::LightComponentType_point:
				m_lightComponent.m_point = p_other.m_lightComponent.m_point;
				break;
			case LightComponent::LightComponentType_spot:
				m_lightComponent.m_spot = p_other.m_lightComponent.m_spot;
				break;
		}
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

	LightComponent &operator=(LightComponent &&p_other) noexcept
	{
		m_lightComponentType = p_other.m_lightComponentType;

		switch(m_lightComponentType)
		{
			case LightComponent::LightComponentType_directional:
				m_lightComponent.m_directional = p_other.m_lightComponent.m_directional;
				break;
			case LightComponent::LightComponentType_point:
				m_lightComponent.m_point = p_other.m_lightComponent.m_point;
				break;
			case LightComponent::LightComponentType_spot:
				m_lightComponent.m_spot = p_other.m_lightComponent.m_spot;
				break;
		}

		return *this;
	}

	ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void loadToMemory() 
	{
	}

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

	BitMask getDesiredSystemChanges()	{ return Systems::Changes::Graphics::AllLighting; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		// Get the active flag from the subject and set the active flag accordingly
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));

		// Check individual light data changes based on light type
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

		// Check if the light type is changed
		if(CheckBitmask(p_changeType, Systems::Changes::Graphics::LightType))
		{
			// Store the current light values
			glm::vec3 color = glm::vec3(1.0f);
			float intensity = 1.0f;
			float cutoffAngle = 1.0f;

			switch(getLightType())
			{
				case LightComponent::LightComponentType_point:
					{
						color = m_lightComponent.m_point.m_color;
						intensity = m_lightComponent.m_point.m_intensity;
					}
					break;
				case LightComponent::LightComponentType_spot:
					{
						color = m_lightComponent.m_spot.m_color;
						intensity = m_lightComponent.m_spot.m_intensity;
						cutoffAngle = m_lightComponent.m_spot.m_cutoffAngle;
					}
					break;
				case LightComponent::LightComponentType_directional:
					{
						color = m_lightComponent.m_directional.m_color;
						intensity = m_lightComponent.m_directional.m_intensity;
					}
					break;
			}

			// Get the new light type
			auto newLightType = p_subject->getUnsignedInt(this, Systems::Changes::Graphics::LightType);

			// Set the light data based on the new light type
			switch(newLightType)
			{
				case LightComponent::LightComponentType_point:
					{
						m_lightComponent.m_point = PointLightDataSet(color, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), intensity);
						m_lightComponentType = LightComponent::LightComponentType::LightComponentType_point;
					}
					break;
				case LightComponent::LightComponentType_spot:
					{
						m_lightComponent.m_spot = SpotLightDataSet(color, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), intensity, cutoffAngle);
						m_lightComponentType = LightComponent::LightComponentType::LightComponentType_spot;
					}
					break;
				case LightComponent::LightComponentType_directional:
					{
						m_lightComponent.m_directional = DirectionalLightDataSet(color, glm::vec3(0.0f, 1.0f, 0.0f), intensity);
						m_lightComponentType = LightComponent::LightComponentType::LightComponentType_directional;
					}
					break;
			}
		}

		// Remove the processed change mask from the bit mask value, and reset the processed change value
		//p_changeType &= ~processedChange;
		//processedChange = 0;
	}

	inline const LightComponentType getLightType() const { return m_lightComponentType; }

	// Get the directional light data set. Unsafe - does not perform a check whether the light type is correct
	inline DirectionalLightDataSet *getDirectionalLight() { return &m_lightComponent.m_directional; }
	// Get the point light data set. Unsafe - does not perform a check whether the light type is correct
	inline PointLightDataSet *getPointLight() { return &m_lightComponent.m_point; }
	// Get the spot light data set. Unsafe - does not perform a check whether the light type is correct
	inline SpotLightDataSet *getSpotLight() { return &m_lightComponent.m_spot; }

	// Get the directional light data set. If the type of light is not directional, returns a null pointer
	inline DirectionalLightDataSet *getDirectionalLightSafe() { return (m_lightComponentType == LightComponentType::LightComponentType_directional) ? &m_lightComponent.m_directional : nullptr; }
	// Get the point light data set. If the type of light is not point, returns a null pointer
	inline PointLightDataSet *getPointLightSafe() { return (m_lightComponentType == LightComponentType::LightComponentType_point) ? &m_lightComponent.m_point : nullptr; }
	// Get the spot light data set. If the type of light is not spot, returns a null pointer
	inline SpotLightDataSet *getSpotLightSafe() { return (m_lightComponentType == LightComponentType::LightComponentType_spot) ? &m_lightComponent.m_spot : nullptr; }

	const inline glm::vec3 getColor() const
	{
		switch(m_lightComponentType)
		{
			case LightComponent::LightComponentType_directional:
				return m_lightComponent.m_directional.m_color;
				break;
			case LightComponent::LightComponentType_point:
				return m_lightComponent.m_point.m_color;
				break;
			case LightComponent::LightComponentType_spot:
				return m_lightComponent.m_spot.m_color;
				break;
		}

		return glm::vec3(1.0f);
	}
	const inline float getIntensity() const
	{
		switch(m_lightComponentType)
		{
			case LightComponent::LightComponentType_directional:
				return m_lightComponent.m_directional.m_intensity;
				break;
			case LightComponent::LightComponentType_point:
				return m_lightComponent.m_point.m_intensity;
				break;
			case LightComponent::LightComponentType_spot:
				return m_lightComponent.m_spot.m_intensity;
				break;
		}

		return 1.0f;
	}

private:
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

	// Union object that hold any one of the three light types
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