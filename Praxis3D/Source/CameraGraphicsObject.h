#pragma once

#include <functional>

#include "BaseGraphicsObjects.h"
#include "Loaders.h"

class RendererScene;

class CameraObject : public BaseGraphicsObject
{
	//friend class RendererScene;
	friend class RendererState;
	//friend class BaseGraphicsObject;
public:
	CameraObject(SystemScene *p_systemScene, std::string p_name)
		: BaseGraphicsObject(p_systemScene, p_name, Properties::Camera)
	{
		// Assign camera position
		m_baseObjectData.m_position = Math::Vec3f(0.0f, 0.0f, 0.0f);

		// Assign the target vector
		m_baseObjectData.m_rotation = Math::Vec3f(1.0f, 1.0f, 1.0f);

		// Assign the up vector
		m_baseObjectData.m_scale = Math::Vec3f(0.0f, 1.0f, 0.0f);
	}
	virtual ~CameraObject() { }

	virtual ErrorCode init()
	{
		return ErrorCode::Success;
	}

	virtual void loadToMemory() { }

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::Camera);
		propertySet.addProperty(Properties::Name, m_name);

		return propertySet;
	}

	inline void update(const float p_deltaTime)
	{
		// NOTE: rotation vector is used as a target vector; scale vector is used as up-vector (to save space)
		//m_baseObjectData.m_modelMat.initCamera(m_baseObjectData.m_position, m_baseObjectData.m_rotation, m_baseObjectData.m_scale);
		//m_needsUpdate = false;
	}

	// Interested in Model Matrix, that would act as view matrix (so camera could be attached to any object)
	//virtual BitMask getDesiredSystemChanges() { return Systems::Changes::Spacial::ModelMatrix; }
	
protected:
	Math::Vec2f m_cameraAngle;
};