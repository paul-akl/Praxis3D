#pragma once

#include "ErrorHandlerLocator.h"
#include "GUIHandler.h"
#include "InheritanceObjects.h"
#include "System.h"

class GUISequenceComponent : public SystemObject, public GUIDataManagerObject, public LoadableGraphicsObject
{
	friend class ScriptScene;
public:
	GUISequenceComponent(SystemScene *p_systemScene, std::string p_name, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::GUISequenceComponent)
	{
		m_staticSequence = false;
	}
	~GUISequenceComponent() { }

	ErrorCode init() final override
	{
		if(Config::GUIVar().gui_sequence_array_reserve_size > 0)
			m_guiSequence.reserve(Config::GUIVar().gui_sequence_array_reserve_size);

		return ErrorCode::Success;
	}

	void loadToMemory()
	{
	}

	void update(const float p_deltaTime)
	{
		for(decltype(m_guiSequence.size()) i = 0, size = m_guiSequence.size(); i < size; i++)
			m_guiSequence[i]();

		if(!m_staticSequence)
			m_guiSequence.clear();
	}

	ErrorCode importObject(const PropertySet &p_properties) final override
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if PropertySet isn't empty and the component hasn't been loaded already
		if(p_properties && !isLoadedToMemory())
		{
			if(p_properties.getPropertyID() == Properties::Sequence)
			{
				importError = ErrorCode::Success;
	/*			auto const &luaFilenameProperty = p_properties.getPropertyByID(Properties::Filename);

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
				}*/
			}

			if(importError == ErrorCode::Success)
			{
				setLoadedToMemory(true);
				setLoadedToVideoMemory(true);
				setActive(true);
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
	BitMask getSystemType() final override { return Systems::GUI; }

	BitMask getDesiredSystemChanges() final override { return Systems::Changes::None; }
	BitMask getPotentialSystemChanges() final override { return Systems::Changes::GUI::All; }

	const Functors &getFunctors(const Observer *p_observer, BitMask p_changedBits)
	{
		if(CheckBitmask(p_changedBits, Systems::Changes::Type::GUI))
				return m_guiSequence;
	}

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) 
	{
		if(CheckBitmask(p_changeType, Systems::Changes::GUI::Sequence))
			m_guiSequence = p_subject->getFunctors(this, Systems::Changes::GUI::Sequence);
	}

private:
	Functors m_guiSequence;

	bool m_staticSequence;
};