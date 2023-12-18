#pragma once

#define SOL_LUAJIT 1

#include "ErrorHandlerLocator.h"
#include "Filesystem.h"
#include "InheritanceObjects.h"
#include "LuaScript.h"
#include "SpatialComponent.h"
#include "System.h"

class LuaComponent : public SystemObject, public SpatialDataManagerObject, public LoadableGraphicsObject
{
	friend class ScriptScene;
public:
	struct LuaComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		LuaComponentConstructionInfo()
		{

		}

		std::string m_luaScriptFilename;
		std::vector<std::pair<std::string, Property>> m_variables;
	};

	LuaComponent(SystemScene *p_systemScene, const std::string &p_name, const EntityID p_entityID) : SystemObject(p_systemScene, p_name, Properties::PropertyID::LuaComponent, p_entityID), m_luaSpatialData(*this), m_GUIData(*this)
	{
		m_luaScript = new LuaScript(m_systemScene, m_luaSpatialData, m_GUIData);
		m_luaScriptLoaded = false;
		m_luaSpatialData.setTrackLocalChanges(true);
	}
	~LuaComponent() 
	{
		delete m_luaScript;
	}

	ErrorCode init() final override
	{
		// Mark the object as loaded, because there is nothing to be specifically load, at least for now
		//setLoadedToMemory(true);
		//setLoadedToVideoMemory(true);

		return ErrorCode::Success;
	}

	void loadToMemory()
	{
		auto luaError = m_luaScript->init();

		if(luaError != ErrorCode::Success)
			ErrHandlerLoc().get().log(luaError, ErrorSource::Source_LuaComponent, m_name);
		else
			m_luaScriptLoaded = true;

		setLoadedToMemory(true);
		setActive(m_setActiveAfterLoading);
	}

	void update(const float p_deltaTime)
	{
		// Perform updates only the if the script is loaded
		if(m_luaScriptLoaded)
		{
			// Update the lua script
			m_luaScript->update(p_deltaTime);

			// Get the changes from the lua script
			auto changes = m_luaScript->getChanges();

			// Add spatial changes to the current changes, because they are tracked separately
			changes += m_luaSpatialData.getCurrentChangesAndReset();

			// Add GUI changes to the current changes, because they are tracked separately
			changes += m_GUIData.getCurrentChangesAndReset();

			// Post the new changes
			postChanges(changes);
		}
	}

	inline void update(const float p_deltaTime, const SpatialComponent &p_spatialComponent)
	{
		// Get the current spatial data
		m_luaSpatialData.setSpatialData(p_spatialComponent.getSpatialDataChangeManager());

		update(p_deltaTime);
	}

	ErrorCode importObject(const PropertySet &p_properties) final override
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if PropertySet isn't empty and the component hasn't been loaded already
		if(p_properties && !isLoadedToMemory())
		{
			if(p_properties.getPropertyID() == Properties::LuaComponent)
			{
				auto const &luaFilenameProperty = p_properties.getPropertyByID(Properties::Filename);
				auto const &luaVariablesProperty = p_properties.getPropertySetByID(Properties::Variables);

				if(luaFilenameProperty)
				{
					std::string luaFilename = luaFilenameProperty.getString();
					if(!luaFilename.empty())
					{
						if(Filesystem::exists(Config::filepathVar().script_path + luaFilename))
						{
							m_luaScript->setScriptFilename(luaFilename);

							if(luaVariablesProperty)
								m_luaScript->setVariables(luaVariablesProperty);

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

	const glm::quat &getQuaternion(const Observer *p_observer, BitMask p_changedBits) const
	{
		return m_luaSpatialData.getQuaternion(p_observer, p_changedBits);
	}
	const glm::vec3 &getVec3(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getVec3(p_observer, p_changedBits);

		return NullObjects::NullVec3f;
	}
	const glm::mat4 &getMat4(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getMat4(p_observer, p_changedBits);

		return NullObjects::NullMat4f;
	}
	const SpatialData &getSpatialData(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getSpatialData(p_observer, p_changedBits);

		return NullObjects::NullSpacialData;
	}
	const SpatialTransformData &getSpatialTransformData(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::Spatial))
			return m_luaSpatialData.getSpatialTransformData(p_observer, p_changedBits);

		return NullObjects::NullSpacialTransformData;
	}
	const Functors &getFunctors(const Observer *p_observer, BitMask p_changedBits) const
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::GUI))
				return m_GUIData.getFunctors(p_observer, p_changedBits);

		return NullObjects::NullFunctors;
	}
	const inline LuaScript *getLuaScript() const { return m_luaScript; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) 
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
		{
			// Get the active flag from the subject and set the active flag accordingly
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Script::Filename))
		{
			if(m_luaScript != nullptr)
			{
				// Get the new LUA script filename from the observed subject
				auto luaScriptFilename = p_subject->getString(this, Systems::Changes::Script::Filename);

				// Check if the lua script exists
				if(!luaScriptFilename.empty())
				{
					m_luaScript->setScriptFilename(luaScriptFilename);

					m_luaScript->reload();
				}
			}
		}

		if(CheckBitmask(p_changeType, Systems::Changes::Script::Reload))
		{
			// Check if the lua script exists
			if(m_luaScript != nullptr)
			{
				m_luaScript->reload();
			}
		}
	}

	void setSpatialDataManagerReference(const SpatialDataManager &p_spatialData) 
	{ 
		m_spatialData = &p_spatialData;

		// Copy data from reference spatial data manager to a copy of it that's used for lua script
		m_luaSpatialData = *m_spatialData;

		// Make sure to track local changes
		m_luaSpatialData.setTrackLocalChanges(true);
	}

private:
	LuaScript *m_luaScript;
	bool m_luaScriptLoaded;

	SpatialDataManager m_luaSpatialData;
	GUIDataManager m_GUIData;
};