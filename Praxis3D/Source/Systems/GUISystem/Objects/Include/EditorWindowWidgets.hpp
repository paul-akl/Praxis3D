#pragma once

// Small, stateless ImGui drawing helpers shared by the EditorWindow screen regions.

#include <string>

#include <imgui.h>

#include "Loaders/Include/Loaders.hpp"

namespace EditorWidgets
{
	// Draws "Label: [............]" with the label left-aligned and the next widget taking the
	// remaining width of the current region (opposite of how ImGui lays it out by default)
	inline void drawLeftAlignedLabelText(const char *p_labelText, const float p_nextWidgetOffset)
	{
		const auto regionWidth = ImGui::GetContentRegionAvail().x;
		ImGui::AlignTextToFramePadding();
		ImGui::Text(p_labelText);
		ImGui::SameLine();
		ImGui::SetCursorPosX(p_nextWidgetOffset);
		ImGui::SetNextItemWidth(regionWidth - p_nextWidgetOffset);
	}
	inline void drawLeftAlignedLabelText(const char *p_labelText, const float p_nextWidgetOffset, const float p_nextItemWidth)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(p_labelText);
		ImGui::SameLine();
		ImGui::SetCursorPosX(p_nextWidgetOffset);
		ImGui::SetNextItemWidth(p_nextItemWidth);
	}

	// Draws an image button that is the size of a line of text, optionally with a delayed tooltip
	inline bool drawTextSizedButton(const TextureLoader2D::Texture2DHandle &p_texture, const std::string &p_buttonLabel, const ImVec2 p_buttonSize, const std::string p_tooltipText = "")
	{
		bool returnBool = false;

		// Draw the open button
		if(ImGui::ImageButton(p_buttonLabel.c_str(),
			(ImTextureID)p_texture.getHandle(),
			p_buttonSize,
			ImVec2(0, 1),
			ImVec2(1, 0),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
		{
			returnBool = true;
		}

		// Draw the tooltip if the tooltip text is not empty and button is hovered over
		if(!p_tooltipText.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
			ImGui::SetTooltip(p_tooltipText.c_str(), ImGui::GetStyle().HoverDelayShort);

		return returnBool;
	}
	inline bool drawTextSizedButtonInverted(const TextureLoader2D::Texture2DHandle &p_texture, const std::string &p_buttonLabel, const ImVec2 p_buttonSize, const std::string p_tooltipText = "")
	{
		bool returnBool = false;

		// Draw the open button
		if(ImGui::ImageButton(p_buttonLabel.c_str(),
			(ImTextureID)p_texture.getHandle(),
			p_buttonSize,
			ImVec2(1, 0),
			ImVec2(0, 1),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
		{
			returnBool = true;
		}

		// Draw the tooltip if the tooltip text is not empty and button is hovered over
		if(!p_tooltipText.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
			ImGui::SetTooltip(p_tooltipText.c_str(), ImGui::GetStyle().HoverDelayShort);

		return returnBool;
	}

	// Calculates the offset for a square text-sized button from the right side of the edge
	// (p_buttonIndex is the button count from the right side)
	inline float calcTextSizedButtonOffset(const float p_contentRegionAvailWidth, const ImVec2 p_buttonSizedByFont, const float p_framePaddingX, const int p_buttonIndex = 0)
	{
		return p_contentRegionAvailWidth - p_buttonSizedByFont.x - (p_framePaddingX * 2.0f) - (p_buttonSizedByFont.x + p_framePaddingX * 3.0f) * p_buttonIndex;
	}
	inline float calcTextSizedButtonSize(const ImVec2 p_buttonSizedByFont, const float p_framePaddingX, const unsigned int p_buttonIndex = 0)
	{
		return p_buttonSizedByFont.x + p_framePaddingX + (p_buttonSizedByFont.x + p_framePaddingX * 3) * p_buttonIndex;
	}
}
