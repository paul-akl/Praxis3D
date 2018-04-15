#pragma once

#include "BaseScriptObject.h"

// Provides bare object rotation functionality for debugging purposes (like making a spotlight rotate)
class DebugRotateScript : public BaseScriptObject
{
	friend class ScriptingScene;
public:
	DebugRotateScript(SystemScene *p_systemScene, std::string p_name)
		: BaseScriptObject(p_systemScene, p_name, Properties::DebugRotateScript)
	{
		m_speed = 10.0f;
		m_axis = Math::normalize(Math::Vec3f(1.0f, 0.0f, 1.0f));
		m_rotation = Math::normalize(Math::Vec3f(1.0f, 0.0f, 1.0f));
	}
	~DebugRotateScript() { }


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
		propertySet.addProperty(Properties::Type, Properties::DebugRotateScript);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Axis, m_axis);
		propertySet.addProperty(Properties::Rotation, m_rotation);
		propertySet.addProperty(Properties::Speed, m_speed);

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		m_rotation.rotate(m_speed * p_deltaTime, m_axis);
		m_rotation.normalize();
		
		postChanges(Systems::Changes::Spacial::Rotation);
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch (p_changedBits)
		{
		case Systems::Changes::Spacial::Rotation:
			return m_rotation;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Setters
	inline void setMovementSpeed(float p_speed) { m_speed = p_speed; }
	inline void setRotation(Math::Vec3f p_rotation) { m_rotation = p_rotation; }
	inline void setRotationAxis(Math::Vec3f p_axis) { m_axis = p_axis; }

protected:
	float m_speed;

	Math::Mat4f m_modelMatrix;
	Math::Vec3f m_rotation,
				m_axis;
};