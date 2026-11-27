#define IMGUI_DEFINE_MATH_OPERATORS

#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"

#include "imgui_internal.h"
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"

void EditorWindow::drawMainMenuBar()
{
// Check for pressed shortcuts
processShortcuts();

ImGui::BeginMainMenuBar();

if(ImGui::BeginMenu("File"))
{
    if(ImGui::MenuItem("New", "CTRL+N"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_New;
    }
    if(ImGui::MenuItem("Open...", "CTRL+O"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Open;
    }

    ImGui::Separator();

    if(ImGui::MenuItem("Save", "CTRL+S"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Save;
    }
    if(ImGui::MenuItem("Save as...", "CTRL+SHIFT+S"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_SaveAs;
    }
    if(ImGui::MenuItem("Reload scene", "CTRL+R"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_ReloadScene;
    }

    ImGui::Separator();

    if(ImGui::MenuItem("Export prefab...", "", false, m_selectedEntity.m_entityID != NULL_ENTITY_ID))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_ExportPrefab;
    }

    ImGui::Separator();

    if(ImGui::MenuItem("Close editor", "ESC"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_CloseEditor;
    }
    if(ImGui::MenuItem("Exit", "ALT+F4"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Exit;
    }

    ImGui::EndMenu();
}
if(ImGui::BeginMenu("Edit"))
{
    if(ImGui::MenuItem("Undo", "CTRL+Z", false, m_currentlyActiveTextEditor != nullptr))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Undo;
    }
    if(ImGui::MenuItem("Redo", "CTRL+Y", false, m_currentlyActiveTextEditor != nullptr))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Redo;
    }

    ImGui::Separator();

    if(ImGui::MenuItem("Cut", "CTRL+X", false, m_currentlyActiveTextEditor != nullptr))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Cut;
    }
    if(ImGui::MenuItem("Copy", "CTRL+C", false, m_currentlyActiveTextEditor != nullptr))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Copy;
    }
    if(ImGui::MenuItem("Paste", "CTRL+V", false, m_currentlyActiveTextEditor != nullptr))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Paste;
    }

    ImGui::EndMenu();
}       
if(ImGui::BeginMenu("View"))
{
    if(ImGui::MenuItem("Enlarge scene viewport", "F11"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_EnlargeSceneViewport;
    }            
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
        ImGui::SetTooltip("Toggle fullscreen mode for the scene viewport. Press F11 to return back to regular view", ImGui::GetStyle().HoverDelayShort);

    if(ImGui::MenuItem("Fullscreen mode", "SHIFT+F11"))
    {
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Fullscreen;
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
        ImGui::SetTooltip("Toggle fullscreen mode. Press SHIFT+F11 to return back to regular view", ImGui::GetStyle().HoverDelayShort);

    ImGui::EndMenu();
}
if(ImGui::BeginMenu("Debug"))
{
    if(ImGui::MenuItem("Show ImGui demo window", "", m_showImGuiDemoWindow)) 
    {
        m_showImGuiDemoWindow = !m_showImGuiDemoWindow;
    }
    if(ImGui::MenuItem("Show Imspinner demo window", "", m_showImspinnerDemoWindow))
    {
        m_showImspinnerDemoWindow = !m_showImspinnerDemoWindow;
    }
    ImGui::EndMenu();
}
if(ImGui::BeginMenu("Help"))
{
    if(ImGui::MenuItem("Open project page"))
    {
        ShellExecuteA(NULL, "open", Config::GUIVar().url_pauldev.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
    if(ImGui::MenuItem("Open Github page"))
    {
        ShellExecuteA(NULL, "open", Config::GUIVar().url_github.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    ImGui::Separator();

    if(ImGui::MenuItem("About"))
    {
        // Sent a message to the GUI scene to enable the about window
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::GUI), DataType::DataType_AboutWindow, (void *)true);
    }
    ImGui::EndMenu();
}

ImGui::EndMainMenuBar();

}

void EditorWindow::drawSecondaryMenuBar(ImGuiViewport *p_mainViewport)
{
    if(ImGui::BeginViewportSideBar("##SecondaryMenuBar", p_mainViewport, ImGuiDir_Up, 50.0f, 
        ImGuiWindowFlags_NoDecoration   | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoTitleBar     | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoDocking      | ImGuiWindowFlags_NoMove))
    {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

    // Draw ENABLE GUI SEQUENCE button
    {
        if(m_GUISequenceEnabled)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        else
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        if(ImGui::ImageButton("##GUISequenceButton",
            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_GUISequence].getHandle(),
            m_playPauseButtonSize,
            ImVec2(0, 1),
            ImVec2(1, 0)))
        {
            m_GUISequenceEnabled = !m_GUISequenceEnabled;

            // Tell the GUI scene to either enable or disable GUI Sequence components
            sendDataTo(m_systemScene, DataType::DataType_EnableGUISequence, (void *)m_GUISequenceEnabled);
        }
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
            ImGui::SetTooltip(m_GUISequenceEnabled ? "Click to disable GUI Sequence components drawing on screen" : "Click to enable GUI Sequence components drawing on screen", ImGui::GetStyle().HoverDelayShort);

        ImGui::PopStyleVar(); // ImGuiStyleVar_FrameBorderSize
    }

    // Draw ENABLE SCRIPTING button
    ImGui::SameLine();
    {
        if(m_LUAScriptingEnabled)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        else
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        if(ImGui::ImageButton("##ScriptingEnableButton",
            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_ScriptingEnable].getHandle(),
            m_playPauseButtonSize,
            ImVec2(0, 1),
            ImVec2(1, 0)))
        {
            m_LUAScriptingEnabled = !m_LUAScriptingEnabled;

            // Tell the Scripting scene to either enable or disable LUA components
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)m_LUAScriptingEnabled);
        }
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
            ImGui::SetTooltip(m_LUAScriptingEnabled ? "Click to disable LUA scripting components" : "Click to enable LUA scripting components", ImGui::GetStyle().HoverDelayShort);

        ImGui::PopStyleVar(); // ImGuiStyleVar_FrameBorderSize
    }

    // Draw GUIZMO TRANSLATE button
    ImGui::SameLine();
    {
        if(m_translateGuizmoEnabled)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        else
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        if(ImGui::ImageButton("##GuizmoTranslateEnableButton",
            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_GuizmoTranslate].getHandle(),
            m_playPauseButtonSize,
            ImVec2(0, 1),
            ImVec2(1, 0)))
        {
            m_translateGuizmoEnabled = !m_translateGuizmoEnabled;
        }
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
            ImGui::SetTooltip(m_translateGuizmoEnabled ? "Click to stop showing position control" : "Click to show position control", ImGui::GetStyle().HoverDelayShort);

        ImGui::PopStyleVar(); // ImGuiStyleVar_FrameBorderSize
    }

    // Draw GUIZMO ROTATE button
    ImGui::SameLine();
    {
        if(m_rotateGuizmoEnabled)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        else
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        if(ImGui::ImageButton("##GuizmoRotateEnableButton",
            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_GuizmoRotate].getHandle(),
            m_playPauseButtonSize,
            ImVec2(0, 1),
            ImVec2(1, 0)))
        {
            m_rotateGuizmoEnabled = !m_rotateGuizmoEnabled;
        }
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
            ImGui::SetTooltip(m_rotateGuizmoEnabled ? "Click to stop showing rotation control" : "Click to show rotation control", ImGui::GetStyle().HoverDelayShort);

        ImGui::PopStyleVar(); // ImGuiStyleVar_FrameBorderSize
    }

    // Get the secondary menu bar style, required for retrieving size information
    ImGuiStyle &secondaryMenuBarStyle = ImGui::GetStyle();

    // Calculate the combined size of a single button (including the inner spacing)
    float playPauseButtonCombinedSize = m_playPauseButtonSize.x + secondaryMenuBarStyle.ItemInnerSpacing.x * 2.0f;

    // Calculate the offset of all buttons to the center
    float offsetToCenter = (playPauseButtonCombinedSize * 3) / 2;

    // Set the starting position of the buttons
    ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 - offsetToCenter);

    // Draw PLAY button
    {
        if(m_sceneState == EditorSceneState::EditorSceneState_Play)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        else
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        if(ImGui::ImageButton("##PlayButton",
            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Play].getHandle(),
            m_playPauseButtonSize,
            ImVec2(0, 1),
            ImVec2(1, 0)))
        {
            m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Play;
        }

        ImGui::PopStyleVar(); // ImGuiStyleVar_FrameBorderSize
    }

    // Draw PAUSE button
    {
        if(m_sceneState == EditorSceneState::EditorSceneState_Pause)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        else
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        ImGui::SameLine();
        if(ImGui::ImageButton("##PauseButton",
            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Pause].getHandle(),
            m_playPauseButtonSize,
            ImVec2(0, 1),
            ImVec2(1, 0)))
        {
            m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Pause;
        }

        ImGui::PopStyleVar(); // ImGuiStyleVar_FrameBorderSize
    }

    // Draw RESTART button
    {
        ImGui::SameLine();
        if(ImGui::ImageButton("##RestartButton",
            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Restart].getHandle(),
            m_playPauseButtonSize,
            ImVec2(0, 1),
            ImVec2(1, 0)))
        {
            m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Restart;
        }
    }

    ImGui::PopStyleColor(2);    // ImGuiCol_Button, ImGuiCol_Border

    ImGui::End();
    }
}

bool EditorWindow::processShortcuts()
{
    // Get ImGui IO (for key presses) and modifier keys
    ImGuiIO &imGuiIO = ImGui::GetIO();
    auto shiftKeyPressed = imGuiIO.KeyShift;
    auto ctrlKeyPressed = imGuiIO.ConfigMacOSXBehaviors ? imGuiIO.KeySuper : imGuiIO.KeyCtrl;
    auto altKeyPressed = imGuiIO.ConfigMacOSXBehaviors ? imGuiIO.KeyCtrl : imGuiIO.KeyAlt;

    // Shortcut for NEW
    if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_N)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_New;

    // Shortcut for OPEN
    if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Open;

    // Shortcut for SAVE
    if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Save;

    // Shortcut for SAVE AS
    if(shiftKeyPressed && ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_SaveAs;

    // Shortcut for RELOAD SCENE
    if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_R)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_ReloadScene;

    // Shortcut for CLOSE EDITOR
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_CloseEditor;

    // Shortcut for ENLARGE SCENE VIEWPORT
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F11)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_EnlargeSceneViewport;

    // Shortcut for FULLSCREEN
    if(shiftKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F11)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Fullscreen;

    // Shortcut for EXIT
    if(altKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4)))
        m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Exit;

    // Shortcut for PLAY / PAUSE
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F5)))
    {
        if(m_sceneState == EditorSceneState::EditorSceneState_Play)
            m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Pause;
        else
            m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Play;
    }

    // Shortcut for UNDO
    //if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
    //    m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Undo;

    // Shortcut for REDO
    //if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y)))
    //    m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Redo;

    // Shortcut for CUT
    //if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_X)))
    //    m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Cut;

    // Shortcut for COPY
    //if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
    //    m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Copy;

    // Shortcut for PASTE
    //if(ctrlKeyPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)))
    //    m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_Paste;

    return m_activatedMainMenuButton != MainMenuButtonType::MainMenuButtonType_None;
}

void EditorWindow::processMainMenuButton(MainMenuButtonType &p_mainMenuButtonType)
{
    // Process activated button
    switch(p_mainMenuButtonType)
    {
        case EditorWindow::MainMenuButtonType_None:
            break;
        case EditorWindow::MainMenuButtonType_Play:
            {
                m_sceneState = EditorSceneState::EditorSceneState_Play;

                // Tell the Physics scene to run the simulation
                sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)true);

                // Tell the Scripting scene to enable LUA components
                sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)true);
            }
            break;
        case EditorWindow::MainMenuButtonType_Pause:
            {
                m_sceneState = EditorSceneState::EditorSceneState_Pause;

                // Tell the Physics scene to pause the simulation
                sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)false);

                // Tell the Scripting scene to either enable or disable LUA components
                sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)m_LUAScriptingEnabled);
            }
            break;
        case EditorWindow::MainMenuButtonType_Restart:
            {
                m_sceneState = EditorSceneState::EditorSceneState_Pause;
            }
            break;
        case EditorWindow::MainMenuButtonType_New:
            // Set the new scene settings tab flag to be selected (bring to focus)
            m_newSceneSettingsTabFlags = ImGuiTabItemFlags_SetSelected;
            m_showNewMapWindow = true;
            break;
        case EditorWindow::MainMenuButtonType_Open:
            // Only open the file browser if it's not opened already
            if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
            {
                // Set the file browser activation to Save Scene
                m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_LoadScene;

                // Define file browser variables
                m_fileBrowserDialog.m_definedFilename = m_systemScene->getSceneLoader()->getSceneFilename();
                m_fileBrowserDialog.m_filter = ".pmap,.*";
                m_fileBrowserDialog.m_title = "Open scene";
                m_fileBrowserDialog.m_name = "OpenSceneFileDialog";
                m_fileBrowserDialog.m_rootPath = Config::filepathVar().map_path;
                m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                // Tell the GUI scene to open the file browser
                sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
            }
            break;
        case EditorWindow::MainMenuButtonType_Save:
            {
                if(m_currentlyActiveTextEditor != nullptr)
                {
                    saveTextFile(*m_currentlyActiveTextEditor);
                }
                else
                {
                    // If the scene filename is empty (meaning the scene wasn't loaded from a file), launch the save-as file browser window
                    if(m_systemScene->getSceneLoader()->getSceneFilename().empty())
                    {
                        // Only open the file browser if it's not opened already
                        if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                        {
                            // Set the file browser activation to Save Scene
                            m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_SaveScene;

                            // Define file browser variables
                            m_fileBrowserDialog.m_definedFilename = m_systemScene->getSceneLoader()->getSceneFilename();
                            m_fileBrowserDialog.m_filter = ".pmap,.*";
                            m_fileBrowserDialog.m_title = "Save scene";
                            m_fileBrowserDialog.m_name = "SaveSceneFileDialog";
                            m_fileBrowserDialog.m_rootPath = Config::filepathVar().map_path;
                            m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_ConfirmOverwrite;

                            // Tell the GUI scene to open the file browser
                            sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                        }
                    }
                    else
                    {
                        m_systemScene->getSceneLoader()->saveToFile(m_systemScene->getSceneLoader()->getSceneFilename());
                    }
                }
            }
            break;
        case EditorWindow::MainMenuButtonType_SaveAs:
            // Only open the file browser if it's not opened already
            if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
            {
                // Set the file browser activation to Save Scene
                m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_SaveScene;

                // Define file browser variables
                m_fileBrowserDialog.m_definedFilename = m_systemScene->getSceneLoader()->getSceneFilename();
                m_fileBrowserDialog.m_filter = ".pmap,.*";
                m_fileBrowserDialog.m_title = "Save scene";
                m_fileBrowserDialog.m_name = "SaveSceneFileDialog";
                m_fileBrowserDialog.m_rootPath = Config::filepathVar().map_path;
                m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_ConfirmOverwrite;

                // Tell the GUI scene to open the file browser
                sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
            }
            break;
        case EditorWindow::MainMenuButtonType_ReloadScene:
            // Send a notification to the engine to reload the current engine state
            m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneReload, EngineStateType::EngineStateType_Editor));
            break;
        case EditorWindow::MainMenuButtonType_CloseEditor:
            if(!m_showingExitDialog)
            {
                m_showingExitDialog = true;
                ImGui::OpenPopup("##CloseEditorPopup");
            }
            break;
        case EditorWindow::MainMenuButtonType_Exit:
            if(!m_showingExitDialog)
            {
                m_showingExitDialog = true;
                ImGui::OpenPopup("##ExitEnginePopup");
            }
            break;
        case EditorWindow::MainMenuButtonType_EnlargeSceneViewport:
            m_enlargedSceneViewport = !m_enlargedSceneViewport;

            // Tell the renderer to draw the scene to the whole viewport
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTexture, (void *)!m_enlargedSceneViewport);
            break;
        case EditorWindow::MainMenuButtonType_Fullscreen:
            m_fullscreen = !m_fullscreen;
            WindowLocator::get().setFullscreen(m_fullscreen);
            break;
        case EditorWindow::MainMenuButtonType_Undo:
            {
                if(m_currentlyActiveTextEditor != nullptr)
                    m_currentlyActiveTextEditor->m_textEditor.Undo();
            }
            break;
        case EditorWindow::MainMenuButtonType_Redo:
            {
                if(m_currentlyActiveTextEditor != nullptr)
                    m_currentlyActiveTextEditor->m_textEditor.Redo();
            }
            break;
        case EditorWindow::MainMenuButtonType_Cut:
            {
                if(m_currentlyActiveTextEditor != nullptr)
                    m_currentlyActiveTextEditor->m_textEditor.Cut();
            }
            break;
        case EditorWindow::MainMenuButtonType_Copy:
            {
                if(m_currentlyActiveTextEditor != nullptr)
                    m_currentlyActiveTextEditor->m_textEditor.Copy();
            }
            break;
        case EditorWindow::MainMenuButtonType_Paste:
            {
                if(m_currentlyActiveTextEditor != nullptr)
                    m_currentlyActiveTextEditor->m_textEditor.Paste();
            }
            break;
        case EditorWindow::MainMenuButtonType_ExportPrefab:
            drawExportPrefabFileBrowser();
            break;
    }

    if(ImGui::BeginPopupModal("##CloseEditorPopup", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("\nYou are about to close the editor\nand go back to the main menu.");
        ImGui::NewLine();
        ImGui::Text("All unsaved progress will be lost.");
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::Text("Continue?");
        ImGui::NewLine();

        ImGui::Separator();

        if(ImGui::Button("Yes", ImVec2(120, 0)))
        {
            // Send a notification to the engine to change the current engine state back to MainMenu
            m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_StateChange, EngineStateType::EngineStateType_MainMenu));
            // Unload the editor state
            m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneUnload, EngineStateType::EngineStateType_Editor));

            m_showingExitDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showingExitDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("##ExitEnginePopup", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("\nYou are about to exit the application.");
        ImGui::NewLine();
        ImGui::Text("All unsaved progress will be lost.");
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::Text("Continue?");
        ImGui::NewLine();

        ImGui::Separator();

        if(ImGui::Button("Yes", ImVec2(140, 0)))
        {
            // Exit engine
            Config::m_engineVar.running = false;

            m_showingExitDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(140, 0)))
        {
            m_showingExitDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Reset activated button
    p_mainMenuButtonType = MainMenuButtonType::MainMenuButtonType_None;
}
