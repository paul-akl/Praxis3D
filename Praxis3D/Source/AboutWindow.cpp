#include "AboutWindow.h"
#include "GUISystem.h"
#include "Version.h"

ErrorCode AboutWindow::init()
{
	ErrorCode returnError = ErrorCode::Success;
	
	// Load the logo texture
	m_logoTexture.loadToMemory();
	m_guiSystemScene->getSceneLoader()->getChangeController()->sendData(m_guiSystemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadTexture2D, (void *)&m_logoTexture);

	// Get the about window font
	m_font = static_cast<GUISystem *>(m_guiSystemScene->getSystem())->getFont(GuiFontType::GuiFontType_AboutWindow);

	return returnError;
}

void AboutWindow::update(const float p_deltaTime)
{
	auto &imguiIO = ImGui::GetIO();

	// Set window background color and make the child window background transparent
	ImGui::PushStyleColor(ImGuiCol_WindowBg, m_windowBackgroundColor);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	// Center the main window
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(imguiIO.DisplaySize);

	// Use the about window font
	ImGui::PushFont(m_font);

	if(ImGui::Begin("##AboutWindow", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
	{
		// Calculate window size based on resolution
		//ImVec2 centerWindowSize = imguiIO.DisplaySize * m_centerWindowScale;
		ImVec2 centerWindowSize = m_centerWindowSize;

		float test = imguiIO.DisplaySize.y - (m_closeButtonSize.y * 2.0f);

		if(centerWindowSize.x > imguiIO.DisplaySize.x)
			centerWindowSize.x = imguiIO.DisplaySize.x;

		if(centerWindowSize.y > test)
			centerWindowSize.y = test;

		// Center the child window
		ImGui::SetNextWindowPos((imguiIO.DisplaySize - centerWindowSize) / 2.0f);
		if(ImGui::BeginChild("##AboutWindowCenter", centerWindowSize, false))
		{
			const float windowWidth = ImGui::GetContentRegionAvail().x;
			// Calculate the logo size based on the logo height
			const float logoAspectRatio = (float)m_logoTexture.getTextureWidth() / (float)m_logoTexture.getTextureHeight();
			ImVec2 logoSize(m_logoHeight * logoAspectRatio, m_logoHeight);

			// Center the logo horizontally
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - logoSize.x) / 2.0f);

			// Draw the logo
			ImGui::Image(m_logoTexture.getHandle(), logoSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			ImGui::SameLine();
			ImGui::BeginGroup();

			drawRightAlignedText("Version: v" + std::string(PRAXIS3D_VERSION_STRING), windowWidth);
			drawRightAlignedText("Commit date: " + std::string(PRAXIS3D_COMMIT_DATE_STRING), windowWidth);

			//ImGui::EndGroup();
			ImGui::EndGroup();

			// Draw the main text using a custom font scale
			ImGui::SetWindowFontScale(m_mainTextFontScale);
			drawCenteredTextColored(m_greenColor, "Praxis3D game engine");
			drawCenteredTextColored(m_greenColor, "Created by Paulius Akulavicius");
			ImGui::SetWindowFontScale(1.0f);

			ImGui::NewLine();

			// Draw project description
			drawCenteredText("Written in C++20, GLSL, Lua, JSON");
			drawCenteredText("Using OpenGL as the graphics API");

			ImGui::NewLine();

			// Draw project links
			drawCenteredText("Find out more about this project at:");
			ImGui::SetWindowFontScale(m_linksTextFontScale);
			drawCenteredTextColored(m_greenColor, "pauldev.org");
			drawCenteredTextColored(m_greenColor, "github.com/paul-akl/Praxis3D");
			ImGui::SetWindowFontScale(1.0f);

			ImGui::NewLine();

			drawCenteredText("Dependencies used:");
			ImGui::SetWindowFontScale(0.75f);

			ImGui::NewLine();

			// Calculate the dependencies table width based on the longest text entry in the dependencies array
			const float dependenciesTableWidth = ImGui::CalcTextSize(m_dependencies[m_dependenciesLongestEntryIndex].c_str()).x * 2.0f;
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - dependenciesTableWidth) / 2.0f);

			// Draw the dependencies table
			if(ImGui::BeginTable("##DependenciesTable", 2, 0, ImVec2(dependenciesTableWidth, 0.0f)))
			{
				for(decltype(m_dependencies.size()) i = 0; i < m_dependenciesHalfSize; i++)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text(m_dependencies[i].c_str());

					// Make sure to not go out of bounds if the dependencies array size is odd
					ImGui::TableSetColumnIndex(1);
					if(i + m_dependenciesHalfSize < m_dependencies.size())
						ImGui::Text(m_dependencies[i + m_dependenciesHalfSize].c_str());
					else
						ImGui::Text("");
				}
				ImGui::EndTable();
			}
			ImGui::SetWindowFontScale(1.0f);

			ImGui::NewLine();

			drawCenteredText("Licenses can be found in LICENSE.md");
		}
		ImGui::EndChild();

		// Set the close button colors and style
		ImGui::PushStyleColor(ImGuiCol_Text, m_closeButtonCurrentTextColor);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_greenColor);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));

		// Set the close button font scale
		ImGui::SetWindowFontScale(m_closeButtonFontScale);

		// Calculate close button height
		const char *closeButtonText = "Close";
		m_closeButtonSize.y = ImGui::CalcTextSize(closeButtonText).y + ImGui::GetStyle().FramePadding.y * 2.0f;

		// Center the close button horizontally and offset vertically from the bottom
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - m_closeButtonSize.x) / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - m_closeButtonSize.y - ImGui::GetStyle().WindowPadding.y);

		// Draw the close button
		if(ImGui::Button(closeButtonText, m_closeButtonSize))
		{
			// If the close button was pressed, sent a message to the GUI scene to disable the about window
			m_guiSystemScene->getSceneLoader()->getChangeController()->sendData(m_guiSystemScene->getSceneLoader()->getSystemScene(Systems::GUI), DataType::DataType_AboutWindow, (void *)false);
		}

		// Reset the font scale back to normal
		ImGui::SetWindowFontScale(1.0f);

		// Set the close button text color based on whether the mouse is hovering over the button
		m_closeButtonCurrentTextColor = ImGui::IsItemHovered() ? m_closeButtonHoveredTextColor : m_closeButtonDefaultTextColor;

		ImGui::PopStyleVar(2);		// ImGuiStyleVar_FrameRounding, ImGuiStyleVar_ButtonTextAlign
		ImGui::PopStyleColor(3);	// ImGuiCol_Text, ImGuiCol_Button, ImGuiCol_ButtonHovered
	}
	ImGui::End();

	// Reset the font
	ImGui::PopFont();

	ImGui::PopStyleColor(2);	// ImGuiCol_WindowBg, ImGuiCol_ChildBg
}
