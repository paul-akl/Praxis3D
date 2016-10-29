#pragma once

#include "GraphicsDataSets.h"
#include "System.h"

class DirectionalLightObject : public SystemObject
{
	friend class RendererScene;
public:
	DirectionalLightObject(SystemScene *p_systemScene, const std::string &p_name, DirectionalLightDataSet p_lightDataSet)
		: SystemObject(p_systemScene, p_name, Properties::DirectionalLight), m_lightDataSet(p_lightDataSet), m_active(true) { }

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

	BitMask getDesiredSystemChanges()	{ return Systems::Changes::Spacial::Rotation; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spacial::Rotation; }

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spacial::Rotation)
		{
			m_lightDataSet.m_direction =
				p_subject->getVec3(this, Systems::Changes::Spacial::Rotation);

			postChanges(p_changeType);
		}
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spacial::Rotation:
			return m_lightDataSet.m_direction;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Getters
	const inline bool active() const { return m_active; }
	const inline DirectionalLightDataSet &getLightDataSet() const { return m_lightDataSet; }

	// Setters
	inline void setActive(bool p_flag) { m_active = p_flag; }

private:
	DirectionalLightDataSet m_lightDataSet;

	bool m_active;
};

class PointLightObject : public SystemObject
{
	friend class RendererScene;
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
		propertySet.addProperty(Properties::Position, m_lightDataSet.m_position);

		return propertySet;
	}

	BitMask getSystemType() { return Systems::Graphics; }

	BitMask getDesiredSystemChanges() { return Systems::Changes::Spacial::Position; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spacial::Position; }

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spacial::Position)
		{
			m_lightDataSet.m_position =
				p_subject->getVec3(this, Systems::Changes::Spacial::Position) + m_offsetPosition;

			postChanges(p_changeType);
		}
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spacial::Position:
			return m_lightDataSet.m_position;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Getters
	const inline bool active() const { return m_active; }
	const inline PointLightDataSet &getLightDataSet() const { return m_lightDataSet; }

	// Setters
	inline void setActive(bool p_flag) { m_active = p_flag; }

private:
	PointLightDataSet m_lightDataSet;

	Math::Vec3f m_offsetPosition;

	bool m_active;
};

class SpotLightObject : public SystemObject
{
	friend class RendererScene;
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
		propertySet.addProperty(Properties::Position, m_lightDataSet.m_position);

		return propertySet;
	}

	BitMask getSystemType() { return Systems::Graphics; }

	BitMask getDesiredSystemChanges() { return Systems::Changes::Spacial::All; }
	BitMask getPotentialSystemChanges() { return Systems::Changes::Spacial::All; }

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		BitMask relevantChanges = 0;
		if(p_changeType & Systems::Changes::Spacial::Position)
		{
			m_lightDataSet.m_position =
				p_subject->getVec3(this, Systems::Changes::Spacial::Position) + m_offsetPosition;

			relevantChanges |= Systems::Changes::Spacial::Position;
		}

		if(p_changeType & Systems::Changes::Spacial::Rotation)
		{
			m_lightDataSet.m_direction =
				p_subject->getVec3(this, Systems::Changes::Spacial::Rotation) + m_offsetRotation;

			relevantChanges |= Systems::Changes::Spacial::Rotation;
		}

		if(relevantChanges)
			postChanges(relevantChanges);
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spacial::Position:
			return m_lightDataSet.m_position;
			break;
		case Systems::Changes::Spacial::Rotation:
			return m_lightDataSet.m_direction;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Getters
	const inline bool active() const { return m_active; }
	const inline SpotLightDataSet &getLightDataSet() const { return m_lightDataSet; }

	// Setters
	inline void setActive(bool p_flag) { m_active = p_flag; }

private:
	SpotLightDataSet m_lightDataSet;

	Math::Vec3f m_offsetPosition,
				m_offsetRotation;

	bool m_active;
};