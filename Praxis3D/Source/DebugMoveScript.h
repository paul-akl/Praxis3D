#pragma once

#include "BaseScriptObject.h"

// Provides bare object movement functionality for debugging purposes (like making a light move around the scene)
class DebugMoveScript : public BaseScriptObject
{
	friend class ScriptingScene;
public:
	DebugMoveScript(SystemScene *p_systemScene, std::string p_name)
		: BaseScriptObject(p_systemScene, p_name, Properties::DebugMoveScript)
	{
		m_speed = 10.0f;
		m_radius = 0.0f;
		m_targetVec = Math::normalize(Math::Vec3f(1.0f, 0.0f, 1.0f));
		m_rotation = Math::Vec3f(0.0f, 1.0f, 0.0f);
	}
	~DebugMoveScript() { }


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
		propertySet.addProperty(Properties::Type, Properties::DebugMoveScript);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::LocalPosition, m_originPosVec);
		propertySet.addProperty(Properties::Radius, m_radius);
		propertySet.addProperty(Properties::LocalRotation, m_rotation);
		propertySet.addProperty(Properties::Speed, m_speed);

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		m_targetVec.rotate(m_speed * p_deltaTime, m_rotation);

		m_positionVec = m_targetVec * m_radius + m_originPosVec;

		postChanges(Systems::Changes::Spatial::LocalPosition);
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		/*switch(p_changedBits)
		{
		case Systems::Changes::Spatial::LocalPosition:
			return m_positionVec;
			break;
		}*/

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	// Setters
	inline void setMovementSpeed(float p_speed)				{ m_speed = p_speed;			}
	inline void setRadius(float p_radius)					{ m_radius = p_radius;			}
	inline void setOriginPosition(Math::Vec3f p_position)	{ m_originPosVec = p_position;	}
	inline void setMovementAxis(Math::Vec3f p_axis)			{ m_rotation = p_axis;			}

protected:
	float	m_speed,
			m_radius;

	Math::Mat4f m_modelMatrix;
	Math::Vec3f m_positionVec,
				m_rotation,
				m_targetVec,
				m_originPosVec;
};