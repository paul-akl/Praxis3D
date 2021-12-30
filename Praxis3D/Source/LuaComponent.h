#pragma once

#define SOL_LUAJIT 1

#include "ErrorHandlerLocator.h"
#include "Filesystem.h"
#include "InheritanceObjects.h"
#include "LuaScript.h"
#include "System.h"

class LuaComponent : public SystemObject, public SpatialDataManagerObject, public LoadableGraphicsObject
{
	friend class ScriptScene;
public:
	LuaComponent(SystemScene *p_systemScene, std::string p_name, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::LuaComponent), m_luaSpatialData(*this), m_luaScript(m_luaSpatialData)
	{
		m_luaScriptLoaded = false;
		m_luaSpatialData.setTrackLocalChanges(true);
	}
	~LuaComponent() { }

	ErrorCode init() final override
	{
		// Mark the object as loaded, because there is nothing to be specifically loaded, at least for now
		//setLoadedToMemory(true);
		//setLoadedToVideoMemory(true);

		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		auto luaError = m_luaScript.init();

		if(luaError != ErrorCode::Success)
			ErrHandlerLoc().get().log(luaError, ErrorSource::Source_LuaComponent, m_name);
		else
			m_luaScriptLoaded = true;

		setActive(true);
	}

	void update(const float p_deltaTime)
	{
		// Get the current spatial data
		m_luaSpatialData.setSpatialData(*m_spatialData);

		if(m_luaScriptLoaded)
			m_luaScript.update(p_deltaTime);

		m_luaSpatialData.update();

		auto changes = m_luaSpatialData.getCurrentChangesAndReset();

		//if(CheckBitmask(changes, Systems::Changes::Spatial::WorldPosition))
		//	std::cout << "Position changes occurred" << std::endl;

		//if(CheckBitmask(changes, Systems::Changes::Spatial::WorldRotation))
		//	std::cout << "Rotation changes occurred" << std::endl;

		postChanges(changes);

		std::cout << m_spatialData->getWorldSpaceData().m_spatialData.m_position.x << " : " << m_spatialData->getWorldSpaceData().m_spatialData.m_position.y << " : " << m_spatialData->getWorldSpaceData().m_spatialData.m_position.z << std::endl;
	}

	ErrorCode importObject(const PropertySet &p_properties) final override
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if PropertySet isn't empty and the component hasn't been loaded already
		if(p_properties && !isLoadedToMemory())
		{
			if(p_properties.getPropertyID() == Properties::Lua)
			{
				auto const &luaFilenameProperty = p_properties.getPropertyByID(Properties::Filename);

				if(luaFilenameProperty)
				{
					std::string luaFilename = luaFilenameProperty.getString();
					if(!luaFilename.empty())
					{
						luaFilename = Config::filepathVar().script_path + luaFilename;
						if(Filesystem::exists(luaFilename))
						{
							m_luaScript.setScriptFilename(luaFilename);

							importError = ErrorCode::Success;
							ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LuaComponent, m_name + " - Script loaded");
						}
						else
						{
							importError = ErrorCode::File_not_found;
							ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LuaComponent, m_name + " - File \'" + luaFilename + "\' not found");
						}
					}
					else
					{
						importError = ErrorCode::Property_no_filename;
						ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LuaComponent, m_name + " - Property \'" + GetString(Properties::Filename) + "\' is empty");
					}
				}
				else
				{
					importError = ErrorCode::Property_no_filename;
					ErrHandlerLoc().get().log(ErrorType::Warning, ErrorSource::Source_LuaComponent, m_name + " - Missing \'" + GetString(Properties::Filename) + "\' property");
				}
			}

			if(importError == ErrorCode::Success)
			{
				setLoadedToMemory(true);
				setLoadedToVideoMemory(true);
			}
		}

		return importError;
	}

	PropertySet exportObject() final override
	{
		// Create the root Camera property set
		PropertySet propertySet(Properties::Camera);

		return propertySet;
	}

	// System type is Graphics
	BitMask getSystemType() final override { return Systems::Script; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::None; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::All; }

	const inline Math::Quaternion &getQuaternion(const Observer *p_observer, BitMask p_changedBits) const
	{
		return m_luaSpatialData.getQuaternion(p_observer, p_changedBits);
	}
	const inline Math::Vec3f &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getVec3(p_observer, p_changedBits);

		return NullObjects::NullVec3f;
	}
	const inline Math::Mat4f &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getMat4(p_observer, p_changedBits);

		return NullObjects::NullMat4f;
	}
	const inline SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getSpatialData(p_observer, p_changedBits);

		return NullObjects::NullSpacialData;
	}
	const inline SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getSpatialTransformData(p_observer, p_changedBits);

		return NullObjects::NullSpacialTransformData;
	}

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	void setSpatialDataManagerReference(const SpatialDataManager &p_spatialData) 
	{ 
		m_spatialData = &p_spatialData;

		// Copy data from reference spatial data manager to a copy of it that's used for lua script
		m_luaSpatialData = *m_spatialData;

		// Make sure to track local changes
		m_luaSpatialData.setTrackLocalChanges(true);
	}

private:
	LuaScript m_luaScript;
	bool m_luaScriptLoaded;

	SpatialDataManager m_luaSpatialData;
};