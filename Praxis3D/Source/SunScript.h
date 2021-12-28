#pragma once

#include "BaseScriptObject.h"

// Provides bare object rotation functionality for debugging purposes (like making a spotlight rotate)
class SunScript : public BaseScriptObject
{
	friend class ScriptingScene;
public:
	SunScript(SystemScene *p_systemScene, std::string p_name)
		: BaseScriptObject(p_systemScene, p_name, Properties::SunScript)
	{
		m_azimuthAngle = 1.0f;
		m_zenithAngle = 1.0f;
	}
	~SunScript()
	{
	}


	virtual ErrorCode init()
	{
		return ErrorCode::Success;
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
		propertySet.addProperty(Properties::Type, Properties::SunScript);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Azimuth, m_azimuthAngle);
		propertySet.addProperty(Properties::Zenith, m_zenithAngle);

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		m_rotation = Math::Vec3f(
			cos(m_azimuthAngle) * sin(m_zenithAngle),
			cos(m_zenithAngle),
			sin(m_azimuthAngle) * sin(m_zenithAngle));

		postChanges(Systems::Changes::Spatial::LocalRotation);
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalRotation:
			return m_rotation;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Setters
	inline void setAzimuthAngle(float p_azimuthAngle)
	{
		m_azimuthAngle = p_azimuthAngle;
	}
	inline void setZenithAngle(float p_zenithAngle)
	{
		m_zenithAngle = p_zenithAngle;
	}

protected:
	float	m_azimuthAngle,
			m_zenithAngle;

	Math::Vec3f m_rotation;
};