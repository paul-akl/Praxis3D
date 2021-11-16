#pragma once

#include <assert.h>
#include "GraphicsDataSets.h"
#include "System.h"

class DirectionalLightObject : public SystemObject
{
	//friend class RendererScene;
public:
	DirectionalLightObject(SystemScene *p_systemScene, const std::string &p_name, DirectionalLightDataSet p_lightDataSet)
		: SystemObject(p_systemScene, p_name, Properties::DirectionalLight), m_lightDataSet(p_lightDataSet), m_active(true) 
	{ 

	}

	ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void update(const float p_deltaTime)
	{

	}
	void loadToMemory()
	{

	}

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);
		
		// Add variables
		propertySet.addProperty(Properties::Type, Properties::DirectionalLight);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Color, m_lightDataSet.m_color);
		propertySet.addProperty(Properties::Direction, m_lightDataSet.m_direction);
		propertySet.addProperty(Properties::Intensity, m_lightDataSet.m_intensity);
		
		return propertySet;
	}

	BitMask getSystemType() { return Systems::Graphics; }

	BitMask getDesiredSystemChanges()	{ return Systems::Changes::Spatial::LocalRotation; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::LocalRotation; }

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spatial::LocalRotation)
		{
			m_lightDataSet.m_direction =
				p_subject->getVec3(this, Systems::Changes::Spatial::LocalRotation);

			postChanges(p_changeType);
		}
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalRotation:
			return m_lightDataSet.m_direction;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Getters
	const inline bool active() const { return m_active; }
	const inline DirectionalLightDataSet &getLightDataSet() const { return m_lightDataSet; }

	// Setters
	inline void setActive(bool p_flag)							{ m_active = p_flag;						}
	inline void setColor(const Math::Vec3f &p_color)			{ m_lightDataSet.m_color = p_color;			}
	inline void setDirection(const Math::Vec3f &p_direction)	{ m_lightDataSet.m_direction = p_direction; }
	inline void setIntensity(const float p_intensity)			{ m_lightDataSet.m_intensity = p_intensity; }

private:
	DirectionalLightDataSet m_lightDataSet;
	
	bool m_active;
};

class PointLightObject : public SystemObject
{
	//friend class RendererScene;
public:
	PointLightObject(SystemScene *p_systemScene, const std::string &p_name, PointLightDataSet p_lightDataSet)
		: SystemObject(p_systemScene, p_name, Properties::PointLight), m_lightDataSet(p_lightDataSet), m_active(true) { }

	ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void update(const float p_deltaTime)
	{

	}
	void loadToMemory()
	{

	}

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);
		
		// Add variables
		propertySet.addProperty(Properties::Type, Properties::PointLight);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Attenuation, m_lightDataSet.m_attenuation);
		propertySet.addProperty(Properties::Color, m_lightDataSet.m_color);
		propertySet.addProperty(Properties::Intensity, m_lightDataSet.m_intensity);
		propertySet.addProperty(Properties::OffsetPosition, m_offsetPosition);
		propertySet.addProperty(Properties::LocalPosition, m_lightDataSet.m_position);

		return propertySet;
	}

	BitMask getSystemType() { return Systems::Graphics; }

	BitMask getDesiredSystemChanges() { return Systems::Changes::Spatial::Position; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::Position; }

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spatial::LocalPosition)
		{
			m_lightDataSet.m_position =
				p_subject->getVec3(this, Systems::Changes::Spatial::LocalPosition) + m_offsetPosition;

			postChanges(p_changeType);
		}
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalPosition:
			return m_lightDataSet.m_position;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Getters
	const inline bool active() const { return m_active; }
	const inline PointLightDataSet &getLightDataSet() const { return m_lightDataSet; }
	const inline Math::Vec3f &getOffsetPosition() const { return m_offsetPosition; }

	// Setters
	inline void setActive(bool p_flag)								{ m_active = p_flag;							}
	inline void setColor(const Math::Vec3f &p_color)				{ m_lightDataSet.m_color = p_color;				}
	inline void setPosition(const Math::Vec3f &p_position)			{ m_lightDataSet.m_position = p_position;		}
	inline void setAttenuation(const Math::Vec3f &p_attenuation)	{ m_lightDataSet.m_attenuation = p_attenuation;	}
	inline void setOffsetPosition(const Math::Vec3f &p_position)	{ m_offsetPosition = p_position;				}
	inline void setIntensity(const float p_intensity)				{ m_lightDataSet.m_intensity = p_intensity;		}
	
private:
	PointLightDataSet m_lightDataSet;

	Math::Vec3f m_offsetPosition;

	bool m_active;
};

class SpotLightObject : public SystemObject
{
	//friend class RendererScene;
public:
	SpotLightObject(SystemScene *p_systemScene, const std::string &p_name, SpotLightDataSet p_lightDataSet)
		: SystemObject(p_systemScene, p_name, Properties::SpotLight), m_lightDataSet(p_lightDataSet), m_active(true) { }

	ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void update(const float p_deltaTime)
	{

	}
	void loadToMemory()
	{

	}

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::SpotLight);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Attenuation, m_lightDataSet.m_attenuation);
		// Make sure to revert the cutoff angle back to degrees
		propertySet.addProperty(Properties::CutoffAngle, Math::toDegree(acosf(m_lightDataSet.m_cutoffAngle)));
		propertySet.addProperty(Properties::Color, m_lightDataSet.m_color);
		propertySet.addProperty(Properties::Direction, m_lightDataSet.m_direction);
		propertySet.addProperty(Properties::Intensity, m_lightDataSet.m_intensity);
		propertySet.addProperty(Properties::OffsetPosition, m_offsetPosition);
		propertySet.addProperty(Properties::OffsetRotation, m_offsetRotation);
		propertySet.addProperty(Properties::LocalPosition, m_lightDataSet.m_position);

		return propertySet;
	}

	BitMask getSystemType() { return Systems::Graphics; }

	BitMask getDesiredSystemChanges() { return Systems::Changes::Spatial::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spatial::All; }

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		BitMask relevantChanges = 0;
		if(p_changeType & Systems::Changes::Spatial::LocalPosition)
		{
			m_lightDataSet.m_position =
				p_subject->getVec3(this, Systems::Changes::Spatial::LocalPosition) + m_offsetPosition;

			relevantChanges |= Systems::Changes::Spatial::LocalPosition;
		}

		if(p_changeType & Systems::Changes::Spatial::LocalRotation)
		{
			m_lightDataSet.m_direction =
				p_subject->getVec3(this, Systems::Changes::Spatial::LocalRotation) + m_offsetRotation;

			relevantChanges |= Systems::Changes::Spatial::LocalRotation;
		}

		if(relevantChanges)
			postChanges(relevantChanges);
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		/*switch(p_changedBits)
		{
		case Systems::Changes::Spacial::Position:
			return m_lightDataSet.m_position;
			break;
		case Systems::Changes::Spacial::Rotation:
			return m_lightDataSet.m_direction;
			break;
		}*/
		assert(true);
		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Getters
	const inline bool active() const { return m_active; }
	const inline SpotLightDataSet &getLightDataSet() const { return m_lightDataSet; }
	const inline Math::Vec3f &getOffsetPosition() const { return m_offsetPosition; }
	const inline Math::Vec3f &getOffsetRotation() const { return m_offsetRotation; }

	// Setters
	inline void setActive(bool p_flag)								{ m_active = p_flag;							}
	inline void setColor(const Math::Vec3f &p_color)				{ m_lightDataSet.m_color = p_color;				}
	inline void setPosition(const Math::Vec3f &p_position)			{ m_lightDataSet.m_position = p_position;		}
	inline void setDirection(const Math::Vec3f &p_direction)		{ m_lightDataSet.m_direction = p_direction;		}
	inline void setAttenuation(const Math::Vec3f &p_attenuation)	{ m_lightDataSet.m_attenuation = p_attenuation; }
	inline void setOffsetPosition(const Math::Vec3f &p_position)	{ m_offsetPosition = p_position;				}
	inline void setOffsetRotation(const Math::Vec3f &p_rotation)	{ m_offsetRotation = p_rotation;				}
	inline void setCutoffAngle(const float p_cutoffAngle)			{ m_lightDataSet.m_cutoffAngle = p_cutoffAngle; }
	inline void setIntensity(const float p_intensity)				{ m_lightDataSet.m_intensity = p_intensity;		}

private:
	SpotLightDataSet m_lightDataSet;

	Math::Vec3f m_offsetPosition,
				m_offsetRotation;

	bool m_active;
};