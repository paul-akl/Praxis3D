#pragma once

#define SOL_LUAJIT 1

#include "EnumFactory.h"
#include "ErrorHandlerLocator.h"
#include "GUIDataManager.h"
#include "GUIHandler.h"
#include "SpatialDataManager.h"
#include "WindowLocator.h"

#include <functional>
#include <sol/sol.hpp>

namespace LuaDefinitions
{
#define LUA_USER_TYPES(Code) \
	Code(Test, ) \
	Code(Conditional, ) \
	Code(EngineVariables, ) \
	Code(GameplayVariables,) \
	Code(GraphicsVariables,) \
	Code(InputVariables,) \
    Code(KeyCommand,) \
    Code(MouseInfo,) \
	Code(SpatialDataManager,) \
	Code(WindowVariables,) \
	Code(ComponentsInfo,) \
	Code(GraphicsComponentInfo,) \
	Code(GUIComponentInfo,) \
	Code(PhysicsComponentInfo,) \
	Code(ScriptComponentInfo,) \
	Code(WorldComponentInfo,) \
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

struct Conditional
{
	Conditional() { m_flag = false; }

	inline bool isChecked() const { return m_flag; }

	inline void check() { m_flag = true; }
	inline void uncheck() { m_flag = false; }
	inline void set(const bool p_flag) { m_flag = p_flag; }

	bool m_flag;
};

class LuaScript
{
public:
	LuaScript(SystemScene *p_scriptScene, SpatialDataManager &p_spatialData, GUIDataManager &p_GUIData) : m_scriptScene(p_scriptScene), m_spatialData(p_spatialData), m_GUIData(p_GUIData)
	{ 
		m_keyCommands.reserve(10);
		m_conditionals.reserve(10);
		m_currentChanges = Systems::Changes::None;
	}
	LuaScript(SystemScene *p_scriptScene, SpatialDataManager &p_spatialData, GUIDataManager &p_GUIData, std::string &p_scriptFilename) : m_scriptScene(p_scriptScene), m_spatialData(p_spatialData), m_GUIData(p_GUIData), m_luaScriptFilename(p_scriptFilename)
	{
		m_keyCommands.reserve(10);
		m_conditionals.reserve(10);
		m_currentChanges = Systems::Changes::None;
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

		// Delete all created conditionals
		for(decltype(m_conditionals.size()) i = 0; i < m_conditionals.size(); i++)
		{
			delete m_conditionals[i];
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
		// Clear changes from the last update
		clearChanges();

		// Clear all the GUI functors from the last update
		m_GUIData.clearFunctors();

		// Call update function in the lua script
		m_luaUpdate(p_deltaTime);
	}

	// Clears the currently registered changes. They are also cleared at the beginning of each update call
	inline void clearChanges() { m_currentChanges = Systems::Changes::None; }

	// Set the filename of the script that should be loaded
	inline void setScriptFilename(std::string &p_filename) { m_luaScriptFilename = p_filename; }

	// Set the variables so they can be accessed from inside the lua script
	inline void setVariables(const std::vector<std::pair<std::string, Property>> &p_variables)
	{
		m_variables = p_variables;
	}

	// Set the variables so they can be accessed from inside the lua script
	inline void setVariables(const PropertySet &p_properties)
	{
		if(p_properties && p_properties.getPropertyID() == Properties::Variables)
		{
			// Loop over each variable entry in the node
			for(decltype(p_properties.getNumPropertySets()) iVariable = 0, numVariables = p_properties.getNumPropertySets(); iVariable < numVariables; iVariable++)
			{
				// Add the variable
				m_variables.emplace_back(
					p_properties.getPropertySet(iVariable).getPropertyByID(Properties::Name).getString(),
					p_properties.getPropertySet(iVariable).getPropertyByID(Properties::Value));
			}
		}
	}

	// Get sequences of function calls so that they can be passed to other objects and executed later
	// They are also cleared at the beginning of each update call, so a copy must be made if there are intentions to store them for later
	inline const Functors &getFunctors() { return m_GUIData.getGUIData().m_functors; }

	// Get changes since the last update (or the last clearChanges call)
	inline BitMask getChanges() const { return m_currentChanges; }
	inline BitMask getChangesAndClear() 
	{ 
		return m_currentChanges; 
		clearChanges();
	}

	const inline std::string &getLuaScriptFilename() const { return m_luaScriptFilename; }

private:
	// Sets lua tables with various definitions and values
	void setDefinitions()
	{
		// Create a table for user types that are supported by Lua scripts
		m_userTypesTable = m_luaState[Config::scriptVar().userTypeTableName].get_or_create<sol::table>();

		// Create a table for ImGUI window flags
		sol::table imGuiWindowFlag = m_luaState["ImGuiWindowFlags"].get_or_create<sol::table>();

		imGuiWindowFlag[sol::update_if_empty]["None"] = ImGuiWindowFlags_::ImGuiWindowFlags_None;
		imGuiWindowFlag[sol::update_if_empty]["NoTitleBar"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar;
		imGuiWindowFlag[sol::update_if_empty]["NoResize"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoResize;
		imGuiWindowFlag[sol::update_if_empty]["NoMove"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;
		imGuiWindowFlag[sol::update_if_empty]["NoScrollbar"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar;
		imGuiWindowFlag[sol::update_if_empty]["NoScrollWithMouse"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse;
		imGuiWindowFlag[sol::update_if_empty]["NoCollapse"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;
		imGuiWindowFlag[sol::update_if_empty]["AlwaysAutoResize"] = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize;
		imGuiWindowFlag[sol::update_if_empty]["NoBackground"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground;
		imGuiWindowFlag[sol::update_if_empty]["NoSavedSettings"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings;
		imGuiWindowFlag[sol::update_if_empty]["NoMouseInputs"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoMouseInputs;
		imGuiWindowFlag[sol::update_if_empty]["MenuBar"] = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
		imGuiWindowFlag[sol::update_if_empty]["HorizontalScrollbar"] = ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar;
		imGuiWindowFlag[sol::update_if_empty]["NoFocusOnAppearing"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoFocusOnAppearing;
		imGuiWindowFlag[sol::update_if_empty]["NoBringToFrontOnFocus"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus;
		imGuiWindowFlag[sol::update_if_empty]["AlwaysVerticalScrollbar"] = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar;
		imGuiWindowFlag[sol::update_if_empty]["AlwaysHorizontalScrollbar"] = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysHorizontalScrollbar;
		imGuiWindowFlag[sol::update_if_empty]["AlwaysUseWindowPadding"] = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding;
		imGuiWindowFlag[sol::update_if_empty]["NoNavInputs"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoNavInputs;
		imGuiWindowFlag[sol::update_if_empty]["NoNavFocus"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus;
		imGuiWindowFlag[sol::update_if_empty]["UnsavedDocument"] = ImGuiWindowFlags_::ImGuiWindowFlags_UnsavedDocument;
		imGuiWindowFlag[sol::update_if_empty]["NoNav"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoNav;
		imGuiWindowFlag[sol::update_if_empty]["NoDecoration"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration;
		imGuiWindowFlag[sol::update_if_empty]["NoInputs"] = ImGuiWindowFlags_::ImGuiWindowFlags_NoInputs;

		// Add each object type to the user type table
		for(int i = 0; i < LuaDefinitions::UserTypes::NumOfTypes; i++)
			m_userTypesTable[sol::update_if_empty][GetString(static_cast<LuaDefinitions::UserTypes>(i))] = i;

		// Create a table for different types of changes
		m_changeTypesTable = m_luaState["Changes"].get_or_create<sol::table>();

		// Create entries for GUI changes
		m_changeTypesTable[sol::update_if_empty]["GUI"]["Sequence"] = Int64Packer(Systems::Changes::GUI::Sequence);

		// Iterate over variables array and set each variable depending on its type
		for(decltype(m_variables.size()) i = 0, size = m_variables.size(); i < size; i++)
		{
			switch(m_variables[i].second.getVariableType())
			{
			case Property::Type_bool:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getBool());
				break;
			case Property::Type_int:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getInt());
				break;
			case Property::Type_float:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getFloat());
				break;
			case Property::Type_double:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getDouble());
				break;
			case Property::Type_vec2i:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getVec2i());
				break;
			case Property::Type_vec2f:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getVec2f());
				break;
			case Property::Type_vec3f:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getVec3f());
				break;
			case Property::Type_vec4f:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getVec4f());
				break;
			case Property::Type_string:
				m_luaState.set(m_variables[i].first, m_variables[i].second.getString());
				break;
			case Property::Type_propertyID:
				m_luaState.set(m_variables[i].first, GetString(m_variables[i].second.getID()));
				break;
			}
		}
	}
	// Binds functions, so that they can be called from the lua script
	void setFunctions();
	// Defines usertypes, so that they can be used in the lua script
	void setUsertypes();

	// Creates and assigns objects to be used in the lua script
	// Should only be called from the lua script
	void createObjectInLua(const unsigned int p_objectType, const std::string p_variableName);

	// Registers a change in some data
	// Should only be called from the lua script
	void registerChange(const Int64Packer &p_packer)
	{
		m_currentChanges += p_packer.get();
	}

	// Lua state for the loaded script
	sol::state m_luaState;
	std::string m_luaScriptFilename;

	// Function binds that call functions inside the lua script
	sol::function m_luaInit;
	sol::function m_luaUpdate;

	// Tables for variable definitions
	sol::table m_userTypesTable;
	sol::table m_changeTypesTable;

	// An array of variable names and their value, that get additionally defined in the lua script
	std::vector<std::pair<std::string, Property>> m_variables;

	// Pointer to the scripting scene that owns this instance; required for getting things like scene loader for entity/component creation
	SystemScene *m_scriptScene;

	// Contains all spatial data
	SpatialDataManager &m_spatialData;

	// Contains all GUI data
	GUIDataManager &m_GUIData;

	// Keeps all created key commands, so they can be unbound and deleted when cleaning up
	std::vector<KeyCommand*> m_keyCommands;

	// Keeps all created conditional objects, so they can be deleted when cleaning up
	std::vector<Conditional*> m_conditionals;

	// Contains sequences of function calls that can be passed to other SystemObjects
	//Functors m_functors;

	// Stores the changes made to the data since the last update
	BitMask m_currentChanges;
};