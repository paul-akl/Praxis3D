#pragma once

#include "ErrorHandlerLocator.h"
#include "System.h"
#include "GUIScene.h"

class GUISystem : public SystemBase
{
public:
	GUISystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			m_GUIScenes[i] = nullptr;

		m_systemName = GetString(Systems::GUI);
		m_fonts.resize(GuiFontType::GuiFontType_NumOfTypes, nullptr);

		// Set the color pallet
		if(Config::GUIVar().gui_color_pallet >= 0 && Config::GUIVar().gui_color_pallet < GuiColorPallet::GuiColorPallet_NumOfPallets)
			m_currentColorPallet = static_cast<GuiColorPallet>(Config::GUIVar().gui_color_pallet);
		else
			m_currentColorPallet = GuiColorPallet::GuiColorPallet_Default;

		// Set the GUI colors
		setColorPallet(m_currentColorPallet);
	}
	~GUISystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			if(m_GUIScenes[i] != nullptr)
				delete m_GUIScenes[i];
	}

	ErrorCode init()
	{
		ErrorCode returnCode = ErrorCode::Success;

		// Load fonts
		loadFonts();

		return returnCode;
	}

	ErrorCode setup(const PropertySet& p_properties)
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}

	virtual ErrorCode preload()
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}
	void loadInBackground() { }

	Systems::TypeID getSystemType() { return Systems::GUI; }

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
	{
		if(m_GUIScenes[p_engineState] == nullptr)
		{
			// Create new scene
			m_GUIScenes[p_engineState] = new GUIScene(this, p_sceneLoader);
			ErrorCode sceneError = m_GUIScenes[p_engineState]->init();

			// Check if it initialized correctly (cannot continue without the scene)
			if (sceneError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(sceneError);
			}
			else
			{
				// Check for errors
				GLenum glError = glGetError();
			}
		}

		return m_GUIScenes[p_engineState];
	}

	SystemScene* getScene(EngineStateType p_engineState) { return m_GUIScenes[p_engineState]; }

	void deleteScene(EngineStateType p_engineState)
	{
		if(m_GUIScenes[p_engineState] != nullptr)
		{
			// Shutdown the scene before destroying it
			m_GUIScenes[p_engineState]->shutdown();

			delete m_GUIScenes[p_engineState];
			m_GUIScenes[p_engineState] = nullptr;
		}
	}

	ImFont *getFont(const GuiFontType p_fontType)
	{
		if(p_fontType == GuiFontType::GuiFontType_NumOfTypes)
			return m_fonts[GuiFontType::GuiFontType_Default];

		return m_fonts[p_fontType];
	}
 
protected:
	void loadFonts()
	{
		for(unsigned int i = 0; i < GuiFontType::GuiFontType_NumOfTypes; i++)
		{
			switch(i)
			{
				case GuiFontType_Default:
					{
						m_fonts[i] = ImGui::GetIO().Fonts->Fonts[0];
					}
					break;
				case GuiFontType_AboutWindow:
					{
						ImFontConfig fontConfig;
						fontConfig.OversampleH = 2;
						fontConfig.OversampleV = 1;
						fontConfig.GlyphExtraSpacing.x = 1.0f;

						m_fonts[i] = ImGui::GetIO().Fonts->AddFontFromFileTTF((Config::filepathVar().font_path + Config::filepathVar().engine_assets_path + Config::GUIVar().about_window_font).c_str(), Config::GUIVar().about_window_font_size, &fontConfig);
					}
					break;
				default:
					{
						// If default is reached, it means that a case statement is missing for a given font type, so log an error and assign a default font instead
						ErrHandlerLoc::get().log(ErrorCode::Font_type_missing_construction, ErrorSource::Source_GUI, "Font type: " + Utilities::toString(i));
						m_fonts[i] = ImGui::GetIO().FontDefault;
					}
					break;
			}
		}
	}

	void setColorPallet(const GuiColorPallet p_colorPallet);

	GUIScene *m_GUIScenes[EngineStateType::EngineStateType_NumOfTypes];
	std::vector<ImFont *> m_fonts;
	GuiColorPallet m_currentColorPallet;
};