#pragma once

#include "Loaders.h"
#include "System.h"
#include "TextureLoader.h"

class EnvironmentMapStatic : public SystemObject
{
public:
	EnvironmentMapStatic(SystemScene *p_systemScene, const std::string &p_name, TextureLoaderCubemap::TextureCubemapHandle &p_cubemap, Properties::PropertyID p_objectType = Properties::EnvironmentMapStatic)
		: SystemObject(p_systemScene, p_name, p_objectType), m_cubemap(p_cubemap) { }
	virtual ~EnvironmentMapStatic() 
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
		ErrorCode returnError;

		returnError = m_cubemap.loadToVideoMemory();

		//if(returnError == ErrorCode::Success)
		//	m_cubemapHandle = m_cubemap.getHandle();

		return returnError;
	}

	// Exports all the data of the object as a PropertySet
	PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::EnvironmentMapStatic);
		propertySet.addProperty(Properties::Position, m_position);

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
	inline void setPosition(const Math::Vec3f &p_position) { m_position = p_position; }

	// Getters
	const inline unsigned int getCubemapHandle() const { return m_cubemap.getHandle(); }

	BitMask getSystemType() { return Systems::Graphics; }

	virtual BitMask getDesiredSystemChanges() { return Systems::Changes::Spacial::All; }
	virtual BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	// Processes any spacial changes
	virtual void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(p_changeType & Systems::Changes::Spacial::Position)
		{
		}

		if(p_changeType & Systems::Changes::Spacial::Rotation)
		{
		}

		if(p_changeType & Systems::Changes::Spacial::Scale)
		{
		}
	}

	const virtual Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		switch(p_changedBits)
		{
		case Systems::Changes::Spacial::Position:
			return m_position;
			break;
		}

		return ObservedSubject::getVec3(p_observer, p_changedBits);
	}

	const virtual bool getBool(const Observer *p_observer, BitMask p_changedBits) const
	{
		//switch(p_changedBits)
		//{
		//}

		return ObservedSubject::getBool(p_observer, p_changedBits);
	}

protected:
	unsigned int m_cubemapHandle;

	Math::Vec3f m_position;

	TextureLoaderCubemap::TextureCubemapHandle m_cubemap;
};