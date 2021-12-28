#pragma once

#include "BaseScriptObject.h"
#include "Math.h"
#include "WindowLocator.h"

class Camera : public BaseScriptObject
{
	friend class ScriptingScene;
public:
	Camera(SystemScene *p_systemScene, std::string p_name, Properties::PropertyID p_objectType)
		: BaseScriptObject(p_systemScene, p_name, p_objectType)
	{
		m_speed = 0.0f;
		m_fasterSpeed = 0.0f;
		m_verticalAngle = 0.0f;
		m_horizontalAngle = 0.0f;
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		/*switch (p_changedBits)
		{
		case Systems::Changes::Spatial::Position:
			return m_positionVec;
			break;
		case Systems::Changes::Spatial::Rotation:
			return m_targetVec;
			break;
		}*/

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	const virtual Math::Mat4f &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{
		/*switch (p_changedBits)
		{
		case Systems::Changes::Spatial::ModelMatrix:
			return m_modelMatrix;
			break;
		}*/

		return ObservedSubject::getMat4(p_observer, p_changedBits);
	}

	// Setters
	const inline void setPosition(const Math::Vec3f &p_position)	{ m_positionVec = p_position; }
	const inline void setFasterSpeed(const float p_speed)			{ m_fasterSpeed = p_speed; }
	const inline void setSpeed(const float p_speed)					{ m_speed = p_speed; }
	const inline void setAngles(const Math::Vec2f &p_angles) 
	{ 
		m_horizontalAngle = p_angles.x;
		m_verticalAngle = p_angles.y;
	}

protected:
	float	m_speed,
			m_fasterSpeed,
			m_verticalAngle,
			m_horizontalAngle;

	Math::Mat4f m_modelMatrix;
	Math::Vec3f m_positionVec,
				m_targetVec,
				m_upVector,
				m_horizontalVec;
};

class FreeCamera : public Camera
{
public:
	FreeCamera(SystemScene *p_systemScene, std::string p_name) : Camera(p_systemScene, p_name, Properties::FreeCamera)
	{
		m_enableLowerLimit = false;
		m_enableUpperLimit = false;

		m_lowerLimit = 0.0f;
		m_upperLimit = 0.0f;
	}

	virtual ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		// Nothing to load
	}

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::FreeCamera);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::LocalPosition, m_positionVec);
		propertySet.addProperty(Properties::Angle, Math::Vec2f(m_horizontalAngle, m_verticalAngle));
		propertySet.addProperty(Properties::Speed, m_speed);
		propertySet.addProperty(Properties::SprintSpeed, m_fasterSpeed);

		if(m_enableLowerLimit)
			propertySet.addProperty(Properties::LowerLimit, m_lowerLimit);
		if(m_enableUpperLimit)
			propertySet.addProperty(Properties::UpperLimit, m_upperLimit);


		// Add root key-binding property set
		auto &keyBinds = propertySet.addPropertySet(Properties::Keybindings);

		// Add individual key-bindings
		keyBinds.addProperty(Properties::ForwardKey, (int)m_forwardKey.getFirstBinding());
		keyBinds.addProperty(Properties::BackwardKey, (int)m_backwardKey.getFirstBinding());
		keyBinds.addProperty(Properties::LeftStrafeKey, (int)m_strafeLeftKey.getFirstBinding());
		keyBinds.addProperty(Properties::RightStrafeKey, (int)m_strafeRightKey.getFirstBinding());
		keyBinds.addProperty(Properties::SprintKey, (int)m_sprintKey.getFirstBinding());

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		// Only move the camera if the mouse is captured by the window
		if(Config::windowVar().mouse_captured)
		{
			const auto &mouseInfo = WindowLocator::get().getMouseInfo();

			m_horizontalAngle -= Config::inputVar().mouse_jaw * mouseInfo.m_movementX * (Config::inputVar().mouse_sensitivity * 0.01f);
			m_verticalAngle -= Config::inputVar().mouse_pitch * mouseInfo.m_movementY * (Config::inputVar().mouse_sensitivity * 0.01f);

			m_verticalAngle = Math::clamp(m_verticalAngle, -Config::inputVar().mouse_pitch_clip, Config::inputVar().mouse_pitch_clip);
		}

		// Calculate camera's rotation
		m_targetVec.target(m_verticalAngle, m_horizontalAngle);
		m_horizontalVec.horizontal(m_horizontalAngle);

		// Set speed for movement
		float speed = m_speed;

		// If sprint key is pressed, increase the movement speed
		if(m_sprintKey.isActivated())
			speed = m_fasterSpeed;

		// Check the status of all the movement keys
		if(m_forwardKey.isActivated())
			m_positionVec += m_targetVec * speed * p_deltaTime;
		if(m_backwardKey.isActivated())
			m_positionVec -= m_targetVec * speed * p_deltaTime;
		if(m_strafeLeftKey.isActivated())
			m_positionVec -= m_horizontalVec * speed * p_deltaTime;
		if(m_strafeRightKey.isActivated())
			m_positionVec += m_horizontalVec * speed * p_deltaTime;

		if(m_enableLowerLimit && m_positionVec.y < m_lowerLimit)
			m_positionVec.y = m_lowerLimit;

		if(m_enableUpperLimit && m_positionVec.y > m_upperLimit)
			m_positionVec.y = m_upperLimit;

		// Calculate camera's position based on the pressed movement keys
		m_upVector = Math::cross(m_horizontalVec, m_targetVec);
		m_modelMatrix.initCamera(m_positionVec, m_targetVec + m_positionVec, m_upVector);

		/*
		float cos_z = cos(m_verticalAngle);
		float sin_z = sin(m_verticalAngle);
		float cos_a = cos(m_horizontalAngle);
		float sin_a = sin(m_horizontalAngle);
		float ux[3] = { -sin_a, cos_a, 0.0 };
		float uy[3] = { -cos_z * cos_a, -cos_z * sin_a, sin_z };
		float uz[3] = { sin_z * cos_a, sin_z * sin_a, cos_z };
		float l = 9000.0 / 1000.0;

		// Transform matrix from camera frame to world space (i.e. the inverse of a
		// GL_MODELVIEW matrix).
		m_modelMatrix = {
			ux[0], uy[0], uz[0], uz[0] * l,
			ux[1], uy[1], uz[1], uz[1] * l,
			ux[2], uy[2], uz[2], uz[2] * l,
			0.0, 0.0, 0.0, 1.0
		};*/

		// Set the target vector variable, so it can be retrieved later by listeners
		m_targetVec = Math::Vec3f(0.0f);
		m_targetVec.y = m_verticalAngle;
		m_targetVec.z = m_horizontalAngle;
		
		// Notify listeners
		postChanges(Systems::Changes::Spatial::WorldPosition | 
					Systems::Changes::Spatial::WorldRotation | 
					Systems::Changes::Spatial::WorldTransform);
	}
	
	// Setters for the movement keys:
	inline void setForwardKey(Scancode p_key)
	{
		m_forwardKey.unbindAll();
		m_forwardKey.bind(p_key);
	}
	inline void setForwardKey(std::string &p_string)
	{
		m_forwardKey.unbindAll();
		m_forwardKey.bind(p_string);
	}

	inline void setBackwardKey(Scancode p_key)
	{
		m_backwardKey.unbindAll();
		m_backwardKey.bind(p_key);
	}
	inline void setBackwardKey(std::string &p_string)
	{
		m_backwardKey.unbindAll();
		m_backwardKey.bind(p_string);
	}

	inline void setStrafeLeftKey(Scancode p_key)
	{
		m_strafeLeftKey.unbindAll();
		m_strafeLeftKey.bind(p_key);
	}
	inline void setStrafeLeftKey(std::string &p_string)
	{
		m_strafeLeftKey.unbindAll();
		m_strafeLeftKey.bind(p_string);
	}

	inline void setStrafeRightKey(Scancode p_key)
	{
		m_strafeRightKey.unbindAll();
		m_strafeRightKey.bind(p_key);
	}
	inline void setStrafeRightKey(std::string &p_string)
	{
		m_strafeRightKey.unbindAll();
		m_strafeRightKey.bind(p_string);
	}

	inline void setSprintKey(Scancode p_key)
	{
		m_sprintKey.unbindAll();
		m_sprintKey.bind(p_key);
	}
	inline void setSprintKey(std::string &p_string)
	{
		m_sprintKey.unbindAll();
		m_sprintKey.bind(p_string);
	}

	inline void setLowerLimit(const float p_limit)
	{
		m_enableLowerLimit = true;
		m_lowerLimit = p_limit;
	}
	inline void setUpperLimit(const float p_limit)
	{
		m_enableUpperLimit = true;
		m_upperLimit = p_limit;
	}

private:
	KeyCommand	m_forwardKey,
				m_backwardKey,
				m_strafeLeftKey,
				m_strafeRightKey,
				m_sprintKey;
	
	float	m_lowerLimit,
			m_upperLimit;

	bool	m_enableLowerLimit,
			m_enableUpperLimit;
};