#pragma once

#include "EngineDefinitions.h"
#include "ErrorHandlerLocator.h"
#include "GUIHandler.h"
#include "GUIHandlerLocator.h"
#include "Loaders.h"
#include "SceneLoader.h"
#include "System.h"

class AboutWindow
{
	//friend class GUIScene;
public:
	AboutWindow(SystemScene *p_guiSystemScene) : m_guiSystemScene(p_guiSystemScene), m_logoTexture(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().about_window_logo_texture))
	{
		m_closeButtonDefaultTextColor = ImVec4(0.588f, 0.773f, 0.29f, 1.0f);
		m_closeButtonHoveredTextColor = ImVec4(0.102f, 0.102f, 0.102f, 1.0f);
		m_closeButtonCurrentTextColor = m_closeButtonDefaultTextColor;

		m_greenColor = ImVec4(0.588f, 0.773f, 0.29f, 1.0f); 
		m_windowBackgroundColor = ImVec4(0.102f, 0.102f, 0.102f, 0.95f);

		m_closeButtonSize = ImVec2(300.0f, 50.0f);
		m_centerWindowScale = ImVec2(0.6f, 0.85f);
		m_centerWindowSize = ImVec2(1200.0f, 1000.0f);

		m_font = nullptr;

		m_logoHeight = 200.0f; 
		m_closeButtonOffsetFromBottom = 75.0f;
		m_closeButtonFontScale = 1.5f;
		m_mainTextFontScale = 2.0f;
		m_linksTextFontScale = 1.25f;

		m_dependencies = {
			"- Assimp (Open Asset Import Library)",
			"- Bullet3 (Bullet Physics)",
			"- Dear ImGui",
			"- EnTT",
			"- FMOD",
			"- FreeImage",
			"- GLEW (OpenGL Extension Wrangler)",
			"- GLM (OpenGL Mathematics)",
			"- ImGuiColorTextEdit",
			"- ImGuiFileDialog",
			"- ImGuiTexInspect",
			"- ImGuizmo",
			"- ImSpinner",
			"- LuaJIT",
			"- SDL (Simple DirectMedia Layer)",
			"- Sol3",
			"- TBB (Intel Threading Building Blocks)" }; 
		
		// Calculate half the size of dependencies array while making sure to round up
		m_dependenciesHalfSize = m_dependencies.size() / 2;
		m_dependenciesHalfSize += m_dependencies.size() - (m_dependenciesHalfSize * 2);

		// Get the longest (text-wise) entry in the dependencies array
		m_dependenciesLongestEntryIndex = 0;
		for(decltype(m_dependencies.size()) i = 0, longest = 0, size = m_dependencies.size(); i < size; i++)
		{
			if(longest < m_dependencies[i].size())
			{
				longest = m_dependencies[i].size();
				m_dependenciesLongestEntryIndex = i;
			}
		}
	}
	~AboutWindow() { }

	ErrorCode init();

	void activate()
	{
	}

	void deactivate()
	{
	}

	void update(const float p_deltaTime);

private:
	const inline void drawRightAlignedText(const std::string &p_text)
	{
		ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(p_text.c_str()).x);

		ImGui::Text(p_text.c_str());
	}
	const inline void drawRightAlignedText(const std::string &p_text, const float p_windowWidth)
	{
		ImGui::SetCursorPosX(p_windowWidth - ImGui::CalcTextSize(p_text.c_str()).x);

		ImGui::Text(p_text.c_str());
	}
	const inline void drawCenteredText(const std::string &p_text)
	{
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(p_text.c_str()).x) / 2.0f);

		ImGui::Text(p_text.c_str());
	}
	const inline void drawCenteredTextColored(const ImVec4 &p_color, const std::string &p_text)
	{
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(p_text.c_str()).x) / 2.0f);

		ImGui::TextColored(p_color, p_text.c_str());
	}
	const inline void drawCenteredTextWrapped(const std::string &p_text)
	{
		const float windowWidth = ImGui::GetContentRegionAvail().x;
		const float textWidth = ImGui::CalcTextSize(p_text.c_str()).x;

		// calculate the indentation that centers the text on one line, relative
		// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
		float textOffset = (windowWidth - textWidth) * 0.5f;

		// if text is too long to be drawn on one line, `text_indentation` can
		// become too small or even negative, so we check a minimum indentation
		const float minOffset = 20.0f;
		if(textOffset <= minOffset)
		{
			textOffset = minOffset;
		}

		ImGui::SameLine(textOffset);
		ImGui::PushTextWrapPos(windowWidth - textOffset);
		ImGui::TextWrapped(p_text.c_str());
		ImGui::PopTextWrapPos();
	}

	// Keep a pointer to the GUI scene for messaging and font retrieval
	SystemScene *m_guiSystemScene;

	ImVec4 m_closeButtonCurrentTextColor;
	ImVec4 m_closeButtonDefaultTextColor;
	ImVec4 m_closeButtonHoveredTextColor;
	ImVec4 m_greenColor;
	ImVec4 m_windowBackgroundColor;
	ImVec2 m_closeButtonSize; 
	ImVec2 m_centerWindowScale;
	ImVec2 m_centerWindowSize;
	ImFont *m_font;

	float m_logoHeight;
	float m_closeButtonOffsetFromBottom;
	float m_closeButtonFontScale;
	float m_mainTextFontScale;
	float m_linksTextFontScale;

	TextureLoader2D::Texture2DHandle m_logoTexture;

	std::vector<std::string> m_dependencies; 
	decltype(m_dependencies.size()) m_dependenciesHalfSize;
	decltype(m_dependencies.size()) m_dependenciesLongestEntryIndex;
};