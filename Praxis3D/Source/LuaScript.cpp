
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include "ComponentConstructorInfo.h"
#include "LuaScript.h"
#include "Loaders.h"

namespace LuaDefinitions
{
	DEFINE_ENUM(UserTypes, LUA_USER_TYPES)

	DEFINE_ENUM(SpatialChanges, LUA_SPATIAL_CHANGES)
}

LuaScript::~LuaScript()
{
	terminate();
}

void LuaScript::terminate()
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
	m_keyCommands.clear();

	// Delete all created conditionals
	for(decltype(m_conditionals.size()) i = 0; i < m_conditionals.size(); i++)
		delete m_conditionals[i];
	m_conditionals.clear();

	// Delete all created component construction info objects
	for(decltype(m_componentsConstructionInfo.size()) i = 0; i < m_componentsConstructionInfo.size(); i++)
	{
		m_componentsConstructionInfo[i]->deleteConstructionInfo();
		delete m_componentsConstructionInfo[i];
	}
	m_componentsConstructionInfo.clear();
}

void LuaScript::setDefinitions()
{
	// Create a table for user types that are supported by Lua scripts
	m_userTypesTable = m_luaState[Config::scriptVar().userTypeTableName].get_or_create<sol::table>();

	// Create a table for error types
	sol::table errorTypes = m_luaState["ErrorType"].get_or_create<sol::table>();
	for(unsigned int i = 0; i < ErrorType::NumberOfErrorTypes; i++)
		errorTypes[sol::update_if_empty][GetString(static_cast<ErrorType>(i))] = i;

	// Create a table for error codes
	sol::table errorCodes = m_luaState["ErrorCode"].get_or_create<sol::table>();
	for(unsigned int i = 0; i < ErrorCode::NumberOfErrorCodes; i++)
		errorCodes[sol::update_if_empty][GetString(static_cast<ErrorCode>(i))] = i;

	// Create a table for error sources
	sol::table errorSources = m_luaState["ErrorSource"].get_or_create<sol::table>();
	for(unsigned int i = 0; i < ErrorSource::Source_NumberOfErrorSources; i++)
		errorSources[sol::update_if_empty][GetString(static_cast<ErrorSource>(i))] = i;

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

	// Create a table for ImGUI color flags
	sol::table imGuiColFlag = m_luaState["ImGuiCol"].get_or_create<sol::table>();

	imGuiColFlag[sol::update_if_empty]["Text"] = ImGuiCol_::ImGuiCol_Text;
	imGuiColFlag[sol::update_if_empty]["TextDisabled"] = ImGuiCol_::ImGuiCol_TextDisabled;
	imGuiColFlag[sol::update_if_empty]["WindowBg"] = ImGuiCol_::ImGuiCol_WindowBg;
	imGuiColFlag[sol::update_if_empty]["ChildBg"] = ImGuiCol_::ImGuiCol_ChildBg;
	imGuiColFlag[sol::update_if_empty]["PopupBg"] = ImGuiCol_::ImGuiCol_PopupBg;
	imGuiColFlag[sol::update_if_empty]["Border"] = ImGuiCol_::ImGuiCol_Border;
	imGuiColFlag[sol::update_if_empty]["BorderShadow"] = ImGuiCol_::ImGuiCol_BorderShadow;
	imGuiColFlag[sol::update_if_empty]["FrameBg"] = ImGuiCol_::ImGuiCol_FrameBg;
	imGuiColFlag[sol::update_if_empty]["FrameBgHovered"] = ImGuiCol_::ImGuiCol_FrameBgHovered;
	imGuiColFlag[sol::update_if_empty]["FrameBgActive"] = ImGuiCol_::ImGuiCol_FrameBgActive;
	imGuiColFlag[sol::update_if_empty]["TitleBg"] = ImGuiCol_::ImGuiCol_TitleBg;
	imGuiColFlag[sol::update_if_empty]["TitleBgActive"] = ImGuiCol_::ImGuiCol_TitleBgActive;
	imGuiColFlag[sol::update_if_empty]["TitleBgCollapsed"] = ImGuiCol_::ImGuiCol_TitleBgCollapsed;
	imGuiColFlag[sol::update_if_empty]["MenuBarBg"] = ImGuiCol_::ImGuiCol_MenuBarBg;
	imGuiColFlag[sol::update_if_empty]["ScrollbarBg"] = ImGuiCol_::ImGuiCol_ScrollbarBg;
	imGuiColFlag[sol::update_if_empty]["ScrollbarGrab"] = ImGuiCol_::ImGuiCol_ScrollbarGrab;
	imGuiColFlag[sol::update_if_empty]["ScrollbarGrabHovered"] = ImGuiCol_::ImGuiCol_ScrollbarGrabHovered;
	imGuiColFlag[sol::update_if_empty]["ScrollbarGrabActive"] = ImGuiCol_::ImGuiCol_ScrollbarGrabActive;
	imGuiColFlag[sol::update_if_empty]["CheckMark"] = ImGuiCol_::ImGuiCol_CheckMark;
	imGuiColFlag[sol::update_if_empty]["SliderGrab"] = ImGuiCol_::ImGuiCol_SliderGrab;
	imGuiColFlag[sol::update_if_empty]["SliderGrabActive"] = ImGuiCol_::ImGuiCol_SliderGrabActive;
	imGuiColFlag[sol::update_if_empty]["Button"] = ImGuiCol_::ImGuiCol_Button;
	imGuiColFlag[sol::update_if_empty]["ButtonHovered"] = ImGuiCol_::ImGuiCol_ButtonHovered;
	imGuiColFlag[sol::update_if_empty]["ButtonActive"] = ImGuiCol_::ImGuiCol_ButtonActive;
	imGuiColFlag[sol::update_if_empty]["Header"] = ImGuiCol_::ImGuiCol_Header;
	imGuiColFlag[sol::update_if_empty]["HeaderHovered"] = ImGuiCol_::ImGuiCol_HeaderHovered;
	imGuiColFlag[sol::update_if_empty]["HeaderActive"] = ImGuiCol_::ImGuiCol_HeaderActive;
	imGuiColFlag[sol::update_if_empty]["Separator"] = ImGuiCol_::ImGuiCol_Separator;
	imGuiColFlag[sol::update_if_empty]["SeparatorHovered"] = ImGuiCol_::ImGuiCol_SeparatorHovered;
	imGuiColFlag[sol::update_if_empty]["SeparatorActive"] = ImGuiCol_::ImGuiCol_SeparatorActive;
	imGuiColFlag[sol::update_if_empty]["ResizeGrip"] = ImGuiCol_::ImGuiCol_ResizeGrip;
	imGuiColFlag[sol::update_if_empty]["ResizeGripHovered"] = ImGuiCol_::ImGuiCol_ResizeGripHovered;
	imGuiColFlag[sol::update_if_empty]["ResizeGripActive"] = ImGuiCol_::ImGuiCol_ResizeGripActive;
	imGuiColFlag[sol::update_if_empty]["Tab"] = ImGuiCol_::ImGuiCol_Tab;
	imGuiColFlag[sol::update_if_empty]["TabHovered"] = ImGuiCol_::ImGuiCol_TabHovered;
	imGuiColFlag[sol::update_if_empty]["TabActive"] = ImGuiCol_::ImGuiCol_TabActive;
	imGuiColFlag[sol::update_if_empty]["TabUnfocused"] = ImGuiCol_::ImGuiCol_TabUnfocused;
	imGuiColFlag[sol::update_if_empty]["TabUnfocusedActive"] = ImGuiCol_::ImGuiCol_TabUnfocusedActive;
	imGuiColFlag[sol::update_if_empty]["PlotLines"] = ImGuiCol_::ImGuiCol_PlotLines;
	imGuiColFlag[sol::update_if_empty]["PlotLinesHovered"] = ImGuiCol_::ImGuiCol_PlotLinesHovered;
	imGuiColFlag[sol::update_if_empty]["PlotHistogram"] = ImGuiCol_::ImGuiCol_PlotHistogram;
	imGuiColFlag[sol::update_if_empty]["PlotHistogramHovered"] = ImGuiCol_::ImGuiCol_PlotHistogramHovered;
	imGuiColFlag[sol::update_if_empty]["TableHeaderBg"] = ImGuiCol_::ImGuiCol_TableHeaderBg;
	imGuiColFlag[sol::update_if_empty]["TableBorderStrong"] = ImGuiCol_::ImGuiCol_TableBorderStrong;
	imGuiColFlag[sol::update_if_empty]["TableBorderLight"] = ImGuiCol_::ImGuiCol_TableBorderLight;
	imGuiColFlag[sol::update_if_empty]["TableRowBg"] = ImGuiCol_::ImGuiCol_TableRowBg;
	imGuiColFlag[sol::update_if_empty]["TableRowBgAlt"] = ImGuiCol_::ImGuiCol_TableRowBgAlt;
	imGuiColFlag[sol::update_if_empty]["TextSelectedBg"] = ImGuiCol_::ImGuiCol_TextSelectedBg;
	imGuiColFlag[sol::update_if_empty]["DragDropTarget"] = ImGuiCol_::ImGuiCol_DragDropTarget;
	imGuiColFlag[sol::update_if_empty]["NavHighlight"] = ImGuiCol_::ImGuiCol_NavHighlight;
	imGuiColFlag[sol::update_if_empty]["NavWindowingHighlight"] = ImGuiCol_::ImGuiCol_NavWindowingHighlight;
	imGuiColFlag[sol::update_if_empty]["NavWindowingDimBg"] = ImGuiCol_::ImGuiCol_NavWindowingDimBg;
	imGuiColFlag[sol::update_if_empty]["ModalWindowDimBg"] = ImGuiCol_::ImGuiCol_ModalWindowDimBg;

	// Create a table for ImGUI file dialog flags
	sol::table imGuiFileDialogFlag = m_luaState["ImGuiStyleVar"].get_or_create<sol::table>();

	imGuiFileDialogFlag[sol::update_if_empty]["ConfirmOverwrite"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_ConfirmOverwrite;
	imGuiFileDialogFlag[sol::update_if_empty]["DontShowHiddenFiles"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_DontShowHiddenFiles;
	imGuiFileDialogFlag[sol::update_if_empty]["DisableCreateDirectoryButton"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_DisableCreateDirectoryButton;
	imGuiFileDialogFlag[sol::update_if_empty]["HideColumnType"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_HideColumnType;
	imGuiFileDialogFlag[sol::update_if_empty]["HideColumnSize"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_HideColumnSize;
	imGuiFileDialogFlag[sol::update_if_empty]["HideColumnDate"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_HideColumnDate;
	imGuiFileDialogFlag[sol::update_if_empty]["NoDialog"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_NoDialog;
	imGuiFileDialogFlag[sol::update_if_empty]["ReadOnlyFileNameField"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_ReadOnlyFileNameField;
	imGuiFileDialogFlag[sol::update_if_empty]["CaseInsensitiveExtention"] = ImGuiFileDialogFlags_::ImGuiFileDialogFlags_CaseInsensitiveExtention;

	// Create a table for ImGUI style flags
	sol::table imGuiStyleFlag = m_luaState["ImGuiStyleVar"].get_or_create<sol::table>();

	imGuiStyleFlag[sol::update_if_empty]["Alpha"] = ImGuiStyleVar_::ImGuiStyleVar_Alpha;
	imGuiStyleFlag[sol::update_if_empty]["DisabledAlpha"] = ImGuiStyleVar_::ImGuiStyleVar_DisabledAlpha;
	imGuiStyleFlag[sol::update_if_empty]["WindowPadding"] = ImGuiStyleVar_::ImGuiStyleVar_WindowPadding;
	imGuiStyleFlag[sol::update_if_empty]["WindowRounding"] = ImGuiStyleVar_::ImGuiStyleVar_WindowRounding;
	imGuiStyleFlag[sol::update_if_empty]["WindowBorderSize"] = ImGuiStyleVar_::ImGuiStyleVar_WindowBorderSize;
	imGuiStyleFlag[sol::update_if_empty]["WindowMinSize"] = ImGuiStyleVar_::ImGuiStyleVar_WindowMinSize;
	imGuiStyleFlag[sol::update_if_empty]["WindowTitleAlign"] = ImGuiStyleVar_::ImGuiStyleVar_WindowTitleAlign;
	imGuiStyleFlag[sol::update_if_empty]["ChildRounding"] = ImGuiStyleVar_::ImGuiStyleVar_ChildRounding;
	imGuiStyleFlag[sol::update_if_empty]["ChildBorderSize"] = ImGuiStyleVar_::ImGuiStyleVar_ChildBorderSize;
	imGuiStyleFlag[sol::update_if_empty]["PopupRounding"] = ImGuiStyleVar_::ImGuiStyleVar_PopupRounding;
	imGuiStyleFlag[sol::update_if_empty]["PopupBorderSize"] = ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize;
	imGuiStyleFlag[sol::update_if_empty]["FramePadding"] = ImGuiStyleVar_::ImGuiStyleVar_FramePadding;
	imGuiStyleFlag[sol::update_if_empty]["FrameRounding"] = ImGuiStyleVar_::ImGuiStyleVar_FrameRounding;
	imGuiStyleFlag[sol::update_if_empty]["FrameBorderSize"] = ImGuiStyleVar_::ImGuiStyleVar_FrameBorderSize;
	imGuiStyleFlag[sol::update_if_empty]["ItemSpacing"] = ImGuiStyleVar_::ImGuiStyleVar_ItemSpacing;
	imGuiStyleFlag[sol::update_if_empty]["ItemInnerSpacing"] = ImGuiStyleVar_::ImGuiStyleVar_ItemInnerSpacing;
	imGuiStyleFlag[sol::update_if_empty]["IndentSpacing"] = ImGuiStyleVar_::ImGuiStyleVar_IndentSpacing;
	imGuiStyleFlag[sol::update_if_empty]["CellPadding"] = ImGuiStyleVar_::ImGuiStyleVar_CellPadding;
	imGuiStyleFlag[sol::update_if_empty]["ScrollbarSize"] = ImGuiStyleVar_::ImGuiStyleVar_ScrollbarSize;
	imGuiStyleFlag[sol::update_if_empty]["ScrollbarRounding"] = ImGuiStyleVar_::ImGuiStyleVar_ScrollbarRounding;
	imGuiStyleFlag[sol::update_if_empty]["GrabMinSize"] = ImGuiStyleVar_::ImGuiStyleVar_GrabMinSize;
	imGuiStyleFlag[sol::update_if_empty]["GrabRounding"] = ImGuiStyleVar_::ImGuiStyleVar_GrabRounding;
	imGuiStyleFlag[sol::update_if_empty]["TabRounding"] = ImGuiStyleVar_::ImGuiStyleVar_TabRounding;
	imGuiStyleFlag[sol::update_if_empty]["ButtonTextAlign"] = ImGuiStyleVar_::ImGuiStyleVar_ButtonTextAlign;
	imGuiStyleFlag[sol::update_if_empty]["SelectableTextAlign"] = ImGuiStyleVar_::ImGuiStyleVar_SelectableTextAlign;

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

void LuaScript::setFunctions()
{
	// Error handler functions
	auto errorTable = m_luaState.create_table("ErrHandlerLoc");
	errorTable.set_function("log", sol::overload(
		[this](const ErrorCode p_v1) -> const void { ErrHandlerLoc::get().log(p_v1); },
		[this](const ErrorCode p_v1, const ErrorSource p_v2) -> const void { ErrHandlerLoc::get().log(p_v1, p_v2); },
		[this](const ErrorCode p_v1, const std::string p_v2, const ErrorSource p_v3) -> const void { ErrHandlerLoc::get().log(p_v1, p_v2, p_v3); }));
	errorTable.set_function("logErrorType", sol::overload(
		[this](const ErrorType p_v1, const std::string p_v2) -> const void { ErrHandlerLoc::get().log(p_v1, ErrorSource::Source_LuaScript, p_v2); },
		[this](const ErrorType p_v1, const ErrorSource p_v2, const std::string p_v3) -> const void { ErrHandlerLoc::get().log(p_v1, p_v2, p_v3); }));
	errorTable.set_function("logErrorCode", sol::overload(
		[this](const ErrorCode p_v1, const std::string p_v2) -> const void { ErrHandlerLoc::get().log(p_v1, ErrorSource::Source_LuaScript, p_v2); },
		[this](const ErrorCode p_v1, const ErrorSource p_v2, const std::string p_v3) -> const void { ErrHandlerLoc::get().log(p_v1, p_v2, p_v3); }));

	// Math functions
	m_luaState.set_function("toRadianF", sol::resolve<float(const float)>(&glm::radians));
	m_luaState.set_function("toRadianVec3", sol::resolve<glm::vec3(const glm::vec3 &)>(&glm::radians));
	m_luaState.set_function("toRadianVec4", sol::resolve<glm::vec4(const glm::vec4 &)>(&glm::radians));
	m_luaState.set_function("toDegreesF", sol::resolve<float(const float)>(&glm::degrees));
	m_luaState.set_function("toDegreesVec3", sol::resolve<glm::vec3(const glm::vec3 &)>(&glm::degrees));
	m_luaState.set_function("toDegreesVec4", sol::resolve<glm::vec4(const glm::vec4 &)>(&glm::degrees));
	m_luaState.set_function("angleAxisQuat", sol::resolve<glm::quat(const float &, const glm::vec3 &)>(&glm::angleAxis));
	m_luaState.set_function("bitwiseOr", sol::overload([this](const int p_v1, const int p_v2) -> const int { return p_v1 | p_v2; },
		[this](const int p_v1, const int p_v2, const int p_v3) -> const int { return p_v1 | p_v2 | p_v3; },
		[this](const int p_v1, const int p_v2, const int p_v3, const int p_v4) -> const int { return p_v1 | p_v2 | p_v3 | p_v4; },
		[this](const int p_v1, const int p_v2, const int p_v3, const int p_v4, const int p_v5) -> const int { return p_v1 | p_v2 | p_v3 | p_v4 | p_v5; }));

	// Input / Window functions
	m_luaState.set_function("getMouseInfo", []() -> const Window::MouseInfo { return WindowLocator::get().getMouseInfo(); });
	m_luaState.set_function("getMouseCapture", []() -> const bool { return Config::windowVar().mouse_captured; });
	m_luaState.set_function("setFullscreen", [](const bool p_v1) -> const void { WindowLocator::get().setFullscreen(p_v1); });
	m_luaState.set_function("setMouseCapture", [](const bool p_v1) -> const void { WindowLocator::get().setMouseCapture(p_v1); });
	m_luaState.set_function("setVerticalSync", [](const bool p_v1) -> const void { WindowLocator::get().setVerticalSync(p_v1); });
	m_luaState.set_function("setWindowTitle", [](const std::string &p_v1) -> const void { WindowLocator::get().setWindowTitle(p_v1); });

	// Entity component functions
	m_luaState.set_function("createEntity", [this](const ComponentsConstructionInfo &p_constructionInfo) -> EntityID { return static_cast<WorldScene *>(m_scriptScene->getSceneLoader()->getSystemScene(Systems::World))->createEntity(p_constructionInfo); });
	m_luaState.set_function("importPrefab", [this](ComponentsConstructionInfo &p_constructionInfo, const std::string &p_filename) -> bool { return m_scriptScene->getSceneLoader()->importPrefab(p_constructionInfo, p_filename) == ErrorCode::Success; });

	// Engine functions
	m_luaState.set_function("setEngineRunning", [](const bool p_v1) -> const void {Config::m_engineVar.running = p_v1; });
	m_luaState.set_function("setEngineState", [this](const EngineStateType p_v1) -> const void { m_scriptScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_StateChange, p_v1)); });

	m_luaState.set_function("sendEngineChange", sol::overload([this](const EngineChangeType p_v1) -> const void { m_scriptScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(p_v1)); },
		[this](const EngineChangeType p_v1, const EngineStateType p_v2) -> const void { m_scriptScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(p_v1, p_v2)); },
		[this](const EngineChangeType p_v1, const EngineStateType p_v2, const std::string p_v3) -> const void { m_scriptScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(p_v1, p_v2, p_v3)); }));

	// GUI functions
	auto GUITable = m_luaState.create_table("GUI");
	GUITable.set_function("Begin", sol::overload([this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::Begin(p_v1.c_str()); }); },
		[this](const std::string &p_v1, const int p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::Begin(p_v1.c_str(), 0, p_v2); }); },
		[this](const std::string &p_v1, Conditional *p_v2, const int p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::Begin(p_v1.c_str(), &p_v2->m_flag, p_v3); }); }));
	GUITable.set_function("BeginChild", [this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::BeginChild(p_v1.c_str()); }); });
	GUITable.set_function("BeginMenu", [this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::BeginMenu(p_v1.c_str()); }); });
	GUITable.set_function("Button", sol::overload([this](const std::string &p_v1, Conditional *p_v2) -> void { m_GUIData.addFunctor([=] { p_v2->m_flag = ImGui::Button(p_v1.c_str()); }); },
		[this](const std::string &p_v1, const float p_v2, const float p_v3, Conditional *p_v4) -> void { m_GUIData.addFunctor([=] { p_v4->m_flag = ImGui::Button(p_v1.c_str(), ImVec2(p_v2, p_v3)); }); }));
	GUITable.set_function("Checkbox", [this](const std::string &p_v1, Conditional *p_v2) -> void { m_GUIData.addFunctor([=] { ImGui::Checkbox(p_v1.c_str(), &p_v2->m_flag); }); });
	GUITable.set_function("ColorEdit3", [this](const std::string &p_v1, glm::vec3 *p_v2) -> void { m_GUIData.addFunctor([=] { ImGui::ColorEdit3(p_v1.c_str(), &(p_v2->x)); }); });
	GUITable.set_function("ColorEdit4", [this](const std::string &p_v1, glm::vec4 *p_v2) -> void { m_GUIData.addFunctor([=] { ImGui::ColorEdit4(p_v1.c_str(), &(p_v2->x)); }); });
	GUITable.set_function("End", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::End(); }); });
	GUITable.set_function("EndChild", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::EndChild(); }); });
	GUITable.set_function("EndMenu", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::EndMenu(); }); });
	GUITable.set_function("EndMenuBar", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::EndMenuBar(); }); });
	GUITable.set_function("FileDialog", [this](FileBrowserDialog *p_v1) -> const void { m_scriptScene->getSceneLoader()->getChangeController()->sendData(m_scriptScene->getSceneLoader()->getSystemScene(Systems::GUI), DataType::DataType_FileBrowserDialog, (void *)p_v1); });
	GUITable.set_function("Image", sol::overload([this](TextureLoader2D::Texture2DHandle &p_v1) -> void { m_GUIData.addFunctor([=] { ImGui::Image((ImTextureID)(uint64_t)p_v1.getHandle(), ImVec2((float)p_v1.getTextureWidth(), (float)p_v1.getTextureHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)); }); },
		[this](TextureLoader2D::Texture2DHandle &p_v1, const float p_v2, const float p_v3) -> void { m_GUIData.addFunctor([=] { ImGui::Image((ImTextureID)(uint64_t)p_v1.getHandle(), ImVec2(p_v2, p_v3), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)); }); },
		[this](TextureLoader2D::Texture2DHandle &p_v1, const float p_v2, const float p_v3, const float p_v4, const float p_v5, const float p_v6, const float p_v7) -> void { m_GUIData.addFunctor([=] { ImGui::Image((ImTextureID)(uint64_t)p_v1.getHandle(), ImVec2(p_v2, p_v3), ImVec2(p_v4, p_v5), ImVec2(p_v6, p_v7)); }); }));
	GUITable.set_function("ImageButton", sol::overload([this](TextureLoader2D::Texture2DHandle &p_v1, Conditional *p_v2) -> void { m_GUIData.addFunctor([=] { p_v2->m_flag = ImGui::ImageButton((ImTextureID)(uint64_t)p_v1.getHandle(), ImVec2((float)p_v1.getTextureWidth(), (float)p_v1.getTextureHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); }); },
		[this](TextureLoader2D::Texture2DHandle &p_v1, const float p_v2, const float p_v3, Conditional *p_v4) -> void { m_GUIData.addFunctor([=] { p_v4->m_flag = ImGui::ImageButton((ImTextureID)(uint64_t)p_v1.getHandle(), ImVec2(p_v2, p_v3), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)); }); },
		[this](TextureLoader2D::Texture2DHandle &p_v1, const float p_v2, const float p_v3, const float p_v4, const float p_v5, const float p_v6, const float p_v7, Conditional *p_v8) -> void { m_GUIData.addFunctor([=] { p_v8->m_flag = ImGui::ImageButton((ImTextureID)(uint64_t)p_v1.getHandle(), ImVec2(p_v2, p_v3), ImVec2(p_v4, p_v5), ImVec2(p_v6, p_v7)); }); }));
	GUITable.set_function("IsItemHovered", [this](Conditional *p_v1) -> const void { m_GUIData.addFunctor([=] { p_v1->m_flag = ImGui::IsItemHovered(); }); });
	GUITable.set_function("MenuItem", [this](const std::string &p_v1, const std::string &p_v2, Conditional *p_v3) -> void { m_GUIData.addFunctor([=] { p_v3->m_flag = ImGui::MenuItem(p_v1.c_str(), p_v2.c_str()); }); });
	GUITable.set_function("PlotLines", [this](const std::string &p_v1, const float *p_v2, int p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::PlotLines(p_v1.c_str(), p_v2, p_v3); }); });
	GUITable.set_function("PopStyleColor", sol::overload([this](const int p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::PopStyleColor(p_v1); }); },
		[this]() -> const void { m_GUIData.addFunctor([=] { ImGui::PopStyleColor(1); }); }));
	GUITable.set_function("PopStyleVar", sol::overload([this](const int p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::PopStyleVar(p_v1); }); },
		[this]() -> const void { m_GUIData.addFunctor([=] { ImGui::PopStyleVar(1); }); }));
	GUITable.set_function("PushStyleColor", [this](const int p_v1, const float p_v2, const float p_v3, const float p_v4, const float p_v5) -> const void { m_GUIData.addFunctor([=] { ImGui::PushStyleColor(p_v1, ImVec4(p_v2, p_v3, p_v4, p_v5)); }); });
	GUITable.set_function("PushStyleVar", sol::overload([this](const int p_v1, const float p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::PushStyleVar(p_v1, p_v2); }); },
		[this](const int p_v1, const float p_v2, const float p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::PushStyleVar(p_v1, ImVec2(p_v2, p_v3)); }); }));
	GUITable.set_function("SetNextWindowPos", [this](const float p_v1, const float p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::SetNextWindowPos(ImVec2(p_v1, p_v2)); }); });
	GUITable.set_function("SetNextWindowContentSize", [this](const float p_v1, const float p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::SetNextWindowContentSize(ImVec2(p_v1, p_v2)); }); });
	GUITable.set_function("SetNextWindowSize", [this](const float p_v1, const float p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::SetNextWindowSize(ImVec2(p_v1, p_v2)); }); });
	GUITable.set_function("ShowMetricsWindow", [this](bool p_v1) -> void { m_GUIData.addFunctor([=] { bool open = p_v1; ImGui::ShowMetricsWindow(&open); }); });
	GUITable.set_function("SliderFloat", [this](const std::string &p_v1, float *p_v2, const float p_v3, const float p_v4) -> const void { m_GUIData.addFunctor([=] { ImGui::SliderFloat(p_v1.c_str(), p_v2, p_v3, p_v4); }); });
	GUITable.set_function("SameLine", [this]() -> const void { m_GUIData.addFunctor([=] { ImGui::SameLine(); }); });
	GUITable.set_function("Text", sol::overload([this](const std::string &p_v1) -> const void { m_GUIData.addFunctor([=] { ImGui::Text(p_v1.c_str()); }); },
		[this](const std::string &p_v1, const float p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::Text(p_v1.c_str(), p_v2); }); },
		[this](const std::string &p_v1, const float p_v2, const float p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::Text(p_v1.c_str(), p_v2, p_v3); }); }));
	GUITable.set_function("TextColored", sol::overload([this](const glm::vec4 p_v1, const std::string &p_v2) -> const void { m_GUIData.addFunctor([=] { ImGui::TextColored(ImVec4(p_v1.x, p_v1.y, p_v1.z, p_v1.w), p_v2.c_str()); }); },
		[this](const glm::vec4 p_v1, const std::string &p_v2, const float p_v3) -> const void { m_GUIData.addFunctor([=] { ImGui::TextColored(ImVec4(p_v1.x, p_v1.y, p_v1.z, p_v1.w), p_v2.c_str(), p_v3); }); },
		[this](const glm::vec4 p_v1, const std::string &p_v2, const float p_v3, const float p_v4) -> const void { m_GUIData.addFunctor([=] { ImGui::TextColored(ImVec4(p_v1.x, p_v1.y, p_v1.z, p_v1.w), p_v2.c_str(), p_v3, p_v4); }); }));

	// Loader functions
	m_luaState.set_function("loadTexture2D", [](const std::string &p_v1) -> TextureLoader2D::Texture2DHandle { return Loaders::texture2D().load(p_v1); });

	// LuaScript callbacks
	m_luaState.set_function("getLuaFilename", &LuaScript::getLuaScriptFilename, this);
	m_luaState.set_function("postChanges", &LuaScript::registerChange, this);
	m_luaState.set_function(Config::scriptVar().createObjectFunctionName, &LuaScript::createObjectInLua, this);
}

void LuaScript::setUsertypes()
{
	// Enums
	m_luaState.new_enum("EngineStateType",
		"MainMenu", EngineStateType::EngineStateType_MainMenu,
		"Play", EngineStateType::EngineStateType_Play,
		"Editor", EngineStateType::EngineStateType_Editor);

	m_luaState.new_enum("EngineChangeType",
		"None", EngineChangeType::EngineChangeType_None,
		"SceneFilename", EngineChangeType::EngineChangeType_SceneFilename,
		"SceneLoad", EngineChangeType::EngineChangeType_SceneLoad,
		"SceneReload", EngineChangeType::EngineChangeType_SceneReload,
		"StateChange", EngineChangeType::EngineChangeType_StateChange);

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

	m_luaState.new_usertype<Config::GraphicsVariables>("GraphicsVariables",
		"current_resolution_x", &Config::GraphicsVariables::current_resolution_x,
		"current_resolution_y", &Config::GraphicsVariables::current_resolution_y);

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

	m_luaState.new_usertype<Config::PathsVariables>("PathsVariables",
		"map_path", &Config::PathsVariables::map_path,
		"model_path", &Config::PathsVariables::model_path,
		"object_path", &Config::PathsVariables::object_path,
		"prefab_path", &Config::PathsVariables::prefab_path,
		"script_path", &Config::PathsVariables::script_path,
		"shader_path", &Config::PathsVariables::shader_path,
		"sound_path", &Config::PathsVariables::sound_path,
		"texture_path", &Config::PathsVariables::texture_path);

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
		"getParemtTransform", &SpatialDataManager::getParentTransform,
		"getWorldTransform", &SpatialDataManager::getWorldTransform,
		"setLocalPosition", &SpatialDataManager::setLocalPosition,
		"setLocalRotationEuler", sol::resolve<const void(const glm::vec3)>(&SpatialDataManager::setLocalRotation),
		"setLocalRotationQuat", sol::resolve<const void(const glm::quat)>(&SpatialDataManager::setLocalRotation),
		"setLocalScale", &SpatialDataManager::setLocalScale,
		"setLocalTransform", &SpatialDataManager::setLocalTransform,
		"setParentTransform", &SpatialDataManager::setParentTransform,
		"setWorldTransform", &SpatialDataManager::setWorldTransform,
		"calculateLocalRotationEuler", &SpatialDataManager::calculateLocalRotationEuler,
		"calculateLocalRotationQuaternion", &SpatialDataManager::calculateLocalRotationQuaternion);

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

	// Graphics types
	m_luaState.new_usertype<TextureLoader2D::Texture2DHandle>("Texture2DHandle",
		"loadToMemory", &TextureLoader2D::Texture2DHandle::loadToMemory,
		"loadToVideoMemory", [this](TextureLoader2D::Texture2DHandle &p_v1) -> void { m_scriptScene->getSceneLoader()->getChangeController()->sendData(m_scriptScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadTexture2D, (void*)&p_v1); },
		"isLoadedToMemory", &TextureLoader2D::Texture2DHandle::isLoadedToMemory,
		"isLoadedToVideoMemory", &TextureLoader2D::Texture2DHandle::isLoadedToVideoMemory,
		"getTextureHeight", &TextureLoader2D::Texture2DHandle::getTextureHeight,
		"getTextureWidth", &TextureLoader2D::Texture2DHandle::getTextureWidth,
		"getFilename", &TextureLoader2D::Texture2DHandle::getFilename,
		"getEnableMipmap", &TextureLoader2D::Texture2DHandle::getEnableMipmap,
		"getHandle", &TextureLoader2D::Texture2DHandle::getHandle);

	// GUI types
	m_luaState.new_usertype<FileBrowserDialog>("FileBrowserDialog",
		//sol::meta_function::garbage_collect, sol::destructor(),
		sol::constructors<FileBrowserDialog()>(),
		"m_name", &FileBrowserDialog::m_name,
		"m_title", &FileBrowserDialog::m_title,
		"m_filter", &FileBrowserDialog::m_filter,
		"m_rootPath", &FileBrowserDialog::m_rootPath,
		"m_definedFilename", &FileBrowserDialog::m_definedFilename,
		"m_filePath", &FileBrowserDialog::m_filePath,
		"m_filename", &FileBrowserDialog::m_filename,
		"m_filePathName", &FileBrowserDialog::m_filePathName,
		"m_numOfSelectableFiles", &FileBrowserDialog::m_numOfSelectableFiles,
		"m_flags", &FileBrowserDialog::m_flags,
		"m_opened", &FileBrowserDialog::m_opened,
		"m_closed", &FileBrowserDialog::m_closed,
		"m_success", &FileBrowserDialog::m_success,
		"reset", &FileBrowserDialog::reset);

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
			m_luaState.set(p_variableName, &Config::engineVar());

			break;

		case LuaDefinitions::GameplayVariables:

			// Set the given variable name in Lua to point to the GameplayVariables object
			m_luaState.set(p_variableName, &Config::gameplayVar());

			break;

		case LuaDefinitions::GraphicsVariables:

			// Set the given variable name in Lua to point to the GraphicsVariables object
			m_luaState.set(p_variableName, &Config::graphicsVar());

			break;

		case LuaDefinitions::InputVariables:

			// Set the given variable name in Lua to point to the InputVariables object
			m_luaState.set(p_variableName, &Config::inputVar());

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

		case LuaDefinitions::PathsVariables:

			// Set the given variable name in Lua to point to the PathsVariables object
			m_luaState.set(p_variableName, &Config::filepathVar());

			break;

		case LuaDefinitions::SpatialDataManager:

			// Set the given variable name in Lua to point to the Spatial Data Manager object
			m_luaState.set(p_variableName, &m_spatialData);

			break;

		case LuaDefinitions::WindowVariables:

			// Set the given variable name in Lua to point to the WindowVariables object
			m_luaState.set(p_variableName, &Config::windowVar());

			break;

		case LuaDefinitions::ComponentsInfo:
		{
			// Create a new object
			ComponentsConstructionInfo *newConstructionInfo = new ComponentsConstructionInfo();

			// Set the given variable name in Lua to point to the created object
			m_luaState.set(p_variableName, newConstructionInfo);

			// Add the object pointer to an array so it is not lost
			m_componentsConstructionInfo.push_back(newConstructionInfo);
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
