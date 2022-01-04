#pragma once

#include "BaseScriptObject.h"

// Provides bare object rotation functionality for debugging purposes (like making a spotlight rotate)
class DebugRotateScript : public BaseScriptObject
{
	friend class ScriptScene;
public:
	DebugRotateScript(SystemScene *p_systemScene, std::string p_name)
		: BaseScriptObject(p_systemScene, p_name, Properties::DebugRotateScript)
	{
		m_speed = 10.0f;
		m_axis = glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f));
		m_rotation = glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f));
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
		propertySet.addProperty(Properties::LocalRotationQuaternion, m_rotation);
		propertySet.addProperty(Properties::Speed, m_speed);

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		//m_rotation.rotate(m_speed * p_deltaTime, m_axis);
		//m_rotation.normalize();
		
		//postChanges(Systems::Changes::Spatial::Rotation);
	}

	const virtual glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		/*switch (p_changedBits)
		{
		case Systems::Changes::Spatial::Rotation:
			return m_rotation;
			break;
		}*/

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Setters
	inline void setMovementSpeed(float p_speed) { m_speed = p_speed; }
	inline void setRotation(glm::vec3 p_rotation) { m_rotation = p_rotation; }
	inline void setRotationAxis(glm::vec3 p_axis) { m_axis = p_axis; }

protected:
	float m_speed;

	glm::mat4 m_modelMatrix;
	glm::vec3 m_rotation,
				m_axis;
};