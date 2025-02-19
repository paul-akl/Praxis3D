#include "Systems/GUISystem/Include/GUISystem.hpp"

void GUISystem::setColorPallet(const GuiColorPallet p_colorPallet)
{
	ImGuiStyle *style = &ImGui::GetStyle();
	ImVec4 *colors = style->Colors;

	switch(p_colorPallet)
	{
		case GuiColorPallet_Default:
		default:
			// Leave the default ImGui colors
			break;
		case GuiColorPallet_Green:
			{
				colors[ImGuiCol_Text]					= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				colors[ImGuiCol_TextDisabled]			= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
				colors[ImGuiCol_WindowBg]				= ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
				colors[ImGuiCol_ChildBg]				= ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
				colors[ImGuiCol_PopupBg]				= ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
				colors[ImGuiCol_Border]					= ImVec4(0.05f, 0.05f, 0.05f, 0.50f);
				colors[ImGuiCol_BorderShadow]			= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
				colors[ImGuiCol_FrameBg]				= ImVec4(0.24f, 0.31f, 0.12f, 1.00f);
				colors[ImGuiCol_FrameBgHovered]			= ImVec4(0.42f, 0.55f, 0.21f, 0.80f);
				colors[ImGuiCol_FrameBgActive]			= ImVec4(0.42f, 0.55f, 0.21f, 1.00f);
				colors[ImGuiCol_TitleBg]				= ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
				colors[ImGuiCol_TitleBgActive]			= ImVec4(0.24f, 0.31f, 0.12f, 1.00f);
				colors[ImGuiCol_TitleBgCollapsed]		= ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
				colors[ImGuiCol_MenuBarBg]				= ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
				colors[ImGuiCol_ScrollbarBg]			= ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
				colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
				colors[ImGuiCol_ScrollbarGrabHovered]	= ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
				colors[ImGuiCol_ScrollbarGrabActive]	= ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
				colors[ImGuiCol_CheckMark]				= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				colors[ImGuiCol_SliderGrab]				= ImVec4(1.00f, 1.00f, 1.00f, 0.71f);
				colors[ImGuiCol_SliderGrabActive]		= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				colors[ImGuiCol_Button]					= ImVec4(0.24f, 0.31f, 0.12f, 1.00f);
				colors[ImGuiCol_ButtonHovered]			= ImVec4(0.42f, 0.55f, 0.21f, 0.80f);
				colors[ImGuiCol_ButtonActive]			= ImVec4(0.42f, 0.55f, 0.21f, 1.00f);
				colors[ImGuiCol_Header]					= ImVec4(0.24f, 0.31f, 0.12f, 1.00f);
				colors[ImGuiCol_HeaderHovered]			= ImVec4(0.42f, 0.55f, 0.21f, 0.80f);
				colors[ImGuiCol_HeaderActive]			= ImVec4(0.42f, 0.55f, 0.21f, 1.00f);
				colors[ImGuiCol_Separator]				= ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
				colors[ImGuiCol_SeparatorHovered]		= ImVec4(0.42f, 0.55f, 0.21f, 0.80f);
				colors[ImGuiCol_SeparatorActive]		= ImVec4(0.24f, 0.31f, 0.12f, 1.00f);
				colors[ImGuiCol_ResizeGrip]				= ImVec4(0.24f, 0.31f, 0.12f, 1.00f);
				colors[ImGuiCol_ResizeGripHovered]		= ImVec4(0.42f, 0.55f, 0.21f, 0.80f);
				colors[ImGuiCol_ResizeGripActive]		= ImVec4(0.42f, 0.55f, 0.21f, 1.00f);
				colors[ImGuiCol_Tab]					= ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
				colors[ImGuiCol_TabHovered]				= ImVec4(0.42f, 0.55f, 0.21f, 1.00f);
				colors[ImGuiCol_TabActive]				= ImVec4(0.24f, 0.31f, 0.12f, 1.00f);
				colors[ImGuiCol_TabUnfocused]			= ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
				colors[ImGuiCol_TabUnfocusedActive]		= ImVec4(0.17f, 0.27f, 0.10f, 0.78f);
				colors[ImGuiCol_DockingPreview]			= ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
				colors[ImGuiCol_DockingEmptyBg]			= ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
				colors[ImGuiCol_PlotLines]				= ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
				colors[ImGuiCol_PlotLinesHovered]		= ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
				colors[ImGuiCol_PlotHistogram]			= ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
				colors[ImGuiCol_PlotHistogramHovered]	= ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
				colors[ImGuiCol_TableHeaderBg]			= ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
				colors[ImGuiCol_TableBorderStrong]		= ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
				colors[ImGuiCol_TableBorderLight]		= ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
				colors[ImGuiCol_TableRowBg]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
				colors[ImGuiCol_TableRowBgAlt]			= ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
				colors[ImGuiCol_TextSelectedBg]			= ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
				colors[ImGuiCol_DragDropTarget]			= ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
				colors[ImGuiCol_NavHighlight]			= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
				colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
				colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
				colors[ImGuiCol_ModalWindowDimBg]		= ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
			}
			break;
	}
}
