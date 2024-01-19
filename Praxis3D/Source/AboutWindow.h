#pragma once

#include "ErrorHandlerLocator.h"
#include "GUIHandler.h"

class AboutWindow
{
	//friend class GUIScene;
public:
	AboutWindow(SystemScene *p_guiSystemScene) : m_guiSystemScene(p_guiSystemScene) { }
	~AboutWindow() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		return returnError;
	}

	void activate()
	{
		//std::cout << "about window activated" << std::endl;
	}

	void deactivate()
	{
		//std::cout << "about window deactivated" << std::endl;
	}

	void update(const float p_deltaTime)
	{
		
	}

private:
	const void drawCenteredTextWrapped(const std::string &p_text)
	{
		const float windowWidth = ImGui::GetWindowSize().x;
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

	SystemScene *m_guiSystemScene;
};