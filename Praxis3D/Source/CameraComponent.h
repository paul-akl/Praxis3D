#pragma once

#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"

class CameraComponent : public SystemObject, public LoadableGraphicsObject
{
	friend class RendererScene;
public:
	struct CameraComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		CameraComponentConstructionInfo()
		{
			m_cameraID = 0;
			m_fov = Config::graphicsVar().fov;
			m_zFar = Config::graphicsVar().z_far;
			m_zNear = Config::graphicsVar().z_near;
		}

		int m_cameraID;

		float m_fov;
		float m_zFar;
		float m_zNear;
	};

	CameraComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::CameraComponent, p_entityID)
	{
		m_cameraID = 0;
		m_fov = Config::graphicsVar().fov;
		m_zFar = Config::graphicsVar().z_far;
		m_zNear = Config::graphicsVar().z_near;
	}
	~CameraComponent() { }

	ErrorCode init() final override
	{
		// Mark the object as loaded, because there is nothing to be specifically loaded, at least for now
		//setLoadedToMemory(true);
		//setLoadedToVideoMemory(true);

		return ErrorCode::Success;
	}

	void update(const float p_deltaTime)
	{

	}

	// System type is Graphics
	BitMask getSystemType() final override { return Systems::Graphics; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::Graphics::Camera; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::None; }

	// Getters
	const inline int getCameraID() const { return m_cameraID; }
	const inline float getCameraFOV() const { return m_fov; }
	const inline float getCameraFarClip() const { return m_zFar; }
	const inline float getCameraNearClip() const { return m_zNear; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) 
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
		{
			// Get the active flag from the subject and set the active flag accordingly
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Graphics::CameraID))
		{
			m_cameraID = p_subject->getInt(this, Systems::Changes::Graphics::CameraID);
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Graphics::FOV))
		{
			m_fov = p_subject->getFloat(this, Systems::Changes::Graphics::FOV);
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Graphics::ZFar))
		{
			m_zFar = p_subject->getFloat(this, Systems::Changes::Graphics::ZFar);
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Graphics::ZNear))
		{
			m_zNear = p_subject->getFloat(this, Systems::Changes::Graphics::ZNear);
		}
	}

private:
	int m_cameraID;

	// Projection data
	float m_fov;
	float m_zFar;
	float m_zNear;
};