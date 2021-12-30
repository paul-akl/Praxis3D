#pragma once

#define SOL_LUAJIT 1

#include "EnumFactory.h"
#include "ErrorHandlerLocator.h"
#include "SpatialDataManager.h"
#include "WindowLocator.h"

#include <sol/sol.hpp>

namespace LuaDefinitions
{
#define LUA_USER_TYPES(Code) \
    Code(MouseInfo,) \
    Code(KeyCommand,) \
	Code(InputVariables,) \
	Code(SpatialDataManager,) \
	Code(NumOfTypes, )
	DECLARE_ENUM(UserTypes, LUA_USER_TYPES)

#define LUA_SPATIAL_CHANGES(Code) \
	Code(LocalPosition,) \
	Code(LocalRotation,) \
	Code(LocalScale,) \
	Code(LocalTransform,) \
	Code(WorldPosition,) \
	Code(WorldRotation,) \
	Code(WorldScale,) \
	Code(WorldTransform,) \
	Code(AllLocalNoTransform,) \
	Code(AllWorldNoTransform,) \
	Code(AllLocal,) \
	Code(AllWorld,) \
	Code(AllSpatial, )
	DECLARE_ENUM(SpatialChanges, LUA_SPATIAL_CHANGES)
}

class LuaScript
{
public:
	LuaScript(SpatialDataManager &p_spatialData) : m_spatialData(p_spatialData)
	{ 
		m_keyCommands.reserve(10);
	}
	LuaScript(SpatialDataManager &p_spatialData, std::string &p_scriptFilename) : m_spatialData(p_spatialData), m_luaScriptFilename(p_scriptFilename)
	{
		m_keyCommands.reserve(10);
	}
	~LuaScript() 
	{
		// Unbind all created key commands
		for(decltype(m_keyCommands.size()) i = 0; i < m_keyCommands.size(); i++)
		{
			if(m_keyCommands[i] != nullptr)
			{
				m_keyCommands[i]->unbindAll();
				delete m_keyCommands[i];
			}
		}
	}

	ErrorCode init()
	{
		// Initialize Lua state
		m_luaState.open_libraries(sol::lib::base);

		// Load script file
		m_luaState.script_file(m_luaScriptFilename);

		// Set enum definitions and function call-backs, and define C++ user-types in Lua
		setDefinitions();
		setFunctions();
		setUsertypes();

		// Get function references that are inside the Lua script
		m_luaInit = m_luaState[Config::scriptVar().iniFunctionName];
		m_luaUpdate = m_luaState[Config::scriptVar().updateFunctionName];

		// Initialize the Lua script
		m_luaInit();

		return ErrorCode::Success;
	}

	inline void update(const float p_deltaTime)
	{
		m_luaUpdate(p_deltaTime);
	}

	inline void setScriptFilename(std::string &p_filename) { m_luaScriptFilename = p_filename; }

private:
	void setDefinitions()
	{
		//m_userTypes[sol::update_if_empty]["TEST"] = 25;
		//m_luaState[sol::update_if_empty]["UserTypes"]["TEST"] = 25;
		//m_userTypes.create_with("TEST", 25);
		//m_luaState.create_table_with("TEST", 25);
		//m_luaState[sol::update_if_empty]["TEST"] = 25;

		// Create a table for user types that are supported by Lua scripts
		m_userTypes = m_luaState[Config::scriptVar().userTypeTableName].get_or_create<sol::table>();

		// Add each object type to the user type table
		for(int i = 0; i < LuaDefinitions::UserTypes::NumOfTypes; i++)
			m_userTypes[sol::update_if_empty][GetString(static_cast<LuaDefinitions::UserTypes>(i))] = i;
	}
	void setFunctions()
	{
		m_luaState.set_function("toRadianF", sol::resolve<float(const float)>(&Math::toRadian));

		m_luaState.set_function(Config::scriptVar().createObjectFunctionName, &LuaScript::createObjectInLua, this);
	}
	void setUsertypes()
	{
		m_luaState.new_usertype<Config::InputVariables>("InputVariables",
			"back_key", &Config::InputVariables::back_key,
			"backward_editor_key", &Config::InputVariables::backward_editor_key,
			"backward_key", &Config::InputVariables::backward_key,
			"center_key", &Config::InputVariables::center_key,
			"clip_mouse_key", &Config::InputVariables::clip_mouse_key,
			"close_window_key", &Config::InputVariables::close_window_key,
			"debug_1_key", &Config::InputVariables::debug_1_key,
			"debug_2_key", &Config::InputVariables::debug_2_key,
			"down_editor_key", &Config::InputVariables::down_editor_key,
			"down_key", &Config::InputVariables::down_key,
			"escape_key", &Config::InputVariables::escape_key,
			"forward_editor_key", &Config::InputVariables::forward_editor_key,
			"forward_key", &Config::InputVariables::forward_key,
			"fullscreen_key", &Config::InputVariables::fullscreen_key,
			"jump_key", &Config::InputVariables::jump_key,
			"left_editor_key", &Config::InputVariables::left_editor_key,
			"left_strafe_key", &Config::InputVariables::left_strafe_key,
			"modifier_editor_key", &Config::InputVariables::modifier_editor_key,
			"next_editor_key", &Config::InputVariables::next_editor_key,
			"num_preallocated_keybinds", &Config::InputVariables::num_preallocated_keybinds,
			"previous_editor_key", &Config::InputVariables::previous_editor_key,
			"right_editor_key", &Config::InputVariables::right_editor_key,
			"right_strafe_key", &Config::InputVariables::right_strafe_key,
			"save_editor_key", &Config::InputVariables::save_editor_key,
			"sprint_key", &Config::InputVariables::sprint_key,
			"up_editor_key", &Config::InputVariables::up_editor_key,
			"up_key", &Config::InputVariables::up_key,
			"vsync_key", &Config::InputVariables::vsync_key,
			"mouse_filter", &Config::InputVariables::mouse_filter,
			"cromouse_warp_modess", &Config::InputVariables::mouse_warp_mode,
			"mouse_jaw", &Config::InputVariables::mouse_jaw,
			"mouse_pitch", &Config::InputVariables::mouse_pitch,
			"mouse_pitch_clip", &Config::InputVariables::mouse_pitch_clip,
			"mouse_sensitivity", &Config::InputVariables::mouse_sensitivity);

		m_luaState.new_usertype<Math::Vec3f>("Vec3f",
			"x", &Math::Vec3f::x,
			"y", &Math::Vec3f::y,
			"z", &Math::Vec3f::z,
			"cross", &Math::Vec3f::cross,
			"dot", &Math::Vec3f::dot,
			"length", &Math::Vec3f::length,
			"normalize", &Math::Vec3f::normalize,
			"target", &Math::Vec3f::target,
			"rotateAngleAxis", sol::resolve<void(float, const Math::Vec3f &)>(&Math::Vec3f::rotate),
			"rotateVec3f", sol::resolve<void (const Math::Vec3f&)>(&Math::Vec3f::rotate),
			"addVec3f", &Math::Vec3f::operator+=,
			"subVec3f", &Math::Vec3f::operator-=,
			"mulF", sol::resolve<const Math::Vec3f &(const float)>(&Math::Vec3f::operator*=),
			"mulVec3f", sol::resolve<const Math::Vec3f &(const Math::Vec3f&)>(&Math::Vec3f::operator*=),
			"divF", sol::resolve<const Math::Vec3f &(const float)>(&Math::Vec3f::operator/=),
			"divVec3f", sol::resolve<const Math::Vec3f &(const Math::Vec3f &)>(&Math::Vec3f::operator/=));

		m_luaState.new_usertype<SpatialData>("SpatialData",
			"clear", &SpatialData::clear,
			"m_position", &SpatialData::m_position,
			"m_rotationEuler", &SpatialData::m_rotationEuler,
			"m_scale", &SpatialData::m_scale);

		m_luaState.new_usertype<SpatialTransformData>("SpatialTransformData",
			"clear", &SpatialTransformData::clear,
			"m_spatialData", &SpatialTransformData::m_spatialData);

		m_luaState.new_usertype<SpatialDataManager>("SpatialDataManager",
			"getLocalSpaceData", &SpatialDataManager::getLocalSpaceData,
			"setLocalPosition", &SpatialDataManager::setLocalPosition,
			"setLocalRotation", sol::resolve<const void (const Math::Vec3f)>(&SpatialDataManager::setLocalRotation),
			"setLocalScale", &SpatialDataManager::setLocalScale,
			"setLocalTransform", &SpatialDataManager::setLocalTransform,
			"getWorldSpaceData", &SpatialDataManager::getWorldSpaceData,
			"setWorldPosition", &SpatialDataManager::setWorldPosition,
			"setWorldRotation", sol::resolve<const void(const Math::Vec3f)>(&SpatialDataManager::setWorldRotation),
			"setWorldScale", &SpatialDataManager::setWorldScale,
			"setWorldTransform", &SpatialDataManager::setWorldTransform,
			"getWorldRotation", &SpatialDataManager::getWorldRotation);

		m_luaState.new_usertype<Window::MouseInfo>("MouseInfo",
			"m_movementCurrentFrameX", &Window::MouseInfo::m_movementCurrentFrameX,
			"m_movementCurrentFrameY", &Window::MouseInfo::m_movementCurrentFrameY,
			"m_movementPrevFrameX", &Window::MouseInfo::m_movementPrevFrameX,
			"m_movementPrevFrameY", &Window::MouseInfo::m_movementPrevFrameY,
			"m_wheelX", &Window::MouseInfo::m_wheelX,
			"m_wheelY", &Window::MouseInfo::m_wheelY,
			"m_movementX", &Window::MouseInfo::m_movementX,
			"activm_movementYate", &Window::MouseInfo::m_movementY);

		m_luaState.new_usertype<KeyCommand>("KeyCommand",
			"activate", &KeyCommand::activate,
			"deactivate", &KeyCommand::deactivate,
			"isActivated", &KeyCommand::isActivated,
			"bind", &KeyCommand::bind,
			"bindByKeyName", &KeyCommand::bindByKeyName,
			"unbind", &KeyCommand::unbind,
			"unbindAll", &KeyCommand::unbindAll);
	}

	void createObjectInLua(const unsigned int p_objectType, const std::string p_variableName)
	{
		// Check if the object type is in a valid range and if the variable name isn't empty
		if(p_objectType >= 0 && p_objectType < LuaDefinitions::UserTypes::NumOfTypes && !p_variableName.empty())
		{
			switch(p_objectType)
			{
			case LuaDefinitions::MouseInfo:

				// Set the given variable name in Lua to point to the MouseInfo object
				m_luaState.set(p_variableName, WindowLocator::get().getMouseInfo());

				break;
			case LuaDefinitions::KeyCommand:
			{
				// Create new key command
				KeyCommand *keyCommand = new KeyCommand();

				// Set the given variable name in Lua to point to the created key command
				m_luaState.set(p_variableName, keyCommand);

				// Add key command pointer to an array so it is not lost
				m_keyCommands.push_back(keyCommand);
			}
				break;
			case LuaDefinitions::InputVariables:

				// Set the given variable name in Lua to point to the MouseInfo object
				m_luaState.set(p_variableName, Config::inputVar());

				break;

			case LuaDefinitions::SpatialDataManager:

				// Set the given variable name in Lua to point to the Spatial Data Manager object
				m_luaState.set(p_variableName, &m_spatialData);

				break;
			default:
				break;
			}
		}
	}

	void recordBoolChange(const bool p_change, const unsigned int p_changeType)
	{

	}
	void recordIntChange(const int p_change, const unsigned int p_changeType)
	{

	}
	void recordFloatChange(const float p_change, const unsigned int p_changeType)
	{

	}
	void recordVec3fChange(const Math::Vec3f p_change, const unsigned int p_changeType)
	{

	}

	SpatialDataManager &m_spatialData;

	std::vector<KeyCommand*> m_keyCommands;

	sol::state m_luaState;
	std::string m_luaScriptFilename;

	sol::function m_luaInit;
	sol::function m_luaUpdate;

	sol::table m_userTypes;
};