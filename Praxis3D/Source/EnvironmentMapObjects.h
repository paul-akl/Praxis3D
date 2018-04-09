#pragma once

#include <functional>

#include "BaseGraphicsObjects.h"
#include "Loaders.h"
#include "Renderer.h"

class EnvironmentMapObject : public BaseGraphicsObject
{
public:
//	EnvironmentMapObject(SystemScene *p_systemScene, TextureLoaderCubemap::TextureCubemapHandle &p_cubemap)
//		: BaseGraphicsObject(p_systemScene, "Null", Properties::EnvironmentMapObject), m_cubemap(p_cubemap) { }
	EnvironmentMapObject(SystemScene *p_systemScene, const std::string &p_name, TextureLoaderCubemap::TextureCubemapHandle &p_cubemap, Properties::PropertyID p_objectType = Properties::EnvironmentMapObject)
		: BaseGraphicsObject(p_systemScene, p_name, p_objectType), m_cubemap(p_cubemap) 
	{ 
		m_cubemapHandle = 0;
	}
	virtual ~EnvironmentMapObject() 
	{
		m_cubemapHandle = 0;
	}

	ErrorCode init()
	{
		return ErrorCode::Success;
	}

	// Loads cubemap from files to memory
	void loadToMemory()
	{
		m_cubemap.loadToMemory();
	}

	// Loads cubemap to video memory; should only be called by renderer thread
	ErrorCode loadToVideoMemory()
	{
		ErrorCode returnError = ErrorCode::Success;
		
		return returnError;
	}

	// Exports all the data of the object as a PropertySet
	PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::EnvironmentMapObject);
		propertySet.addProperty(Properties::Position, m_baseObjectData.m_position);

		// Iterate over each cubemap face and add its material filename
		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		{
			auto &materialEntry = propertySet.addPropertySet(Properties::ArrayEntry);
			materialEntry.addProperty(Properties::Filename, m_cubemap.getFaceFilename(face));
		}

		return propertySet;
	}

	void update(const float p_deltaTime)
	{

	}

	// Setters
	inline void setCubemap(const TextureLoaderCubemap::TextureCubemapHandle &p_cubemap) { m_cubemap = p_cubemap; }
	inline void setPosition(const Math::Vec3f &p_position) { m_baseObjectData.m_position = p_position; }

	// Getters
	const inline unsigned int getCubemapHandle() const { return m_cubemap.getHandle(); }

	BitMask getSystemType() { return Systems::Graphics; }

	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::Spacial::All; }
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::None; }
	
protected:
	unsigned int m_cubemapHandle;

	TextureLoaderCubemap::TextureCubemapHandle m_cubemap;
};