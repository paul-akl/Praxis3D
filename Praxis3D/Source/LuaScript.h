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
	Code(InputVariables,) \
    Code(KeyCommand,) \
    Code(MouseInfo,) \
	Code(SpatialDataManager,) \
	Code(WindowVariables,) \
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
	LuaScript(SpatialDataManager &p_spatialData, GUIDataManager &p_GUIData) : m_spatialData(p_spatialData), m_GUIData(p_GUIData)
	{ 
		m_keyCommands.reserve(10);
		m_conditionals.reserve(10);
		m_currentChanges = Systems::Changes::None;
	}
	LuaScript(SpatialDataManager &p_spatialData, GUIDataManager &p_GUIData, std::string &p_scriptFilename) : m_spatialData(p_spatialData), m_GUIData(p_GUIData), m_luaScriptFilename(p_scriptFilename)
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
	void setFunctions()
	{
		// Math functions
		m_luaState.set_function("toRadianF", sol::resolve<float(const float)>(&glm::radians));
		m_luaState.set_function("toRadianVec3", sol::resolve<glm::vec3(const glm::vec3 &)>(&glm::radians));
		m_luaState.set_function("toRadianVec4", sol::resolve<glm::vec4(const glm::vec4 &)>(&glm::radians));
		m_luaState.set_function("toDegreesF", sol::resolve<float(const float)>(&glm::degrees));
		m_luaState.set_function("toDegreesVec3", sol::resolve<glm::vec3(const glm::vec3 &)>(&glm::degrees));
		m_luaState.set_function("toDegreesVec4", sol::resolve<glm::vec4(const glm::vec4 &)>(&glm::degrees));
		m_luaState.set_function("angleAxisQuat", sol::resolve<glm::quat(const float &, const glm::vec3 &)>(&glm::angleAxis));

		// Input / Window functions
		m_luaState.set_function("getMouseInfo", []() -> const Window::MouseInfo { return WindowLocator::get().getMouseInfo(); });
		m_luaState.set_function("mouseCaptured", []() -> const bool { return Config::windowVar().mouse_captured; });
		m_luaState.set_function("setFullscreen", [](const bool p_v1) -> const void { WindowLocator::get().setFullscreen(p_v1); });
		m_luaState.set_function("setMouseCapture", [](const bool p_v1) -> const void { WindowLocator::get().setMouseCapture(p_v1); });
		m_luaState.set_function("setVerticalSync", [](const bool p_v1) -> const void { WindowLocator::get().setVerticalSync(p_v1); });
		m_luaState.set_function("setWindowTitle", [](const std::string &p_v1) -> const void { WindowLocator::get().setWindowTitle(p_v1); });

		// Engine functions
		m_luaState.set_function("setEngineRunning", [](const bool p_v1) -> const void {Config::m_engineVar.running = p_v1; });

		// GUI functions
		auto GUITable = m_luaState.create_table("GUI");
		GUITable.set_function("Begin", [this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::Begin(p_v1.c_str()); }); });
		GUITable.set_function("BeginChild", [this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::BeginChild(p_v1.c_str()); }); });
		GUITable.set_function("BeginMenu", [this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::BeginMenu(p_v1.c_str()); }); });
		GUITable.set_function("Button", [this](const std::string &p_v1, Conditional *p_v2) -> void { m_GUIData.addFunctor([=] { p_v2->m_flag = ImGui::Button(p_v1.c_str()); }); });
		GUITable.set_function("Checkbox", [this](const std::string &p_v1, Conditional *p_v2) -> void { m_GUIData.addFunctor([=] { ImGui::Checkbox(p_v1.c_str(), &p_v2->m_flag); }); });
		GUITable.set_function("ColorEdit3", [this](const std::string &p_v1, glm::vec3 *p_v2) -> void { m_GUIData.addFunctor([=] { ImGui::ColorEdit3(p_v1.c_str(), &(p_v2->x)); }); });
		GUITable.set_function("ColorEdit4", [this](const std::string &p_v1, glm::vec4 *p_v2) -> void { m_GUIData.addFunctor([=] { ImGui::ColorEdit4(p_v1.c_str(), &(p_v2->x)); }); });
		GUITable.set_function("End", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::End(); }); });
		GUITable.set_function("EndChild", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::EndChild(); }); });
		GUITable.set_function("EndMenu", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::EndMenu(); }); });
		GUITable.set_function("EndMenuBar", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::EndMenuBar(); }); });
		GUITable.set_function("MenuItem", [this](const std::string &p_v1, const std::string &p_v2, Conditional *p_v3) -> void { m_GUIData.addFunctor([=] { p_v3->m_flag = ImGui::MenuItem(p_v1.c_str(), p_v2.c_str()); }); });
		GUITable.set_function("PlotLines", [this](const std::string &p_v1, const float *p_v2, int p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::PlotLines(p_v1.c_str(), p_v2, p_v3); }); });
		GUITable.set_function("SliderFloat", [this](const std::string &p_v1, float *p_v2, const float p_v3, const float p_v4) -> const void { m_GUIData.addFunctor([=] { ImGui::SliderFloat(p_v1.c_str(), p_v2, p_v3, p_v4); }); });
		GUITable.set_function("SameLine", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::SameLine(); }); });
		GUITable.set_function("Text", sol::overload([this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::Text(p_v1.c_str()); }); },
			[this](const std::string &p_v1, const float p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::Text(p_v1.c_str(), p_v2); }); },
			[this](const std::string &p_v1, const float p_v2, const float p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::Text(p_v1.c_str(), p_v2, p_v3); }); }));
		GUITable.set_function("TextColored", sol::overload([this](const glm::vec4 p_v1, const std::string &p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::TextColored(ImVec4(p_v1.x, p_v1.y, p_v1.z, p_v1.w), p_v2.c_str()); }); },
			[this](const glm::vec4 p_v1, const std::string &p_v2, const float p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::TextColored(ImVec4(p_v1.x, p_v1.y, p_v1.z, p_v1.w), p_v2.c_str(), p_v3); }); },
			[this](const glm::vec4 p_v1, const std::string &p_v2, const float p_v3, const float p_v4) -> const void { m_GUIData.addFunctor([=] { ImGui::TextColored(ImVec4(p_v1.x, p_v1.y, p_v1.z, p_v1.w), p_v2.c_str(), p_v3, p_v4); }); }));

		// LuaScript callbacks
		m_luaState.set_function("postChanges", &LuaScript::registerChange, this);
		m_luaState.set_function(Config::scriptVar().createObjectFunctionName, &LuaScript::createObjectInLua, this);
	}
	// Defines usertypes, so that they can be used in the lua script
	void setUsertypes()
	{
		// Config variables
		m_luaState.new_usertype<Config::EngineVariables>("EngineVariables",
			"change_ctrl_cml_notify_list_reserv", &Config::EngineVariables::change_ctrl_cml_notify_list_reserv,
			"change_ctrl_grain_size", &Config::EngineVariables::change_ctrl_grain_size,
			"change_ctrl_notify_list_reserv", &Config::EngineVariables::change_ctrl_notify_list_reserv,
			"change_ctrl_oneoff_notify_list_reserv", &Config::EngineVariables::change_ctrl_oneoff_notify_list_reserv,
			"change_ctrl_subject_list_reserv", &Config::EngineVariables::change_ctrl_subject_list_reserv,
			"delta_time_divider", &Config::EngineVariables::delta_time_divider,
			"gl_context_major_version", &Config::EngineVariables::gl_context_major_version,
			"gl_context_minor_version", &Config::EngineVariables::gl_context_minor_version,
			"object_directory_init_pool_size", &Config::EngineVariables::object_directory_init_pool_size,
			"smoothing_tick_samples", &Config::EngineVariables::smoothing_tick_samples,
			"running", &Config::EngineVariables::running);

		m_luaState.new_usertype<Config::GameplayVariables>("GameplayVariables",
			"camera_freelook_speed", &Config::GameplayVariables::camera_freelook_speed);

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
			"mouse_warp_mode", &Config::InputVariables::mouse_warp_mode,
			"mouse_jaw", &Config::InputVariables::mouse_jaw,
			"mouse_pitch", &Config::InputVariables::mouse_pitch,
			"mouse_pitch_clip", &Config::InputVariables::mouse_pitch_clip,
			"mouse_sensitivity", &Config::InputVariables::mouse_sensitivity);

		m_luaState.new_usertype<Config::WindowVariables>("WindowVariables",
			"name", &Config::WindowVariables::name,
			"default_display", &Config::WindowVariables::default_display,
			"window_position_x", &Config::WindowVariables::window_position_x,
			"window_position_y", &Config::WindowVariables::window_position_y,
			"window_size_fullscreen_x", &Config::WindowVariables::window_size_fullscreen_x,
			"window_size_fullscreen_y", &Config::WindowVariables::window_size_fullscreen_y,
			"window_size_windowed_x", &Config::WindowVariables::window_size_windowed_x,
			"window_size_windowed_y", &Config::WindowVariables::window_size_windowed_y,
			"fullscreen", &Config::WindowVariables::fullscreen,
			"fullscreen_borderless", &Config::WindowVariables::fullscreen_borderless,
			"mouse_captured", &Config::WindowVariables::mouse_captured,
			"mouse_release_on_lost_focus", &Config::WindowVariables::mouse_release_on_lost_focus,
			"resizable", &Config::WindowVariables::resizable,
			"vertical_sync", &Config::WindowVariables::vertical_sync,
			"window_in_focus", &Config::WindowVariables::window_in_focus);

		// Math types
		m_luaState.new_usertype<Int64Packer>("Int64");

		m_luaState.new_usertype<glm::vec3>("Vec3",
			sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float), glm::vec3(glm::vec4)>(),
			"x", &glm::vec3::x,
			"y", &glm::vec3::y,
			"z", &glm::vec3::z,
			"addF", [](const glm::vec3 &v1, const float f) -> glm::vec3 { return v1 + f; },
			"subF", [](const glm::vec3 &v1, const float f) -> glm::vec3 { return v1 - f; },
			"mulF", [](const glm::vec3 &v1, const float f) -> glm::vec3 { return v1 * f; },
			"divF", [](const glm::vec3 &v1, const float f) -> glm::vec3 { return v1 / f; },
			"addVec3", [](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 + v2; },
			"subVec3", [](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 - v2; },
			"mulVec3", [](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 * v2; },
			"divVec3", [](const glm::vec3 &v1, const glm::vec3 &v2) -> glm::vec3 { return v1 / v2; },
			"normalize", [](const glm::vec3 &v1) -> glm::vec3 { return glm::normalize(v1); },
			sol::meta_function::addition,		[](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 + v2; },
			sol::meta_function::subtraction,	[](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 - v2; },
			sol::meta_function::multiplication, [](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 * v2; },
			sol::meta_function::division,		[](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 / v2; });

		m_luaState.new_usertype<glm::vec4>("Vec4",
			sol::constructors<glm::vec4(), glm::vec4(float), glm::vec4(glm::vec3, float), glm::vec4(float, float, float, float)>(),
			"x", &glm::vec4::x,
			"y", &glm::vec4::y,
			"z", &glm::vec4::z,
			"w", &glm::vec4::w,
			"addF", [](const glm::vec4 &v1, const float f) -> glm::vec4 { return v1 + f; },
			"subF", [](const glm::vec4 &v1, const float f) -> glm::vec4 { return v1 - f; },
			"mulF", [](const glm::vec4 &v1, const float f) -> glm::vec4 { return v1 * f; },
			"divF", [](const glm::vec4 &v1, const float f) -> glm::vec4 { return v1 / f; },
			"addVec4", [](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 + v2; },
			"subVec4", [](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 - v2; },
			"mulVec4", [](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 * v2; },
			"divVec4", [](const glm::vec4 &v1, const glm::vec4 &v2) -> glm::vec4 { return v1 / v2; },
			"normalize", [](const glm::vec4 &v1) -> glm::vec4 { return glm::normalize(v1); },
			sol::meta_function::addition, [](const glm::vec4 &v1, glm::vec4 &v2) -> glm::vec4 { return v1 + v2; },
			sol::meta_function::subtraction, [](const glm::vec4 &v1, glm::vec4 &v2) -> glm::vec4 { return v1 - v2; },
			sol::meta_function::multiplication, [](const glm::vec4 &v1, glm::vec4 &v2) -> glm::vec4 { return v1 * v2; },
			sol::meta_function::division, [](const glm::vec4 &v1, glm::vec4 &v2) -> glm::vec4 { return v1 / v2; });

		m_luaState.new_usertype<glm::quat>("Quat",
			sol::constructors<glm::quat(), glm::quat(glm::vec4)>(),
			"x", &glm::quat::x,
			"y", &glm::quat::y,
			"z", &glm::quat::z,
			"w", &glm::quat::w,
			"mulF", [](const glm::quat &q1, const float f) -> glm::quat { return q1 * f; },
			"divF", [](const glm::quat &q1, const float f) -> glm::quat { return q1 / f; },
			"mulVec3", [](const glm::quat &q1, const glm::vec3 &v2) -> glm::quat { return q1 * v2; },
			"normalize", [](const glm::quat &q1) -> glm::quat { return glm::normalize(q1); },
			"inverse", [](const glm::quat &q1) -> glm::quat { return glm::inverse(q1); },
			"rotateVec3", [](const glm::quat &q1, const glm::vec3 &v2) -> glm::vec3 { return glm::rotate(q1, v2); },
			"toMat4", [](const glm::quat &q1) -> glm::mat4 { return glm::toMat4(q1); },
			sol::meta_function::addition, [](const glm::quat &q1, glm::quat &q2) -> glm::quat { return q1 + q2; },
			sol::meta_function::subtraction, [](const glm::quat &q1, glm::quat &q2) -> glm::quat { return q1 - q2; },
			sol::meta_function::multiplication, [](const glm::quat &q1, glm::quat &q2) -> glm::quat { return q1 * q2; });

		m_luaState.new_usertype<glm::mat4>("Mat4",
			sol::constructors<glm::mat4(), glm::mat4(float)>(),
			"mulF", [](const glm::mat4 &v1, const float f) -> glm::mat4 { return v1 * f; },
			"divF", [](const glm::mat4 &v1, const float f) -> glm::mat4 { return v1 / f; },
			"mulVec3", [](const glm::mat4 &m1, const glm::vec3 &v2) -> glm::vec4 { return m1 * glm::vec4(v2, 1.0f); },
			"divVec3", [](const glm::mat4 &m1, const glm::vec3 &v2) -> glm::vec4 { return m1 / glm::vec4(v2, 1.0f); },
			"mulVec4", [](const glm::mat4 &m1, const glm::vec4 &v2) -> glm::vec4 { return m1 * v2; },
			"divVec4", [](const glm::mat4 &m1, const glm::vec4 &v2) -> glm::vec4 { return m1 / v2; },
			"inverse", [](const glm::mat4 &m1) -> glm::mat4 { return glm::inverse(m1); },
			"rotate", [](const glm::mat4 &m1, const float f1, const glm::vec3 &v1) -> glm::mat4 { return glm::rotate(m1, f1, v1); },
			"toQuat", [](const glm::mat4 &m1) -> glm::quat { return glm::toQuat(m1); },
			"getRotXVec3", [](const glm::mat4 &m1) -> glm::vec3 { return m1[0]; },
			"getRotYVec3", [](const glm::mat4 &m1) -> glm::vec3 { return m1[1]; },
			"getRotZVec3", [](const glm::mat4 &m1) -> glm::vec3 { return m1[2]; },
			"getPosVec3", [](const glm::mat4 &m1) -> glm::vec3 { return m1[3]; },
			"getRotXVec4", [](const glm::mat4 &m1) -> glm::vec4 { return m1[0]; },
			"getRotYVec4", [](const glm::mat4 &m1) -> glm::vec4 { return m1[1]; },
			"getRotZVec4", [](const glm::mat4 &m1) -> glm::vec4 { return m1[2]; },
			"getPosVec4", [](const glm::mat4 &m1) -> glm::vec4 { return m1[3]; },
			"setPosVec3", [](glm::mat4 &m1, const glm::vec3 &v2) -> void { m1[3] = glm::vec4(v2, 1.0f); },
			"setPosVec4", [](glm::mat4 &m1, const glm::vec4 &v2) -> void { m1[3] = v2; },
			sol::meta_function::addition, [](const glm::mat4 &m1, glm::mat4 &m2) -> glm::mat4 { return m1 + m2; },
			sol::meta_function::subtraction, [](const glm::mat4 &m1, glm::mat4 &m2) -> glm::mat4 { return m1 - m2; },
			sol::meta_function::multiplication, [](const glm::mat4 &m1, glm::mat4 &m2) -> glm::mat4 { return m1 * m2; },
			sol::meta_function::division, [](const glm::mat4 &m1, glm::mat4 &m2) -> glm::mat4 { return m1 / m2; });

		m_luaState.new_usertype<SpatialData>("SpatialData",
			"clear", &SpatialData::clear,
			"m_position", &SpatialData::m_position,
			"m_rotationEuler", &SpatialData::m_rotationEuler,
			"m_scale", &SpatialData::m_scale);

		m_luaState.new_usertype<SpatialTransformData>("SpatialTransformData",
			"clear", &SpatialTransformData::clear,
			"m_spatialData", &SpatialTransformData::m_spatialData);

		m_luaState.new_usertype<SpatialDataManager>("SpatialDataManager",
			"update", &SpatialDataManager::update,
			"getLocalSpaceData", &SpatialDataManager::getLocalSpaceData,
			"getLocalTransform", &SpatialDataManager::getLocalTransform,
			"getParemtTransform", &SpatialDataManager::getParemtTransform,
			"getWorldTransform", &SpatialDataManager::getWorldTransform,
			"setLocalPosition", &SpatialDataManager::setLocalPosition,
			"setLocalRotationEuler", sol::resolve<const void (const glm::vec3)>(&SpatialDataManager::setLocalRotation),
			"setLocalRotationQuat", sol::resolve<const void(const glm::quat)>(&SpatialDataManager::setLocalRotation),
			"setLocalScale", &SpatialDataManager::setLocalScale,
			"setLocalTransform", &SpatialDataManager::setLocalTransform,
			"setParentTransform", &SpatialDataManager::setParentTransform,
			"setWorldTransform", &SpatialDataManager::setWorldTransform);

		// Input types
		m_luaState.new_usertype<Window::MouseInfo>("MouseInfo",
			"m_movementCurrentFrameX", &Window::MouseInfo::m_movementCurrentFrameX,
			"m_movementCurrentFrameY", &Window::MouseInfo::m_movementCurrentFrameY,
			"m_movementPrevFrameX", &Window::MouseInfo::m_movementPrevFrameX,
			"m_movementPrevFrameY", &Window::MouseInfo::m_movementPrevFrameY,
			"m_wheelX", &Window::MouseInfo::m_wheelX,
			"m_wheelY", &Window::MouseInfo::m_wheelY,
			"m_movementX", &Window::MouseInfo::m_movementX,
			"m_movementY", &Window::MouseInfo::m_movementY);

		m_luaState.new_usertype<KeyCommand>("KeyCommand",
			"activate", &KeyCommand::activate,
			"deactivate", &KeyCommand::deactivate,
			"isActivated", &KeyCommand::isActivated,
			"bind", sol::resolve<void(const Scancode)>(&KeyCommand::bind),
			"bindByName", sol::resolve<void(const std::string&)>(&KeyCommand::bind),
			"unbind", &KeyCommand::unbind,
			"unbindAll", &KeyCommand::unbindAll);

		// Misc types
		m_luaState.new_usertype<Conditional>("Conditional",
			"isChecked", &Conditional::isChecked,
			"check", &Conditional::check,
			"uncheck", &Conditional::uncheck,
			"set", &Conditional::set);
	}

	// Creates and assigns objects to be used in the lua script
	// Should only be called from the lua script
	void createObjectInLua(const unsigned int p_objectType, const std::string p_variableName)
	{
		// Check if the object type is in a valid range and if the variable name isn't empty
		if(p_objectType >= 0 && p_objectType < LuaDefinitions::UserTypes::NumOfTypes && !p_variableName.empty())
		{
			switch(p_objectType)
			{
			case LuaDefinitions::Conditional:
			{
				// Create a new Conditional
				Conditional *newConditional = new Conditional();

				// Set the given variable name in Lua to point to the created Conditional
				m_luaState.set(p_variableName, newConditional);

				// Add the conditional pointer to an array so it is not lost
				m_conditionals.push_back(newConditional);
			}
				break;

			case LuaDefinitions::EngineVariables:

				// Set the given variable name in Lua to point to the EngineVariables object
				m_luaState.set(p_variableName, Config::engineVar());

				break;

			case LuaDefinitions::GameplayVariables:

				// Set the given variable name in Lua to point to the GameplayVariables object
				m_luaState.set(p_variableName, Config::gameplayVar());

				break;

			case LuaDefinitions::InputVariables:

				// Set the given variable name in Lua to point to the InputVariables object
				m_luaState.set(p_variableName, Config::inputVar());

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

			case LuaDefinitions::MouseInfo:

				// Set the given variable name in Lua to point to the MouseInfo object
				m_luaState.set(p_variableName, WindowLocator::get().getMouseInfo());

				break;

			case LuaDefinitions::SpatialDataManager:

				// Set the given variable name in Lua to point to the Spatial Data Manager object
				m_luaState.set(p_variableName, &m_spatialData);

				break;

			case LuaDefinitions::WindowVariables:

				// Set the given variable name in Lua to point to the WindowVariables object
				m_luaState.set(p_variableName, Config::windowVar());

				break;
			default:
				break;
			}
		}
	}

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