
#include "ComponentConstructorInfo.h"
#include "LuaScript.h"

namespace LuaDefinitions
{
	DEFINE_ENUM(UserTypes, LUA_USER_TYPES)

	DEFINE_ENUM(SpatialChanges, LUA_SPATIAL_CHANGES)
}

void LuaScript::setFunctions()
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

	// Entity component functions
	m_luaState.set_function("createEntity", [this](const ComponentsConstructionInfo &p_constructionInfo) -> EntityID { return static_cast<WorldScene *>(m_scriptScene->getSceneLoader()->getSystemScene(Systems::World))->createEntity(p_constructionInfo); });
	m_luaState.set_function("importPrefab", [this](ComponentsConstructionInfo &p_constructionInfo, const std::string &p_filename) -> bool { return m_scriptScene->getSceneLoader()->importPrefab(p_constructionInfo, p_filename) == ErrorCode::Success; });

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

void LuaScript::setUsertypes()
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
		sol::meta_function::addition, [](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 + v2; },
		sol::meta_function::subtraction, [](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 - v2; },
		sol::meta_function::multiplication, [](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 * v2; },
		sol::meta_function::division, [](const glm::vec3 &v1, glm::vec3 &v2) -> glm::vec3 { return v1 / v2; });

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
		"m_rotationQuaternion", &SpatialData::m_rotationQuat,
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
		"setLocalRotationEuler", sol::resolve<const void(const glm::vec3)>(&SpatialDataManager::setLocalRotation),
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
		"bindByName", sol::resolve<void(const std::string &)>(&KeyCommand::bind),
		"unbind", &KeyCommand::unbind,
		"unbindAll", &KeyCommand::unbindAll);

	// Component construction info
	m_luaState.new_usertype<ComponentsConstructionInfo>("ComponentsConstructionInfo",
		sol::meta_function::garbage_collect, sol::destructor(&ComponentsConstructionInfo::deleteConstructionInfo),
		"m_name", &ComponentsConstructionInfo::m_name,
		"m_id", &ComponentsConstructionInfo::m_id,
		"m_parent", &ComponentsConstructionInfo::m_parent,
		"m_graphicsComponents", &ComponentsConstructionInfo::m_graphicsComponents,
		"m_guiComponents", &ComponentsConstructionInfo::m_guiComponents,
		"m_physicsComponents", &ComponentsConstructionInfo::m_physicsComponents,
		"m_scriptComponents", &ComponentsConstructionInfo::m_scriptComponents,
		"m_worldComponents", &ComponentsConstructionInfo::m_worldComponents);

	m_luaState.new_usertype<GraphicsComponentsConstructionInfo>("GraphicsComponentsConstructionInfo",
		sol::meta_function::garbage_collect, sol::destructor(&GraphicsComponentsConstructionInfo::deleteConstructionInfo),
		"m_cameraConstructionInfo", &GraphicsComponentsConstructionInfo::m_cameraConstructionInfo,
		"m_lightConstructionInfo", &GraphicsComponentsConstructionInfo::m_lightConstructionInfo,
		"m_modelConstructionInfo", &GraphicsComponentsConstructionInfo::m_modelConstructionInfo,
		"m_shaderConstructionInfo", &GraphicsComponentsConstructionInfo::m_shaderConstructionInfo,
		"cameraPresent", [](const GraphicsComponentsConstructionInfo &c1) -> bool { return c1.m_cameraConstructionInfo != nullptr; },
		"lightPresent", [](const GraphicsComponentsConstructionInfo &c1) -> bool { return c1.m_lightConstructionInfo != nullptr; },
		"modelPresent", [](const GraphicsComponentsConstructionInfo &c1) -> bool { return c1.m_modelConstructionInfo != nullptr; },
		"shaderPresent", [](const GraphicsComponentsConstructionInfo &c1) -> bool { return c1.m_shaderConstructionInfo != nullptr; },
		"createCamera", [](GraphicsComponentsConstructionInfo &c1) -> void { if(c1.m_cameraConstructionInfo != nullptr) delete c1.m_cameraConstructionInfo; c1.m_cameraConstructionInfo = new CameraComponent::CameraComponentConstructionInfo(); },
		"createLight", [](GraphicsComponentsConstructionInfo &c1) -> void { if(c1.m_lightConstructionInfo != nullptr) delete c1.m_lightConstructionInfo; c1.m_lightConstructionInfo = new LightComponent::LightComponentConstructionInfo(); },
		"createModel", [](GraphicsComponentsConstructionInfo &c1) -> void { if(c1.m_modelConstructionInfo != nullptr) delete c1.m_modelConstructionInfo; c1.m_modelConstructionInfo = new ModelComponent::ModelComponentConstructionInfo(); },
		"createShader", [](GraphicsComponentsConstructionInfo &c1) -> void { if(c1.m_shaderConstructionInfo != nullptr) delete c1.m_shaderConstructionInfo; c1.m_shaderConstructionInfo = new ShaderComponent::ShaderComponentConstructionInfo(); });

	m_luaState.new_usertype<GUIComponentsConstructionInfo>("GUIComponentsConstructionInfo",
		sol::meta_function::garbage_collect, sol::destructor(&GUIComponentsConstructionInfo::deleteConstructionInfo),
		"m_guiSequenceConstructionInfo", &GUIComponentsConstructionInfo::m_guiSequenceConstructionInfo,
		"guiSequencePresent", [](const GUIComponentsConstructionInfo &c1) -> bool { return c1.m_guiSequenceConstructionInfo != nullptr; },
		"createGUISequence", [](GUIComponentsConstructionInfo &c1) -> void { if(c1.m_guiSequenceConstructionInfo != nullptr) delete c1.m_guiSequenceConstructionInfo; c1.m_guiSequenceConstructionInfo = new GUISequenceComponent::GUISequenceComponentConstructionInfo(); });

	m_luaState.new_usertype<PhysicsComponentsConstructionInfo>("PhysicsComponentsConstructionInfo",
		sol::meta_function::garbage_collect, sol::destructor(&PhysicsComponentsConstructionInfo::deleteConstructionInfo),
		"m_rigidBodyConstructionInfo", &PhysicsComponentsConstructionInfo::m_rigidBodyConstructionInfo,
		"rigidBodyPresent", [](const PhysicsComponentsConstructionInfo &c1) -> bool { return c1.m_rigidBodyConstructionInfo != nullptr; },
		"createRigidBody", [](PhysicsComponentsConstructionInfo &c1) -> void { if(c1.m_rigidBodyConstructionInfo != nullptr) delete c1.m_rigidBodyConstructionInfo; c1.m_rigidBodyConstructionInfo = new RigidBodyComponent::RigidBodyComponentConstructionInfo(); });

	m_luaState.new_usertype<ScriptComponentsConstructionInfo>("ScriptComponentsConstructionInfo",
		sol::meta_function::garbage_collect, sol::destructor(&ScriptComponentsConstructionInfo::deleteConstructionInfo),
		"m_luaConstructionInfo", &ScriptComponentsConstructionInfo::m_luaConstructionInfo,
		"luaPresent", [](const ScriptComponentsConstructionInfo &c1) -> bool { return c1.m_luaConstructionInfo != nullptr; },
		"createLua", [](ScriptComponentsConstructionInfo &c1) -> void { if(c1.m_luaConstructionInfo != nullptr) delete c1.m_luaConstructionInfo; c1.m_luaConstructionInfo = new LuaComponent::LuaComponentConstructionInfo(); });

	m_luaState.new_usertype<WorldComponentsConstructionInfo>("WorldComponentsConstructionInfo",
		sol::meta_function::garbage_collect, sol::destructor(&WorldComponentsConstructionInfo::deleteConstructionInfo),
		"m_spatialConstructionInfo", &WorldComponentsConstructionInfo::m_spatialConstructionInfo,
		"spatialPresent", [](const WorldComponentsConstructionInfo &c1) -> bool { return c1.m_spatialConstructionInfo != nullptr; },
		"createSpatial", [](WorldComponentsConstructionInfo &c1) -> void { if(c1.m_spatialConstructionInfo != nullptr) delete c1.m_spatialConstructionInfo; c1.m_spatialConstructionInfo = new SpatialComponent::SpatialComponentConstructionInfo(); });

	m_luaState.new_usertype<CameraComponent::CameraComponentConstructionInfo>("CameraComponentConstructionInfo",
		"m_active", &CameraComponent::CameraComponentConstructionInfo::m_active,
		"m_name", &CameraComponent::CameraComponentConstructionInfo::m_name,
		"m_fov", &CameraComponent::CameraComponentConstructionInfo::m_fov);

	m_luaState.new_usertype<LightComponent::LightComponentConstructionInfo>("LightComponentConstructionInfo",
		"m_active", &LightComponent::LightComponentConstructionInfo::m_active,
		"m_name", &LightComponent::LightComponentConstructionInfo::m_name,
		"m_color", &LightComponent::LightComponentConstructionInfo::m_color,
		"m_direction", &LightComponent::LightComponentConstructionInfo::m_direction,
		"m_position", &LightComponent::LightComponentConstructionInfo::m_position,
		"m_intensity", &LightComponent::LightComponentConstructionInfo::m_intensity,
		"m_cutoffAngle", &LightComponent::LightComponentConstructionInfo::m_cutoffAngle,
		"m_lightComponentType", &LightComponent::LightComponentConstructionInfo::m_lightComponentType);

	m_luaState.new_usertype<ModelComponent::ModelComponentConstructionInfo>("ModelComponentConstructionInfo",
		"m_active", &ModelComponent::ModelComponentConstructionInfo::m_active,
		"m_name", &ModelComponent::ModelComponentConstructionInfo::m_name);

	m_luaState.new_usertype<ShaderComponent::ShaderComponentConstructionInfo>("ShaderComponentConstructionInfo",
		"m_active", &ShaderComponent::ShaderComponentConstructionInfo::m_active,
		"m_name", &ShaderComponent::ShaderComponentConstructionInfo::m_name,
		"m_geometryShaderFilename", &ShaderComponent::ShaderComponentConstructionInfo::m_geometryShaderFilename,
		"m_vetexShaderFilename", &ShaderComponent::ShaderComponentConstructionInfo::m_vetexShaderFilename,
		"m_fragmentShaderFilename", &ShaderComponent::ShaderComponentConstructionInfo::m_fragmentShaderFilename);

	m_luaState.new_usertype<RigidBodyComponent::RigidBodyComponentConstructionInfo>("RigidBodyComponentConstructionInfo",
		"m_active", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_active,
		"m_name", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_name,
		"m_friction", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_friction,
		"m_mass", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_mass,
		"m_restitution", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_restitution,
		"m_kinematic", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_kinematic,
		"m_linearVelocity", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_linearVelocity,
		"m_collisionShapeType", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_collisionShapeType,
		"m_collisionShapeSize", &RigidBodyComponent::RigidBodyComponentConstructionInfo::m_collisionShapeSize);

	m_luaState.new_usertype<LuaComponent::LuaComponentConstructionInfo>("LuaComponentConstructionInfo",
		"m_active", &LuaComponent::LuaComponentConstructionInfo::m_active,
		"m_name", &LuaComponent::LuaComponentConstructionInfo::m_name,
		"m_luaScriptFilename", &LuaComponent::LuaComponentConstructionInfo::m_luaScriptFilename);

	m_luaState.new_usertype<SpatialComponent::SpatialComponentConstructionInfo>("SpatialComponentConstructionInfo",
		"m_active", &SpatialComponent::SpatialComponentConstructionInfo::m_active,
		"m_name", &SpatialComponent::SpatialComponentConstructionInfo::m_name,
		"m_localPosition", &SpatialComponent::SpatialComponentConstructionInfo::m_localPosition,
		"m_localRotationEuler", &SpatialComponent::SpatialComponentConstructionInfo::m_localRotationEuler,
		"m_localRotationQuaternion", &SpatialComponent::SpatialComponentConstructionInfo::m_localRotationQuaternion,
		"m_localScale", &SpatialComponent::SpatialComponentConstructionInfo::m_localScale);

	// Misc types
	m_luaState.new_usertype<Conditional>("Conditional",
		"isChecked", &Conditional::isChecked,
		"check", &Conditional::check,
		"uncheck", &Conditional::uncheck,
		"set", &Conditional::set);
}

void LuaScript::createObjectInLua(const unsigned int p_objectType, const std::string p_variableName)
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

		case LuaDefinitions::ComponentsInfo:
		{
			// Create a new Conditional
			ComponentsConstructionInfo *newConstructionInfo = new ComponentsConstructionInfo();

			// Set the given variable name in Lua to point to the created Conditional
			m_luaState.set(p_variableName, newConstructionInfo);

			// Add the conditional pointer to an array so it is not lost
			//m_conditionals.push_back(newConstructionInfo);
		}
		break;

		case LuaDefinitions::GraphicsComponentInfo:
		{
			// Create a new Conditional
			GraphicsComponentsConstructionInfo *newConstructionInfo = new GraphicsComponentsConstructionInfo();

			// Set the given variable name in Lua to point to the created Conditional
			m_luaState.set(p_variableName, newConstructionInfo);
		}
		break;

		case LuaDefinitions::GUIComponentInfo:
		{
			// Create a new Conditional
			GUIComponentsConstructionInfo *newConstructionInfo = new GUIComponentsConstructionInfo();

			// Set the given variable name in Lua to point to the created Conditional
			m_luaState.set(p_variableName, newConstructionInfo);
		}
		break;

		case LuaDefinitions::PhysicsComponentInfo:
		{
			// Create a new Conditional
			PhysicsComponentsConstructionInfo *newConstructionInfo = new PhysicsComponentsConstructionInfo();

			// Set the given variable name in Lua to point to the created Conditional
			m_luaState.set(p_variableName, newConstructionInfo);
		}
		break;

		case LuaDefinitions::ScriptComponentInfo:
		{
			// Create a new Conditional
			ScriptComponentsConstructionInfo *newConstructionInfo = new ScriptComponentsConstructionInfo();

			// Set the given variable name in Lua to point to the created Conditional
			m_luaState.set(p_variableName, newConstructionInfo);
		}
		break;

		case LuaDefinitions::WorldComponentInfo:
		{
			// Create a new Conditional
			WorldComponentsConstructionInfo *newConstructionInfo = new WorldComponentsConstructionInfo();

			// Set the given variable name in Lua to point to the created Conditional
			m_luaState.set(p_variableName, newConstructionInfo);
		}
		break;

		default:
			break;
		}
	}
}
