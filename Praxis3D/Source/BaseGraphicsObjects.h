#pragma once

#include <atomic>

#include "GraphicsDataSets.h"
#include "Loaders.h"
#include "Math.h"
#include "NullSystemObjects.h"
#include "System.h"

class ModelObject;
class EnvironmentMapObject;
class ShaderModelGraphicsObject;

class BaseGraphicsObject : public SystemObject
{
public:
	BaseGraphicsObject(SystemScene *p_systemScene, const std::string &p_name, Properties::PropertyID p_objectType)
		: SystemObject(p_systemScene, p_name, p_objectType), m_needsUpdate(true), m_affectedByLighting(true), m_loadedToVideoMemory(false), m_loadedToMemory(false), m_isActive(false) { }
	virtual ~BaseGraphicsObject() { }
		
	BitMask getSystemType() { return Systems::Graphics; }

	virtual BitMask getDesiredSystemChanges()	{ return Systems::Changes::Spatial::All;	}
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::None;			}

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		/*if(p_changeType & Systems::Changes::Spatial::WorldPosition)
		{
			m_baseObjectData.m_position = 
				p_subject->getVec3(this, Systems::Changes::Spatial::WorldPosition) + m_baseObjectData.m_offsetPosition;
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spatial::WorldRotation)
		{
			m_baseObjectData.m_rotation = 
				p_subject->getVec3(this, Systems::Changes::Spatial::WorldRotation) + m_baseObjectData.m_offsetRotation;
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spatial::WorldScale)
		{
			m_baseObjectData.m_scale = p_subject->getVec3(this, Systems::Changes::Spatial::WorldScale);
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Spatial::WorldTransform)
		{
			m_baseObjectData.m_modelMat = p_subject->getMat4(this, Systems::Changes::Spatial::WorldTransform);
			m_needsUpdate = true;
		}

		if(p_changeType & Systems::Changes::Graphics::Lighting)
		{
			m_affectedByLighting = p_subject->getBool(this, Systems::Changes::Graphics::Lighting);
		}*/
	}

	// Has the object been already loaded to memory (RAM)?
	const inline bool isLoadedToMemory() const { return m_loadedToMemory; }

	// Has the object been already loaded to video memory (GPU VRAM)
	const inline bool isLoadedToVideoMemory() const { return m_loadedToVideoMemory; }

	// Is the object active (i.e. should be drawned, updated, etc...)
	const inline bool isObjectActive() const { return m_isActive; }

	// Getters
	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		/*switch(p_changedBits)
		{
		case Systems::Changes::Spatial::Position:
			return m_baseObjectData.m_position;
			break;
		case Systems::Changes::Spatial::Rotation:
			return m_baseObjectData.m_rotation;
			break;
		case Systems::Changes::Spatial::Scale:
			return m_baseObjectData.m_scale;
			break;
		}*/

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}
	const virtual bool getBool(const Observer *p_observer, BitMask p_changedBits) const
	{
		/*switch(p_changedBits)
		{
		case Systems::Changes::Graphics::Lighting:
			return m_affectedByLighting;
			break;
		}*/

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
	inline void setAffectedByLighting(const bool p_flag)		{ m_affectedByLighting = p_flag;					}
	inline void setAlphaThreshold(const float p_value)			{ m_baseObjectData.m_alphaThreshold = p_value;		}
	inline void setEmissiveThreshold(const float p_value)		{ m_baseObjectData.m_emissiveThreshold = p_value;	}
	inline void setHeightScale(const float p_value)				{ m_baseObjectData.m_heightScale = p_value;			}
	inline void setLoadedToMemory(const bool p_loadedToMemory)	{ m_loadedToMemory = p_loadedToMemory;				}
	inline void setLoadedToVideoMemory(const bool p_loaded)		{ m_loadedToVideoMemory = p_loaded;					}
	inline void setObjectActive(const bool p_objectIsActive)	{ m_isActive = p_objectIsActive;					}
	inline void setTextureTilingFactor(const float p_value)		{ m_baseObjectData.m_textureTilingFactor = p_value; }

protected:
	// A flag telling if this object should be rendered during geometry pass or as a post-process (i.e. after lighting)
	bool m_affectedByLighting;

	// Does the object need to be updated after any of its data has been changed
	bool m_needsUpdate;

	// Is the object active (i.e. should be drawn, updated, etc...)
	bool m_isActive;

	// Is the object loaded to GPU
	bool m_loadedToVideoMemory;

	// Atomic, so it can be changed from different threads (loading to memory is multi-threaded)
	std::atomic<bool> m_loadedToMemory;

	// Spatial and misc data of an object
	GraphicsData m_baseObjectData;
};

// Used to hold objects that need to be loaded or are already being loaded, in a list
// Holds any of the graphics object (in a union) so the data of an object can be access and be loaded
class LoadableGraphicsObject
{
	friend class RendererScene;
public:
	LoadableGraphicsObject(ModelObject *p_modelObject, size_t p_index);
	LoadableGraphicsObject(EnvironmentMapObject *p_envMapStatic, size_t p_index);
	LoadableGraphicsObject(ShaderModelGraphicsObject *p_shaderModelObject, size_t p_index);

	// Load object data to memory (RAM)
	void LoadToMemory();

	// Has the object been already loaded to memory (RAM)?
	const inline bool isLoadedToMemory() const { return m_baseGraphicsObject->isLoadedToMemory(); }

	// Has the object been already loaded to video memory (GPU VRAM)?
	const inline bool isLoadedToVideoMemory() const { return m_baseGraphicsObject->isLoadedToVideoMemory(); }

	// Should the object be activated after loading
	const inline bool isActivatedAfterLoading() const { return m_activateAfterLoading; }

	// Is the object active (i.e. should be drawn, updated, etc...)
	const inline bool isObjectActive() const { return m_baseGraphicsObject->isObjectActive(); }

	// Getters
	const inline size_t getIndex() const					{ return m_index;		}
	const inline size_t getObjectID() const					{ return m_objectID;	}
	const inline std::string &getName() const				{ return m_name;		}
	const inline LoadableObjectType getObjectType() const	{ return m_objectType;	}

	// Setters
	inline void setActivateAfterLoading(const bool p_activateAfterLoading)	{ m_activateAfterLoading = p_activateAfterLoading;			}
	inline void setLoadedToVideoMemory(const bool p_loaded)					{ m_baseGraphicsObject->setLoadedToVideoMemory(p_loaded);	}
	inline void setObjectActive(const bool p_objectIsActive)				{ m_baseGraphicsObject->setObjectActive(p_objectIsActive);	}

	// Comparator operators; uses object ID to determine if the object is the same
	bool operator==(const SystemObject &p_systemObject) const { return m_objectID == p_systemObject.getObjectID() ? true : false;	}
	bool operator==(const SystemObject *p_systemObject) const { return m_objectID == p_systemObject->getObjectID() ? true : false;	}
private:
	union ObjectData
	{
		ModelObject *m_modelObject;
		EnvironmentMapObject *m_envMapStatic;
		ShaderModelGraphicsObject *m_shaderModelObject;
	};

	ObjectData m_objectData;
	LoadableObjectType m_objectType;

	// Holds the object's name so it doesn't have to be retrieved every time
	std::string m_name;

	// This should always be true, unless object was set to be removed before loading completed
	bool m_activateAfterLoading;

	// Unique index of the object in corresponding pool (used for fast access)
	size_t m_index;

	// Holds the base class that the objects are derived from, so it's faster to access the base data
	BaseGraphicsObject *m_baseGraphicsObject;

	// Pointer to the base class of an object, so some functionality can be easily accessed
	SystemObject *m_baseSystemObject;

	// A copy of system object ID 
	size_t m_objectID;
};