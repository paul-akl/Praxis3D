#pragma once

#include "EnumFactory.h"
#include "ErrorHandlerLocator.h"
#include "GUIDataManager.h"
#include "GUIHandler.h"
#include "SpatialDataManager.h"
#include "WindowLocator.h"

#include <functional>
#include <random>
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
    Code(PathsVariables,) \
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

struct ComponentsConstructionInfo;

struct Conditional
{
	Conditional() { m_flag = false; }

	inline bool isChecked() const { return m_flag; }

	inline void check() { m_flag = true; }
	inline void uncheck() { m_flag = false; }
	inline void set(const bool p_flag) { m_flag = p_flag; }

	bool m_flag;
};

struct RandomIntGenerator
{
	RandomIntGenerator(const int p_min, const int p_max, const int p_seed) : m_generator(p_seed), m_distribution(p_min, p_max) { }
	RandomIntGenerator(const int p_min, const int p_max) : m_distribution(p_min, p_max) 
	{ 
		m_generator.seed((unsigned int)time(NULL));
	}

	int generate() { return m_distribution(m_generator); }

	std::mt19937 m_generator;
	std::uniform_int_distribution<int> m_distribution;
};

struct RandomFloatGenerator
{
	RandomFloatGenerator(const float p_min, const float p_max, const int p_seed) : m_generator(p_seed), m_distribution(p_min, p_max) { }
	RandomFloatGenerator(const float p_min, const float p_max) : m_distribution(p_min, p_max)
	{
		m_generator.seed((unsigned int)time(NULL));
	}

	float generate() { return m_distribution(m_generator); }

	std::mt19937 m_generator;
	std::uniform_real_distribution<float> m_distribution;
};

struct SingleChange
{
	SingleChange(const Observer *p_observer, const BitMask p_changeType, const Property &p_changeValue) : m_observer(p_observer), m_changeType(p_changeType), m_changeData(p_changeValue) { }

	const Observer *m_observer;
	BitMask m_changeType;
	Property m_changeData;
};

class LuaScript
{
	friend class LuaComponent;
public:
	LuaScript(SystemScene *p_scriptScene, SystemObject *p_luaComponent, SpatialDataManager &p_spatialData, GUIDataManager &p_GUIData) : 
		m_scriptScene(p_scriptScene), m_luaComponent(p_luaComponent), m_spatialData(p_spatialData), m_GUIData(p_GUIData)
	{ 
		m_keyCommands.reserve(10);
		m_conditionals.reserve(10);
		m_currentChanges = Systems::Changes::None;
		resetErrorFlag();
	}
	LuaScript(SystemScene *p_scriptScene, SystemObject *p_luaComponent, SpatialDataManager &p_spatialData, GUIDataManager &p_GUIData, std::string &p_scriptFilename) : 
		m_scriptScene(p_scriptScene), m_luaComponent(p_luaComponent), m_spatialData(p_spatialData), m_GUIData(p_GUIData), m_luaScriptFilename(p_scriptFilename)
	{
		m_keyCommands.reserve(10);
		m_conditionals.reserve(10);
		m_currentChanges = Systems::Changes::None;
		resetErrorFlag();
	}
	~LuaScript();

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		resetErrorFlag();

		// Initialize Lua state
		m_luaState.open_libraries(sol::lib::base);

		// Load script file
		if(auto loadScriptError = m_luaState.script_file(Config::filepathVar().script_path + m_luaScriptFilename); loadScriptError.valid())
		{
			// Set enum definitions and function call-backs, and define C++ user-types in Lua
			setDefinitions();
			setFunctions();
			setUsertypes();
			setLuaVariables();

			// Get function references that are inside the Lua script
			m_luaInit = m_luaState[Config::scriptVar().iniFunctionName];
			m_luaUpdate = m_luaState[Config::scriptVar().updateFunctionName];

			// Initialize the Lua script
			if(auto initError = m_luaInit(); !initError.valid())
			{
				sol::error error = initError;
				std::string errorString = error.what();

				ErrHandlerLoc().get().log(ErrorCode::Lua_init_func_failed, ErrorSource::Source_LuaScript, errorString);

				returnError = ErrorCode::Lua_init_func_failed;
			}
		}
		else
		{
			sol::error error = loadScriptError;
			std::string errorString = error.what();

			ErrHandlerLoc().get().log(ErrorCode::Lua_load_script_failed, ErrorSource::Source_LuaScript, errorString);

			returnError = ErrorCode::Lua_load_script_failed;
		}

		return returnError;
	}

	inline void update(const float p_deltaTime)
	{
		if(!m_updateFuncGeneratedError)
		{
			// Clear changes from the last update
			clearChanges();
			m_queuedChanges.clear();

			// Clear all the GUI functors from the last update
			m_GUIData.clearFunctors();

			// Call update function in the lua script
			if(!Config::scriptVar().luaUpdateErrorsEveryFrame && m_updateFuncRanWithNoErrors)
			{
				m_luaUpdate(p_deltaTime);
			}
			else
			{
				// Check for errors if the update function is called for the first time
				if(auto updateError = m_luaUpdate(p_deltaTime); !updateError.valid())
				{
					sol::error error = updateError;
					std::string errorString = error.what();

					ErrHandlerLoc().get().log(ErrorCode::Lua_update_func_failed, ErrorSource::Source_LuaScript, errorString);
					m_updateFuncGeneratedError = true;
				}
				else
					m_updateFuncRanWithNoErrors = true;
			}
		}
	}

	// Clears the currently registered changes. They are also cleared at the beginning of each update call
	inline void clearChanges() { m_currentChanges = Systems::Changes::None; }

	// Set the filename of the script that should be loaded
	inline void setScriptFilename(const std::string &p_filename) { m_luaScriptFilename = p_filename; }

	// Set the variables so they can be accessed from inside the lua script
	inline void setVariables(const std::vector<std::pair<std::string, Property>> &p_variables, const bool p_setVariablesInsideLua = false)
	{
		m_variables = p_variables;
		if(p_setVariablesInsideLua)
			setLuaVariables();
	}

	// Set the variables so they can be accessed from inside the lua script
	inline void setVariables(const PropertySet &p_properties, const bool p_setVariablesInsideLua = false)
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

			if(p_setVariablesInsideLua)
				setLuaVariables();
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
	const inline std::vector<std::pair<std::string, Property>> &getLuaVariables() const { return m_variables; }
	const inline std::vector<std::pair<std::string, KeyCommand *>> &getBoundKeys() const { return m_keyCommands; }

private:
	// Terminate the current LUA script and initialize it again
	void reload()
	{
		terminate();
		init();
	}
	// Remove the LUA script and delete all objects created by it
	void terminate();

	// Sets lua tables with various definitions and values
	void setDefinitions();
	// Binds functions, so that they can be called from the lua script
	void setFunctions();
	// Defines usertypes, so that they can be used in the lua script
	void setUsertypes();
	// Sets the defined variables inside the lua script
	void setLuaVariables();

	template <class T_Variable>
	inline void queueChange(SystemObject *p_observer, const BitMask p_changeType, const T_Variable p_changeValue)
	{
		m_queuedChanges.emplace_back(p_observer, p_changeType, Property(Properties::PropertyID::Null, p_changeValue));

		m_scriptScene->getSceneLoader()->getChangeController()->sendChange(m_luaComponent, p_observer, p_changeType);
	}

	// Returns the change value matching the change type bitmask
	// Returns nullptr if no match was found
	inline const Property *getQueuedChange(const Observer *p_observer, const BitMask p_changeType)
	{
		for(decltype(m_queuedChanges.size()) i = 0, size = m_queuedChanges.size(); i < size; i++)
			if(m_queuedChanges[i].m_changeType == p_changeType && m_queuedChanges[i].m_observer == p_observer)
				return &m_queuedChanges[i].m_changeData;

		return nullptr;
	}

	// Creates and assigns objects to be used in the lua script
	// Should only be called from the lua script
	void createObjectInLua(const unsigned int p_objectType, const std::string p_variableName);

	// Registers a change in some data
	// Should only be called from the lua script
	void registerChange(const Int64Packer &p_packer)
	{
		m_currentChanges += p_packer.get();
	}

	// Errors flags should be reset after reinitializing
	void resetErrorFlag()
	{
		m_updateFuncGeneratedError = false;
		m_updateFuncRanWithNoErrors = false;
	}

	// Used to stop running after an error is generated
	bool m_updateFuncGeneratedError;
	bool m_updateFuncRanWithNoErrors;

	// Lua state for the loaded script
	sol::state m_luaState;
	std::string m_luaScriptFilename;

	// Function binds that call functions inside the lua script
	sol::protected_function m_luaInit;
	sol::protected_function m_luaUpdate;

	// Tables for variable definitions
	sol::table m_userTypesTable;
	sol::table m_changeTypesTable;

	// An array of queued changes, that get saved when a change is sent from a lua script
	std::vector<SingleChange> m_queuedChanges;

	// An array of variable names and their value, that get additionally defined in the lua script
	std::vector<std::pair<std::string, Property>> m_variables;

	// Pointer to the scripting scene that owns this instance; required for getting things like scene loader for entity/component creation
	SystemScene *m_scriptScene;

	// Pointer to the Lua component that owns this instance; required for sending changes
	SystemObject *m_luaComponent;

	// Contains all spatial data
	SpatialDataManager &m_spatialData;

	// Contains all GUI data
	GUIDataManager &m_GUIData;

	// Keeps all created key commands (and the variable name in Lua the key is assigned to), so they can be unbound and deleted when cleaning up
	std::vector<std::pair<std::string, KeyCommand*>> m_keyCommands;

	// Keep all created objects, so they can be deleted when cleaning up
	std::vector<Conditional*> m_conditionals;
	std::vector<ComponentsConstructionInfo*> m_componentsConstructionInfo;

	// Stores the changes made to the data since the last update
	BitMask m_currentChanges;
};