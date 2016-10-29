#pragma once

#include <atomic>

#include "GraphicsDataSets.h"
#include "Loaders.h"
#include "Math.h"
#include "NullSystemObjects.h"
#include "System.h"

class BaseGraphicsObject : public SystemObject
{
public:
	BaseGraphicsObject(SystemScene *p_systemScene, const std::string &p_name, Properties::PropertyID p_objectType)
		: SystemObject(p_systemScene, p_name, p_objectType), m_needsUpdate(true), m_affectedByLighting(true) { }
	virtual ~BaseGraphicsObject() { }
		
	BitMask getSystemType() { return Systems::Graphics; }

	virtual BitMask getDesiredSystemChanges()	{ return Systems::Changes::Spacial::All;	}
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::None;			}

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spacial::Position)
		{
			m_baseObjectData.m_position = 
				p_subject->getVec3(this, Systems::Changes::Spacial::Position) + m_baseObjectData.m_offsetPosition;
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spacial::Rotation)
		{
			m_baseObjectData.m_rotation = 
				p_subject->getVec3(this, Systems::Changes::Spacial::Rotation) + m_baseObjectData.m_offsetRotation;
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spacial::Scale)
		{
			m_baseObjectData.m_scale = p_subject->getVec3(this, Systems::Changes::Spacial::Scale);
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spacial::ModelMatrix)
		{
			m_baseObjectData.m_modelMat = p_subject->getMat4(this, Systems::Changes::Spacial::ModelMatrix);
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Graphics::Lighting)
		{
			m_affectedByLighting = p_subject->getBool(this, Systems::Changes::Graphics::Lighting);
		}
	}
	
	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spacial::Position:
			return m_baseObjectData.m_position;
			break;
		case Systems::Changes::Spacial::Rotation:
			return m_baseObjectData.m_rotation;
			break;
		case Systems::Changes::Spacial::Scale:
			return m_baseObjectData.m_scale;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	const virtual bool getBool(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Graphics::Lighting:
			return m_affectedByLighting;
			break;
		}

		return ObservedSubject::getBool(p_observer, p_changedBits);
	}
	
	const inline GraphicsData &getBaseObjectData() const { return m_baseObjectData; }

	// Setters for spacial data
	inline void setScale(const Math::Vec3f &p_scale)				{ m_baseObjectData.m_scale = p_scale;				}
	inline void setPosition(const Math::Vec3f &p_position)			{ m_baseObjectData.m_position = p_position;			}
	inline void setRotation(const Math::Vec3f &p_rotation)			{ m_baseObjectData.m_rotation = p_rotation;			}
	inline void setOffsetPosition(const Math::Vec3f &p_position)	{ m_baseObjectData.m_offsetPosition = p_position;	}
	inline void setOffsetRotation(const Math::Vec3f &p_rotation)	{ m_baseObjectData.m_offsetRotation = p_rotation;	}

	// Setters for misc data
	inline void setAlphaThreshold(const float p_value)		{ m_baseObjectData.m_alphaThreshold = p_value;		}
	inline void setEmissiveThreshold(const float p_value)	{ m_baseObjectData.m_emissiveThreshold = p_value;	}
	inline void setHeightScale(const float p_value)			{ m_baseObjectData.m_heightScale = p_value;			}
	inline void setTextureTilingFactor(const float p_value) { m_baseObjectData.m_textureTilingFactor = p_value; }
	inline void setAffectedByLighting(const bool p_flag)	{ m_affectedByLighting = p_flag;					}

protected:
	// A flag telling if this object should be rendered during geometry pass or as a post-process (i.e. after lighting)
	bool m_affectedByLighting;
	bool m_needsUpdate;

	GraphicsData m_baseObjectData;
};

class LoadableGraphicsObject : public BaseGraphicsObject
{
public:
	LoadableGraphicsObject(SystemScene *p_systemScene, const std::string &p_name, Properties::PropertyID p_objectType,
						   ModelLoader::ModelHandle p_model, ShaderLoader::ShaderProgram *p_shader)
	: BaseGraphicsObject(p_systemScene, p_name, p_objectType), m_rendererData(p_model, p_shader, m_baseObjectData)
	{
		m_active = false;
		m_loadedToMemory = false;
	}

	virtual void loadToMemory() { m_loadedToMemory = true; }

	virtual ErrorCode loadToVideoMemory() { return ErrorCode::Success; }

	const inline bool loadedToMemory() const { return m_loadedToMemory; }
	const inline bool active() const { return m_active; }

	inline void setActive(bool p_flag) { m_active = p_flag; }
	
	inline RenderableObjectData &getRenderableObjectData() { return m_rendererData; }

protected:
	inline void setLoadedToMemory(bool p_flag) { m_loadedToMemory = p_flag; }

	RenderableObjectData m_rendererData;

	std::atomic<bool> m_loadedToMemory;
	bool m_active;
};
