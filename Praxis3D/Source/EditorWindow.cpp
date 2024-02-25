
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>

#define IMSPINNER_DEMO
#include "EngineDefinitions.h"
#include "EditorWindow.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "imspinner.h"
#include "RendererScene.h"
#include "ShaderUniformUpdater.h"
#include "WorldScene.h"

// Include every component
#include "ImpactSoundComponent.h"
#include "SoundComponent.h"
#include "SoundListenerComponent.h"
#include "GUISequenceComponent.h"
#include "CollisionEventComponent.h"
#include "CollisionShapeComponent.h"
#include "RigidBodyComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "ModelComponent.h"
#include "ShaderComponent.h"
#include "LuaComponent.h"
#include "ObjectMaterialComponent.h"
#include "SpatialComponent.h"

#include <tex_inspect_opengl.h>
#include <imgui_tex_inspect.h>
#include <imgui/imgui_stdlib.h>

EditorWindow::~EditorWindow()
{
    // If engine is still running
    if(Config::engineVar().running == true)
    {
        // Tell the renderer to draw the scene to the screen
        //m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTexture, (void *)false);
    }

    // Delete all text editor data entries
    for(decltype(m_textEditorFiles.size()) i = 0, size = m_textEditorFiles.size(); i < size; i++)
        if(m_textEditorFiles[i] != nullptr)
            delete m_textEditorFiles[i];
}

ErrorCode EditorWindow::init()
{
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    ImGuizmo::SetOrthographic(false);
    return ErrorCode::Success;
}

void EditorWindow::update(const float p_deltaTime)
{
    // Don't draw the GUI if the scene viewport is enlarged
    if(m_enlargedSceneViewport)
    {
        // Process key presses
        if(processShortcuts())
            processMainMenuButton(m_activatedMainMenuButton);

        return;
    }

    // Set the docking area over the whole screen
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    // Clear the texture inspector functor sequence from the last frame
    m_textureInspectorSequence.swapBuffer();
    m_textureInspectorSequence.getFront().clear();

    // Update the entity list
    updateEntityList(); 
    
    // Update component list
    updateComponentList();

    // Update the hierarchy list
    updateHierarchyList();

    // Update the asset lists
    updateAssetLists();

    // If there is a pending entity to select, iterate over each entity and find the one that needs to be selected
    if(m_pendingEntityToSelect)
    {
        if(m_nextEntityToSelect != nullptr)
        {
            m_nextEntityIDToSelect = m_nextEntityToSelect->m_id;
            m_nextEntityToSelect = nullptr;
        }

        for(decltype(m_entityList.size()) size = m_entityList.size(), i = 0; i < size; i++)
            if(m_entityList[i].m_entityID == m_nextEntityIDToSelect)
                m_selectedEntity.setEntity(m_entityList[i].m_entityID);

        m_pendingEntityToSelect = false;
    }

    // Clear the entity and component creation / deletion pools left from the previous frame
    clearEntityAndComponentPool();
    clearConstructionInfoPool();
    resetActivateAllComponentFlags();

    if(m_showImGuiDemoWindow)
        ImGui::ShowDemoWindow();

    {
        ImGuiStyle *style = &ImGui::GetStyle();
        ImVec4 *colors = style->Colors;

        //colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        //colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.122f, 0.122f, 0.122f, 1.0f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.122f, 0.122f, 0.122f, 1.0f);
        //colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.05f, 0.05f, 0.05f, 0.50f);
        //colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        //colors[ImGuiCol_FrameBg] = ImVec4(0.325f, 0.533f, 0.188f, 1.0f);
        //colors[ImGuiCol_FrameBgHovered] = ImVec4(0.341f, 0.651f, 0.29f, 1.0f);
        //colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        //colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        //colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        //colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.122f, 0.122f, 0.122f, 1.0f);
        //colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        //colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        //colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        //colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        //colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        //colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        //colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        //colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
        //colors[ImGuiCol_ButtonHovered] = ImVec4(0.341f, 0.651f, 0.29f, 1.0f);
        //colors[ImGuiCol_ButtonActive] = ImVec4(0.52f, 0.52f, 0.52f, 0.5f);
        //colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
        //colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        //colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        //colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        //colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        //colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        //colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        //colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        //colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        //colors[ImGuiCol_Tab] = ImVec4(0.118f, 0.118f, 0.118f, 1.0f);
        //colors[ImGuiCol_Tab] = ImVec4(0.122f, 0.122f, 0.122f, 0.5f);
        //colors[ImGuiCol_TabHovered] = ImVec4(0.341f, 0.651f, 0.29f, 1.0f);
        //colors[ImGuiCol_TabActive] = ImVec4(0.325f, 0.533f, 0.188f, 1.0f);
        //colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
        //colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
        //colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
        //colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        //colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        //colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        //colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        //colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        //colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
        //colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
        //colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        //colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        //colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        //colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        //colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        //colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        //colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    // Update ImGui style
    ImVec2 mainMenuBarSize;
    ImGuiViewport *mainViewport = ImGui::GetMainViewport();

    auto *rendererScene = m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics);

    WorldScene *worldScene = static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World));
    auto &entityRegistry = worldScene->getEntityRegistry();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::PopStyleColor();

    m_imguiStyle.TabRounding = 0.0f;

    ImGuiWindowClass windowClassWithNoTabBar;
    windowClassWithNoTabBar.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.122f, 0.122f, 0.122f, 0.0f));
    //	 ____________________________
    //	|							 |
    //	|	    MAIN MENU BAR	     |
    //	|____________________________|
    //
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
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::GUI), DataType::DataType_AboutWindow, (void *)true);
            }
            ImGui::EndMenu();
        }

        // Get the main menu bar size needed for the secondary menu bar
        mainMenuBarSize = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleColor();

    //	 ____________________________
    //	|							 |
    //	|	  SECONDARY MENU BAR	 |
    //	|____________________________|
    //
    {
        if(ImGui::BeginViewportSideBar("##SecondaryMenuBar", mainViewport, ImGuiDir_Up, 50.0f, 
            ImGuiWindowFlags_NoDecoration   | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoTitleBar     | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoDocking      | ImGuiWindowFlags_NoMove))
        {
            // Make button background transparent
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
                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_EnableGUISequence, (void *)m_GUISequenceEnabled);
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
                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)m_LUAScriptingEnabled);
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

    // Process the pressed main menu buttons
    processMainMenuButton(m_activatedMainMenuButton);

    // Make the padding above tabs smaller
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 1));

    //	 ____________________________
    //	|							 |
    //	|	     LEFT WINDOW         |
    //	|____________________________|
    //
    {
        ImGui::SetNextWindowClass(&windowClassWithNoTabBar);
        if(ImGui::Begin("##LeftWindow", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
        {
            if(ImGui::BeginTabBar("##LeftWindowTabBar", ImGuiTabBarFlags_None))
            {
                if(ImGui::BeginTabItem("Hierarchy"))
                {
                    if(ImGui::BeginChild("##HierarchyWindow", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_::ImGuiWindowFlags_None))
                    {
                        // Draw every entry from the hierarchy list
                        drawEntityHierarchy(&m_rootEntityHierarchyEntry);

                        ImGui::Separator();

                        const std::string componentTypeSelectionPopupName = "##AddEntityPopup";

                        // Calculate button size
                        const char *buttonLabel = "Add entity";
                        float buttonWidth = ImGui::CalcTextSize(buttonLabel).x * Config::GUIVar().editor_inspector_button_width_multiplier;

                        // Set the button position to the right-most side
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth);

                        // Draw ADD ENTITY button
                        if(ImGui::Button(buttonLabel, ImVec2(buttonWidth, 0.0f)))
                        {
                            if(m_newEntityConstructionInfo == nullptr)
                            {
                                m_newEntityConstructionInfo = new ComponentsConstructionInfo();
                                m_newEntityConstructionInfo->m_name = "New entity";
                                m_newEntityConstructionInfo->m_id = 0;

                                // Reset the duplicate parent flag for new entity
                                m_duplicateParent = false;

                                // Open the pop-up with the new entity settings
                                m_openNewEntityPopup = true;
                            }
                        }

                        if(m_openEntityRightClickOptionsPopup)
                        {
                            ImGui::OpenPopup(m_entityRightClickOptionsPopup);
                            m_openEntityRightClickOptionsPopup = false;
                        }

                        // Draw a pop-up with the entity options
                        if(ImGui::BeginPopup(m_entityRightClickOptionsPopup))
                        {
                            ImGui::Separator();
                            if(ImGui::MenuItem("Add child"))
                            {
                                if(m_newEntityConstructionInfo == nullptr)
                                {
                                    m_newEntityConstructionInfo = new ComponentsConstructionInfo();
                                    m_newEntityConstructionInfo->m_name = "New entity";
                                    m_newEntityConstructionInfo->m_id = 0;
                                    m_newEntityConstructionInfo->m_parent = m_selectedEntity.m_entityID;

                                    // Reset the duplicate parent flag for new entity
                                    m_duplicateParent = false;

                                    // Open the pop-up with the new entity settings
                                    m_openNewEntityPopup = true;
                                }
                            }
                            if(ImGui::MenuItem("Duplicate"))
                            {
                                duplicateEntity(m_selectedEntity.m_entityID);
                            }
                            if(ImGui::MenuItem("Delete"))
                            {
                                deleteEntityAndChildren(m_selectedEntity.m_entityID);
                                m_selectedEntity.unselect();
                            }

                            ImGui::Separator();

                            if(ImGui::BeginMenu("Activate all children.."))
                            {
                                // Get entity children
                                std::vector<EntityID> childrenEntityIDs;
                                childrenEntityIDs.push_back(m_selectedEntity.m_entityID);
                                if(auto *entityListEntry = getEntityListEntry(m_selectedEntity.m_entityID); entityListEntry != nullptr)
                                    getEntityChildren(childrenEntityIDs, *entityListEntry);

                                if(ImGui::MenuItem("Light components"))
                                {
                                    m_lightComponentActivateAllSet = true;
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_active = true;

                                    // Go over each child
                                    for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
                                    {
                                        // Try to get the light component
                                        if(auto *lightComponent = entityRegistry.try_get<LightComponent>(childrenEntityIDs[i]); lightComponent != nullptr)
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Generic::Active);
                                        }
                                    }
                                }
                                if(ImGui::MenuItem("Model components"))
                                {
                                    m_modelComponentActivateAllSet = true;
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_active = true;

                                    // Go over each child
                                    for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
                                    {
                                        // Try to get the model component
                                        if(auto *modelComponent = entityRegistry.try_get<ModelComponent>(childrenEntityIDs[i]); modelComponent != nullptr)
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, modelComponent, Systems::Changes::Generic::Active);
                                        }
                                    }
                                }
                                if(ImGui::MenuItem("Rigid body components"))
                                {
                                    m_rigidBodyComponentActivateAllSet = true;
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_active = true;

                                    // Go over each child
                                    for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
                                    {
                                        // Try to get the rigid body component
                                        if(auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(childrenEntityIDs[i]); rigidBodyComponent != nullptr)
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Generic::Active);
                                        }
                                    }
                                }
                                ImGui::EndMenu();
                            }

                            if(ImGui::BeginMenu("Deactivate all children.."))
                            {
                                // Get entity children
                                std::vector<EntityID> childrenEntityIDs;
                                childrenEntityIDs.push_back(m_selectedEntity.m_entityID);
                                if(auto *entityListEntry = getEntityListEntry(m_selectedEntity.m_entityID); entityListEntry != nullptr)
                                    getEntityChildren(childrenEntityIDs, *entityListEntry);

                                if(ImGui::MenuItem("Light components"))
                                {
                                    m_lightComponentActivateAllSet = true;
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_active = false;

                                    // Go over each child
                                    for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
                                    {
                                        // Try to get the light component
                                        if(auto *lightComponent = entityRegistry.try_get<LightComponent>(childrenEntityIDs[i]); lightComponent != nullptr)
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Generic::Active);
                                        }
                                    }
                                }
                                if(ImGui::MenuItem("Model components"))
                                {
                                    m_modelComponentActivateAllSet = true;
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_active = false;

                                    // Go over each child
                                    for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
                                    {
                                        // Try to get the model component
                                        if(auto *modelComponent = entityRegistry.try_get<ModelComponent>(childrenEntityIDs[i]); modelComponent != nullptr)
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, modelComponent, Systems::Changes::Generic::Active);
                                        }
                                    }
                                }
                                if(ImGui::MenuItem("Rigid body components"))
                                {
                                    m_rigidBodyComponentActivateAllSet = true;
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_active = false;

                                    // Go over each child
                                    for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
                                    {
                                        // Try to get the rigid body component
                                        if(auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(childrenEntityIDs[i]); rigidBodyComponent != nullptr)
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Generic::Active);
                                        }
                                    }
                                }
                                ImGui::EndMenu();
                            }

                            ImGui::Separator();

                            if(ImGui::MenuItem("Export as prefab"))
                            {
                                drawExportPrefabFileBrowser();
                            }

                            ImGui::Separator();
                            ImGui::EndPopup();
                        }

                        // Draw a pop-up with the new entity settings
                        if(m_openNewEntityPopup)
                        {
                            if(!m_newEntityWindowInitialized)
                            {
                                m_newEntityWindowInitialized = true;
                                m_mousePositionOnNewEntity = ImGui::GetMousePos();

                                // Assign a next available entity ID (start the available ID search from the next ID after the parent)
                                EntityID newEntityID = m_newEntityConstructionInfo->m_parent + 1;
                                for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size;)
                                {
                                    if(newEntityID == m_entityList[i].m_entityID)
                                    {
                                        newEntityID++;
                                        i = 0;
                                    }
                                    else
                                        i++;
                                }
                                m_newEntityConstructionInfo->m_id = newEntityID;

                                // Assign a next available entity name
                                std::string newEntityName = Config::GUIVar().editor_new_entity_name + " " + Utilities::toString(1);
                                for(decltype(m_entityList.size()) i = 0, nameIndex = 1, size = m_entityList.size(); i < size; i++)
                                {
                                    if(newEntityName == m_entityList[i].m_name)
                                    {
                                        newEntityName = Config::GUIVar().editor_new_entity_name + " " + Utilities::toString(nameIndex);
                                        nameIndex++;
                                        i = 0;
                                    }
                                }
                                m_newEntityConstructionInfo->m_name = newEntityName;
                            }

                            ImGui::SetNextWindowPos(m_mousePositionOnNewEntity);
                            ImGui::SetNextWindowSize(ImVec2(400.0f, 155.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(m_imguiStyle.WindowPadding.x, 10.0f));
                            if(ImGui::Begin(componentTypeSelectionPopupName.c_str(), (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
                            {
                                // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                                float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                                // Draw NAME
                                ImGui::Text("Name:");
                                ImGui::SameLine();
                                ImGui::SetCursorPosX(inputWidgetOffset);
                                if(ImGui::InputText("##NewEntityNameStringInput", &m_newEntityConstructionInfo->m_name, ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                }

                                // Draw ENTITY ID
                                ImGui::Text("Entity ID:");
                                ImGui::SameLine();
                                ImGui::SetCursorPosX(inputWidgetOffset);
                                if(ImGui::InputScalar("##NewEntityEntityIDInput", ImGuiDataType_U32, &m_newEntityConstructionInfo->m_id))
                                {
                                }

                                // Draw PARENT
                                ImGui::Text("Parent ID:");
                                ImGui::SameLine();
                                ImGui::SetCursorPosX(inputWidgetOffset);
                                if(ImGui::BeginCombo("##NewEntityParentIDCombo", Utilities::toString(m_newEntityConstructionInfo->m_parent).c_str()))
                                {
                                    // Go over all existing entities
                                    for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size; i++)
                                    {
                                        // Mark which parent ID is selected
                                        const bool is_selected = (m_newEntityConstructionInfo->m_parent == m_entityList[i].m_entityID);

                                        // Don't show entities own ID as a parent ID selection
                                        if(m_entityList[i].m_entityID != m_newEntityConstructionInfo->m_id)
                                        {
                                            if(ImGui::Selectable(Utilities::toString(m_entityList[i].m_entityID).c_str(), is_selected))
                                            {
                                                m_newEntityConstructionInfo->m_parent = m_entityList[i].m_entityID;
                                            }
                                        }

                                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                        if(is_selected)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }

                                // Draw PREFAB
                                drawLeftAlignedLabelText("Prefab:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                                if(ImGui::InputText("##NewEntityPrefabStringInput", &m_newEntityConstructionInfo->m_prefab, ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                    // Cannot duplicate when a prefab is set
                                    m_duplicateParent = false;
                                }

                                // Draw PREFAB OPEN button
                                ImGui::SameLine(calcTextSizedButtonOffset(1));
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##PrefabOpenButton", "Open a prefab file"))
                                {
                                    // Only open the file browser if it's not opened already
                                    if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                    {
                                        // Set the file browser activation to Prefab File
                                        m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_PrefabFile;

                                        // Define file browser variables
                                        m_fileBrowserDialog.m_filter = "Prefab files (.prefab){.prefab,.PREFAB},All files{.*}";
                                        m_fileBrowserDialog.m_title = "Open a prefab file";
                                        m_fileBrowserDialog.m_name = "OpenPrefabFileFileDialog";
                                        m_fileBrowserDialog.m_rootPath = Config::filepathVar().prefab_path;
                                        m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;
                                        m_fileBrowserDialog.m_userStringPointer = &m_newEntityConstructionInfo->m_prefab;

                                        // Cannot duplicate when a prefab is set
                                        m_duplicateParent = false;

                                        // Tell the GUI scene to open the file browser
                                        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                    }
                                }

                                const std::string prefabSelectionPopupName = "##NewEntityPrefabSelectionPopup";

                                // Draw OPEN PREFAB LIST button
                                ImGui::SameLine(calcTextSizedButtonOffset(0));
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##OpenPrefabListButton", "Choose a prefab from the loaded prefabs"))
                                {
                                    // Open the pop-up with the prefab asset list
                                    ImGui::OpenPopup(prefabSelectionPopupName.c_str());
                                }

                                // Draw PREFABLIST
                                if(ImGui::BeginPopup(prefabSelectionPopupName.c_str()))
                                {
                                    // Remove selection border and align text vertically
                                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
                                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                                    auto &prefabList = m_systemScene->getSceneLoader()->getPrefabs();

                                    for(auto const &[name, prefab] : prefabList)
                                    {
                                        // Draw PREFAB NAME selection
                                        // Set the text height to the texture image button height
                                        if(ImGui::Selectable(name.c_str(), (m_selectedEntity.m_componentData.m_prefab == name), 0, ImVec2(0.0f, m_assetSelectionPopupImageSize.y)))
                                        {
                                            m_newEntityConstructionInfo->m_prefab = name;

                                            // Cannot duplicate when a prefab is set
                                            m_duplicateParent = false;
                                        }
                                    }

                                    ImGui::PopStyleVar(2); //ImGuiStyleVar_SelectableTextAlign, ImGuiStyleVar_FramePadding
                                    ImGui::EndPopup();
                                }

                                // Draw DUPLICATE PARENT
                                ImGui::Text("Duplicate parent:");
                                ImGui::SameLine();
                                ImGui::SetCursorPosX(inputWidgetOffset);
                                if(ImGui::Checkbox("##DuplicateParentCheckbox", &m_duplicateParent))
                                {
                                }

                                // Draw CREATE button
                                if(ImGui::Button("Create"))
                                {
                                    // If a prefab was set, load it
                                    if(!m_newEntityConstructionInfo->m_prefab.empty())
                                    {
                                        // Backup the current construction info
                                        ComponentsConstructionInfo currentConstructionInfo = *m_newEntityConstructionInfo;

                                        // Import prefab
                                        m_systemScene->getSceneLoader()->importPrefab(*m_newEntityConstructionInfo, m_newEntityConstructionInfo->m_prefab);

                                        // Re-set the previous construction info values that were set during the entity creation, as these override the values that are imported from a prefab
                                        m_newEntityConstructionInfo->m_id = currentConstructionInfo.m_id;
                                        m_newEntityConstructionInfo->m_name = currentConstructionInfo.m_name;
                                        m_newEntityConstructionInfo->m_parent = currentConstructionInfo.m_parent;
                                        m_newEntityConstructionInfo->m_prefab = currentConstructionInfo.m_prefab;
                                    }
                                    else
                                    {
                                        // If duplicate parent flag was set, get the parent construction info
                                        if(m_duplicateParent)
                                        {
                                            // Reset the duplicate parent flag for new entity
                                            m_duplicateParent = false;

                                            // Backup the current construction info
                                            ComponentsConstructionInfo currentConstructionInfo = *m_newEntityConstructionInfo;

                                            // Get the construction info of the parent entity
                                            WorldScene *worldScene = static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World));
                                            worldScene->exportEntity(currentConstructionInfo.m_parent, *m_newEntityConstructionInfo);

                                            // Set the new entity name by adding a count at the end and checking if an entity of the same name doesn't exist
                                            {
                                                int newEntityNameCount = 2;
                                                std::string baseEntityName = m_newEntityConstructionInfo->m_name;

                                                // If the entity name ends with ' 1', replace the existing number
                                                if(baseEntityName.size() > 2 && baseEntityName[baseEntityName.size() - 1] == '1' && baseEntityName[baseEntityName.size() - 2] == ' ')
                                                {
                                                    baseEntityName.pop_back();  // Remove '1'
                                                    baseEntityName.pop_back();  // Remove ' '
                                                }
                                                std::string newEntityName = baseEntityName + " " + Utilities::toString(newEntityNameCount);

                                                // Keep increasing the number at the end until there is no other entity with the same name
                                                while(worldScene->getEntity(newEntityName) != NULL_ENTITY_ID)
                                                {
                                                    newEntityNameCount++;
                                                    newEntityName = baseEntityName + " " + Utilities::toString(newEntityNameCount);
                                                }
                                                m_newEntityConstructionInfo->m_name = newEntityName;
                                            }

                                            // Re-set the previous construction info values that were set during the entity creation, as these override the values that are imported from a prefab
                                            m_newEntityConstructionInfo->m_id = currentConstructionInfo.m_id;
                                            m_newEntityConstructionInfo->m_parent = currentConstructionInfo.m_parent;
                                        }
                                    }

                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_CreateEntity, (void *)m_newEntityConstructionInfo, false);

                                    // Make the new entity be selected next frame
                                    m_nextEntityIDToSelect = m_newEntityConstructionInfo->m_id;
                                    m_pendingEntityToSelect = true;

                                    // Add the new entity construction info to the pool (so it will be deleted the next frame)
                                    m_componentConstructionInfoPool.push_back(m_newEntityConstructionInfo);
                                    m_newEntityConstructionInfo = nullptr;

                                    m_openNewEntityPopup = false;
                                    m_newEntityWindowInitialized = false;
                                }

                                // Draw CANCEL button
                                ImGui::SameLine();
                                if(ImGui::Button("Cancel"))
                                {
                                    delete m_newEntityConstructionInfo;
                                    m_newEntityConstructionInfo = nullptr;
                                    m_openNewEntityPopup = false;
                                    m_newEntityWindowInitialized = false;
                                }
                            }
                            ImGui::End();
                            ImGui::PopStyleVar(); //ImGuiStyleVar_WindowPadding
                        }
                        else
                        {
                            if(m_newEntityConstructionInfo != nullptr)
                            {
                                delete m_newEntityConstructionInfo;
                                m_newEntityConstructionInfo = nullptr;
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Entities"))
                {
                    // Draw every entry from the entities list
                    for(decltype(m_entityList.size()) size = m_entityList.size(), i = 0; i < size; i++)
                    {
                        // Highlight the previously selected item and set the currently selected item if it is clicked on
                        if(ImGui::Selectable(m_entityList[i].m_combinedEntityIdAndName.c_str(), m_entityList[i].m_entityID == m_selectedEntity.m_entityID))
                        {
                            m_selectedEntity.setEntity(m_entityList[i].m_entityID);
                        }
                    }

                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Components"))
                {
                    // Draw every entry from the component list
                    for(decltype(m_componentList.size()) size = m_componentList.size(), i = 0; i < size; i++)
                    {
                        // Highlight all the components that belong to the previously selected entity 
                        // and set the currently selected entity to an entity that the component belongs to (if it is clicked on)
                        if(ImGui::Selectable(m_componentList[i].m_combinedEntityIdAndName.c_str(), m_componentList[i].m_entityID == m_selectedEntity.m_entityID))
                        {
                            m_selectedEntity.setEntity(m_componentList[i].m_entityID);
                        }
                    }

                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Prefabs"))
                {
                    auto &prefabList = m_systemScene->getSceneLoader()->getPrefabs();

                    for(auto const &[name, prefab] : prefabList)
                    {
                        if(ImGui::Selectable(name.c_str(), false))
                        {
                            //m_selectedEntity.setEntity(m_componentList[i].m_entityID);
                        }
                    }

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    //	 ____________________________
    //	|							 |
    //	|	     RIGHT WINDOW        |
    //	|____________________________|
    //
    {
        ImGui::SetNextWindowClass(&windowClassWithNoTabBar);
        if(ImGui::Begin("##RightWindow", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
        {
            if(ImGui::BeginTabBar("##RightWindowTabBar", ImGuiTabBarFlags_None))
            {
                if(ImGui::BeginTabItem("Inspector"))
                {
                    if(m_selectedEntity)
                    {
                        if(ImGui::BeginChild("##InspectorWindow", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_::ImGuiWindowFlags_None))
                        {
                            // Clear the component exist flags as they are reset every frame
                            m_selectedEntity.clearComponentExistFlags();

                            // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                            float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                            // Calculate the offset for the collapsing header that is drawn after the delete component button of each component type
                            const float headerOffsetAfterDeleteButton = m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 4.0f;

                            // WORLD COMPONENTS
                            auto *metadataComponent = entityRegistry.try_get<MetadataComponent>(m_selectedEntity.m_entityID);
                            if(metadataComponent != nullptr)
                            {
                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::MetadataComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    // Get the current entity name
                                    m_selectedEntity.m_componentData.m_name = metadataComponent->getName();
                                    m_selectedEntity.m_componentData.m_id = metadataComponent->getEntityID();
                                    m_selectedEntity.m_componentData.m_parent = metadataComponent->getParentEntityID();

                                    if(m_selectedEntity.m_prefabNameModified)
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, metadataComponent, Systems::Changes::World::PrefabName);
                                    else
                                        m_selectedEntity.m_componentData.m_prefab = metadataComponent->getPrefabName();

                                    const std::string parentSelectionPopupName = "##ParentSelectionPopup";

                                    // Draw NAME
                                    drawLeftAlignedLabelText("Name:", inputWidgetOffset);
                                    if(ImGui::InputText("##NameStringInput", &m_selectedEntity.m_componentData.m_name, ImGuiInputTextFlags_EnterReturnsTrue))
                                    {
                                        // If the prefab name was changed, send a notification to the Metadata Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, metadataComponent, Systems::Changes::Generic::Name);
                                    }

                                    // Draw PREFAB
                                    if(!m_selectedEntity.m_componentData.m_prefab.empty())
                                    {
                                        drawLeftAlignedLabelText("Prefab:", inputWidgetOffset, calcTextSizedButtonOffset(2) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                                        if(ImGui::InputText("##PrefabStringInput", &m_selectedEntity.m_componentData.m_prefab, ImGuiInputTextFlags_EnterReturnsTrue))
                                        {
                                            // If the prefab name was changed, send a notification to the Metadata Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, metadataComponent, Systems::Changes::World::PrefabName);
                                        }                                    
                                    
                                        // Draw PREFAB OPEN button
                                        ImGui::SameLine(calcTextSizedButtonOffset(2));
                                        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##PrefabOpenButton", "Open a prefab file"))
                                        {
                                            // Only open the file browser if it's not opened already
                                            if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                            {
                                                // Set the file browser activation to Prefab File
                                                m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_PrefabFile;

                                                // Define file browser variables
                                                m_fileBrowserDialog.m_filter = "Prefab files (.prefab){.prefab,.PREFAB},All files{.*}";
                                                m_fileBrowserDialog.m_title = "Open a prefab file";
                                                m_fileBrowserDialog.m_name = "OpenPrefabFileFileDialog";
                                                m_fileBrowserDialog.m_rootPath = Config::filepathVar().prefab_path;
                                                m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;
                                                m_fileBrowserDialog.m_userStringPointer = &m_selectedEntity.m_componentData.m_prefab;

                                                // Tell the GUI scene to open the file browser
                                                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                            }
                                        }

                                        const std::string prefabSelectionPopupName = "##PrefabSelectionPopup";

                                        // Draw OPEN PREFAB LIST button
                                        ImGui::SameLine(calcTextSizedButtonOffset(1));
                                        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##OpenPrefabListButton", "Choose a prefab from the loaded prefabs"))
                                        {
                                            // Open the pop-up with the prefab asset list
                                            ImGui::OpenPopup(prefabSelectionPopupName.c_str());
                                        }                                
                                    
                                        // Draw PREFAB DELETE button
                                        ImGui::SameLine(calcTextSizedButtonOffset(0));
                                        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##PrefabDeleteButton", "Delete the prefab"))
                                        {
                                            m_selectedEntity.m_componentData.m_prefab = "";

                                            // If the prefab name was changed, send a notification to the Metadata Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, metadataComponent, Systems::Changes::World::PrefabName);
                                        }                                        
                                    
                                        // Draw PREFABLIST
                                        if(ImGui::BeginPopup(prefabSelectionPopupName.c_str()))
                                        {
                                            // Remove selection border and align text vertically
                                            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
                                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                                            auto &prefabList = m_systemScene->getSceneLoader()->getPrefabs();

                                            for(auto const &[name, prefab] : prefabList)
                                            {
                                                // Draw PREFAB NAME selection
                                                // Set the text height to the texture image button height
                                                if(ImGui::Selectable(name.c_str(), (m_selectedEntity.m_componentData.m_prefab == name), 0, ImVec2(0.0f, m_assetSelectionPopupImageSize.y)))
                                                {
                                                    m_selectedEntity.m_componentData.m_prefab = name;

                                                    // If the prefab name was changed, send a notification to the Metadata Component
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, metadataComponent, Systems::Changes::World::PrefabName);
                                                }
                                            }

                                            ImGui::PopStyleVar(2); //ImGuiStyleVar_SelectableTextAlign, ImGuiStyleVar_FramePadding
                                            ImGui::EndPopup();
                                        }
                                    }

                                    // Draw ENTITY ID
                                    drawLeftAlignedLabelText("Entity ID:", inputWidgetOffset);
                                    if(ImGui::InputScalar("##EntityIDInput", ImGuiDataType_U32, &m_selectedEntity.m_componentData.m_id))
                                    {

                                    }

                                    // Draw PARENT
                                    drawLeftAlignedLabelText("Parent ID:", inputWidgetOffset);
                                    if(ImGui::BeginCombo("##ParentIDCombo", Utilities::toString(m_selectedEntity.m_componentData.m_parent).c_str()))
                                    {
                                        // Go over all existing entities
                                        for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size; i++)
                                        {
                                            // Mark which parent ID is selected
                                            const bool is_selected = (m_selectedEntity.m_componentData.m_parent == m_entityList[i].m_entityID);

                                            // Don't show entities own ID as a parent ID selection
                                            if(m_entityList[i].m_entityID != m_selectedEntity.m_componentData.m_id)
                                            {
                                                if(ImGui::Selectable(Utilities::toString(m_entityList[i].m_entityID).c_str(), is_selected))
                                                {
                                                    m_selectedEntity.m_componentData.m_parent = m_entityList[i].m_entityID;
                                                }
                                            }

                                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                            if(is_selected)
                                                ImGui::SetItemDefaultFocus();
                                        }
                                        ImGui::EndCombo();
                                    }
                                }
                            }
                            auto *spatialComponent = entityRegistry.try_get<SpatialComponent>(m_selectedEntity.m_entityID);
                            if(spatialComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_SpatialComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##SpatialComponentDeleteButton", "Delete the Spatial component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_SpatialComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::SpatialComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(m_selectedEntity.m_entityID);

                                    // Get the current spatial data from the selected entity spatial component
                                    m_selectedEntity.m_spatialDataManager = spatialComponent->getSpatialDataChangeManager();

                                    // Draw ACTIVE
                                    m_selectedEntity.m_componentData.m_worldComponents.m_spatialConstructionInfo->m_active = spatialComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##SpatialComponentActive", &m_selectedEntity.m_componentData.m_worldComponents.m_spatialConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Spatial Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Generic::Active);
                                    }

                                    if(ImGui::BeginTabBar("##SpatialComponentTabBar", ImGuiTabBarFlags_None))
                                    {
                                        if(ImGui::BeginTabItem("Local"))
                                        {
                                            // Draw POSITION
                                            drawLeftAlignedLabelText("Position:", inputWidgetOffset);
                                            if(ImGui::DragFloat3("##LocalPositionDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_position), Config::GUIVar().editor_float_slider_speed))
                                            {
                                                // If the position vector was changed, set the new position in the spatial data manager (so it can set the appropriate dirty flags internally)
                                                m_selectedEntity.m_spatialDataManager.setLocalPosition(m_selectedEntity.m_spatialDataManager.getLocalSpaceData().m_spatialData.m_position);
                                                // Update the spatial data manager (so it updates the transform matrix internally)
                                                m_selectedEntity.m_spatialDataManager.update();

                                                // If the position vector was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                                if(rigidBodyComponent != nullptr)
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                                else
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                            }
                                            captureMouseWhileItemActive();

                                            // Draw ROTATION
                                            // Make sure to get the current local rotation euler angles, as they are not automatically updated
                                            m_selectedEntity.m_spatialDataManager.calculateLocalRotationEuler();
                                            drawLeftAlignedLabelText("Rotation:", inputWidgetOffset);
                                            if(ImGui::DragFloat3("##LocalRotationDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_rotationEuler), Config::GUIVar().editor_float_slider_speed))
                                            {
                                                // If the rotation vector was changed, set the new rotation in the spatial data manager (so it can set the appropriate dirty flags internally)
                                                m_selectedEntity.m_spatialDataManager.setLocalRotation(m_selectedEntity.m_spatialDataManager.getLocalSpaceData().m_spatialData.m_rotationEuler);
                                                // Update the spatial data manager (so it updates the transform matrix internally)
                                                m_selectedEntity.m_spatialDataManager.update();

                                                // If the rotation vector was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                                if(rigidBodyComponent != nullptr)
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                                else
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                            }
                                            captureMouseWhileItemActive();

                                            // Draw SCALE
                                            drawLeftAlignedLabelText("Scale:", inputWidgetOffset);
                                            if(ImGui::DragFloat3("##LocalScaleDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_scale), Config::GUIVar().editor_float_slider_speed, 0.01f, 10000.0f))
                                            {
                                                // If the scale vector was changed, send a notification to the spatial component
                                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalScale);
                                            }
                                            captureMouseWhileItemActive();

                                            ImGui::EndTabItem();
                                        }

                                        // World spatial data is read-only, i.e. only used for viewing the data, as there is no useful need for modifying it
                                        if(ImGui::BeginTabItem("World"))
                                        {
                                            auto worldTransformNoScale = m_selectedEntity.m_spatialDataManager.getWorldTransformWithScale();
                                            auto rotationEuler = glm::degrees(glm::eulerAngles(glm::toQuat(worldTransformNoScale)));

                                            // Draw POSITION
                                            drawLeftAlignedLabelText("Position:", inputWidgetOffset);
                                            if(ImGui::DragFloat3("##WorldPositionDrag", glm::value_ptr(worldTransformNoScale[3]), Config::GUIVar().editor_float_slider_speed))
                                            {
                                                /*/ If the position vector was changed, set the new world transform in the spatial data manager (so it can set the appropriate dirty flags internally)
                                                m_selectedEntity.m_spatialDataManager.setWorldTransform(worldTransformNoScale);
                                                // Update the spatial data manager (so it updates the transform matrix internally)
                                                m_selectedEntity.m_spatialDataManager.update();

                                                // If the position vector was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                                if(rigidBodyComponent != nullptr)
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::WorldTransformNoScale);
                                                else
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::WorldTransformNoScale);*/
                                            }

                                            // Draw ROTATION
                                            drawLeftAlignedLabelText("Rotation:", inputWidgetOffset);
                                            if(ImGui::DragFloat3("##WorldRotationDrag", glm::value_ptr(rotationEuler), Config::GUIVar().editor_float_slider_speed))
                                            {
                                                /*/ If the rotation vector was changed, set the new world transform in the spatial data manager (so it can set the appropriate dirty flags internally)
                                                m_selectedEntity.m_spatialDataManager.setWorldTransform(Math::createTransformMat(worldTransformNoScale[3], Math::eulerDegreesToQuaterion(rotationEuler)));
                                                // Update the spatial data manager (so it updates the transform matrix internally)
                                                m_selectedEntity.m_spatialDataManager.update();

                                                // If the rotation vector was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                                if(rigidBodyComponent != nullptr)
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::WorldTransformNoScale);
                                                else
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::WorldTransformNoScale);*/
                                            }

                                            // Draw SCALE
                                            drawLeftAlignedLabelText("Scale:", inputWidgetOffset);
                                            if(ImGui::DragFloat3("##WorldScaleDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_scale), Config::GUIVar().editor_float_slider_speed, 0.01f, 10000.0f))
                                            {
                                                // If the scale vector was changed, send a notification to the spatial component
                                                //m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalScale);
                                            }
                                            ImGui::EndTabItem();
                                        }
                                        ImGui::EndTabBar();
                                    }
                                }
                            }
                            auto *objectMaterialComponent = entityRegistry.try_get<ObjectMaterialComponent>(m_selectedEntity.m_entityID);
                            if(objectMaterialComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_ObjectMaterialComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##ObjectMaterialComponentDeleteButton", "Delete the Object Material component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_ObjectMaterialComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::ObjectMaterialComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    // Get the current object material type from the selected entity Object Material Component
                                    m_selectedEntity.m_objectMaterialType = objectMaterialComponent->getObjectMaterialType();

                                    // Draw ACTIVE
                                    m_selectedEntity.m_componentData.m_worldComponents.m_objectMaterialConstructionInfo->m_active = objectMaterialComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##ObjectMaterialComponentActive", &m_selectedEntity.m_componentData.m_worldComponents.m_objectMaterialConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Object Material Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, objectMaterialComponent, Systems::Changes::Generic::Active);
                                    }

                                    // Draw OBJECT MATERIAL TYPE
                                    drawLeftAlignedLabelText("Material type:", inputWidgetOffset);
                                    if(ImGui::Combo("##ObjectMaterialTypePicker", &m_selectedEntity.m_objectMaterialType, &m_physicalMaterialProperties[0], ObjectMaterialType::NumberOfMaterialTypes))
                                    {
                                        // If the object material type was changed, send a notification to the Object Material Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, objectMaterialComponent, Systems::Changes::World::ObjectMaterialType);
                                    }
                                }
                            }

                            // GRAPHICS COMPONENTS
                            auto *cameraComponent = entityRegistry.try_get<CameraComponent>(m_selectedEntity.m_entityID);
                            if(cameraComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_CameraComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##CameraComponentDeleteButton", "Delete the Camera component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_CameraComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::CameraComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_active = cameraComponent->isObjectActive();
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_cameraID = cameraComponent->getCameraID();
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_fov = cameraComponent->getCameraFOV();
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zFar = cameraComponent->getCameraFarClip();
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zNear = cameraComponent->getCameraNearClip();

                                    // Draw ACTIVE
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##CameraActive", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Camera Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, cameraComponent, Systems::Changes::Generic::Active);
                                    }                                
                                
                                    // Draw CAMERA ID
                                    drawLeftAlignedLabelText("Camera ID:", inputWidgetOffset);
                                    if(ImGui::InputInt("##CameraIDInput", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_cameraID))
                                    {
                                        // If the camera ID was changed, send a notification to the Camera Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, cameraComponent, Systems::Changes::Graphics::CameraID);
                                    }

                                    // Draw FOV
                                    drawLeftAlignedLabelText("FOV:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##CameraFOVDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_fov, 1.0f, 1.0f, 180.0f, "%.0f"))
                                    {
                                        // If the FOV was changed, send a notification to the Camera Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, cameraComponent, Systems::Changes::Graphics::FOV);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw FAR PLANE
                                    drawLeftAlignedLabelText("Far plane:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##CameraFarPlaneDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zFar, 0.1f, 0.0f, 100000.0f, "%.5f"))
                                    {
                                        // If the far plane was changed, send a notification to the Camera Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, cameraComponent, Systems::Changes::Graphics::ZFar);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw NEAR PLANE
                                    drawLeftAlignedLabelText("Near plane:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##CameraNearPlaneDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zNear, 0.0001f, 0.0f, 100000.0f, "%.5f"))
                                    {
                                        // If the near plane was changed, send a notification to the Camera Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, cameraComponent, Systems::Changes::Graphics::ZNear);
                                    }
                                    captureMouseWhileItemActive();
                                }
                            }
                            auto *lightComponent = entityRegistry.try_get<LightComponent>(m_selectedEntity.m_entityID);
                            if(lightComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_LightComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##LightComponentDeleteButton", "Delete the Light component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_LightComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::LightComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    const char *lightTypeStrings[] = { "Directional", "Point", "Spot", "null" };
                                    m_selectedEntity.m_lightType = lightComponent->getLightType();

                                    // Adjust the light type number to the different lineup of light type strings (so that the null type wouldn't get shown as an option)
                                    auto lightTypeAdjusted = m_selectedEntity.m_lightType - 1;
                                    if(lightTypeAdjusted < 0)
                                        lightTypeAdjusted = LightComponent::LightComponentType::LightComponentType_spot;

                                    // Draw ACTIVE
                                    if(!m_lightComponentActivateAllSet)
                                        m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_active = lightComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##LightComponentActive", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Light Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Generic::Active);
                                    }

                                    // Draw LIGHT TYPE
                                    drawLeftAlignedLabelText("Light type:", inputWidgetOffset);
                                    if(ImGui::Combo("##LightTypePicker", &lightTypeAdjusted, lightTypeStrings, m_selectedEntity.m_lightType == LightComponent::LightComponentType::LightComponentType_null ? 4 : 3))
                                    {
                                        // Convert the adjusted light type back to the original
                                        m_selectedEntity.m_lightType = lightTypeAdjusted + 1;
                                        if(m_selectedEntity.m_lightType > LightComponent::LightComponentType::LightComponentType_spot)
                                            m_selectedEntity.m_lightType = LightComponent::LightComponentType::LightComponentType_null;

                                        if(m_selectedEntity.m_lightType != LightComponent::LightComponentType::LightComponentType_null)
                                        {
                                            // If the light type was changed, send a notification to the Light Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::LightType);
                                        }
                                    }

                                    switch(lightComponent->getLightType())
                                    {
                                        case LightComponent::LightComponentType::LightComponentType_null:
                                            break;

                                        case LightComponent::LightComponentType::LightComponentType_directional:
                                            {
                                                auto lightDataSet = lightComponent->getDirectionalLightSafe();
                                                if(lightDataSet != nullptr)
                                                {
                                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color = lightDataSet->m_color;
                                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity = lightDataSet->m_intensity;

                                                    // Draw COLOR
                                                    drawLeftAlignedLabelText("Color:", inputWidgetOffset);
                                                    if(ImGui::ColorEdit3("##DirectionalLightColorEdit", glm::value_ptr(m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color), m_colorEditFlags))
                                                    {
                                                        // If the light color was changed, send a notification to the Light Component
                                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::Color);
                                                    }

                                                    // Draw INTENSITY
                                                    drawLeftAlignedLabelText("Intensity:", inputWidgetOffset);
                                                    if(ImGui::DragFloat("##DirectionalLightIntensityDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                                    {
                                                        // If the light intensity was changed, send a notification to the Light Component
                                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::Intensity);
                                                    }
                                                    captureMouseWhileItemActive();
                                                }
                                            }
                                            break;

                                        case LightComponent::LightComponentType::LightComponentType_point:
                                            {
                                                auto lightDataSet = lightComponent->getPointLightSafe();
                                                if(lightDataSet != nullptr)
                                                {
                                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color = lightDataSet->m_color;
                                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity = lightDataSet->m_intensity;

                                                    // Draw COLOR
                                                    drawLeftAlignedLabelText("Color:", inputWidgetOffset);
                                                    if(ImGui::ColorEdit3("##DirectionalLightColorEdit", glm::value_ptr(m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color), m_colorEditFlags))
                                                    {
                                                        // If the light color was changed, send a notification to the Light Component
                                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::Color);
                                                    }

                                                    // Draw INTENSITY
                                                    drawLeftAlignedLabelText("Intensity:", inputWidgetOffset);
                                                    if(ImGui::DragFloat("##PointLightIntensityDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                                    {
                                                        // If the light intensity was changed, send a notification to the Light Component
                                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::Intensity);
                                                    }
                                                    captureMouseWhileItemActive();
                                                }
                                            }
                                            break;

                                        case LightComponent::LightComponentType::LightComponentType_spot:
                                            {
                                                auto lightDataSet = lightComponent->getSpotLightSafe();
                                                if(lightDataSet != nullptr)
                                                {
                                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color = lightDataSet->m_color;
                                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity = lightDataSet->m_intensity;
                                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle = lightDataSet->m_cutoffAngle;

                                                    // Draw COLOR
                                                    drawLeftAlignedLabelText("Color:", inputWidgetOffset);
                                                    if(ImGui::ColorEdit3("##DirectionalLightColorEdit", glm::value_ptr(m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color), m_colorEditFlags))
                                                    {
                                                        // If the light color was changed, send a notification to the Light Component
                                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::Color);
                                                    }

                                                    // Draw INTENSITY
                                                    drawLeftAlignedLabelText("Intensity:", inputWidgetOffset);
                                                    if(ImGui::DragFloat("##SpotLightIntensityDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                                    {
                                                        // If the light intensity was changed, send a notification to the Light Component
                                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::Intensity);
                                                    }
                                                    captureMouseWhileItemActive();

                                                    // Draw CUTOFF ANGLE
                                                    auto cutoffAngleInDegrees = glm::degrees(m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle);
                                                    drawLeftAlignedLabelText("Cutoff angle:", inputWidgetOffset);
                                                    if(ImGui::DragFloat("##SpotLightCutoffAngleDrag", &cutoffAngleInDegrees, Config::GUIVar().editor_float_slider_speed, 0.0f, 360.0f))
                                                    {
                                                        m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle = glm::radians(cutoffAngleInDegrees);

                                                        // If the light cutoff angle was changed, send a notification to the Light Component
                                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::CutoffAngle);
                                                    }
                                                    captureMouseWhileItemActive();
                                                }
                                            }
                                            break;
                                    }
                                }
                            }
                            auto *modelComponent = entityRegistry.try_get<ModelComponent>(m_selectedEntity.m_entityID);
                            if(modelComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_ModelComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##ModelComponentDeleteButton", "Delete the Model component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_ModelComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::ModelComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    if(auto *loadComponent = entityRegistry.try_get<GraphicsLoadToMemoryComponent>(m_selectedEntity.m_entityID); loadComponent == nullptr)
                                    {
                                        if(!modelComponent->getLoadPending())
                                        {
                                            bool modelComponentDataNeedsUpdating = false;

                                            // If the model data was modified the previous frame, set the flag to update the ModelComponent
                                            if(m_selectedEntity.m_modelDataModified)
                                            {
                                                m_selectedEntity.m_modelDataModified = false;
                                                modelComponentDataNeedsUpdating = true;
                                            }
                                            else
                                            {
                                                // If the model data was recreated (meaning model data was changed), update the ModelComponent data
                                                auto &currentModelData = modelComponent->getModelData();
                                                if(m_selectedEntity.m_modelDataPointer != &currentModelData)
                                                {
                                                    m_selectedEntity.m_modelDataPointer = &currentModelData;
                                                    modelComponentDataNeedsUpdating = true;
                                                }
                                                else
                                                {
                                                    // If ModelComponent has been loaded after sending it new data, update the ModelComponent data
                                                    if(m_selectedEntity.m_modelDataUpdateAfterLoading)
                                                        if(modelComponent->isLoadedToMemory())
                                                        {
                                                            m_selectedEntity.m_modelDataUpdateAfterLoading = false;
                                                            modelComponentDataNeedsUpdating = true;
                                                        }
                                                }
                                            }

                                            // Update the Models Properties of the currently selected ModelComponent
                                            if(modelComponentDataNeedsUpdating)
                                            {
                                                m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.clear();
                                                modelComponent->getModelsProperties(m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties);
                                            }
                                        }

                                        // Draw ACTIVE
                                        if(!m_modelComponentActivateAllSet)
                                            m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_active = modelComponent->isObjectActive();
                                        drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                        if(ImGui::Checkbox("##ModelComponentActive", &m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_active))
                                        {
                                            // If the active flag was changed, send a notification to the Model Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, modelComponent, Systems::Changes::Generic::Active);
                                        }

                                        // Go over each model
                                        for(decltype(m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.size()) modelSize = m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.size(),
                                            modelIndex = 0; modelIndex < modelSize; modelIndex++)
                                        {
                                            auto &modelEntry = m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models[modelIndex];

                                            // Draw MODEL FILENAME
                                            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(3) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                                            if(ImGui::InputText(("##" + Utilities::toString(modelIndex) + "ModelFileInput").c_str(), &modelEntry.m_modelName, ImGuiInputTextFlags_EnterReturnsTrue))
                                            {
                                                // If the model filename was changed, set the modified flag
                                                m_selectedEntity.m_modelDataModified = true;
                                            }

                                            // Draw MODEL OPEN button
                                            ImGui::SameLine(calcTextSizedButtonOffset(3));
                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##" + Utilities::toString(modelIndex) + "ModelFileOpenButton", "Open a model file"))
                                            {
                                                // Only open the file browser if it's not opened already
                                                if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                                {
                                                    // Set the selected model filename handle
                                                    m_selectedEntity.m_selectedModelName = &modelEntry.m_modelName;

                                                    // Set the file browser activation to Model File
                                                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_ModelFile;

                                                    // Define file browser variables
                                                    m_fileBrowserDialog.m_filter = "Model files (.obj .3ds .fbx){.obj,.OBJ,.3ds,.3DS,.fbx,.FBX},All files{.*}";
                                                    m_fileBrowserDialog.m_title = "Open a model file";
                                                    m_fileBrowserDialog.m_name = "OpenModelFileFileDialog";
                                                    m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                                                    // Set the root path only if it isn't saved from the last file dialog
                                                    if(m_previouslyOpenedFileBrowser != m_currentlyOpenedFileBrowser)
                                                        m_fileBrowserDialog.m_rootPath = Config::filepathVar().model_path;

                                                    // Tell the GUI scene to open the file browser
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                                }
                                            }

                                            const std::string modelSelectionPopupName = "##" + Utilities::toString(modelIndex) + "ModelSelectionPopup";

                                            // Draw OPEN ASSET LIST button
                                            ImGui::SameLine(calcTextSizedButtonOffset(2));
                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##" + Utilities::toString(modelIndex) + "ModelOpenAssetListButton", "Choose a model from the loaded assets"))
                                            {
                                                // Open the pop-up with the model asset list
                                                ImGui::OpenPopup(modelSelectionPopupName.c_str());
                                            }

                                            // Draw MODEL RELOAD button
                                            ImGui::SameLine(calcTextSizedButtonOffset(1));
                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##" + Utilities::toString(modelIndex) + "ModelFileReloadButton", "Reload the model file"))
                                            {
                                                // Set the modified flag
                                                m_selectedEntity.m_modelDataModified = true;
                                            }

                                            // Draw MODEL DELETE button
                                            ImGui::SameLine(calcTextSizedButtonOffset(0));
                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##" + Utilities::toString(modelIndex) + "ModelFileDeleteButton", "Delete the model entry"))
                                            {
                                                m_selectedEntity.m_modelDataModified = true;
                                                m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.erase(m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.begin() + modelIndex);
                                                modelSize = m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.size();
                                                modelIndex--;
                                            }
                                            else
                                            {
                                                // Draw MODEL ASSET LIST
                                                if(ImGui::BeginPopup(modelSelectionPopupName.c_str()))
                                                {
                                                    // Calculate the text size based on the longest model asset name
                                                    ImVec2 nameTextSize(ImGui::CalcTextSize(m_modelAssetLongestName.c_str()).x + m_imguiStyle.FramePadding.x * 2.0f, m_assetSelectionPopupImageSize.y);

                                                    // Remove selection border and align text vertically
                                                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
                                                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                                                    for(decltype(m_modelAssets.size()) i = 0, size = m_modelAssets.size(); i < size; i++)
                                                    {
                                                        // Draw MODEL NAME selection
                                                        // Set the text height to the texture image button height
                                                        if(ImGui::Selectable(m_modelAssets[i].second.c_str(), (modelEntry.m_modelName == m_modelAssets[i].second), 0, nameTextSize))
                                                        {
                                                            // Set the selected model
                                                            modelEntry.m_modelName = m_modelAssets[i].second;

                                                            // Set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                                    }
                                                    ImGui::PopStyleVar(2); //ImGuiStyleVar_SelectableTextAlign, ImGuiStyleVar_FramePadding
                                                    ImGui::EndPopup();
                                                }

                                                const ImVec2 faceCullingWindowSize(0.0f, (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * 3.0f);
                                                const char *faceCullingTypeString[] = { "Front", "Back" };
                                            
                                                {
                                                    // Draw FACE CULLING
                                                    drawLeftAlignedLabelText("Face culling (draw):", inputWidgetOffset);
                                                    if(ImGui::Checkbox("##MCCullingDrawEnabledCheckbox", &modelEntry.m_drawFaceCulling.m_faceCullingEnabled))
                                                    {
                                                        // Set the modified flag
                                                        m_selectedEntity.m_modelDataModified = true;
                                                    }

                                                    ImGui::SameLine();
                                                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                                                    ImGui::BeginDisabled(!modelEntry.m_drawFaceCulling.m_faceCullingEnabled);

                                                    // Draw CULL FACE TYPE
                                                    int faceCullingType = modelEntry.m_drawFaceCulling.m_backFaceCulling ? 1 : 0;
                                                    if(ImGui::Combo("##MCCullingDrawFacePicker", &faceCullingType, faceCullingTypeString, 2))
                                                    {
                                                        modelEntry.m_drawFaceCulling.m_backFaceCulling = faceCullingType == 1 ? true : false;

                                                        // Set the modified flag
                                                        m_selectedEntity.m_modelDataModified = true;
                                                    }

                                                    ImGui::EndDisabled();
                                                }

                                                {
                                                    // Draw FACE CULLING CSM
                                                    drawLeftAlignedLabelText("Face culling (CSM):", inputWidgetOffset);
                                                    if(ImGui::Checkbox("##MCCullingShadowEnabledCheckbox", &modelEntry.m_shadowFaceCulling.m_faceCullingEnabled))
                                                    {
                                                        // Set the modified flag
                                                        m_selectedEntity.m_modelDataModified = true;
                                                    }

                                                    ImGui::SameLine();
                                                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                                                    ImGui::BeginDisabled(!modelEntry.m_shadowFaceCulling.m_faceCullingEnabled);

                                                    // Draw CULL FACE
                                                    int faceCullingType = modelEntry.m_shadowFaceCulling.m_backFaceCulling ? 1 : 0;
                                                    if(ImGui::Combo("##MCCullingShadowFacePicker", &faceCullingType, faceCullingTypeString, 2))
                                                    {
                                                        modelEntry.m_shadowFaceCulling.m_backFaceCulling = faceCullingType == 1 ? true : false;

                                                        // Set the modified flag
                                                        m_selectedEntity.m_modelDataModified = true;
                                                    }

                                                    ImGui::EndDisabled();
                                                }

                                                if(!modelEntry.m_meshData.empty())
                                                {
                                                    // Draw ACTIVATE ALL MESHES
                                                    if(ImGui::Button("Activate all meshes"))
                                                    {
                                                        // Go over each mesh and set the active flag to true
                                                        for(decltype(modelEntry.m_meshData.size()) meshSize = modelEntry.m_meshData.size(), meshIndex = 0; meshIndex < meshSize; meshIndex++)
                                                            modelEntry.m_meshData[meshIndex].m_active = true;

                                                        // Set the modified flag
                                                        m_selectedEntity.m_modelDataModified = true;
                                                    }

                                                    ImGui::SameLine();

                                                    // Draw DEACTIVATE ALL MESHES
                                                    if(ImGui::Button("Deactivate all meshes"))
                                                    {
                                                        // Go over each mesh and set the active flag to false
                                                        for(decltype(modelEntry.m_meshData.size()) meshSize = modelEntry.m_meshData.size(), meshIndex = 0; meshIndex < meshSize; meshIndex++)
                                                            modelEntry.m_meshData[meshIndex].m_active = false;

                                                        // Set the modified flag
                                                        m_selectedEntity.m_modelDataModified = true;
                                                    }
                                                }

                                                ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));
                                                for(decltype(modelEntry.m_meshData.size()) meshSize = modelEntry.m_meshData.size(), meshIndex = 0; meshIndex < meshSize; meshIndex++)
                                                {
                                                    // Get the mesh name
                                                    std::string meshName = modelEntry.m_meshData[meshIndex].m_meshName;
                                                    if(!meshName.empty())
                                                        meshName = " (" + meshName + ")";

                                                    // Draw MESH
                                                    if(ImGui::TreeNodeEx(("Mesh " + Utilities::toString(meshIndex) + meshName + ":").c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) // ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf
                                                    {
                                                        const float cursorStartingPos = ImGui::GetCursorPosX();

                                                        ImGui::SeparatorText("Mesh settings:");

                                                        // Draw ACTIVE
                                                        bool active = modelEntry.m_meshData[meshIndex].m_active;
                                                        drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                                        if(ImGui::Checkbox(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "ActiveCheckbox").c_str(), &active))
                                                        {
                                                            modelEntry.m_meshData[meshIndex].m_active = active;

                                                            // If the active flag was changed, set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }

                                                        // Draw HEIGHT SCALE
                                                        drawLeftAlignedLabelText("Height scale:", inputWidgetOffset);
                                                        if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "HeightScaleDrag").c_str(), &modelEntry.m_meshData[meshIndex].m_heightScale, Config::GUIVar().editor_float_slider_speed, 0.0f, 100000.0f))
                                                        {
                                                            // If the height scale was changed, set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                                        captureMouseWhileItemActive();

                                                        // Draw ALPHA THRESHOLD
                                                        drawLeftAlignedLabelText("Alpha Threshold:", inputWidgetOffset);
                                                        if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "AlphaThresholdDrag").c_str(), &modelEntry.m_meshData[meshIndex].m_alphaThreshold, Config::GUIVar().editor_float_slider_speed, 0.0f, 1.0f))
                                                        {
                                                            // If the alpha threshold was changed, set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                                        captureMouseWhileItemActive();

                                                        // Draw EMISSIVE INTENSITY
                                                        drawLeftAlignedLabelText("Emissive intensity:", inputWidgetOffset);
                                                        if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "EmissiveIntensityDrag").c_str(), &modelEntry.m_meshData[meshIndex].m_emissiveIntensity, Config::GUIVar().editor_float_slider_speed, 0.0f, 100000.0f))
                                                        {
                                                            // If the emissive intensity was changed, set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                                        captureMouseWhileItemActive();

                                                        // Draw STOCHASTIC SAMPLING
                                                        bool textureRepetition = modelEntry.m_meshData[meshIndex].m_stochasticSampling;
                                                        drawLeftAlignedLabelText("Stochastic sampling:", inputWidgetOffset);
                                                        if(ImGui::Checkbox(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "TextureRepetitionCheckbox").c_str(), &textureRepetition))
                                                        {
                                                            modelEntry.m_meshData[meshIndex].m_stochasticSampling = textureRepetition;

                                                            // If the texture repetition flag was changed, set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                                    
                                                        // Draw TEXTURE REPETITION SALCE
                                                        drawLeftAlignedLabelText("Stochastic scale:", inputWidgetOffset);
                                                        if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "TextureRepetitionScaleDrag").c_str(), &modelEntry.m_meshData[meshIndex].m_stochasticSamplingScale, Config::GUIVar().editor_float_slider_speed))
                                                        {
                                                            // If the texture repetition scale was changed, set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                                        captureMouseWhileItemActive();

                                                        // Get the current texture wrap mode
                                                        int textureWrapMode = 0;
                                                        for(decltype(m_textureWrapModeTypes.size()) i = 0, size = m_textureWrapModeTypes.size(); i < size; i++)
                                                        {
                                                            if(m_textureWrapModeTypes[i] == modelEntry.m_meshData[meshIndex].m_textureWrapMode)
                                                            {
                                                                textureWrapMode = (int)i;
                                                                break;
                                                            }
                                                        }

                                                        // Draw TEXTURE WRAP MODE
                                                        drawLeftAlignedLabelText("Texture wrap mode:", inputWidgetOffset);
                                                        if(ImGui::Combo(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "TextureWrapModeCombo").c_str(), &textureWrapMode, &m_textureWrapModeStrings[0], (int)m_textureWrapModeStrings.size()))
                                                        {
                                                            // Set the texture wrap mode
                                                            modelEntry.m_meshData[meshIndex].m_textureWrapMode = m_textureWrapModeTypes[textureWrapMode];

                                                            // Set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                               
                                                        // Draw 2D TEXTURE SCALE
                                                        drawLeftAlignedLabelText("2D texture scale:", inputWidgetOffset);
                                                        if(ImGui::Checkbox(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "2DTextureScaleCheckbox").c_str(), &m_2DTextureScale))
                                                        {

                                                        }

                                                        ImGui::SameLine();
                                                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (inputWidgetOffset / 5.0f));

                                                        // Draw SYNCHRONIZE TEXTURE SCALE
                                                        ImGui::Text("Sync across all textures:");
                                                        ImGui::SameLine();
                                                        if(ImGui::Checkbox(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "SyncTextureScaleCheckbox").c_str(), &m_synchronizeTextureScale))
                                                        {

                                                        }

                                                        // Draw 2D TEXTURE FRAMING
                                                        drawLeftAlignedLabelText("2D texture framing:", inputWidgetOffset);
                                                        if(ImGui::Checkbox(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "2DTextureFramingCheckbox").c_str(), &m_2DTextureFraming))
                                                        {

                                                        }

                                                        ImGui::SameLine();
                                                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (inputWidgetOffset / 5.0f));

                                                        // Draw SYNCHRONIZE TEXTURE FRAMING
                                                        ImGui::Text("Sync across all textures:");
                                                        ImGui::SameLine();
                                                        if(ImGui::Checkbox(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "SyncTextureFramingCheckbox").c_str(), &m_synchronizeTextureFraming))
                                                        {

                                                        }

                                                        for(unsigned int materialIndex = 0; materialIndex < MaterialType::MaterialType_NumOfTypes; materialIndex++)
                                                        {
                                                            // Convert material type to text
                                                            std::string materialTypeName;
                                                            switch(materialIndex)
                                                            {
                                                                case MaterialType_Diffuse:
                                                                    materialTypeName = "Diffuse texture:";
                                                                    break;
                                                                case MaterialType_Normal:
                                                                    materialTypeName = "Normal texture:";
                                                                    break;
                                                                case MaterialType_Emissive:
                                                                    materialTypeName = "Emissive texture:";
                                                                    break;
                                                                case MaterialType_Combined:
                                                                    materialTypeName = "RMHAO texture:";
                                                                    break;
                                                            }

                                                            ImGui::SeparatorText(materialTypeName.c_str());

                                                            // Draw TEXTURE FILENAME
                                                            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(2) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                                                            if(ImGui::InputText(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureFilenameInput").c_str(), &modelEntry.m_meshData[meshIndex].m_meshMaterials[materialIndex], ImGuiInputTextFlags_EnterReturnsTrue))
                                                            {
                                                                // If the texture filename was changed, set the modified flag
                                                                m_selectedEntity.m_modelDataModified = true;
                                                                modelEntry.m_meshData[meshIndex].m_present = true;
                                                            }

                                                            // Draw TEXTURE OPEN button
                                                            ImGui::SameLine(calcTextSizedButtonOffset(2));
                                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureOpenButton", "Open a texture file"))
                                                            {
                                                                // Only open the file browser if it's not opened already
                                                                if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                                                {
                                                                    // Set the selected texture filename handle
                                                                    m_selectedEntity.m_selectedTextureName = &modelEntry.m_meshData[meshIndex].m_meshMaterials[materialIndex];

                                                                    // Set the file browser activation to Texture File
                                                                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_TextureFile;

                                                                    // Define file browser variables
                                                                    m_fileBrowserDialog.m_filter = "Texture files (.png .tga .tif .tiff .jpg .jpeg .bmp){.png,.PNG,.tga,.TGA,.tif,.tiff,.jpg,.jpeg,.bmp},All files{.*}";
                                                                    m_fileBrowserDialog.m_title = "Open a texture file";
                                                                    m_fileBrowserDialog.m_name = "OpenTextureFileFileDialog";
                                                                    m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                                                                    // Set the root path only if it isn't saved from the last file dialog
                                                                    if(m_previouslyOpenedFileBrowser != m_currentlyOpenedFileBrowser)
                                                                        m_fileBrowserDialog.m_rootPath = Config::filepathVar().texture_path;

                                                                    // Tell the GUI scene to open the file browser
                                                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                                                }
                                                            }

                                                            const std::string textureSelectionPopupName = "##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureSelectionPopup";

                                                            // Draw OPEN ASSET LIST button
                                                            ImGui::SameLine(calcTextSizedButtonOffset(1));
                                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureOpenAssetListButton", "Choose a texture from the loaded assets"))
                                                            {
                                                                // Open the pop-up with the texture asset list
                                                                ImGui::OpenPopup(textureSelectionPopupName.c_str());
                                                            }

                                                            // Draw TEXTURE RELOAD button
                                                            ImGui::SameLine(calcTextSizedButtonOffset(0));
                                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureReloadButton", "Reload the texture file"))
                                                            {
                                                                // Set the modified flag
                                                                m_selectedEntity.m_modelDataModified = true;
                                                            }

                                                            // Draw TEXTURE ASSET LIST
                                                            if(ImGui::BeginPopup(textureSelectionPopupName.c_str()))
                                                            {
                                                                // Calculate the text size based on the longest texture asset name and the height of the texture image
                                                                ImVec2 nameTextSize(ImGui::CalcTextSize(m_textureAssetLongestName.c_str()).x + m_imguiStyle.FramePadding.x * 2.0f, m_assetSelectionPopupImageSize.y);

                                                                // Make button background transparent, remove button and selection border and align selection text vertically
                                                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                                                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                                                                ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));

                                                                for(decltype(m_textureAssets.size()) i = 0, size = m_textureAssets.size(); i < size; i++)
                                                                {
                                                                    if(m_textureAssets[i].first->isLoadedFromFile())
                                                                    {
                                                                        // Draw TEXTURE IMAGE
                                                                        if(ImGui::ImageButton((textureSelectionPopupName + "Image").c_str(),
                                                                            (ImTextureID)m_textureAssets[i].first->getHandle(),
                                                                            m_assetSelectionPopupImageSize,
                                                                            ImVec2(0, 1),
                                                                            ImVec2(1, 0),
                                                                            ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
                                                                        {
                                                                            // Set the selected texture
                                                                            modelEntry.m_meshData[meshIndex].m_meshMaterials[materialIndex] = m_textureAssets[i].second;

                                                                            // Set the modified flag
                                                                            m_selectedEntity.m_modelDataModified = true;

                                                                            ImGui::CloseCurrentPopup();
                                                                        }

                                                                        ImGui::SameLine();

                                                                        // Draw TEXTURE NAME selection
                                                                        // Set the text height to the texture image button height
                                                                        if(ImGui::Selectable(m_textureAssets[i].second.c_str(), (modelEntry.m_meshData[meshIndex].m_meshMaterials[materialIndex] == m_textureAssets[i].second), 0, nameTextSize))
                                                                        {
                                                                            // Set the selected texture
                                                                            modelEntry.m_meshData[meshIndex].m_meshMaterials[materialIndex] = m_textureAssets[i].second;
                                                                            modelEntry.m_meshData[meshIndex].m_present = true;

                                                                            // Set the modified flag
                                                                            m_selectedEntity.m_modelDataModified = true;
                                                                        }
                                                                    }
                                                                }
                                                                ImGui::PopStyleVar(2); //ImGuiStyleVar_FramePadding, ImGuiStyleVar_SelectableTextAlign
                                                                ImGui::PopStyleColor(); //ImGuiCol_Button
                                                                ImGui::EndPopup();
                                                            }

                                                            // Draw TEXTURE SCALE
                                                            drawLeftAlignedLabelText("Texture scale:", inputWidgetOffset);
                                                            if(m_2DTextureScale)
                                                            {
                                                                if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureScaleDrag").c_str(), &modelEntry.m_meshData[meshIndex].m_meshMaterialScales[materialIndex].x, Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    modelEntry.m_meshData[meshIndex].m_meshMaterialScales[materialIndex].y = modelEntry.m_meshData[meshIndex].m_meshMaterialScales[materialIndex].x;

                                                                    // Synchronize all the texture scales if a flag is set
                                                                    if(m_synchronizeTextureScale)
                                                                    {
                                                                        auto textureScale = modelEntry.m_meshData[meshIndex].m_meshMaterialScales[materialIndex];
                                                                        for(unsigned int i = 0; i < MaterialType::MaterialType_NumOfTypes; i++)
                                                                            modelEntry.m_meshData[meshIndex].m_meshMaterialScales[i] = textureScale;
                                                                    }

                                                                    // If the texture scale was changed, set the modified flag
                                                                    m_selectedEntity.m_modelDataModified = true;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if(ImGui::DragFloat2(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureScaleDrag2").c_str(), glm::value_ptr(modelEntry.m_meshData[meshIndex].m_meshMaterialScales[materialIndex]), Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    // Synchronize all the texture scales if a flag is set
                                                                    if(m_synchronizeTextureScale)
                                                                    {
                                                                        auto textureScale = modelEntry.m_meshData[meshIndex].m_meshMaterialScales[materialIndex];
                                                                        for(unsigned int i = 0; i < MaterialType::MaterialType_NumOfTypes; i++)
                                                                            modelEntry.m_meshData[meshIndex].m_meshMaterialScales[i] = textureScale;
                                                                    }

                                                                    // If the texture scale was changed, set the modified flag
                                                                    m_selectedEntity.m_modelDataModified = true;
                                                                }
                                                            }
                                                            captureMouseWhileItemActive();

                                                            // Draw TEXTURE FRAMING
                                                            drawLeftAlignedLabelText("Texture framing:", inputWidgetOffset);
                                                            if(m_2DTextureFraming)
                                                            {
                                                                if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureFramingDrag").c_str(), &modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[materialIndex].x, Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[materialIndex].y = modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[materialIndex].x;

                                                                    // Synchronize all the texture framings if a flag is set
                                                                    if(m_synchronizeTextureFraming)
                                                                    {
                                                                        auto textureFraming = modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[materialIndex];
                                                                        for(unsigned int i = 0; i < MaterialType::MaterialType_NumOfTypes; i++)
                                                                            modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[i] = textureFraming;
                                                                    }

                                                                    // If the texture framing was changed, set the modified flag
                                                                    m_selectedEntity.m_modelDataModified = true;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                if(ImGui::DragFloat2(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureFramingDrag2").c_str(), glm::value_ptr(modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[materialIndex]), Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    // Synchronize all the texture framings if a flag is set
                                                                    if(m_synchronizeTextureFraming)
                                                                    {
                                                                        auto textureFraming = modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[materialIndex];
                                                                        for(unsigned int i = 0; i < MaterialType::MaterialType_NumOfTypes; i++)
                                                                            modelEntry.m_meshData[meshIndex].m_meshMaterialFraming[i] = textureFraming;
                                                                    }

                                                                    // If the texture framing was changed, set the modified flag
                                                                    m_selectedEntity.m_modelDataModified = true;
                                                                }
                                                            }
                                                            captureMouseWhileItemActive();

                                                            // Draw TEXTURE COLOR
                                                            drawLeftAlignedLabelText("Texture color:", inputWidgetOffset);
                                                            if(ImGui::ColorEdit4(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureColorEdit").c_str(), glm::value_ptr(modelEntry.m_meshData[meshIndex].m_meshMaterialColors[materialIndex]), m_colorEditFlags))
                                                            {
                                                                // If the texture color was changed, set the modified flag
                                                                m_selectedEntity.m_modelDataModified = true;
                                                            }
                                                            captureMouseWhileItemActive();
                                                        }

                                                        ImGui::SeparatorText("");
                                                        ImGui::TreePop();
                                                    }
                                                }
                                                ImGui::PopStyleVar(); // ImGuiStyleVar_SeparatorTextAlign
                                            }
                                        }

                                        ImGui::Separator();

                                        // Calculate button size
                                        const char *addModelButtonLabel = "Add model";
                                        float addModelButtonWidth = ImGui::CalcTextSize(addModelButtonLabel).x * Config::GUIVar().editor_inspector_button_width_multiplier;

                                        // Set the button position to the right-most side
                                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - addModelButtonWidth);

                                        // Draw ADD MODEL button
                                        if(ImGui::Button(addModelButtonLabel, ImVec2(addModelButtonWidth, 0.0f)))
                                        {
                                            // Add an empty model entry
                                            m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.push_back(ModelComponent::MeshProperties());

                                            // Set the modified flag
                                            m_selectedEntity.m_modelDataModified = true;
                                        }

                                        // If the model data was modified, send the new data to the ModelComponent
                                        if(m_selectedEntity.m_modelDataUpdatedFromFilebrowser || m_selectedEntity.m_modelDataModified)
                                        {
                                            m_selectedEntity.m_modelDataModified = true;
                                            m_selectedEntity.m_modelDataUpdatedFromFilebrowser = false;

                                            m_systemScene->getSceneLoader()->getChangeController()->sendData(modelComponent, DataType::DataType_ModelsProperties, (void *)&m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties);
                                        }
                                    }
                                    else
                                    {
                                        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 32.0f) / 2.0f);
                                        ImSpinner::SpinnerFadeDots("##ModelLoadingSpinner", 32.0f, 4.0f, ImSpinner::white, 16.0f, 8);
                                    }
                                }
                            }
                            auto *shaderComponent = entityRegistry.try_get<ShaderComponent>(m_selectedEntity.m_entityID);
                            if(shaderComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_ShaderComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##ShaderComponentDeleteButton", "Delete the Shader component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_ShaderComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::ShaderComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    // Draw ACTIVE
                                    m_selectedEntity.m_componentData.m_graphicsComponents.m_shaderConstructionInfo->m_active = shaderComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##ShaderComponentActive", &m_selectedEntity.m_componentData.m_graphicsComponents.m_shaderConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Shader Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, shaderComponent, Systems::Changes::Generic::Active);
                                    }
                                }
                            }

                            // PHYSICS COMPONENTS                    
                            auto *collisionShapeComponent = entityRegistry.try_get<CollisionShapeComponent>(m_selectedEntity.m_entityID);
                            if(collisionShapeComponent != nullptr)
                            {
                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::CollisionShapeComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {

                                }
                            }
                            auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(m_selectedEntity.m_entityID);
                            if(rigidBodyComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_RigidBodyComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##RigidBodyComponentDeleteButton", "Delete the Rigid Body component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_RigidBodyComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::RigidBodyComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    // Get the bullet physics rigid body object
                                    auto rigidBody = rigidBodyComponent->getRigidBody();

                                    // Get the current rigid body data
                                    m_selectedEntity.m_collisionShapeType = rigidBodyComponent->getCollisionShapeType();
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction = rigidBody->getFriction();
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_kinematic = rigidBody->isKinematicObject();
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_mass = rigidBody->getMass();
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_restitution = rigidBody->getRestitution();
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_rollingFriction = rigidBody->getRollingFriction();
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_spinningFriction = rigidBody->getSpinningFriction();

                                    // Draw ACTIVE
                                    if(!m_rigidBodyComponentActivateAllSet)
                                        m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_active = rigidBodyComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##RigidBodyComponentActive", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Generic::Active);
                                    }

                                    // Draw KINEMATIC
                                    drawLeftAlignedLabelText("Kinematic:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##KinematicCheck", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_kinematic))
                                    {
                                        // If the kinematic flag was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::Kinematic);
                                    }

                                    // Draw MASS
                                    drawLeftAlignedLabelText("Mass:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##MassDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_mass, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                    {
                                        // If the mass was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::Mass);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw FRICTION
                                    drawLeftAlignedLabelText("Friction:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##FrictionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction, Config::GUIVar().editor_float_slider_speed))
                                    {
                                        // If the friction was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::Friction);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw ROLLING FRICTION
                                    drawLeftAlignedLabelText("Rolling friction:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##RollingFrictionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_rollingFriction, Config::GUIVar().editor_float_slider_speed))
                                    {
                                        // If the rolling friction was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::RollingFriction);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw SPINNING FRICTION
                                    drawLeftAlignedLabelText("Spinning friction:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##SpinningFrictionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_spinningFriction, Config::GUIVar().editor_float_slider_speed))
                                    {
                                        // If the spinning friction was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::SpinningFriction);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw RESTITUTION
                                    drawLeftAlignedLabelText("Restitution:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##RestitutionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_restitution, Config::GUIVar().editor_float_slider_speed))
                                    {
                                        // If the m_restitution was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::Restitution);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw COLLISION SHAPE TYPE
                                    drawLeftAlignedLabelText("Collision shape:", inputWidgetOffset);
                                    if(ImGui::Combo("##CollisionShapePicker", &m_selectedEntity.m_collisionShapeType, &(rigidBodyComponent->getCollisionTypeText()[0]), RigidBodyComponent::CollisionShapeType::CollisionShapeType_NumOfTypes))
                                    {
                                        // If the collision shape type was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeType);
                                    }

                                    switch(rigidBodyComponent->getCollisionShapeType())
                                    {
                                        case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Box:
                                            {
                                                // Get the collision shape data
                                                m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize = rigidBodyComponent->getCollisionShapeSize();

                                                // Draw BOX HALF EXTENTS
                                                drawLeftAlignedLabelText("Box half extents:", inputWidgetOffset);
                                                if(ImGui::DragFloat3("##BoxHalfExtentsDrag", glm::value_ptr(m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize), Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                                {
                                                    // If the box half extents size vector was changed, send a notification to the Rigid Body Component
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
                                                }
                                                captureMouseWhileItemActive();

                                            }
                                            break;
                                        case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Capsule:
                                            {

                                            }
                                            break;
                                        case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Cone:
                                            {

                                            }
                                            break;
                                        case RigidBodyComponent::CollisionShapeType::CollisionShapeType_ConvexHull:
                                            {

                                            }
                                            break;
                                        case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Cylinder:
                                            {

                                            }
                                            break;
                                        case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Sphere:
                                            {
                                                // Get the collision shape data
                                                m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize = rigidBodyComponent->getCollisionShapeSize();

                                                // Draw SPHERE RADIUS
                                                drawLeftAlignedLabelText("Sphere radius:", inputWidgetOffset);
                                                if(ImGui::DragFloat3("##SphereRadiusDrag", glm::value_ptr(m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize), Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                                {
                                                    // If the box half extents size vector was changed, send a notification to the Rigid Body Component
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
                                                }
                                                captureMouseWhileItemActive();
                                            }
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }

                            // AUDIO COMPONENTS
                            auto *soundComponent = entityRegistry.try_get<SoundComponent>(m_selectedEntity.m_entityID);
                            if(soundComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_SoundComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##SoundComponentDeleteButton", "Delete the Sound component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_SoundComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::SoundComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    // Get Sound Component data
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_active = soundComponent->isObjectActive();
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_volume = soundComponent->getVolume();
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_loop = soundComponent->getLoop();
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_spatialized = soundComponent->getSpatialized();
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_startPlaying = soundComponent->getStartPlaying();
                                    m_selectedEntity.m_soundType = soundComponent->getSoundType();
                                    m_selectedEntity.m_soundSourceType = soundComponent->getSoundSourceType();
                                    m_selectedEntity.m_playing = soundComponent->getPlaying();

                                    // If the sound filename was changed (by file browser), send a notification to the Sound Component
                                    // Otherwise just get the current sound filename
                                    if(m_selectedEntity.m_soundFilenameModified)
                                    {
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::SoundName);
                                        m_selectedEntity.m_soundFilenameModified = false;
                                    }
                                    else
                                        m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundName = soundComponent->getSoundName();

                                    // Draw ACTIVE
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_active = soundComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##SoundComponentActive", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Generic::Active);
                                    }

                                    // Draw SOUND FILENAME
                                    drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                                    if(ImGui::InputText("##SoundFilenameInput", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundName, ImGuiInputTextFlags_EnterReturnsTrue))
                                    {
                                        // If the sound filename was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::SoundName);
                                    }

                                    // Draw OPEN button
                                    ImGui::SameLine(calcTextSizedButtonOffset(1));
                                    if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##SoundFileOpenFileButton", "Open an audio file"))
                                    {
                                        // Only open the file browser if it's not opened already
                                        if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                        {
                                            // Set the file browser activation to Lua Script
                                            m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_SoundFile;

                                            // Define file browser variables
                                            m_fileBrowserDialog.m_filter = "Audio files (.wav .flac .mp3 .ogg){.wav,.flac,.mp3,.ogg},All files{.*}";
                                            m_fileBrowserDialog.m_title = "Open an audio file";
                                            m_fileBrowserDialog.m_name = "OpenAudioFileFileDialog";
                                            m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                                            // Set the root path only if it isn't saved from the last file dialog
                                            if(m_previouslyOpenedFileBrowser != m_currentlyOpenedFileBrowser)
                                                m_fileBrowserDialog.m_rootPath = Config::filepathVar().sound_path;

                                            // Tell the GUI scene to open the file browser
                                            m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                        }
                                    }

                                    // Draw RELOAD button
                                    ImGui::SameLine(calcTextSizedButtonOffset(0));
                                    if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##SoundFileReloadButton", "Reload the audio file"))
                                    {
                                        // Send a reload notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::Reload);
                                    }

                                    // Draw SOUND TYPE
                                    drawLeftAlignedLabelText("Sound type:", inputWidgetOffset);
                                    if(ImGui::Combo("##SoundTypePicker", &m_selectedEntity.m_soundType, &(soundComponent->getSoundTypeText()[0]), SoundComponent::SoundType_NumOfTypes))
                                    {
                                        // If the sound type was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::SoundType);
                                    }

                                    // Draw SOUND SOURCE TYPE
                                    drawLeftAlignedLabelText("Sound source type:", inputWidgetOffset);
                                    if(ImGui::Combo("##SoundSourceTypePicker", &m_selectedEntity.m_soundSourceType, &(soundComponent->getSoundSourceTypeText()[0]), SoundComponent::SoundSourceType_NumOfTypes))
                                    {
                                        // If the sound source type was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::SoundSourceType);
                                    }

                                    // Draw VOLUME
                                    drawLeftAlignedLabelText("Volume:", inputWidgetOffset);
                                    if(ImGui::DragFloat("##SoundVolumeDrag", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_volume, Config::GUIVar().editor_float_slider_speed, 0.0f, 1.0f))
                                    {
                                        // If the sound volume was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::Volume);
                                    }
                                    captureMouseWhileItemActive();

                                    // Draw LOOP
                                    drawLeftAlignedLabelText("Loop:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##SoundLoopCheckbox", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_loop))
                                    {
                                        // If the loop flag was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::Loop);
                                    }

                                    // Draw SPATIALIZED
                                    drawLeftAlignedLabelText("Spatialized:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##SoundSpatializedCheckbox", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_spatialized))
                                    {
                                        // If the spatialized flag was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::Spatialized);
                                    }

                                    // Draw START PLAYING
                                    drawLeftAlignedLabelText("Start playing:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##SoundStartPlayingCheckbox", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_startPlaying))
                                    {
                                        // If the start-playing flag was changed, send a notification to the Sound Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::StartPlaying);
                                    }
                                }
                            }
                            auto *soundListenerComponent = entityRegistry.try_get<SoundListenerComponent>(m_selectedEntity.m_entityID);
                            if(soundListenerComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_SoundListenerComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##SoundListenerComponentDeleteButton", "Delete the Sound Listener component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_SoundListenerComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::SoundListenerComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID = soundListenerComponent->getListenerID();

                                    // Draw ACTIVE
                                    m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_active = soundListenerComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##SoundListenerComponentActive", &m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the Sound Listener Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundListenerComponent, Systems::Changes::Generic::Active);
                                    }

                                    // Draw SOUND LISTENER ID
                                    drawLeftAlignedLabelText("Listener ID:", inputWidgetOffset);
                                    if(ImGui::InputInt("##ListenerIDInput", &m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID))
                                    {
                                        // If the sound listener ID was changed, send a notification to the Sound Listener Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundListenerComponent, Systems::Changes::Audio::ListenerID);
                                    }
                                }
                            }

                            // SCRIPTING COMPONENTS
                            auto *luaComponent = entityRegistry.try_get<LuaComponent>(m_selectedEntity.m_entityID);
                            if(luaComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_LuaComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##LUAComponentDeleteButton", "Delete the LUA component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_LuaComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::LuaComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    auto luaScript = luaComponent->getLuaScript();

                                    if(luaScript != nullptr)
                                    {
                                        // If the lua script filename was changed (by file browser), send a notification to the Lua Script Component
                                        // Otherwise just get the current lua script filename
                                        if(m_selectedEntity.m_luaScriptFilenameModified)
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, luaComponent, Systems::Changes::Script::Filename);
                                            m_selectedEntity.m_luaScriptFilenameModified = false;
                                        }
                                        else
                                            m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename = luaScript->getLuaScriptFilename();

                                        m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_active = luaComponent->isObjectActive();
                                        m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_pauseInEditor = luaComponent->pauseInEditor();

                                        // Draw ACTIVE
                                        drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                        if(ImGui::Checkbox("##LUAComponentActive", &m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_active))
                                        {
                                            // If the active flag was changed, send a notification to the LUA Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, luaComponent, Systems::Changes::Generic::Active);
                                        }

                                        // Draw PAUSE IN EDITOR
                                        drawLeftAlignedLabelText("Pause in editor:", inputWidgetOffset);
                                        if(ImGui::Checkbox("##LUAPauseInEditorCheckbox", &m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_pauseInEditor))
                                        {
                                            // If the pause-in-editor flag was changed, send a notification to the LUA Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, luaComponent, Systems::Changes::Script::PauseInEditor);
                                        }

                                        // Draw LUA FILENAME
                                        drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                                        if(ImGui::InputText("##LuaScriptFilenameInput", &m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename, ImGuiInputTextFlags_EnterReturnsTrue))
                                        {
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, luaComponent, Systems::Changes::Script::Filename);
                                        }

                                        // Draw OPEN button
                                        ImGui::SameLine(calcTextSizedButtonOffset(1));
                                        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##LuaScriptOpenFileButton", "Open a Lua file script"))
                                        {
                                            // Only open the file browser if it's not opened already
                                            if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                            {
                                                // Set the file browser activation to Lua Script
                                                m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_LuaScript;

                                                // Define file browser variables
                                                m_fileBrowserDialog.m_filter = "LUA script files (.lua){.lua},All files{.*}";
                                                m_fileBrowserDialog.m_title = "Open LUA script file";
                                                m_fileBrowserDialog.m_name = "OpenLuaScriptFileDialog";
                                                m_fileBrowserDialog.m_rootPath = Config::filepathVar().script_path;
                                                m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                                                // Tell the GUI scene to open the file browser
                                                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                            }
                                        }

                                        // Draw RELOAD button
                                        ImGui::SameLine(calcTextSizedButtonOffset(0));
                                        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##LuaScriptReloadButton", "Reload the Lua file script"))
                                        {
                                            // Send a reload notification to the LUA Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, luaComponent, Systems::Changes::Script::Reload);
                                        }

                                        // Update lua variables from the LUA Component only if the previous variables haven't been modified
                                        if(!m_selectedEntity.m_luaVariablesModified)
                                            m_selectedEntity.m_luaVariables = luaScript->getLuaVariables();

                                        // Calculate lua variables window height and cap it to a max height value
                                        float childWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * (m_selectedEntity.m_luaVariables.size() + 2);
                                        childWindowHeight = childWindowHeight > Config::GUIVar().editor_lua_variables_max_height ? Config::GUIVar().editor_lua_variables_max_height : childWindowHeight;

                                        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
                                        if(ImGui::BeginChild("##LuaVariables", ImVec2(0, childWindowHeight), true, ImGuiWindowFlags_None))
                                        {
                                            if(!m_selectedEntity.m_luaVariables.empty())
                                            {
                                                // Calculate item sizes and offsets
                                                const float itemSpacing = m_imguiStyle.ItemInnerSpacing.x;
                                                const float windowWidth = ImGui::GetContentRegionAvail().x;
                                                const float itemSpace = windowWidth - (itemSpacing * 3) - m_buttonSizedByFont.x - m_imguiStyle.FramePadding.x * 2;
                                                const float itemSizes[3] = { itemSpace / 2.5f, itemSpace / 5.0f, itemSpace / 2.5f };
                                                const float offsets[2] = { itemSizes[0] + itemSpacing, itemSizes[0] + itemSizes[1] + (itemSpacing * 2) };

                                                // Draw LUA VARIABLES table column labels
                                                float textSize = ImGui::CalcTextSize("Names:").x;
                                                ImGui::AlignTextToFramePadding();
                                                ImGui::SetCursorPosX(textSize + (textSize / 2.0f) > itemSizes[0] ? 0.0f : (itemSizes[0] / 2.0f) - (textSize / 2.0f));
                                                ImGui::SetNextItemWidth(itemSizes[0]);
                                                ImGui::Text("Names:");

                                                textSize = ImGui::CalcTextSize("Types:").x;
                                                ImGui::SameLine();
                                                ImGui::SetCursorPosX(textSize + (textSize / 2.0f) > itemSizes[1] ? offsets[0] : offsets[0] + (itemSizes[1] / 2.0f) - (textSize / 2.0f));
                                                ImGui::SetNextItemWidth(itemSizes[1]);
                                                ImGui::Text("Types:");

                                                textSize = ImGui::CalcTextSize("Values:").x;
                                                ImGui::SameLine();
                                                ImGui::SetCursorPosX(textSize + (textSize / 2.0f) > itemSizes[2] ? offsets[1] : offsets[1] + (itemSizes[2] / 2.0f) - (textSize / 2.0f));
                                                ImGui::SetNextItemWidth(itemSizes[2]);
                                                ImGui::Text("Values:");

                                                // Draw LUA VARIABLES table
                                                for(decltype(m_selectedEntity.m_luaVariables.size()) i = 0, size = m_selectedEntity.m_luaVariables.size(); i < size; i++)
                                                {
                                                    const std::string widgetName = ("##" + Utilities::toString(i));

                                                    ImGui::AlignTextToFramePadding();
                                                    ImGui::SetNextItemWidth(itemSizes[0]);
                                                    if(ImGui::InputText((widgetName + "LuaVariableName").c_str(), &m_selectedEntity.m_luaVariables[i].first, ImGuiInputTextFlags_EnterReturnsTrue))
                                                    {
                                                        m_selectedEntity.m_luaVariablesModified = true;
                                                    }

                                                    ImGui::SameLine();
                                                    ImGui::SetCursorPosX(offsets[0]);
                                                    ImGui::SetNextItemWidth(itemSizes[1]);
                                                    int variableType = m_selectedEntity.m_luaVariables[i].second.getVariableType();
                                                    if(ImGui::Combo((widgetName + "LuaVariableTypeCombo").c_str(), &variableType, &m_luaVariableTypeStrings[0], (int)m_luaVariableTypeStrings.size()))
                                                    {
                                                        m_selectedEntity.m_luaVariablesModified = true;
                                                        switch(variableType)
                                                        {
                                                            case Property::PropertyVariableType::Type_null:
                                                                m_selectedEntity.m_luaVariables[i].second = Property();
                                                                break;
                                                            case Property::PropertyVariableType::Type_bool:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getBool());
                                                                break;
                                                            case Property::PropertyVariableType::Type_int:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getInt());
                                                                break;
                                                            case Property::PropertyVariableType::Type_float:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getFloat());
                                                                break;
                                                            case Property::PropertyVariableType::Type_double:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getDouble());
                                                                break;
                                                            case Property::PropertyVariableType::Type_vec2i:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getVec2i());
                                                                break;
                                                            case Property::PropertyVariableType::Type_vec2f:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getVec2f());
                                                                break;
                                                            case Property::PropertyVariableType::Type_vec3f:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getVec3f());
                                                                break;
                                                            case Property::PropertyVariableType::Type_vec4f:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getVec4f());
                                                                break;
                                                            case Property::PropertyVariableType::Type_string:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getString());
                                                                break;
                                                            case Property::PropertyVariableType::Type_propertyID:
                                                                m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), m_selectedEntity.m_luaVariables[i].second.getPropertyID());
                                                                break;
                                                        }
                                                    }

                                                    ImGui::SameLine();
                                                    ImGui::SetCursorPosX(offsets[1]);
                                                    ImGui::SetNextItemWidth(itemSizes[2] - m_imguiStyle.FramePadding.x);
                                                    switch(m_selectedEntity.m_luaVariables[i].second.getVariableType())
                                                    {
                                                        case Property::PropertyVariableType::Type_null:
                                                            {
                                                                ImGui::Text("");
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_bool:
                                                            {
                                                                bool value = m_selectedEntity.m_luaVariables[i].second.getBool();
                                                                if(ImGui::Checkbox((widgetName + "LuaVariableBoolCheckbox").c_str(), &value))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_int:
                                                            {
                                                                int value = m_selectedEntity.m_luaVariables[i].second.getInt();
                                                                ImGui::InputInt((widgetName + "LuaVariableIntInput").c_str(), &value);
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                                captureMouseWhileItemActive();
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_float:
                                                            {
                                                                float value = m_selectedEntity.m_luaVariables[i].second.getFloat();
                                                                if(ImGui::DragFloat((widgetName + "LuaVariableFloatDrag").c_str(), &value, Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                                captureMouseWhileItemActive();
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_double:
                                                            {
                                                                double value = m_selectedEntity.m_luaVariables[i].second.getDouble();
                                                                if(ImGui::DragScalar((widgetName + "LuaVariableDoubleDrag").c_str(), ImGuiDataType_Double, &value, 0.0005f))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                                captureMouseWhileItemActive();
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_vec2i:
                                                            {
                                                                glm::ivec2 value = m_selectedEntity.m_luaVariables[i].second.getVec2i();
                                                                if(ImGui::InputInt2((widgetName + "LuaVariableVec2iDrag").c_str(), glm::value_ptr(value)))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                                captureMouseWhileItemActive();
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_vec2f:
                                                            {
                                                                glm::vec2 value = m_selectedEntity.m_luaVariables[i].second.getVec2f();
                                                                if(ImGui::DragFloat2((widgetName + "LuaVariableVec2fDrag").c_str(), glm::value_ptr(value), Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                                captureMouseWhileItemActive();
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_vec3f:
                                                            {
                                                                glm::vec3 value = m_selectedEntity.m_luaVariables[i].second.getVec3f();
                                                                if(ImGui::DragFloat3((widgetName + "LuaVariableVec3fDrag").c_str(), glm::value_ptr(value), Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                                captureMouseWhileItemActive();
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_vec4f:
                                                            {
                                                                glm::vec4 value = m_selectedEntity.m_luaVariables[i].second.getVec4f();
                                                                if(ImGui::DragFloat4((widgetName + "LuaVariableVec4fDrag").c_str(), glm::value_ptr(value), Config::GUIVar().editor_float_slider_speed))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                                captureMouseWhileItemActive();
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_string:
                                                            {
                                                                std::string value = m_selectedEntity.m_luaVariables[i].second.getString();
                                                                if(ImGui::InputText((widgetName + "LuaVariableStringInput").c_str(), &value, ImGuiInputTextFlags_EnterReturnsTrue))
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), value);
                                                                }
                                                            }
                                                            break;
                                                        case Property::PropertyVariableType::Type_propertyID:
                                                            {
                                                                unsigned int value = m_selectedEntity.m_luaVariables[i].second.getID();
                                                                ImGui::InputScalar((widgetName + "LuaVariablePropertyIDInput").c_str(), ImGuiDataType_U32, &value);
                                                                {
                                                                    m_selectedEntity.m_luaVariablesModified = true;
                                                                    if(value >= 0 && value < Properties::PropertyID::NumberOfPropertyIDs)
                                                                        m_selectedEntity.m_luaVariables[i].second = Property(m_selectedEntity.m_luaVariables[i].second.getPropertyID(), static_cast<Properties::PropertyID>(value));
                                                                }
                                                            }
                                                            break;
                                                    }

                                                    // Draw DELETE button
                                                    ImGui::SameLine(calcTextSizedButtonOffset(0) - m_imguiStyle.FramePadding.x);
                                                    if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], widgetName + "LuaVariablesDeleteButton", "Delete Lua variable"))
                                                    {
                                                        m_selectedEntity.m_luaVariablesModified = true;
                                                        m_selectedEntity.m_luaVariables.erase(m_selectedEntity.m_luaVariables.begin() + i);
                                                        size = m_selectedEntity.m_luaVariables.size();
                                                        i--;
                                                    }
                                                }
                                            }

                                            // Draw ADD button
                                            ImGui::SetCursorPosX(calcTextSizedButtonOffset(0) - m_imguiStyle.FramePadding.x);
                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##AddLuaVariableButton", "Add a new Lua variable"))
                                            {
                                                m_selectedEntity.m_luaVariablesModified = true;
                                                m_selectedEntity.m_luaVariables.push_back(std::make_pair(std::string(), Property()));
                                            }
                                        }
                                        ImGui::EndChild();

                                        if(m_selectedEntity.m_luaVariablesModified)
                                        {
                                            m_selectedEntity.m_luaVariablesModified = false;
                                            m_systemScene->getSceneLoader()->getChangeController()->sendData(luaComponent, DataType::DataType_LuaVariables, (void *)&m_selectedEntity.m_luaVariables, false);
                                        }

                                        ImGui::PopStyleVar();
                                    }
                                }
                            }

                            // GUI COMPONENTS
                            auto *guiSequenceComponent = entityRegistry.try_get<GUISequenceComponent>(m_selectedEntity.m_entityID);
                            if(guiSequenceComponent != nullptr)
                            {
                                // Set the corresponding component type to be existing
                                m_selectedEntity.m_componentTypeText[ComponentType::ComponentType_GUISequenceComponent].second = true;

                                // Draw DELETE COMPONENT button
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##GUISequenceDeleteButton", "Delete the GUI Sequence component"))
                                {
                                    // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Component change with the attached container
                                    EntityAndComponent *deleteComponentData = new EntityAndComponent(m_selectedEntity.m_entityID, ComponentType::ComponentType_GUISequenceComponent);
                                    m_entityAndComponentPool.push_back(deleteComponentData);
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::GUI), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
                                }
                                ImGui::SameLine(headerOffsetAfterDeleteButton);

                                if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::GUISequenceComponent), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    // Get the current GUI Sequence data
                                    m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_staticSequence = guiSequenceComponent->isStaticSequence();

                                    // Draw ACTIVE
                                    m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_active = guiSequenceComponent->isObjectActive();
                                    drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##GUISequenceComponentActive", &m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_active))
                                    {
                                        // If the active flag was changed, send a notification to the GUI Sequence Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, guiSequenceComponent, Systems::Changes::Generic::Active);
                                    }

                                    // Draw STATIC
                                    drawLeftAlignedLabelText("Static sequence:", inputWidgetOffset);
                                    if(ImGui::Checkbox("##StaticSequenceCheck", &m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_staticSequence))
                                    {
                                        // If the static sequence flag was changed, send a notification to the GUI Sequence Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, guiSequenceComponent, Systems::Changes::GUI::StaticSequence);
                                    }
                                }
                            }

                            ImGui::NewLine();
                            ImGui::Separator();

                            const std::string componentTypeSelectionPopupName = "##ComponentTypeSelectionPopup";

                            // Calculate button size
                            const char *addComponentButtonLabel = "Add component";
                            float addComponentButtonWidth = ImGui::CalcTextSize(addComponentButtonLabel).x * Config::GUIVar().editor_inspector_button_width_multiplier;

                            // Set the button position to the right-most side
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - addComponentButtonWidth);

                            // Draw ADD COMPONENT button
                            if(ImGui::Button(addComponentButtonLabel, ImVec2(addComponentButtonWidth, 0.0f)))
                            {
                                // Open the pop-up with the component type list
                                ImGui::OpenPopup(componentTypeSelectionPopupName.c_str());
                            }

                            // Draw COMPONENT TYPE LIST
                            if(ImGui::BeginPopup(componentTypeSelectionPopupName.c_str()))
                            {
                                // Make an array of component types and type text
                                std::vector<std::pair<std::string, ComponentType>> componentTypes;

                                // Populate the array with component types that aren't present in this entity
                                for(unsigned int i = 0; i < ComponentType::ComponentType_NumOfTypes; i++)
                                    if(!m_selectedEntity.m_componentTypeText[i].second)
                                        componentTypes.push_back(std::make_pair(m_selectedEntity.m_componentTypeText[i].first, static_cast<ComponentType>(i)));

                                // Sort the array alphabetically (based on component type text)
                                std::sort(componentTypes.begin(), componentTypes.end());

                                // Remove selection border and align text vertically
                                ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                                // Go over each non-present component type
                                for(decltype(componentTypes.size()) i = 0, size = componentTypes.size(); i < size; i++)
                                {
                                    if(ImGui::Selectable(componentTypes[i].first.c_str(), false, 0, ImVec2(0.0f, m_fontSize * 2.0f)))
                                    {
                                        ComponentsConstructionInfo *newComponentInfo = new ComponentsConstructionInfo();
                                        newComponentInfo->m_id = m_selectedEntity.m_entityID;
                                        newComponentInfo->m_name = m_selectedEntity.m_componentData.m_name;
                                        bool newComponentInfoSet = true;

                                        switch(componentTypes[i].second)
                                        {
                                            case ComponentType::ComponentType_SoundComponent:
                                                {
                                                    newComponentInfo->m_audioComponents.m_soundConstructionInfo = new SoundComponent::SoundComponentConstructionInfo();
                                                }
                                                break;
                                            case ComponentType::ComponentType_SoundListenerComponent:
                                                {
                                                    newComponentInfo->m_audioComponents.m_soundListenerConstructionInfo = new SoundListenerComponent::SoundListenerComponentConstructionInfo();
                                                }
                                                break;
                                            case ComponentType::ComponentType_CameraComponent:
                                                {
                                                    newComponentInfo->m_graphicsComponents.m_cameraConstructionInfo = new CameraComponent::CameraComponentConstructionInfo();
                                                }
                                                break;
                                            case ComponentType::ComponentType_LightComponent:
                                                {
                                                    newComponentInfo->m_graphicsComponents.m_lightConstructionInfo = new LightComponent::LightComponentConstructionInfo();
                                                    newComponentInfo->m_graphicsComponents.m_lightConstructionInfo->m_lightComponentType = LightComponent::LightComponentType::LightComponentType_point;
                                                }
                                                break;
                                            case ComponentType::ComponentType_ModelComponent:
                                                {
                                                    newComponentInfo->m_graphicsComponents.m_modelConstructionInfo = new ModelComponent::ModelComponentConstructionInfo();
                                                }
                                                break;
                                            case ComponentType::ComponentType_ShaderComponent:
                                                {
                                                    newComponentInfo->m_graphicsComponents.m_shaderConstructionInfo = new ShaderComponent::ShaderComponentConstructionInfo();
                                                }
                                                break;
                                            case ComponentType::ComponentType_GUISequenceComponent:
                                                {
                                                    newComponentInfo->m_guiComponents.m_guiSequenceConstructionInfo = new GUISequenceComponent::GUISequenceComponentConstructionInfo();
                                                }
                                                break;
                                            case ComponentType::ComponentType_RigidBodyComponent:
                                                {
                                                    newComponentInfo->m_physicsComponents.m_rigidBodyConstructionInfo = new RigidBodyComponent::RigidBodyComponentConstructionInfo();
                                                    newComponentInfo->m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeType = RigidBodyComponent::CollisionShapeType::CollisionShapeType_Box;
                                                }
                                                break;
                                            case ComponentType::ComponentType_LuaComponent:
                                                {
                                                    newComponentInfo->m_scriptComponents.m_luaConstructionInfo = new LuaComponent::LuaComponentConstructionInfo();
                                                    newComponentInfo->m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename = Config::scriptVar().defaultScriptFilename;
                                                }
                                                break;
                                            case ComponentType::ComponentType_ObjectMaterialComponent:
                                                {
                                                    newComponentInfo->m_worldComponents.m_objectMaterialConstructionInfo = new ObjectMaterialComponent::ObjectMaterialComponentConstructionInfo();
                                                }
                                                break;
                                            case ComponentType::ComponentType_SpatialComponent:
                                                {
                                                    newComponentInfo->m_worldComponents.m_spatialConstructionInfo = new SpatialComponent::SpatialComponentConstructionInfo();
                                                }
                                                break;
                                            default:
                                                {
                                                    newComponentInfoSet = false;
                                                }
                                                break;
                                        }

                                        if(newComponentInfoSet)
                                        {
                                            m_componentConstructionInfoPool.push_back(newComponentInfo);

                                            m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_CreateComponent, (void *)newComponentInfo, false);
                                        }
                                        else
                                            delete newComponentInfo;
                                    }
                                }

                                ImGui::PopStyleVar(2); //ImGuiStyleVar_SelectableTextAlign, ImGuiStyleVar_FramePadding
                                ImGui::EndPopup();
                            }
                            ImGui::EndChild();
                        }
                    }
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Scene settings"))
                {
                    if(ImGui::BeginChild("##SceneSettingsWindow", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_::ImGuiWindowFlags_None))
                    {
                        if(m_currentSceneData.m_modified)
                        {
                            updateSceneData(m_currentSceneData);
                            m_currentSceneData.m_modified = false;
                        }
                        drawSceneData(m_currentSceneData, true);

                        // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                        float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                        // Center the separator text
                        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));

                        int numOfAASettingsWindowItems = 2;
                        switch(Config::m_graphicsVar.antialiasing_type)
                        {
                            case AntiAliasingType_MSAA:
                                numOfAASettingsWindowItems = 3;
                                break;
                            case AntiAliasingType_FXAA:
                                numOfAASettingsWindowItems = 6;
                                break;
                        }
                        if(ImGui::BeginChild("##AASettings", ImVec2(0.0f, (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * numOfAASettingsWindowItems), true))
                        {
                            //ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
                            ImGui::SeparatorText("Anti-aliasing settings:");
                            //ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

                            // Draw ANTI-ALIASING TYPE
                            drawLeftAlignedLabelText("Anti-aliasing type:", inputWidgetOffset);
                            ImGui::Combo("##AATypePicker", &Config::m_graphicsVar.antialiasing_type, &m_antialiasingTypeText[0], (int)m_antialiasingTypeText.size());

                            switch(Config::m_graphicsVar.antialiasing_type)
                            {
                                case AntiAliasingType_MSAA:
                                    break;
                                case AntiAliasingType_FXAA:
                                    {
                                        // Draw ITERATIONS
                                        drawLeftAlignedLabelText("Iterations:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                                        ImGui::InputInt("##FXAAIterationsInput", &Config::m_rendererVar.fxaa_iterations);

                                        // Draw SUBPIXEL QUALITY
                                        drawLeftAlignedLabelText("Subpixel quality:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                                        ImGui::DragFloat("##FXAASubpixelQualityDrag", &Config::m_rendererVar.fxaa_edge_subpixel_quality, 0.01f, 0.0f, 10.0f, "%.5f");

                                        // Draw EDGE THRESHOLD MIN
                                        drawLeftAlignedLabelText("Edge threshold min:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                                        ImGui::DragFloat("##FXAAEdgeThresholdMinDrag", &Config::m_rendererVar.fxaa_edge_threshold_min, 0.0001f, 0.0f, 1.0f, "%.5f");

                                        // Draw EDGE THRESHOLD MAX
                                        drawLeftAlignedLabelText("Edge threshold max:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                                        ImGui::DragFloat("##FXAAEdgeThresholdMaxDrag", &Config::m_rendererVar.fxaa_edge_threshold_max, 0.0001f, 0.0f, 1.0f, "%.5f");
                                    }
                                    break;
                            }
                        }
                        ImGui::EndChild();

                        ImGui::SeparatorText("Luminance settings:");

                        // Draw TONEMAP METHOD
                        drawLeftAlignedLabelText("Tonemap method:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::Combo("##TonemapMethodPicker", &Config::m_graphicsVar.tonemap_method, &m_tonemappingMethodText[0], (int)m_tonemappingMethodText.size());

                        // Draw LUMINANCE RANGE MIN
                        drawLeftAlignedLabelText("Luminance range min:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::DragFloat("##LuminanceRangeMinDrag", &Config::m_graphicsVar.luminance_range_min, 0.001f, 0.0f, 100.0f, "%.5f");
                        captureMouseWhileItemActive();

                        // Draw LUMINANCE RANGE MAX
                        drawLeftAlignedLabelText("Luminance range max:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::DragFloat("##LuminanceRangeMaxDrag", &Config::m_graphicsVar.luminance_range_max, 0.001f, 0.0f, 100.0f, "%.5f");
                        captureMouseWhileItemActive();

                        // Draw LUMINANCE MULTIPLIER
                        drawLeftAlignedLabelText("Luminance multiplier:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::DragFloat("##LuminanceMultiplierDrag", &Config::m_graphicsVar.luminance_multiplier, 0.001f, 0.0f, 100.0f, "%.5f");
                        captureMouseWhileItemActive();

                        ImGui::SeparatorText("Graphics settings:");

                        // Draw PARALLAX METHOD
                        drawLeftAlignedLabelText("Parallax method:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::SliderInt("##ParallaxMethodSlider", &Config::m_rendererVar.parallax_mapping_method, 1, 5, "%d");

                        // Draw PARALLAX LOD
                        drawLeftAlignedLabelText("Parallax LOD:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::DragFloat("##ParallaxLODDrag", &Config::m_graphicsVar.LOD_parallax_mapping, 0.1f, 0.0f, 100000.0f, "%.5f");
                        captureMouseWhileItemActive();

                        // Draw PARALLAX MIN STEPS
                        drawLeftAlignedLabelText("Parallax min steps:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::SliderFloat("##ParallaxMinStepsDrag", &Config::m_rendererVar.parallax_mapping_min_steps, 1.0f, 128.0f, "%.0f");
                        captureMouseWhileItemActive();

                        // Draw PARALLAX MAX STEPS
                        drawLeftAlignedLabelText("Parallax max steps:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::SliderFloat("##ParallaxMaxStepsDrag", &Config::m_rendererVar.parallax_mapping_max_steps, 1.0f, 128.0f, "%.0f");
                        captureMouseWhileItemActive();

                        ImGui::SeparatorText("Renderer settings:");

                        // Draw OBJECTS LOADER PER FRAME
                        drawLeftAlignedLabelText("Object loads per frame:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset);
                        ImGui::InputInt("##ObjectsLoadedPerFrameInput", &Config::m_rendererVar.objects_loaded_per_frame);

                        ImGui::NewLine();

                        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextAlign
                    }
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    //	 ____________________________
    //	|							 |
    //	|	    BOTTOM WINDOW        |
    //	|____________________________|
    //
    {
        ImGui::SetNextWindowClass(&windowClassWithNoTabBar);
        if(ImGui::Begin("##BottomWindow", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar))
        {
            if(ImGui::BeginTabBar("##BottomWindowTabBar", ImGuiTabBarFlags_None))
            {
                if(ImGui::BeginTabItem("Textures"))
                {
                    // Set color and style for texture buttons
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Border, m_buttonBackgroundEnabled);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

                    // Calculate the available window size
                    float visibleWindowSize = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

                    // Draw each texture in the 2D texture loader pool
                    for(decltype(m_textureAssets.size()) i = 0, size = m_textureAssets.size(); i < size; i++)
                    {
                        ImGui::PushID((int)i);

                        // Draw the texture
                        if(ImGui::ImageButton((ImTextureID)m_textureAssets[i].first->getHandle(), m_textureAssetImageSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 0))
                        {
                            m_selectedTexture = m_textureAssets[i].first;

                            // Set the texture inspector tab flag to be selected (bring to focus), because a texture has just been selected
                            m_textureInspectorTabFlags = ImGuiTabItemFlags_SetSelected;
                        }

                        // Show the tool tip with the information of the texture (if the mouse is hovered over it)
                        if(ImGui::BeginItemTooltip())
                        {
                            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
                            ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));
                            ImGui::Text("");
                            ImGui::SeparatorText("Click to open in Texture Inspector");
                            ImGui::Text("");
                            ImGui::PopStyleVar(2); // ImGuiStyleVar_ItemSpacing, ImGuiStyleVar_SeparatorTextAlign
                            ImGui::Separator();

                            ImGui::Text(("Filename: " + m_textureAssets[i].second).c_str());
                            ImGui::Text(("Size: " + Utilities::toString(m_textureAssets[i].first->getTextureWidth()) + "x" + Utilities::toString(m_textureAssets[i].first->getTextureHeight())).c_str());

                            ImGui::Text(("Texture format: " + getTextureFormatString(m_textureAssets[i].first->getTextureFormat())).c_str());
                            ImGui::Text(("Texture data type: " + getTextureDataTypeString(m_textureAssets[i].first->getTextureDataType())).c_str());
                            ImGui::Text(("Texture data format: " + getTextureDataFormat(m_textureAssets[i].first->getTextureDataFormat())).c_str());

                            ImGui::Text(m_textureAssets[i].first->getCompressionEnabled() ? "Compression enabled" : "Compression disabled");
                            ImGui::Text(m_textureAssets[i].first->getDownsamplingEnabled() ? "Downsampling enabled" : "Downsampling disabled");

                            ImGui::Text(m_textureAssets[i].first->getMipmapEnabled() ? "Mipmap enabled" : "Mipmap disabled");
                            if(m_textureAssets[i].first->getMipmapEnabled())
                                ImGui::Text(("Mipmap level: " + Utilities::toString(m_textureAssets[i].first->getMipmapLevel())).c_str());

                            ImGui::Text(("Referene counter: " + Utilities::toString(m_textureAssets[i].first->getReferenceCounter())).c_str());

                            ImGui::Text(m_textureAssets[i].first->isLoadedToMemory() ? "Loaded to system memory: true" : "Loaded to system memory: false");
                            ImGui::Text(m_textureAssets[i].first->isLoadedToVideoMemory() ? "Loaded to video memory: true" : "Loaded to video memory: false");

                            ImGui::Separator();

                            ImGui::EndTooltip();
                        }

                        // Calculate expected position if the next texture was on the same line
                        float nextTextureSize = ImGui::GetItemRectMax().x + m_imguiStyle.ItemSpacing.x + m_textureAssetImageSize.x;

                        // If this is not the last texture and the next text will fit on the same line, set next draw to be on the same line
                        if(i + 1 < size && nextTextureSize < visibleWindowSize)
                            ImGui::SameLine();

                        ImGui::PopID();
                    }

                    ImGui::PopStyleColor(2);    // ImGuiCol_Border, ImGuiCol_Button
                    ImGui::PopStyleVar();       // ImGuiStyleVar_FrameBorderSize

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Models"))
                {
                    auto contentRegionWidth = ImGui::GetContentRegionAvail().x;

                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.26f, 0.26f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                    if(ImGui::BeginChild("##ModelAssetsSelection", ImVec2(contentRegionWidth * 0.2f, 0), true))
                    {
                        ImGui::SeparatorText("Models:");
                        for(decltype(m_modelAssets.size()) i = 0, size = m_modelAssets.size(); i < size; i++)
                        {
                            if(ImGui::Selectable(m_modelAssets[i].second.c_str(), m_selectedModel != nullptr ? m_modelAssets[i].first->getUniqueID() == m_selectedModel->getUniqueID() : false))
                            {
                                m_selectedModel = m_modelAssets[i].first;
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::PopStyleVar(4); // ImGuiStyleVar_ChildBorderSize, ImGuiStyleVar_SeparatorTextAlign, ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                    ImGui::PopStyleColor(); // ImGuiCol_Border

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Shaders"))
                {
                    if(!m_selectedShaderFilename.empty())
                    {
                        m_selectedProgram->setShaderFilename(static_cast<ShaderType>(m_selectedShaderType), m_selectedShaderFilename);
                        m_selectedProgram->reloadToMemory();
                        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);

                        m_selectedShaderFilename.clear();
                    }

                    auto contentRegionWidth = ImGui::GetContentRegionAvail().x;

                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.26f, 0.26f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                    if(ImGui::BeginChild("##ShaderAssetsProgramSelection", ImVec2(contentRegionWidth * 0.2f, 0), true))
                    {
                        ImGui::SeparatorText("Shader programs:");
                        //for(decltype(m_shaderAssets.size()) shaderIndex = 0, size = m_shaderAssets.size(); shaderIndex < size; shaderIndex++)
                        //{
                        //    if(ImGui::Selectable(m_shaderAssets[shaderIndex].second.c_str(), m_selectedProgram != nullptr ? m_shaderAssets[shaderIndex].first->getCombinedFilename() == m_selectedProgram->getCombinedFilename() : false))
                        //    {
                        //        m_selectedProgram = m_shaderAssets[shaderIndex].first;
                        //        m_selectedShaderType = -1;
                        //    }
                        //}


                        static ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

                        for(decltype(m_shaderAssets.size()) shaderIndex = 0, size = m_shaderAssets.size(); shaderIndex < size; shaderIndex++)
                        {
                            if(ImGui::TreeNodeEx(m_shaderAssets[shaderIndex].second.c_str(), baseNodeFlags))
                            {
                                for(unsigned int shaderType = 0; shaderType < ShaderType::ShaderType_NumOfTypes; shaderType++)
                                {
                                    if(!m_shaderAssets[shaderIndex].first->getShaderFilename(shaderType).empty())
                                    {
                                        if(ImGui::TreeNodeEx(m_shaderAssets[shaderIndex].first->getShaderFilename(shaderType).c_str(), baseNodeFlags | ImGuiTreeNodeFlags_Leaf))
                                        {
                                            if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                                            {
                                                m_selectedProgram = m_shaderAssets[shaderIndex].first;
                                                m_selectedShaderType = shaderType;
                                            }
                                            ImGui::TreePop();
                                        }
                                    }
                                }

                                ImGui::TreePop();
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing

                    if(m_selectedShaderType >= 0 && m_selectedShaderType < ShaderType::ShaderType_NumOfTypes)
                    {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                        if(ImGui::BeginChild("##ShaderAssetsUniformUpdate", ImVec2(contentRegionWidth * 0.2f, 0), true))
                        {
                            // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                            float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.75f + ImGui::GetStyle().ItemInnerSpacing.x;

                            ImGui::SeparatorText("Uniform updater:");

                            // Draw UNIFORM UPDATER SETTINGS
                            if(ImGui::TreeNodeEx("Uniform updater settings:", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                            {
                                // Get uniform updater
                                auto const &uniformUpdater = m_selectedProgram->getUniformUpdater();

                                // Draw UPDATES PER FRAME
                                auto numUpdatesPerFrame = uniformUpdater.getNumUpdatesPerFrame();
                                drawLeftAlignedLabelText("Updates per frame:", inputWidgetOffset);
                                if(numUpdatesPerFrame > 0)
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(numUpdatesPerFrame).c_str());
                                else
                                    ImGui::TextDisabled(Utilities::toString(numUpdatesPerFrame).c_str());

                                // Draw UPDATES PER MODEL
                                auto numUpdatesPerModel = uniformUpdater.getNumUpdatesPerModel();
                                drawLeftAlignedLabelText("Updates per model:", inputWidgetOffset);
                                if(numUpdatesPerModel > 0)
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(numUpdatesPerModel).c_str());
                                else
                                    ImGui::TextDisabled(Utilities::toString(numUpdatesPerModel).c_str());

                                // Draw UPDATES PER MESH
                                auto numUpdatesPerMesh = uniformUpdater.getNumUpdatesPerMesh();
                                drawLeftAlignedLabelText("Updates per mesh:", inputWidgetOffset);
                                if(numUpdatesPerMesh > 0)
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(numUpdatesPerMesh).c_str());
                                else
                                    ImGui::TextDisabled(Utilities::toString(numUpdatesPerMesh).c_str());

                                // Draw TEXTURE UPDATES
                                auto numTextureUpdates = uniformUpdater.getNumTextureUpdates();
                                drawLeftAlignedLabelText("Texture updates:", inputWidgetOffset);
                                if(numTextureUpdates > 0)
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(numTextureUpdates).c_str());
                                else
                                    ImGui::TextDisabled(Utilities::toString(numTextureUpdates).c_str());

                                // Draw UNIFORM BLOCKS
                                auto numUniformBlocks = uniformUpdater.getNumUniformBlocks();
                                drawLeftAlignedLabelText("Uniform blocks:", inputWidgetOffset);
                                if(numUniformBlocks > 0)
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(numUniformBlocks).c_str());
                                else
                                    ImGui::TextDisabled(Utilities::toString(numUniformBlocks).c_str());

                                // Draw SSB BLOCKS
                                auto numSSBBlocks = uniformUpdater.getNumSSBBufferBlocks();
                                drawLeftAlignedLabelText("SSB blocks:", inputWidgetOffset);
                                if(numSSBBlocks > 0)
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(numSSBBlocks).c_str());
                                else
                                    ImGui::TextDisabled(Utilities::toString(numSSBBlocks).c_str());

                                ImGui::TreePop();
                            }
                        }
                        ImGui::EndChild();

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##ShaderAssetsBoundUniforms", ImVec2(contentRegionWidth * 0.2f, 0), true))
                        {
                            // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                            float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.75f + ImGui::GetStyle().ItemInnerSpacing.x;

                            // Get uniform updater
                            auto const &uniformUpdater = m_selectedProgram->getUniformUpdater();

                            ImGui::SeparatorText("Bound uniforms:");

                            // Draw PER-FRAME UPDATES
                            if(uniformUpdater.getNumUpdatesPerFrame() > 0 && ImGui::TreeNodeEx("Per-frame updates:", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                            {
                                auto const &perFrameUniforms = uniformUpdater.getPerFrameUniforms();
                                for(auto *uniform : perFrameUniforms)
                                {
                                    ImGui::Text((uniform->getName() + " (").c_str());
                                    ImGui::SameLine();
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(uniform->getHandle()).c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(")");
                                }

                                ImGui::TreePop();
                            }                            

                            // Draw PER-MODEL UPDATES
                            if(uniformUpdater.getNumUpdatesPerModel() > 0 && ImGui::TreeNodeEx("Per-model updates:", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                            {
                                auto const &perModelUniforms = uniformUpdater.getPerModelUniforms();
                                for(auto *uniform : perModelUniforms)
                                {
                                    ImGui::Text((uniform->getName() + " (").c_str());
                                    ImGui::SameLine();
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(uniform->getHandle()).c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(")");
                                }

                                ImGui::TreePop();
                            }

                            // Draw PER-MESH UPDATES
                            if(uniformUpdater.getNumUpdatesPerMesh() > 0 && ImGui::TreeNodeEx("Per-mesh updates:", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                            {
                                auto const &perMeshUniforms = uniformUpdater.getPerMeshUniforms();
                                for(auto *uniform : perMeshUniforms)
                                {
                                    ImGui::Text((uniform->getName() + " (").c_str());
                                    ImGui::SameLine();
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(uniform->getHandle()).c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(")");
                                }

                                ImGui::TreePop();
                            }

                            // Draw TEXTURE UPDATES
                            if(uniformUpdater.getNumTextureUpdates() > 0 && ImGui::TreeNodeEx("Texture updates:", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                            {
                                auto const &textureUniforms = uniformUpdater.getTextureUpdateUniforms();
                                for(auto *uniform : textureUniforms)
                                {
                                    ImGui::Text((uniform->getName() + " (").c_str());
                                    ImGui::SameLine();
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(uniform->getHandle()).c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(")");
                                }

                                ImGui::TreePop();
                            }

                            // Draw UNIFORM BLOCKS UPDATES
                            if(uniformUpdater.getNumUniformBlocks() > 0 && ImGui::TreeNodeEx("Uniform block updates:", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                            {
                                auto const &uniformBlocks = uniformUpdater.getUniformBlocks();
                                for(auto *uniform : uniformBlocks)
                                {
                                    ImGui::Text((uniform->getName() + " (").c_str());
                                    ImGui::SameLine();
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(uniform->getHandle()).c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(")");
                                }
                                ImGui::TreePop();
                            }

                            // Draw SSBO UPDATES
                            if(uniformUpdater.getNumSSBBufferBlocks() > 0 && ImGui::TreeNodeEx("SSBO updates:", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                            {
                                auto const &shaderStorageBlocks = uniformUpdater.getSSBblocks();
                                for(auto *uniform : shaderStorageBlocks)
                                {
                                    ImGui::Text((uniform->getName() + " (").c_str());
                                    ImGui::SameLine();
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Utilities::toString(uniform->getHandle()).c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(")");
                                }

                                ImGui::TreePop();
                            }
                        }
                        ImGui::EndChild();

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##ShaderAssetsSettings", ImVec2(contentRegionWidth * 0.3f, 0), true))
                        {
                            ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                            ImGui::SeparatorText("Shader settings:");

                            auto buttonWidth = ImGui::GetContentRegionAvail().x / 4.0f;

                            // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                            float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                            // Draw SHADER FILENAME
                            auto shaderFilename = m_selectedProgram->getShaderFilename(m_selectedShaderType);
                            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                            if(ImGui::InputText("##ShaderFilenameInput", &shaderFilename, ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                m_selectedProgram->setShaderFilename(static_cast<ShaderType>(m_selectedShaderType), shaderFilename);
                                m_selectedProgram->reloadToMemory();
                                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
                            }

                            // Draw OPEN button
                            ImGui::SameLine(calcTextSizedButtonOffset(1));
                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##ShaderFileOpenFileButton", "Open a shader file"))
                            {
                                // Only open the file browser if it's not opened already
                                if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                {
                                    // Set the file browser activation to Shader File
                                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_ShaderFile;

                                    // Define file browser variables
                                    m_fileBrowserDialog.m_filter = "Shader files (.frag .vert .geom .tesc .tese .comp .glsl .vs .fs){.frag,.vert,.geom,.tesc,.tese,.comp,.glsl,.vs,.fs},All files{.*}";
                                    m_fileBrowserDialog.m_title = "Open a shader file";
                                    m_fileBrowserDialog.m_name = "OpenShaderFileFileDialog";
                                    m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                                    // Set the root path only if it isn't saved from the last file dialog
                                    if(m_previouslyOpenedFileBrowser != m_currentlyOpenedFileBrowser)
                                        m_fileBrowserDialog.m_rootPath = Config::filepathVar().shader_path;

                                    // Tell the GUI scene to open the file browser
                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                }
                            }

                            const std::string shaderSelectionPopupName = "##ShaderSelectionPopup";

                            // Draw OPEN ASSET LIST button
                            ImGui::SameLine(calcTextSizedButtonOffset(0));
                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##ShaderOpenAssetListButton", "Choose a shader from the loaded assets"))
                            {
                                // Open the pop-up with the shader asset list
                                ImGui::OpenPopup(shaderSelectionPopupName.c_str());
                            }

                            // Draw SHADER ASSET LIST
                            if(ImGui::BeginPopup(shaderSelectionPopupName.c_str()))
                            {
                                // Remove selection border and align text vertically
                                ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                                for(decltype(m_shaderAssets.size()) shaderAssetIndex = 0, shaderAssetSize = m_shaderAssets.size(); shaderAssetIndex < shaderAssetSize; shaderAssetIndex++)
                                {
                                    for(unsigned int shaderAssetType = 0; shaderAssetType < ShaderType::ShaderType_NumOfTypes; shaderAssetType++)
                                    {
                                        if(!m_shaderAssets[shaderAssetIndex].first->getShaderFilename(shaderAssetType).empty())
                                        {
                                            if(ImGui::Selectable(m_shaderAssets[shaderAssetIndex].first->getShaderFilename(shaderAssetType).c_str(), 
                                                m_shaderAssets[shaderAssetIndex].first->getShaderFilename(shaderAssetType) == m_selectedProgram->getShaderFilename(m_selectedShaderType)))
                                            {
                                                m_selectedProgram->setShaderFilename(static_cast<ShaderType>(m_selectedShaderType), m_shaderAssets[shaderAssetIndex].first->getShaderFilename(shaderAssetType));
                                                m_selectedProgram->reloadToMemory();
                                                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
                                            }
                                        }
                                    }
                                }

                                ImGui::PopStyleVar(2); //ImGuiStyleVar_SelectableTextAlign, ImGuiStyleVar_FramePadding
                                ImGui::EndPopup();
                            }

                            // Draw SHADER TYPE
                            auto shaderType = m_selectedShaderType;
                            drawLeftAlignedLabelText("Shader type:", inputWidgetOffset, ImGui::GetWindowWidth() - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                            if(ImGui::Combo("##ShaderTypePicker", &shaderType, &m_shaderTypeStrings[0], (int)m_shaderTypeStrings.size()))
                            {
                            }

                            // Draw DEFAULT SHADER
                            auto defaultShader = m_selectedProgram->isDefaultProgram();
                            drawLeftAlignedLabelText("Default shader:", inputWidgetOffset, ImGui::GetWindowWidth() - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                            if(ImGui::Checkbox("##DefaultShaderCheck", &defaultShader))
                            {
                            }                           
                            
                            // Draw LOADED-TO-VIDEO-MEMORY
                            auto loadedToVideoMemory = m_selectedProgram->isLoadedToVideoMemory();
                            drawLeftAlignedLabelText("Loaded to VRAM:", inputWidgetOffset, ImGui::GetWindowWidth() - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                            if(ImGui::Checkbox("##LoadedToVideoMemoryCheck", &loadedToVideoMemory))
                            {
                            }
                        }
                        else
                            ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                        ImGui::EndChild();

                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##ShaderAssetsActions", ImVec2(contentRegionWidth * 0.1f, 0), true))
                        {
                            ImGui::SeparatorText("Actions:");

                            // Calculate button width so they span across the whole window width
                            auto buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f);

                            if(ImGui::Button("Edit", ImVec2(buttonWidth, 20.0f)))
                            {
                                TextEditorData *newTextEditorData = new TextEditorData();

                                auto &shaderFilename = m_selectedProgram->getShaderFilename(m_selectedShaderType);

                                newTextEditorData->m_filename = Utilities::stripFilename(shaderFilename);
                                newTextEditorData->m_filePath = Config::PathsVariables().shader_path + Utilities::stripFilePath(shaderFilename);

                                newTextEditorData->setLanguage(TextEditorLanguageType::TextEditorLanguageType_GLSL);
                                newTextEditorData->setText(m_selectedProgram->getShaderSource(static_cast<ShaderType>(m_selectedShaderType)));
                                newTextEditorData->enable();

                                m_textEditorFiles.push_back(newTextEditorData);
                            }

                            if(ImGui::Button("Reload", ImVec2(buttonWidth, 20.0f)))
                            {
                                m_selectedProgram->reloadToMemory();
                                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
                            }

                            ImGui::NewLine();

                            if(ImGui::Button("Open directory", ImVec2(buttonWidth, 20.0f)))
                            {
                                ShellExecuteA(NULL, "explore", (Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().shader_path + Utilities::stripFilePath(m_selectedProgram->getShaderFilename(m_selectedShaderType))).c_str(), NULL, NULL, SW_SHOWDEFAULT);
                            }
                        }
                        ImGui::EndChild();

                        ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                    }

                    ImGui::PopStyleVar(2); // ImGuiStyleVar_ChildBorderSize, ImGuiStyleVar_SeparatorTextAlign
                    ImGui::PopStyleColor(); // ImGuiCol_Border

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Scripts"))
                {
                    auto contentRegionWidth = ImGui::GetContentRegionAvail().x;

                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.26f, 0.26f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                    if(ImGui::BeginChild("##LuaScriptAssetsSelection", ImVec2(contentRegionWidth * 0.2f, 0), true))
                    {
                        ImGui::SeparatorText("Lua scripts:");
                        for(decltype(m_luaScriptAssets.size()) i = 0, size = m_luaScriptAssets.size(); i < size; i++)
                        {
                            if(ImGui::Selectable(m_luaScriptAssets[i].second.c_str(), m_selectedLuaScript != NULL_ENTITY_ID ? m_luaScriptAssets[i].first == m_selectedLuaScript : false))
                            {
                                m_selectedLuaScript = m_luaScriptAssets[i].first;
                            }
                        }
                    }
                    ImGui::EndChild();

                    auto *luaComponent = entityRegistry.try_get<LuaComponent>(m_selectedLuaScript);
                    if(luaComponent != nullptr)
                    {
                        auto *luaScript = luaComponent->getLuaScript();

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##LuaScriptAssetsVariables", ImVec2(contentRegionWidth * 0.2f, 0), true))
                        {
                            ImGui::SeparatorText("Variables:");

                            const auto &luaVariables = luaScript->getLuaVariables();

                            for(auto &variable : luaVariables)
                            {
                                ImGui::Text((variable.first + ": ").c_str());
                                ImGui::SameLine();
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), variable.second.getString().c_str());
                            }
                        }
                        ImGui::EndChild();

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##LuaScriptAssetsKeys", ImVec2(contentRegionWidth * 0.2f, 0), true))
                        {
                            ImGui::SeparatorText("Bound keys:");

                            const auto &keybinds = luaScript->getBoundKeys();

                            for(auto &keybind : keybinds)
                            {
                                if(ImGui::TreeNodeEx(keybind.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    for(decltype(keybind.second->getNumBindings()) i = 0, size = keybind.second->getNumBindings(); i < size; i++)
                                    {
                                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), GetString((*keybind.second)[i]));
                                    }
                                    ImGui::TreePop();
                                }
                            }
                        }
                        ImGui::EndChild();

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##LuaScriptAssetsSettings", ImVec2(contentRegionWidth * 0.3f, 0), true))
                        {

                            ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing

                            auto buttonWidth = ImGui::GetContentRegionAvail().x / 4.0f;

                            // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                            float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                            ImGui::SeparatorText("Lua script settings:");

                            // Draw SCRIPT FILENAME
                            auto scriptFilename = luaScript->getLuaScriptFilename();
                            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                            if(ImGui::InputText("##LuaScriptFilenameInput", &scriptFilename, ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                            }

                            // Draw OPEN button
                            ImGui::SameLine(calcTextSizedButtonOffset(1));
                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##LuaScriptOpenFileButton", "Open a Lua script file"))
                            {
                            }

                            const std::string luaScriptSelectionPopupName = "##LuaScriptSelectionPopup";

                            // Draw OPEN ASSET LIST button
                            ImGui::SameLine(calcTextSizedButtonOffset(0));
                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##LuaScriptOpenAssetListButton", "Choose a Lua script from the loaded scripts"))
                            {
                                // Open the pop-up with the shader asset list
                                ImGui::OpenPopup(luaScriptSelectionPopupName.c_str());
                            }

                            // Draw SHADER ASSET LIST
                            if(ImGui::BeginPopup(luaScriptSelectionPopupName.c_str()))
                            {
                                // Remove selection border and align text vertically
                                ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                                ImGui::PopStyleVar(2); //ImGuiStyleVar_SelectableTextAlign, ImGuiStyleVar_FramePadding
                                ImGui::EndPopup();
                            }
                        }
                        else
                            ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                        ImGui::EndChild();

                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##LuaScriptAssetsActions", ImVec2(contentRegionWidth * 0.1f, 0), true))
                        {
                            ImGui::SeparatorText("Actions:");

                            // Calculate button width so they span across the whole window width
                            auto buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f);

                            if(ImGui::Button("Edit", ImVec2(buttonWidth, 20.0f)))
                            {
                                // Get lua script filename
                                const std::string &filename = luaScript->getLuaScriptFilename();
                                std::string luaSource;

                                // Read lua script source from file
                                if(Filesystem::readTextFromFile(Config::PathsVariables().script_path + filename, luaSource))
                                {
                                    TextEditorData *newTextEditorData = new TextEditorData();

                                    newTextEditorData->m_filename = Utilities::stripFilename(filename);
                                    newTextEditorData->m_filePath = Config::PathsVariables().script_path + Utilities::stripFilePath(filename);

                                    newTextEditorData->setLanguage(TextEditorLanguageType::TextEditorLanguageType_Lua);
                                    newTextEditorData->setText(luaSource);
                                    newTextEditorData->enable();

                                    m_textEditorFiles.push_back(newTextEditorData);
                                }
                            }

                            if(ImGui::Button("Reload", ImVec2(buttonWidth, 20.0f)))
                            {
                                //m_selectedProgram->reloadToMemory();
                                //m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
                            }

                            ImGui::NewLine();

                            if(ImGui::Button("Open directory", ImVec2(buttonWidth, 20.0f)))
                            {
                                ShellExecuteA(NULL, "explore", (Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().script_path + Utilities::stripFilePath(luaScript->getLuaScriptFilename())).c_str(), NULL, NULL, SW_SHOWDEFAULT);
                            }


                        }
                        ImGui::EndChild();
                    }

                    ImGui::PopStyleVar(4); // ImGuiStyleVar_ChildBorderSize, ImGuiStyleVar_SeparatorTextAlign, ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                    ImGui::PopStyleColor(); // ImGuiCol_Border

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Console"))
                {
                    if(ImGui::BeginChild("##BottomConsoleWindow", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_::ImGuiWindowFlags_None))
                    {
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                        const auto &errorData = ErrHandlerLoc::get().getLogData();
                        const auto logSize = errorData.m_logs.size();

                        // Draw only the last 50 log messages
                        decltype(errorData.m_logs.size()) logIndex = logSize > 50 ? logSize - 50 : 0;

                        for(; logIndex < logSize; logIndex++)
                        {
                            // Draw ERROR TYPE
                            switch(errorData.m_logs[logIndex].m_logType)
                            {
                                case ErrorType::Info:
                                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ("[" + ErrHandlerLoc::get().getErrorTypeString(errorData.m_logs[logIndex].m_logType) + "] ").c_str());
                                    break;
                                case ErrorType::Warning:
                                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ("[" + ErrHandlerLoc::get().getErrorTypeString(errorData.m_logs[logIndex].m_logType) + "] ").c_str());
                                    break;
                                case ErrorType::Error:
                                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ("[" + ErrHandlerLoc::get().getErrorTypeString(errorData.m_logs[logIndex].m_logType) + "] ").c_str());
                                    break;
                                case ErrorType::FatalError:
                                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ("[" + ErrHandlerLoc::get().getErrorTypeString(errorData.m_logs[logIndex].m_logType) + "] ").c_str());
                                    break;
                            }

                            // Draw ERROR SOURCE
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), ("[" + ErrHandlerLoc::get().getErrorSourceString(errorData.m_logs[logIndex].m_logSource) + "]: ").c_str());

                            // Draw ERROR MESSAGE
                            ImGui::SameLine();
                            ImGui::TextWrapped(errorData.m_logs[logIndex].m_logMessage.c_str());
                        }

                        // If a new message was logged, scroll to the bottom
                        if(m_numOfLogs < errorData.m_logs.size())
                        {
                            m_numOfLogs = errorData.m_logs.size();
                            ImGui::SetScrollHereY(1.0f);
                        }

                        ImGui::PopStyleVar(); // ImGuiStyleVar_ItemSpacing
                    }
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    //	 ____________________________
    //	|							 |
    //	|	    CENTER WINDOW        |
    //	|____________________________|
    //
    {
        ImGui::SetNextWindowClass(&windowClassWithNoTabBar);
        if(ImGui::Begin("##CenterWindow", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar))
        {
            if(ImGui::BeginTabBar("##CenterWindowTabBar", ImGuiTabBarFlags_None))
            {
                if(ImGui::BeginTabItem("Scene viewport"))
                {
                    //	 ____________________________
                    //	|							 |
                    //	|	     DRAW SCENE          |
                    //	|____________________________|
                    //
                    // Get window starting position and the size of available space inside the window
                    m_sceneViewportPosition = ImGui::GetCursorScreenPos();
                    m_sceneViewportSize = ImGui::GetContentRegionAvail();

                    m_centerWindowSize.x = (int)m_sceneViewportSize.x;
                    m_centerWindowSize.y = (int)m_sceneViewportSize.y;

                    Config::m_rendererVar.current_viewport_position_x = m_sceneViewportPosition.x;
                    Config::m_rendererVar.current_viewport_position_y = m_sceneViewportPosition.y;

                    // Tell the renderer the size of available window space as a render to texture resolution, so that the rendered scene fill the whole window
                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTextureResolution, (void *)&m_centerWindowSize);

                    // Get the handle to the buffer that the scene is rendered into
                    const unsigned int sceneRenderBufferHandle = rendererScene->getUnsignedInt(this, Systems::Changes::Graphics::RenderToTextureBuffer);

                    //// Fill the whole window with the rendered scene
                    //ImGui::GetWindowDrawList()->AddImage(
                    //    (ImTextureID)sceneRenderBufferHandle,
                    //    windowPosition,
                    //    ImVec2(windowPosition.x + contentRegionSize.x, windowPosition.y + contentRegionSize.y),
                    //    ImVec2(0, 1),
                    //    ImVec2(1, 0)
                    //);

                    // Fill the whole window with the rendered scene, use an ImageButton with no border, so we can check if the mouse is hovering over it
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    ImGui::ImageButton(
                        (ImTextureID)sceneRenderBufferHandle,
                        m_sceneViewportSize,
                        ImVec2(0, 1),
                        ImVec2(1, 0)
                    );
                    ImGui::PopStyleVar();

                    //	 ____________________________
                    //	|							 |
                    //	|  DRAW MANIPULATION GUIZMO  |
                    //	|____________________________|
                    //
                    // Tell ImGuizmo to render inside the current window
                    ImGuizmo::SetDrawlist();

                    // Set the screen size for ImGuizmo
                    ImGuizmo::SetRect(m_sceneViewportPosition.x, m_sceneViewportPosition.y, m_sceneViewportSize.x, m_sceneViewportSize.y);

                    // Draw ImGuizmo only if there's an entity selected
                    if(m_selectedEntity)
                    {
                        // Draw ImGuizmo only if the selected entity has a spatial component
                        auto *spatialComponent = entityRegistry.try_get<SpatialComponent>(m_selectedEntity.m_entityID);
                        if(spatialComponent != nullptr)
                        {
                            ImGuizmo::Enable(true);

                            // Get the renderer scene (required for view and projection matrices)
                            auto const *rendererScene = static_cast<RendererScene*>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics));

                            // Try to get the rigid body component for sending spatial changes
                            auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(m_selectedEntity.m_entityID);

                            // Get the current world transform matrix of the selected entity
                            glm::mat4 worldMatrix = spatialComponent->getSpatialDataChangeManager().getWorldTransform();

                            // Draw TRANSLATE MANIPULATION ImGuizmo
                            if(m_translateGuizmoEnabled && ImGuizmo::Manipulate(glm::value_ptr(rendererScene->getViewMatrix()[0]), glm::value_ptr(rendererScene->getProjectionMatrix()[0]), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, glm::value_ptr(worldMatrix[0])))
                            {
                                // Get the local transform from the world matrix by multiplying it with the inverse of the parent transform
                                m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_transformMatNoScale = glm::inverse(spatialComponent->getSpatialDataChangeManager().getParentTransform()) * worldMatrix;

                                // If the model transform matrix was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                if(rigidBodyComponent != nullptr)
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                else
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                            }

                            // Draw ROTATE MANIPULATION ImGuizmo
                            if(m_rotateGuizmoEnabled && ImGuizmo::Manipulate(glm::value_ptr(rendererScene->getViewMatrix()[0]), glm::value_ptr(rendererScene->getProjectionMatrix()[0]), ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(worldMatrix[0])))
                            {
                                // Get the local transform from the world matrix by multiplying it with the inverse of the parent transform
                                m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_transformMatNoScale = glm::inverse(spatialComponent->getSpatialDataChangeManager().getParentTransform()) * worldMatrix;

                                // If the model transform matrix was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                if(rigidBodyComponent != nullptr)
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                else
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                            }
                        }
                        else
                            ImGuizmo::Enable(false);
                    }

                    // If the mouse is hovering over the viewport, stop capturing it, so the engine can handle mouse events
                    if(ImGui::IsItemHovered())
                        ImGui::CaptureMouseFromApp(false);

                    // If the viewport is focused, stop capturing key presses, so the engine can handle keyboard events
                    if(ImGui::IsItemFocused())
                        ImGui::CaptureKeyboardFromApp(false);

                    ImGui::EndTabItem();
                }

                // Show the Texture inspector tab only when there's a texture selected
                if(m_selectedTexture != nullptr)
                {
                    if(ImGui::BeginTabItem("Texture inspector", 0, m_textureInspectorTabFlags))
                    {
                        // Reset the tab flags (that previously been set to focus the window, when the texture was selected), so it doesn't get continuously focused
                        m_textureInspectorTabFlags = 0;

                        // Get the functor sequence container from the front buffer, because the back buffer is being used in GUI Pass
                        auto &functorSequence = m_textureInspectorSequence.getFront();

                        // Construct a call to texture inspector, that appends to the already created Texture inspector window straight from the GUI Pass
                        // Texture inspector cannot be called from here directly, as it needs access to OpenGL API and that can only be achieved from the Graphics thread (the thread that created the OpenGL context)
                        // All Rendering Passes (including GUI Pass) are run from the Graphics thread
                        functorSequence.addFunctor([=]
                            {
                                if(ImGui::Begin("##CenterWindow"))
                                {
                                    if(ImGui::BeginTabBar("##CenterWindowTabBar", ImGuiTabBarFlags_None))
                                    {
                                        if(ImGui::BeginTabItem("Texture inspector"))
                                        {
                                            ImVec2 textureSize = ImVec2((float)m_selectedTexture->getTextureWidth(), (float)m_selectedTexture->getTextureHeight());

                                            ImGuiTexInspect::BeginInspectorPanel("Inspector", (ImTextureID)m_selectedTexture->getHandle(), textureSize, ImGuiTexInspect::InspectorFlags_FlipY, ImGuiTexInspect::SizeIncludingBorder(ImGui::GetContentRegionAvail()));
                                            ImGuiTexInspect::DrawAnnotations(ImGuiTexInspect::ValueText(ImGuiTexInspect::ValueText::Floats));
                                            ImGuiTexInspect::EndInspectorPanel();

                                            ImGui::EndTabItem();
                                        }

                                        ImGui::EndTabBar();
                                    }
                                }
                                ImGui::End();
                                    });

                        // Send the functor sequence to the Graphics scene so it can be further sent to GUI Pass
                        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_GUIPassFunctors, (void *)&functorSequence);

                        ImGui::EndTabItem();
                    }
                }

                // Reset the currently active text editor
                m_currentlyActiveTextEditor = nullptr;

                // Draw TEXT EDITOR
                for(decltype(m_textEditorFiles.size()) i = 0, size = m_textEditorFiles.size(); i < size; i++)
                {
                    if(m_textEditorFiles[i]->m_textEditorEnabled)
                    {
                        bool tabOpened = true;

                        if(ImGui::BeginTabItem((m_textEditorFiles[i]->m_filename + "##" + Utilities::toString(i)).c_str(), &tabOpened, m_textEditorFiles[i]->m_textEditorTabFlags | (m_textEditorFiles[i]->m_textEditor.HasTextChanged() ? ImGuiTabItemFlags_UnsavedDocument : 0)))
                        {
                            // Reset the tab flags (that previously been set to focus the window, when the text editor was launched), so it doesn't get continuously focused
                            if(m_textEditorFiles[i]->m_textEditorTabFlags != 0)
                                m_textEditorFiles[i]->m_textEditorTabFlags = 0;

                            //auto cursorPosition = m_textEditorFiles[i]->m_textEditor.GetCursorPosition();
                            int linePosition;
                            int columnPosition;
                            m_textEditorFiles[i]->m_textEditor.GetCursorPosition(linePosition, columnPosition);

                            const char *whitespacesText = "Show whitespaces";
                            const char *autoIndentText = "Auto indent";
                            const char *saveToFileText = "Save to file";

                            auto contentRegionMax = ImGui::GetWindowContentRegionMax().x;
                            auto languageTypeComboWidgetSize = ImGui::CalcTextSize(m_textEditorLanguageTypeText[TextEditorLanguageType::TextEditorLanguageType_AngelScript]).x + ImGui::GetFrameHeight() + m_imguiStyle.FramePadding.x * 2.0f;
                            auto languageTypeComboSize = languageTypeComboWidgetSize + m_imguiStyle.ItemInnerSpacing.x;
                            auto whitespacesCheckboxSize = ImGui::GetFrameHeight() + ImGui::CalcTextSize(whitespacesText).x + m_imguiStyle.FramePadding.x * 2.0f + m_imguiStyle.ItemInnerSpacing.x;
                            auto autoIndentCheckboxSize = ImGui::GetFrameHeight() + ImGui::CalcTextSize(autoIndentText).x + m_imguiStyle.FramePadding.x * 2.0f + m_imguiStyle.ItemInnerSpacing.x;
                            auto saveButtonSize = ImGui::CalcTextSize(saveToFileText).x + m_imguiStyle.FramePadding.x * 2.0f + m_imguiStyle.ItemInnerSpacing.x;
                            
                            // Draw status bar
                            ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s", linePosition + 1, columnPosition + 1, m_textEditorFiles[i]->m_textEditor.GetLineCount(),
                                m_textEditorFiles[i]->m_textEditor.IsOverwriteEnabled() ? "Ovr" : "Ins",
                                m_textEditorFiles[i]->m_textEditor.CanUndo() ? "*" : " ",
                                m_textEditorFiles[i]->m_filename.c_str());

                            // Draw SHOW WHITESPACES
                            ImGui::SameLine(contentRegionMax - saveButtonSize - languageTypeComboSize - autoIndentCheckboxSize - whitespacesCheckboxSize);
                            if(bool showWhitespaces = m_textEditorFiles[i]->m_textEditor.IsShowWhitespacesEnabled(); ImGui::Checkbox(whitespacesText, &showWhitespaces))
                                m_textEditorFiles[i]->m_textEditor.SetShowWhitespacesEnabled(showWhitespaces);

                            // Draw AUTO INDENT
                            ImGui::SameLine(contentRegionMax - saveButtonSize - languageTypeComboSize - autoIndentCheckboxSize);
                            if(bool autoIndent = m_textEditorFiles[i]->m_textEditor.IsAutoIndentEnabled(); ImGui::Checkbox(autoIndentText, &autoIndent))
                                m_textEditorFiles[i]->m_textEditor.SetAutoIndentEnabled(autoIndent);

                            // Draw LANGUAGE TYPE
                            ImGui::SameLine(contentRegionMax - saveButtonSize - languageTypeComboSize);
                            ImGui::SetNextItemWidth(languageTypeComboWidgetSize);
                            if(int languageType = m_textEditorFiles[i]->m_languageType; ImGui::Combo(("##" + Utilities::toString(i) + "TextEditorLanguage").c_str(), &languageType, &m_textEditorLanguageTypeText[0], (int)m_textEditorLanguageTypeText.size()))
                                m_textEditorFiles[i]->setLanguage(static_cast<TextEditorLanguageType>(languageType));

                            // Enable button only when the text was changed
                            // Remember the text-changed flag, because it might get reset if the save-to-file button is pressed
                            bool textChanged = m_textEditorFiles[i]->m_textEditor.HasTextChanged();
                            if(!textChanged)
                                ImGui::BeginDisabled();

                            // Draw SAVE TO FILE
                            ImGui::SameLine(contentRegionMax - saveButtonSize);
                            if(ImGui::Button(saveToFileText))
                                saveTextFile(*m_textEditorFiles[i]);

                            // Enable button only when the text was changed
                            if(!textChanged)
                                ImGui::EndDisabled();

                            // Draw text editor
                            m_textEditorFiles[i]->m_textEditor.Render(("##" + Utilities::toString(i) + m_textEditorFiles[i]->m_filename).c_str());

                            // Set the flag for changed text
                            //if(!m_textEditorFiles[i]->m_textChanged && m_textEditorFiles[i]->m_textEditor.IsTextChanged())
                                //m_textEditorFiles[i]->m_textChanged = true;

                            // Set the currently active text editor
                            m_currentlyActiveTextEditor = m_textEditorFiles[i];

                            ImGui::EndTabItem();
                        }

                        // If the tab was closed, delete the text editor entry
                        if(!tabOpened)
                        {
                            m_currentlyActiveTextEditor = nullptr;
                            delete m_textEditorFiles[i];
                            m_textEditorFiles.erase(m_textEditorFiles.begin() + i);
                            size = m_textEditorFiles.size();
                            i--;
                        }
                    }
                }

                // Show the New Scene Settings tab only when New Scene button was pressed
                if(m_showNewMapWindow)
                {
                    if(ImGui::BeginTabItem("New scene settings", 0, m_newSceneSettingsTabFlags))
                    {
                        // Reset the tab flags (that previously been set to focus the window, when the new scene button was pressed), so it doesn't get continuously focused
                        m_newSceneSettingsTabFlags = 0;

                        auto contentRegionSize = ImGui::GetContentRegionAvail();

                        ImGui::NewLine();

                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.26f, 0.26f, 1.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                        if(ImGui::BeginChild("##NewSceneSettingsWindow", ImVec2(contentRegionSize.x * 0.6f, contentRegionSize.y * 0.9f), true))
                        {
                            ImGui::PopStyleVar(); // ImGuiStyleVar_ChildBorderSize
                            ImGui::PopStyleColor(); // ImGuiCol_Border

                            ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));
                            ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 5.0f);
                            ImGui::PopStyleVar(2); //ImGuiStyleVar_SeparatorTextAlign, ImGuiStyleVar_SeparatorTextBorderSize

                            ImGui::NewLine();

                            drawSceneData(m_newSceneData);

                        }
                        else
                        {
                            ImGui::PopStyleVar(); // ImGuiStyleVar_ChildBorderSize
                            ImGui::PopStyleColor(); // ImGuiCol_Border
                        }
                        ImGui::EndChild();

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##NewSceneButtonsWindow"))
                        {
                            const ImVec2 buttonSize(contentRegionSize.x * 0.2f, ImGui::GetFrameHeight() * 1.5f);
                            const float centerButtonOffset = (ImGui::GetContentRegionAvail().x / 2.0f) - (buttonSize.x / 2.0f);

                            // Draw CREATE SCENE button
                            ImGui::SetCursorPosX(centerButtonOffset);
                            if(ImGui::Button("Create scene", buttonSize))
                            {
                                PropertySet sceneProperties(Properties::Default);

                                generateNewMap(sceneProperties, m_newSceneData);

                                // Send a notification to the engine to reload the current engine state
                                m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneReload, EngineStateType::EngineStateType_Editor, sceneProperties));
                            }

                            // Draw CANCEL button
                            ImGui::SetCursorPosX(centerButtonOffset);
                            if(ImGui::Button("Cancel", buttonSize))
                            {
                                m_newSceneData = SceneData();
                                m_showNewMapWindow = false;
                            }

                            // Draw RELOAD TO DEFAULT button
                            ImGui::NewLine();
                            ImGui::SetCursorPosX(centerButtonOffset);
                            if(ImGui::Button("Reload to default", buttonSize))
                            {
                                m_newSceneData = SceneData();
                            }
                        }
                        ImGui::EndChild();

                        ImGui::EndTabItem();
                    }
                }

                if(m_showImspinnerDemoWindow)
                {
                    if(ImGui::BeginTabItem("Demo spinners", 0, m_textureInspectorTabFlags))
                    {
                        ImSpinner::demoSpinners();
                        ImGui::EndTabItem();
                    }
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    // Pop window padding style variable
    ImGui::PopStyleVar();

    // Handle the file browser dialog
    switch(m_currentlyOpenedFileBrowser)
    {
        case EditorWindow::FileBrowserActivated_LuaScript:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().script_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            // Set the selected file path as a relative path from current directory
                            m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                            // If the Lua script filename was changed, send a notification to the LUA Component
                            m_selectedEntity.m_luaScriptFilenameModified = true;

                            // Remember the last opened file browser type
                            m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_LoadScene:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Send a notification to the engine to reload the current engine state
                        m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneFilename, EngineStateType::EngineStateType_Editor, m_fileBrowserDialog.m_filename));
                        m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneReload, EngineStateType::EngineStateType_Editor, m_fileBrowserDialog.m_filename));
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_SaveScene:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().map_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            // Get the filename path in the Maps directory
                            auto filename = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                            // Save the scene to the chosen file
                            m_systemScene->getSceneLoader()->saveToFile(filename);

                            // Send a notification to the editor state of a changed scene filename
                            m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneFilename, EngineStateType::EngineStateType_Editor, filename));

                            // Update the current scene data
                            m_currentSceneData.m_sceneFilename = Utilities::stripFilename(filename);
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_SoundFile:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().sound_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            // Set the selected file path as a relative path from current directory
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundName = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                            // If the Lua script filename was changed, set a flag for it
                            m_selectedEntity.m_soundFilenameModified = true;
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_ModelFile:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().model_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            if(m_selectedEntity.m_selectedModelName != nullptr)
                            {
                                *m_selectedEntity.m_selectedModelName = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                                // If the model filename was changed, set a flag for it
                                m_selectedEntity.m_modelDataUpdatedFromFilebrowser = true;
                            }
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_TextureFile:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().texture_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            if(m_selectedEntity.m_selectedTextureName != nullptr)
                            {
                                *m_selectedEntity.m_selectedTextureName = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                                // If the texture filename was changed, set a flag for it
                                m_selectedEntity.m_modelDataUpdatedFromFilebrowser = true;
                            }
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_AudioBankFile:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().sound_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            if(m_fileBrowserDialog.m_userStringPointer != nullptr)
                            {
                                *m_fileBrowserDialog.m_userStringPointer = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());
                            }
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.resetAll();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_PrefabFile:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().prefab_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            if(m_fileBrowserDialog.m_userStringPointer != nullptr)
                            {
                                *m_fileBrowserDialog.m_userStringPointer = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                                // If the prefab name was changed, set a flag for it
                                m_selectedEntity.m_prefabNameModified = true;
                            }
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_SavePrefabFile:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().prefab_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            if(m_selectedEntity.m_entityID != NULL_ENTITY_ID)
                                exportPrefab(m_selectedEntity.m_entityID, m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size()));
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
        case EditorWindow::FileBrowserActivated_ShaderFile:
            {
                // If the file browser was activated and it is now closed, process the result
                if(m_fileBrowserDialog.m_closed)
                {
                    if(m_fileBrowserDialog.m_success)
                    {
                        // Get the current directory path
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().shader_path;

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePathName.rfind(currentDirectory, 0) == 0)
                        {
                            m_selectedShaderFilename = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

                    // Remember the last opened file browser type
                    m_previouslyOpenedFileBrowser = m_currentlyOpenedFileBrowser;

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.reset();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
    }
}

void EditorWindow::activate()
{
    WindowLocator::get().setWindowTitle(Config::windowVar().name + " - Editor");
    WindowLocator::get().setMouseCapture(false);
}

void EditorWindow::deactivate()
{
    WindowLocator::get().setWindowTitle(Config::windowVar().name);
}

void EditorWindow::setup(EditorWindowSettings &p_editorWindowSettings)
{
    ImGuiTexInspect::ImplOpenGL3_Init();
    ImGuiTexInspect::Init();
    ImGuiTexInspect::CreateContext();

    //	 ____________________________
    //	|							 |
    //	|  WARNING: ORDER DEPENDENT  |
    //	|      TEXTURE CREATION      |
    //	|____________________________|
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_pause_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_play_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_restart_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_gui_sequence_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_scripting_enabled_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_delete_entry_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_add_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_duplicate_entry_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_open_file_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_reload_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_open_asset_list_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_arrow_up_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_guizmo_rotate_texture, m_buttonMaterialType));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::filepathVar().gui_assets_path + Config::GUIVar().editor_button_guizmo_translate_texture, m_buttonMaterialType));

    assert(m_buttonTextures.size() == ButtonTextureType::ButtonTextureType_NumOfTypes && "m_buttonTextures array is different size than the number of button textures, in EditorWindow.cpp");

    // Load button textures to memory
    for(decltype(m_buttonTextures.size()) size = m_buttonTextures.size(), i = 0; i < size; i++)
    {
        m_buttonTextures[i].setEnableDownsampling(false);
        m_buttonTextures[i].loadToMemory();
    }

    // Load button textures to GPU
    for(decltype(m_buttonTextures.size()) size = m_buttonTextures.size(), i = 0; i < size; i++)
        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadTexture2D, (void *)&m_buttonTextures[i]);

    // Tell the renderer to draw the scene to a texture instead of the screen
    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTexture, (void *)true);

    // If the GUI Sequence Enabled flag is set to false, tell the GUI scene to disable GUI Sequence components
    if(!m_GUISequenceEnabled)
        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_EnableGUISequence, (void *)m_GUISequenceEnabled);

    // If the LUA Scripting flag is set to false, tell the Scripting scene to disable LUA components
    if(!m_LUAScriptingEnabled)
        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)m_LUAScriptingEnabled);

    // If the scene is paused, tell the Physics scene to pause the simulation
    if(m_sceneState == EditorSceneState::EditorSceneState_Pause)
        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)false);
}

void EditorWindow::drawSceneData(SceneData &p_sceneData, const bool p_sendChanges)
{
    // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
    float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.26f, 0.26f, 1.0f));

    // Center the separator text
    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));
    ImGui::SeparatorText("Scene settings:");

    // Draw SCENE FILENAME
    if(!p_sceneData.m_sceneFilename.empty())
    {
        drawLeftAlignedLabelText("Filename:", inputWidgetOffset);
        ImGui::Text(p_sceneData.m_sceneFilename.c_str());
    }

    // Draw LOAD IN BACKGROUND
    drawLeftAlignedLabelText("Load in background:", inputWidgetOffset);
    if(ImGui::Checkbox("##LoadInBackgroundCheckbox", &p_sceneData.m_loadInBackground) && p_sendChanges)
    {
        p_sceneData.m_modified = true;
        m_systemScene->getSceneLoader()->setLoadInBackground(p_sceneData.m_loadInBackground);
    }

    // Draw GRAVITY
    drawLeftAlignedLabelText("Gravity:", inputWidgetOffset);
    if(ImGui::DragFloat3("##GravityDrag", glm::value_ptr(p_sceneData.m_gravity), Config::GUIVar().editor_float_slider_speed) && p_sendChanges)
    {
        p_sceneData.m_modified = true;
        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), Systems::Changes::Physics::Gravity);
    }
    captureMouseWhileItemActive();

    ImGui::SeparatorText("Audio scene settings:");

    // Create a duplicate variable for each volume type and multiply it by 100, so it can be shown as percentage
    float volumeMult[AudioBusType::AudioBusType_NumOfTypes];
    for(unsigned int i = 0; i < AudioBusType::AudioBusType_NumOfTypes; i++)
        volumeMult[i] = p_sceneData.m_volume[i] * 100.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
    ImGui::SeparatorText("Volume control:");
    ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

    // Draw MASTER VOLUME
    drawLeftAlignedLabelText("Master volume:", inputWidgetOffset);
    if(ImGui::SliderFloat("##MasterVolumeSlider", &volumeMult[AudioBusType::AudioBusType_Master], 0.0f, 100.0f, "%.0f%%", ImGuiSliderFlags_None))
    {
        p_sceneData.m_volume[AudioBusType::AudioBusType_Master] = volumeMult[AudioBusType::AudioBusType_Master] / 100.0f;
        if(p_sendChanges)
        {
            p_sceneData.m_modified = true;
            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeMaster);
        }
    }

    // Draw AMBIENT VOLUME
    drawLeftAlignedLabelText("Ambient volume:", inputWidgetOffset);
    if(ImGui::SliderFloat("##AmbientVolumeSlider", &volumeMult[AudioBusType::AudioBusType_Ambient], 0.0f, 100.0f, "%.0f%%", ImGuiSliderFlags_None))
    {
        p_sceneData.m_volume[AudioBusType::AudioBusType_Ambient] = volumeMult[AudioBusType::AudioBusType_Ambient] / 100.0f;
        if(p_sendChanges)
        {
            p_sceneData.m_modified = true;
            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeAmbient);
        }
    }

    // Draw MUSIC VOLUME
    drawLeftAlignedLabelText("Music volume:", inputWidgetOffset);
    if(ImGui::SliderFloat("##MusicVolumeSlider", &volumeMult[AudioBusType::AudioBusType_Music], 0.0f, 100.0f, "%.0f%%", ImGuiSliderFlags_None))
    {
        p_sceneData.m_volume[AudioBusType::AudioBusType_Music] = volumeMult[AudioBusType::AudioBusType_Music] / 100.0f;
        if(p_sendChanges)
        {
            p_sceneData.m_modified = true;
            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeMusic);
        }
    }

    // Draw SFX VOLUME
    drawLeftAlignedLabelText("Sound effect volume:", inputWidgetOffset);
    if(ImGui::SliderFloat("##SFXVolumeSlider", &volumeMult[AudioBusType::AudioBusType_SFX], 0.0f, 100.0f, "%.0f%%", ImGuiSliderFlags_None))
    {
        p_sceneData.m_volume[AudioBusType::AudioBusType_SFX] = volumeMult[AudioBusType::AudioBusType_SFX] / 100.0f;
        if(p_sendChanges)
        {
            p_sceneData.m_modified = true;
            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeSFX);
        }
    }

    // Calculate audio banks window height and cap it to a max height value
    float audioBanksWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * (p_sceneData.m_audioBanks.size() + 2);
    audioBanksWindowHeight = audioBanksWindowHeight > Config::GUIVar().editor_audio_banks_max_height ? Config::GUIVar().editor_audio_banks_max_height : audioBanksWindowHeight;

    if(ImGui::BeginChild("##AudioBanks", ImVec2(0, audioBanksWindowHeight), true))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
        ImGui::SeparatorText("Audio banks:");
        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

        for(decltype(p_sceneData.m_audioBanks.size()) size = p_sceneData.m_audioBanks.size(), i = 0; i < size; i++)
        {
            // Draw BANK FILENAME
            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(2) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
            if(ImGui::InputText(("##AudioBankFilenameInput" + Utilities::toString(i)).c_str(), &p_sceneData.m_audioBanks[i], ImGuiInputTextFlags_EnterReturnsTrue))
            {
            }

            // Draw OPEN button
            ImGui::SameLine(calcTextSizedButtonOffset(2));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##" + Utilities::toString(i) + "AudioBankOpenButton", "Browse for an Audio Bank file"))
            {
                // Only open the file browser if it's not opened already
                if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                {
                    // Set the file browser activation to Lua Script
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_AudioBankFile;

                    // Define file browser variables
                    m_fileBrowserDialog.m_filter = "Audio Bank files (.bank){.bank},All files{.*}";
                    m_fileBrowserDialog.m_title = "Select an Audio Bank file";
                    m_fileBrowserDialog.m_name = "OpenAudioBankFileDialog";
                    m_fileBrowserDialog.m_rootPath = Config::filepathVar().sound_path;
                    m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;
                    m_fileBrowserDialog.m_userStringPointer = &p_sceneData.m_audioBanks[i];

                    // Tell the GUI scene to open the file browser
                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                }
            }

            // Draw DELETE button
            ImGui::SameLine(calcTextSizedButtonOffset(1));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##" + Utilities::toString(i) + "AudioBanksDeleteButton", "Remove this Audio Bank entry"))
            {
                p_sceneData.m_audioBanks.erase(p_sceneData.m_audioBanks.begin() + i);
                size = p_sceneData.m_audioBanks.size();
                i--;
            }

            // Draw ADD button
            ImGui::SameLine(calcTextSizedButtonOffset(0));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##" + Utilities::toString(i) + "AudioBanksAddButton", "Add a new Audio Bank entry"))
            {
                p_sceneData.m_audioBanks.insert(p_sceneData.m_audioBanks.begin() + i + 1, "");
                size = p_sceneData.m_audioBanks.size();
            }
        }

        // Draw ADD button
        ImGui::SetCursorPosX(calcTextSizedButtonOffset(0));
        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##AudioBanksAddAtEndButton", "Add a new Audio Bank entry"))
        {
            p_sceneData.m_audioBanks.push_back("");
        }

    }
    ImGui::EndChild();

    ImGui::SeparatorText("Graphics scene settings:");

    bool miscSceneDataChanged = false;

    // Draw ACTIVE CAMERA ID
    drawLeftAlignedLabelText("Active camera ID:", inputWidgetOffset);
    if(ImGui::InputInt("##ActiveCameraIDInput", &p_sceneData.m_activeCameraID) && p_sendChanges)
    {
        // If the active camera ID was changed, send a notification to the Graphics Scene
        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), Systems::Changes::Graphics::ActiveCameraID);
    }

    // Draw STOCHASTIC SAMPLING SEAM FIX
    drawLeftAlignedLabelText("Stochastic sampling seam fix:", inputWidgetOffset * 2.0f);
    if(ImGui::Checkbox("##StochasticSamplingSeamFixCheckbox", &p_sceneData.m_miscSceneData.m_stochasticSamplingSeamFix) && p_sendChanges)
    {
        miscSceneDataChanged = true;
    }

    if(ImGui::BeginChild("##AmbientLightIntensitySettings", ImVec2(0.0f, (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * 4), true))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
        ImGui::SeparatorText("Ambient light intensity:");
        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

        // Draw AMBIENT LIGHT INTENSITY
        drawLeftAlignedLabelText("Directional light:", inputWidgetOffset);
        if(ImGui::DragFloat("##DirLightAmbientIntensityDrag", &p_sceneData.m_miscSceneData.m_ambientIntensityDirectional, 0.001f, 0.0f, 100000.0f, "%.5f") && p_sendChanges)
        {
            miscSceneDataChanged = true;
        }
        captureMouseWhileItemActive();

        // Draw AMBIENT LIGHT INTENSITY
        drawLeftAlignedLabelText("Point light:", inputWidgetOffset);
        if(ImGui::DragFloat("##PointLightAmbientIntensityDrag", &p_sceneData.m_miscSceneData.m_ambientIntensityPoint, 0.001f, 0.0f, 100000.0f, "%.5f") && p_sendChanges)
        {
            miscSceneDataChanged = true;
        }
        captureMouseWhileItemActive();

        // Draw AMBIENT LIGHT INTENSITY
        drawLeftAlignedLabelText("Spot light:", inputWidgetOffset);
        if(ImGui::DragFloat("##SpotLightAmbientIntensityDrag", &p_sceneData.m_miscSceneData.m_ambientIntensitySpot, 0.001f, 0.0f, 100000.0f, "%.5f") && p_sendChanges)
        {
            miscSceneDataChanged = true;
        }
        captureMouseWhileItemActive();
    }
    ImGui::EndChild();

    // Send the new misc scene data to the renderer
    if(miscSceneDataChanged)
    {
        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_MiscSceneData, (void *)&p_sceneData.m_miscSceneData, false);
        p_sceneData.m_modified = true;
    }

    // Calculate ambient occlusion window height
    float ambientOcclusionWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y);
    ambientOcclusionWindowHeight *= p_sceneData.m_aoData.m_aoType == AmbientOcclusionType::AmbientOcclusionType_None ? 2.0f : 10.0f;

    // Draw AMBIENT OCCLUSION SETTINGS
    if(ImGui::BeginChild("##AmbientOcclusionSettings", ImVec2(0.0f, ambientOcclusionWindowHeight), true))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
        ImGui::SeparatorText("Ambient occlusion:");
        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

        bool aoDataChanged = false;
        int aoType = p_sceneData.m_aoData.m_aoType;
        drawLeftAlignedLabelText("Ambient occlusion type:", inputWidgetOffset);
        if(ImGui::Combo("##AOTypeCombo", &aoType, &m_ambientOcclusionTypeText[0], (int)m_ambientOcclusionTypeText.size()))
        {
            p_sceneData.m_aoData.m_aoType = AmbientOcclusionData::ambientOcclusionTypeToAmbientOcclusionType(aoType);

            if(p_sendChanges)
                aoDataChanged = true;

            // Apply default AO type specific values
            switch(p_sceneData.m_aoData.m_aoType)
            {
                case AmbientOcclusionType_SSAO:
                    p_sceneData.m_aoData.m_aoBias = Config::graphicsVar().ao_ssao_bias;
                    p_sceneData.m_aoData.m_aoIntensity = Config::graphicsVar().ao_ssao_intensity;
                    p_sceneData.m_aoData.m_aoRadius = Config::graphicsVar().ao_ssao_radius;
                    break;
                case AmbientOcclusionType_HBAO:
                    p_sceneData.m_aoData.m_aoBias = Config::graphicsVar().ao_hbao_bias;
                    p_sceneData.m_aoData.m_aoIntensity = Config::graphicsVar().ao_hbao_intensity;
                    p_sceneData.m_aoData.m_aoRadius = Config::graphicsVar().ao_hbao_radius;
                    break;
            }
        }

        if(p_sceneData.m_aoData.m_aoType != AmbientOcclusionType::AmbientOcclusionType_None)
        {
            // Draw NUMBER OF DIRECTIONS
            drawLeftAlignedLabelText("Number of directions:", inputWidgetOffset);
            if(ImGui::InputInt("##AONumberOfDirectionsInputInt", &p_sceneData.m_aoData.m_aoNumOfDirections) && p_sendChanges)
            {
                aoDataChanged = true;
            }

            // Draw NUMBER OF STEPS
            drawLeftAlignedLabelText("Number of steps:", inputWidgetOffset);
            if(ImGui::InputInt("##AONumberOfStepsInputInt", &p_sceneData.m_aoData.m_aoNumOfSteps) && p_sendChanges)
            {
                aoDataChanged = true;
            }

            // Draw NUMBER OF SAMPLES
            drawLeftAlignedLabelText("Number of samples:", inputWidgetOffset);
            if(ImGui::InputInt("##AONumberOfSamplesInputInt", &p_sceneData.m_aoData.m_aoNumOfSamples) && p_sendChanges)
            {
                aoDataChanged = true;
            }

            // Draw NUMBER OF BLUR SAMPLES
            drawLeftAlignedLabelText("Number of blur samples:", inputWidgetOffset);
            if(ImGui::InputInt("##AONumberOfBlurSamplesInputInt", &p_sceneData.m_aoData.m_aoBlurNumOfSamples) && p_sendChanges)
            {
                aoDataChanged = true;
            }

            // Draw AO INTENSITY
            drawLeftAlignedLabelText("Intensity:", inputWidgetOffset);
            if(ImGui::DragFloat("##AOIntensityDrag", &p_sceneData.m_aoData.m_aoIntensity, 0.1f, 0.0f, 100000.0f, "%.3f") && p_sendChanges)
            {
                aoDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw AO BIAS
            drawLeftAlignedLabelText("Bias:", inputWidgetOffset);
            if(ImGui::DragFloat("##AOBiasDrag", &p_sceneData.m_aoData.m_aoBias, 0.0001f, 0.0f, 1.0f, "%.5f") && p_sendChanges)
            {
                aoDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw AO RADIUS
            drawLeftAlignedLabelText("Radius:", inputWidgetOffset);
            if(ImGui::DragFloat("##AORadiusDrag", &p_sceneData.m_aoData.m_aoRadius, 0.01f, 0.0f, 100000.0f, "%.3f") && p_sendChanges)
            {
                aoDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw AO BLUR SHARPNESS
            drawLeftAlignedLabelText("Blur sharpness:", inputWidgetOffset);
            if(ImGui::DragFloat("##AOBlurSharpnessDrag", &p_sceneData.m_aoData.m_aoBlurSharpness, 0.1f, 0.0f, 100000.0f, "%.3f") && p_sendChanges)
            {
                aoDataChanged = true;
            }
            captureMouseWhileItemActive();

        }

        if(aoDataChanged)
        {
            m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_AmbientOcclusionData, (void *)&p_sceneData.m_aoData, false);
            p_sceneData.m_modified = true;
        }
    }
    ImGui::EndChild();

    // Calculate atmospheric light scattering window height
    float atmScatteringWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * 10.0f;

    // Draw ATMOSPHERIC LIGHT SCATTERING SETTINGS
    if(ImGui::BeginChild("##AtmScatteringSettings", ImVec2(0.0f, atmScatteringWindowHeight), true))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
        ImGui::SeparatorText("Atmospheric light scattering:");

        bool atmScatteringDataChanged = false;

        ImGui::SeparatorText("Atmosphere:");

        // Draw BOTTOM RADIUS
        drawLeftAlignedLabelText("Bottom radius:", inputWidgetOffset);
        if(ImGui::DragFloat("##AtmBottomRadiusDrag", &p_sceneData.m_atmScatteringData.m_atmosphereBottomRadius, 10.0f, 0.0f, 1000000.0f, "%.1f") && p_sendChanges)
        {
            atmScatteringDataChanged = true;
        }
        captureMouseWhileItemActive();

        // Draw TOP RADIUS
        drawLeftAlignedLabelText("Top radius:", inputWidgetOffset);
        if(ImGui::DragFloat("##AtmTopRadiusDrag", &p_sceneData.m_atmScatteringData.m_atmosphereTopRadius, 10.0f, 0.0f, 1000000.0f, "%.1f") && p_sendChanges)
        {
            atmScatteringDataChanged = true;
        }
        captureMouseWhileItemActive();

        ImGui::SeparatorText("Ground:");

        // Draw GROUND COLOR
        drawLeftAlignedLabelText("Ground color:", inputWidgetOffset);
        if(ImGui::ColorEdit3("##AtmGroundColorEdit", glm::value_ptr(p_sceneData.m_atmScatteringData.m_planetGroundColor), m_colorEditFlags) && p_sendChanges)
        {
            atmScatteringDataChanged = true;
        }

        // Draw EARTH CENTER
        drawLeftAlignedLabelText("Earth center:", inputWidgetOffset);
        if(ImGui::DragFloat3("##AtmEarthCenterDrag", glm::value_ptr(p_sceneData.m_atmScatteringData.m_planetCenterPosition), Config::GUIVar().editor_float_slider_speed) && p_sendChanges)
        {
            atmScatteringDataChanged = true;
        }

        ImGui::SeparatorText("Sun:");

        // Draw SUN SIZE
        drawLeftAlignedLabelText("Sun size:", inputWidgetOffset);
        if(ImGui::DragFloat("##AtmSunSizeDrag", &p_sceneData.m_atmScatteringData.m_sunSize, 0.00001f, 0.0f, 10.0f, "%.5f") && p_sendChanges)
        {
            atmScatteringDataChanged = true;
        }
        captureMouseWhileItemActive();

        if(atmScatteringDataChanged)
        {
            m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_AtmScatteringData, (void *)&p_sceneData.m_atmScatteringData, false);
            p_sceneData.m_modified = true;
        }

        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize
    }
    ImGui::EndChild();

    // Calculate cascaded shadow mapping window height
    float shadowMappingWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y);
    shadowMappingWindowHeight *= p_sceneData.m_shadowMappingData.m_shadowMappingEnabled ? 11 + 3 * p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.size() : 2.0f;
    shadowMappingWindowHeight += m_imguiStyle.ItemSpacing.y;

    if(ImGui::BeginChild("##CascadedShadowMappingSettings", ImVec2(0.0f, shadowMappingWindowHeight), true))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
        ImGui::SeparatorText("Cascaded shadow mapping:");
        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

        bool csmDataChanged = false;

        // Draw ENABLED
        drawLeftAlignedLabelText("Enabled:", inputWidgetOffset);
        if(ImGui::Checkbox("##CSMEnabledCheckbox", &p_sceneData.m_shadowMappingData.m_shadowMappingEnabled) && p_sendChanges)
        {
            csmDataChanged = true;
        }

        // Show the rest of the shadow mapping settings only of the shadow mapping is enabled
        if(p_sceneData.m_shadowMappingData.m_shadowMappingEnabled)
        {
            // Draw DEPTH MAP Z-CLIPPING
            drawLeftAlignedLabelText("Depth map Z-clipping:", inputWidgetOffset);
            if(ImGui::Checkbox("##CSMZClippingCheckbox", &p_sceneData.m_shadowMappingData.m_zClipping) && p_sendChanges)
            {
                csmDataChanged = true;
            }

            // Draw BIAS SCALE
            drawLeftAlignedLabelText("Bias scale:", inputWidgetOffset);
            if(ImGui::DragFloat("##CSMBiasScaleDrag", &p_sceneData.m_shadowMappingData.m_csmBiasScale, 0.00001f, 0.00001f, 10.0f, "%.5f") && p_sendChanges)
            {
                csmDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw PENUMBRA SIZE
            drawLeftAlignedLabelText("Penumbra size:", inputWidgetOffset);
            if(ImGui::DragFloat("##CSMPenumbraSizeDrag", &p_sceneData.m_shadowMappingData.m_penumbraSize, 0.01f, 0.01f, 10.0f, "%.3f") && p_sendChanges)
            {
                csmDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw PENUMBRA SCALE RANGE
            drawLeftAlignedLabelText("Penumbra scale range:", inputWidgetOffset);
            if(ImGui::DragFloat2("##CSMPenumbraScaleRangeDrag", glm::value_ptr(p_sceneData.m_shadowMappingData.m_penumbraScaleRange), 1.0f, 1.0f, 10000.0f) && p_sendChanges)
            {
                csmDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw RESOLUTION
            drawLeftAlignedLabelText("Resolution:", inputWidgetOffset);
            if(int resolution = (int)p_sceneData.m_shadowMappingData.m_csmResolution; ImGui::DragInt("##CSMResolutionDrag", &resolution, 64.0f, 64, 10240))
            {
                p_sceneData.m_shadowMappingData.m_csmResolution = (unsigned int)resolution;

                if(p_sendChanges)
                    csmDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw Z-PLANE MULTIPLIER
            drawLeftAlignedLabelText("Z-plane multiplier:", inputWidgetOffset);
            if(ImGui::DragFloat("##CSMZPlaneMultiplierDrag", &p_sceneData.m_shadowMappingData.m_csmCascadePlaneZMultiplier, 0.1f, 1.0f, 10000.0f, "%.1f") && p_sendChanges)
            {
                csmDataChanged = true;
            }
            captureMouseWhileItemActive();

            // Draw PCF SAMPLES
            drawLeftAlignedLabelText("PCF samples:", inputWidgetOffset);
            if(int numOfPCFSamples = (int)p_sceneData.m_shadowMappingData.m_numOfPCFSamples; ImGui::InputInt("##CSMPCFSamplesInputInt", &numOfPCFSamples) && p_sendChanges)
            {
                if(numOfPCFSamples > 0)
                {
                    p_sceneData.m_shadowMappingData.m_numOfPCFSamples = (unsigned int)numOfPCFSamples;
                    csmDataChanged = true;
                }
            }

            // Calculate CSM cascades window height
            float cascadesWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y);
            cascadesWindowHeight *= 2 + 3 * p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.size();

            if(ImGui::BeginChild("##CSMCascades", ImVec2(0, cascadesWindowHeight), true))
            {
                // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                float inputWidgetOffsetCascades = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
                ImGui::SeparatorText("CSM cascades:");
                ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

                for(decltype(p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.size()) size = p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.size(), i = 0; i < size; i++)
                {
                    // Calculate item width to fit both DISTANCE and DISTANCE TYPE items on the same line
                    auto itemWidth = (ImGui::GetContentRegionAvail().x - inputWidgetOffset) / 2.0f;

                    // Draw CASCADE DISTANCE
                    drawLeftAlignedLabelText((Utilities::toString(i + 1) + ". Cascade distance:").c_str(), inputWidgetOffset, itemWidth);
                    if(ImGui::DragFloat(("##CascadesDistanceDrag" + Utilities::toString(i)).c_str(), &p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_cascadeFarDistance, 1.0f, 0.0f, 10000.0f, "%.1f") && p_sendChanges)
                    {
                        csmDataChanged = true;
                    }

                    // Draw CASCADE DISTANCE UNIT TYPE
                    int distanceType = p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_distanceIsDivider ? 1 : 0;
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(itemWidth - calcTextSizedButtonSize(1) - m_imguiStyle.FramePadding.x * 4);
                    if(ImGui::Combo(("##CascadesDistanceTypeCombo" + Utilities::toString(i)).c_str(), &distanceType, &m_cascadeDistanceTypeText[0], (int)m_cascadeDistanceTypeText.size()))
                    {
                        p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_distanceIsDivider = (distanceType == 1);
                        csmDataChanged = true;
                    }

                    // Draw DELETE button
                    ImGui::SameLine(calcTextSizedButtonOffset(1));
                    if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##CSMCascadeDeleteButton" + Utilities::toString(i), "Remove shadow cascade entry"))
                    {
                        p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.erase(p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.begin() + i);
                        size = p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.size();
                        i--;

                        if(p_sendChanges)
                            csmDataChanged = true;
                    }

                    // Draw ADD button
                    ImGui::SameLine(calcTextSizedButtonOffset(0));
                    if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "CSMCascadeAddButton" + Utilities::toString(i), "Add a shadow cascade entry"))
                    {
                        p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.insert(p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.begin() + i + 1, ShadowCascadeData());
                        size = p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.size();

                        if(p_sendChanges)
                            csmDataChanged = true;
                    }

                    // Draw BIAX MAX
                    drawLeftAlignedLabelText("   Bias max:", inputWidgetOffset, itemWidth);
                    if(ImGui::DragFloat(("##CascadesBiasMaxDrag" + Utilities::toString(i)).c_str(), &p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_maxBias, 0.000001f, 0.0f, 1.0f, "%.5f") && p_sendChanges)
                    {
                        csmDataChanged = true;
                    }

                    // Draw PENUMBRA SCALE
                    drawLeftAlignedLabelText("   Penumbra scale:", inputWidgetOffset, itemWidth);
                    if(ImGui::DragFloat(("##CascadesPenumbraScaleDrag" + Utilities::toString(i)).c_str(), &p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances[i].m_penumbraScale, 0.001f, 0.0001f, 1000.0f, "%.3f") && p_sendChanges)
                    {
                        csmDataChanged = true;
                    }
                }

                // Draw ADD button
                ImGui::SetCursorPosX(calcTextSizedButtonOffset(0));
                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##CSMCascadeAddAtEndButton", "Add a shadow cascade entry"))
                {
                    p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances.push_back(ShadowCascadeData());

                    if(p_sendChanges)
                        csmDataChanged = true;
                }
            }
            ImGui::EndChild();
        }

        // If CSM data was changed, send the new data to the renderer and mark the scene data as modified
        if(csmDataChanged)
        {
            m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_ShadowMappingData, (void *)&p_sceneData.m_shadowMappingData, false);
            p_sceneData.m_modified = true;
        }
    }
    ImGui::EndChild();

    // Calculate rendering passes window height and cap it to a max height value
    float renderPassWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * (p_sceneData.m_renderingPasses.size() + 2);
    //renderPassWindowHeight = renderPassWindowHeight > Config::GUIVar().editor_render_pass_max_height ? Config::GUIVar().editor_render_pass_max_height : renderPassWindowHeight;

    if(ImGui::BeginChild("##RenderingPasses", ImVec2(0.0f, renderPassWindowHeight), true))//, ImVec2(0, childWindowHeight), true, ImGuiWindowFlags_None)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
        ImGui::SeparatorText("Rendering passes:");
        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

        bool renderingPassesModified = false;

        for(decltype(p_sceneData.m_renderingPasses.size()) size = p_sceneData.m_renderingPasses.size(), i = 0; i < size; i++)
        {
            // Draw RENDERING PASS COMBO
            int renderType = (int)p_sceneData.m_renderingPasses[i];		
            ImGui::AlignTextToFramePadding();
            ImGui::Text((Utilities::toString(i + 1) + ". Render Pass type:").c_str());

            // Draw UP button
            ImGui::SameLine();
            ImGui::SetCursorPosX(inputWidgetOffset);
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_ArrowUp], "##" + Utilities::toString(i) + "RenderingPassUpButton", "Move up"))
            {
                if(i > 0)
                {
                    auto tempRenderType = p_sceneData.m_renderingPasses[i];
                    p_sceneData.m_renderingPasses[i] = p_sceneData.m_renderingPasses[i - 1];
                    p_sceneData.m_renderingPasses[i - 1] = tempRenderType;
                    renderingPassesModified = true;
                }
            }

            // Draw DOWN button
            ImGui::SameLine();
            ImGui::SetCursorPosX(inputWidgetOffset + m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 3);
            if(drawTextSizedButtonInverted(m_buttonTextures[ButtonTextureType::ButtonTextureType_ArrowUp], "##" + Utilities::toString(i) + "RenderingPassDownButton", "Move down"))
            {
                if(i < size - 1)
                {
                    auto tempRenderType = p_sceneData.m_renderingPasses[i];
                    p_sceneData.m_renderingPasses[i] = p_sceneData.m_renderingPasses[i + 1];
                    p_sceneData.m_renderingPasses[i + 1] = tempRenderType;
                    renderingPassesModified = true;
                }
            }

            // Draw RENDER PASSES
            ImGui::SameLine();
            ImGui::SetCursorPosX(inputWidgetOffset + (m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 3) * 2);
            ImGui::SetNextItemWidth(calcTextSizedButtonOffset(1) - m_imguiStyle.FramePadding.x * 2);
            if(ImGui::Combo(("##RenderingPassCombo" + Utilities::toString(i)).c_str(), &renderType, &m_renderingPassesTypeText[0], (int)m_renderingPassesTypeText.size()))
            {
                p_sceneData.m_renderingPasses[i] = static_cast<RenderPassType>(renderType);
                renderingPassesModified = true;
            }

            // Draw DELETE button
            ImGui::SameLine(calcTextSizedButtonOffset(1));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##" + Utilities::toString(i) + "RenderingPassDeleteButton", "Remove this Render Pass entry"))
            {
                p_sceneData.m_renderingPasses.erase(p_sceneData.m_renderingPasses.begin() + i);
                size = p_sceneData.m_renderingPasses.size();
                i--;
                renderingPassesModified = true;
            }

            // Draw ADD button
            ImGui::SameLine(calcTextSizedButtonOffset(0));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##" + Utilities::toString(i) + "RenderingPassAddButton", "Add a new Render Pass entry"))
            {
                p_sceneData.m_renderingPasses.insert(p_sceneData.m_renderingPasses.begin() + i + 1, RenderPassType::RenderPassType_AtmScattering);
                size = p_sceneData.m_renderingPasses.size();
                renderingPassesModified = true;
            }
        }

        // Draw ADD button
        ImGui::SetCursorPosX(calcTextSizedButtonOffset(0));
        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##RenderingPassAddAtEndButton", "Add a new Render Pass entry"))
        {
            p_sceneData.m_renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
            renderingPassesModified = true;
        }

        if(renderingPassesModified)
        {
            if(p_sendChanges)
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderingPasses, (void *)&p_sceneData.m_renderingPasses, false);
            p_sceneData.m_modified = true;
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextAlign
    ImGui::PopStyleColor(); // ImGuiCol_Border
}

void EditorWindow::drawEntityHierarchy(EntityHierarchyEntry *p_rootEntry)
{
    // Set the indent spacing to a lower value, so more entries can be fit horizontally
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

    // Make button background transparent
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Draw every entry from the hierarchy list
    if(m_rootEntityHierarchyEntry.m_entityID != NULL_ENTITY_ID)
        drawEntityHierarchyEntry(&m_rootEntityHierarchyEntry);
    else
        ImGui::Text("No root entity");

    ImGui::PopStyleColor(); // ImGuiCol_Button
    ImGui::PopStyleVar(); // ImGuiStyleVar_IndentSpacing
}

void EditorWindow::drawEntityHierarchyEntry(EntityHierarchyEntry *p_entityEntry)
{
    const ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGuiTreeNodeFlags flags = p_entityEntry->m_children.empty() ? baseNodeFlags | ImGuiTreeNodeFlags_Leaf : baseNodeFlags;

    // Draw the root node opened
    if(p_entityEntry->m_entityID == 0)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

    // Calculate the offset for the collapsing header that is drawn after the delete component button of each component type
    const float headerOffsetAfterDeleteButton = m_buttonSizedByFont.x * 2.0f + m_imguiStyle.FramePadding.x * 6.0f;

    ImGui::SetNextItemAllowOverlap();
    if(ImGui::TreeNodeEx(p_entityEntry->m_combinedEntityIdAndName.c_str(), 
        m_selectedEntity.m_entityID == p_entityEntry->m_entityID ? flags | ImGuiTreeNodeFlags_Selected : flags))
    {
        // Process mouse clicks
        //if(ImGui::IsItemHovered())
        {
            // Left-mouse-click to select an entity
            if(ImGui::IsItemClicked(ImGuiMouseButton_Left))
                m_selectedEntity.setEntity(p_entityEntry->m_entityID);

            // Right-mouse-click to open the options menu
            if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                m_openEntityRightClickOptionsPopup = true;
                m_selectedEntity.setEntity(p_entityEntry->m_entityID);
            }
        }

        // Remove frame padding to make the buttons smaller
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

        // Draw DUPLICATE button
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - m_buttonSizedByFont.x * 3.0f - m_imguiStyle.ItemInnerSpacing.x * 2.0f);
        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Duplicate], ("##" + Utilities::toString(p_entityEntry->m_entityID) + "EntityDuplicateButton").c_str(), ("Duplicate \"" + p_entityEntry->m_combinedEntityIdAndName + "\" entity").c_str()))
        {
            duplicateEntity(p_entityEntry->m_entityID);
        }

        // Draw ENTITY ADD CHILD button
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - m_buttonSizedByFont.x * 2.0f - m_imguiStyle.ItemInnerSpacing.x);
        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], ("##" + Utilities::toString(p_entityEntry->m_entityID) + "EntityAddChildButton").c_str(), ("Add child entity to \"" + p_entityEntry->m_combinedEntityIdAndName + "\"").c_str()))
        {
            if(m_newEntityConstructionInfo == nullptr)
            {
                m_newEntityConstructionInfo = new ComponentsConstructionInfo();
                m_newEntityConstructionInfo->m_name = "New entity";
                m_newEntityConstructionInfo->m_id = 0;
                m_newEntityConstructionInfo->m_parent = p_entityEntry->m_entityID;

                // Reset the duplicate parent flag for new entity
                m_duplicateParent = false;

                // Open the pop-up with the new entity settings
                m_openNewEntityPopup = true;
            }
        }

        // Do not allow the deletion of root entity
        if(p_entityEntry->m_entityID != 0)
        {
            // Draw ENTITY DELETE button
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - m_buttonSizedByFont.x);
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], ("##" + Utilities::toString(p_entityEntry->m_entityID) + "EntityDeleteButton").c_str(), ("Delete \"" + p_entityEntry->m_combinedEntityIdAndName + "\" entity").c_str()))
            {
                deleteEntityAndChildren(p_entityEntry->m_entityID);
                m_selectedEntity.unselect();
            }

        }

        ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding

        for(decltype(p_entityEntry->m_children.size()) size = p_entityEntry->m_children.size(), i = 0; i < size; i++)
        {
            drawEntityHierarchyEntry(p_entityEntry->m_children[i]);
        }

        ImGui::TreePop();
    }
    else
    {
        // Process mouse clicks even if the tree node is closed
        if(ImGui::IsItemHovered())
        {
            // Left-mouse-click to select an entity
            if(ImGui::IsItemClicked())// && !ImGui::IsItemToggledOpen())
                m_selectedEntity.setEntity(p_entityEntry->m_entityID);

            // Right-mouse-click to open the options menu
            if(ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                m_openEntityRightClickOptionsPopup = true;
                m_selectedEntity.setEntity(p_entityEntry->m_entityID);
            }
        }
    }
}

void EditorWindow::captureMouseWhileItemActive()
{
    // Get current item ID
    ImGuiID itemID = ImGui::GetItemID();

    // Check if the current item is active and it is not in text input mode
    if(ImGui::IsItemActive() && !ImGui::TempInputIsActive(itemID))
    {
        // If the same item continue being active, don't let the mouse leave the screen
        if(m_activeItemID == itemID)
        {
            const auto currentMousePosition = WindowLocator::get().getMousePosition();
            const auto currentScreenSize = WindowLocator::get().getScreenSize() - 2;

            // If the mouse is near the edge of the screen, teleport it back to the starting position
            if(currentMousePosition.x >= currentScreenSize.x || currentMousePosition.x <= 2 ||
                currentMousePosition.y >= currentScreenSize.y || currentMousePosition.y <= 2)
            {
                ImGuiContext &imGuiContext = *ImGui::GetCurrentContext();

                // Teleport mouse
                WindowLocator::get().setMousePosition(m_mousePositionBeforeCapture.x, m_mousePositionBeforeCapture.y);

                // Make sure to not count the mouse teleportation as a mouse movement
                imGuiContext.IO.MousePos = imGuiContext.IO.MousePosPrev = ImVec2((float)m_mousePositionBeforeCapture.x, (float)m_mousePositionBeforeCapture.y);
                imGuiContext.IO.MouseDelta = ImVec2(0.0f, 0.0f);
            }

            // Hide the mouse cursor
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }
        else
        {
            // If a new item became active, remember its ID, current mouse position and start confining mouse to the screen
            m_activeItemID = itemID;
            m_mousePositionBeforeCapture = WindowLocator::get().getMousePosition();

            // Confine mouse to the window
            WindowLocator::get().setMouseGrab(true);
        }
    }
    else
    {
        // If the current items was activated before and just became deactivated,
        // return mouse settings to normal
        if(m_activeItemID == itemID)
        {
            // Reset active item ID
            m_activeItemID = -1;

            // Stop confining mouse to the window
            WindowLocator::get().setMouseGrab(false);

            // Return the mouse to the starting position
            WindowLocator::get().setMousePosition(m_mousePositionBeforeCapture.x, m_mousePositionBeforeCapture.y);
        }
    }
}

void EditorWindow::exportPrefab(const EntityID p_entityID, std::string p_filename)
{
    WorldScene *worldScene = static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World));
    auto &entityRegistry = worldScene->getEntityRegistry();

    // Check if entity exists
    if(entityRegistry.try_get<MetadataComponent>(p_entityID) != nullptr)
    {
        m_systemScene->getSceneLoader()->exportPrefab(p_entityID, p_filename);
    }
}

void EditorWindow::saveTextFile(TextEditorData &p_textEditorData)
{
    // Get the text
   // std::string textFromEditor = p_textEditorData.m_textEditor.GetText();

    // Erase the NULL characters that are left because the text editor works with chars internally
    //textFromEditor.erase(std::find(textFromEditor.begin(), textFromEditor.end(), '\0'), textFromEditor.end());

    // Write text to file
    Filesystem::writeTextToFile(p_textEditorData.m_filePath + p_textEditorData.m_filename, p_textEditorData.m_textEditor.GetText());

    // Reset the text changed flag
    p_textEditorData.m_textEditor.ResetTextChanged();
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
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)true);

                // Tell the Scripting scene to enable LUA components
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)true);
            }
            break;
        case EditorWindow::MainMenuButtonType_Pause:
            {
                m_sceneState = EditorSceneState::EditorSceneState_Pause;

                // Tell the Physics scene to pause the simulation
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)false);

                // Tell the Scripting scene to either enable or disable LUA components
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)m_LUAScriptingEnabled);
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
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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
                            m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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
            m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTexture, (void *)!m_enlargedSceneViewport);
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

void EditorWindow::updateSceneData(SceneData &p_sceneData)
{
    // Get the required system scenes
    const auto *audioScene = static_cast<AudioScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio));
    const auto *graphicsScene = static_cast<RendererScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics));
    const auto *physicsScene = static_cast<PhysicsScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics));

    // Set scene filename
    p_sceneData.m_sceneFilename = Utilities::stripFilename(m_systemScene->getSceneLoader()->getSceneFilename());

    // Set load-in-background flag
    p_sceneData.m_loadInBackground = m_systemScene->getSceneLoader()->getLoadInBackground();

    // Add audio bank filenames
    p_sceneData.m_audioBanks.clear();
    const auto &bankFilenames = audioScene->getBankFilenames();
    for(const auto &bank : bankFilenames)
        p_sceneData.m_audioBanks.push_back(bank.first);

    // Set audio volume
    for(unsigned int i = 0; i < AudioBusType::AudioBusType_NumOfTypes; i++)
        p_sceneData.m_volume[i] = audioScene->getVolume(static_cast<AudioBusType>(i));

    // Set graphics data
    p_sceneData.m_activeCameraID = graphicsScene->getSceneObjects().m_activeCameraID;
    p_sceneData.m_aoData = graphicsScene->getAmbientOcclusionData();
    p_sceneData.m_atmScatteringData = graphicsScene->getAtmScatteringData();
    p_sceneData.m_miscSceneData = graphicsScene->getMiscSceneData();
    p_sceneData.m_shadowMappingData = graphicsScene->getShadowMappingData();

    // Add rendering passes
    p_sceneData.m_renderingPasses.clear();
    p_sceneData.m_renderingPasses = graphicsScene->getRenderingPasses();

    // Set gravity
    p_sceneData.m_gravity = physicsScene->getGravity();
}

void EditorWindow::updateEntityList()
{
    // Make sure to clear old entity list entries
    m_entityList.clear();

    // Get the entity registry from World Scene
    auto &entityRegistry = static_cast<WorldScene*>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World))->getEntityRegistry();

    // Iterate every entity and add its entity ID and name to the list
    for(auto entity : entityRegistry.view<EntityID>())
    {
        // Try to get the metadata component
        auto metadataComponent = entityRegistry.try_get<MetadataComponent>(entity);

        // If the metadata component is present, add it to the list
        if(metadataComponent != nullptr)
        {
            std::string entityIdPlusName = metadataComponent->getName() + " (" + Utilities::toString(entity) + ")";
            m_entityList.emplace_back(entity, metadataComponent->getParentEntityID(), metadataComponent->getName(), entityIdPlusName);
        }
    }

    // Sort the list based on entity name, so they are shown more conveniently
    std::sort(m_entityList.begin(), m_entityList.end(),
        [](const EntityListEntry &p_a, const EntityListEntry &p_b) -> bool { return p_a.m_name < p_b.m_name; });
}

void EditorWindow::updateHierarchyList()
{
    // Clear the old hierarchy list
    m_rootEntityHierarchyEntry.clear();

    // Define a root entity flag and a temp entity list
    bool rootEntryPresent = false;
    std::list<EntityListEntry*> parentlessEntityList;

    // Find the root entity and add all non-root entities to the temp entity list
    for(decltype(m_entityList.size()) size = m_entityList.size(), i = 0; i < size; i++)
    {
        if(m_entityList[i].m_entityID == 0 && m_entityList[i].m_parentEntityID == 0)
        {
            m_rootEntityHierarchyEntry.init(m_entityList[i].m_entityID, m_entityList[i].m_parentEntityID, m_entityList[i].m_name, m_entityList[i].m_combinedEntityIdAndName, m_entityList[i].m_componentFlag);
            rootEntryPresent = true;
        }
        else
        {
            parentlessEntityList.push_back(&m_entityList[i]);
        }
    }

    // Check if the root entity is present
    if(rootEntryPresent)
    {
        // Define a list that hold all the children that have been added during the last loop
        // and add the root entity to it
        std::vector<EntityHierarchyEntry*> newlyAddedChildren;
        newlyAddedChildren.push_back(&m_rootEntityHierarchyEntry);

        // Continue the loop until there are no newly added children
        while(!newlyAddedChildren.empty())
        {
            // Save a copy of the children list that have been generated during the last loop
            // and clear the children list for the current loop
            std::vector<EntityHierarchyEntry *> newChildrenFromPreviousRun = newlyAddedChildren;
            newlyAddedChildren.clear();

            // Go over each new children from the previous loop
            for(decltype(newChildrenFromPreviousRun.size()) childrenSize = newChildrenFromPreviousRun.size(), childrenIndex = 0; childrenIndex < childrenSize; childrenIndex++)
            {
                // Go over each entity left, that haven't been added as children
                auto &parentlessEntity = parentlessEntityList.begin();
                while(parentlessEntity != parentlessEntityList.end())
                {
                    // If the entity ID of the current child and the parent entity ID of the parent-less entity matches, add the parent-less entity as a child
                    // if it doesn't match, continue to the next entity
                    if(newChildrenFromPreviousRun[childrenIndex]->m_entityID == (*parentlessEntity)->m_parentEntityID)
                    {
                        // Add the parent-less entity as a child
                        newChildrenFromPreviousRun[childrenIndex]->addChild((*parentlessEntity)->m_entityID, (*parentlessEntity)->m_parentEntityID, (*parentlessEntity)->m_name, (*parentlessEntity)->m_combinedEntityIdAndName, (*parentlessEntity)->m_componentFlag);

                        // Add the newly created child to the child list for the next loop
                        newlyAddedChildren.push_back(newChildrenFromPreviousRun[childrenIndex]->m_children.back());

                        // Remove the former parent-less entity, as it has been added as a child
                        parentlessEntityList.erase(parentlessEntity++);
                    }
                    else
                        parentlessEntity++;
                }
            }
        }

        // Check if all the entities (except root entity) have been added as children
        if(!parentlessEntityList.empty())
        {
            std::cout << "CONTAINS PARENTLESS CHILDREN:" << std::endl;

            for(auto const &i : parentlessEntityList)
            {
                std::cout << i->m_entityID << std::endl;
            }
        }
    }
    else
    {
        m_rootEntityHierarchyEntry.m_entityID = NULL_ENTITY_ID;
        m_rootEntityHierarchyEntry.m_parent = NULL_ENTITY_ID;
    }

    return;
}

void EditorWindow::updateComponentList()
{
    // Clear the old component list
    m_componentList.clear();

    // Get the entity registry from World Scene
    auto &entityRegistry = static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World))->getEntityRegistry();

    for(decltype(m_entityList.size()) size = m_entityList.size(), i = 0; i < size; i++)
    {
        // AUDIO components
        auto soundComp = entityRegistry.try_get<SoundComponent>(m_entityList[i].m_entityID);
        if(soundComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, soundComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + soundComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::AudioSoundComponent;
        }
        auto soundListenerComp = entityRegistry.try_get<SoundListenerComponent>(m_entityList[i].m_entityID);
        if(soundListenerComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, soundListenerComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + soundListenerComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::AudioSoundListenerComponent;
        }

        // GUI components
        auto guiSequenceComp = entityRegistry.try_get<GUISequenceComponent>(m_entityList[i].m_entityID);
        if(guiSequenceComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, guiSequenceComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + guiSequenceComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::GUISequenceComponent;
        }
        
        // GRAPHICS components
        auto cameraComp = entityRegistry.try_get<CameraComponent>(m_entityList[i].m_entityID);
        if(cameraComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, cameraComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + cameraComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::GraphicsCameraComponent;
        }
        auto lightComp = entityRegistry.try_get<LightComponent>(m_entityList[i].m_entityID);
        if(lightComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, lightComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + lightComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::GraphicsLightingComponent;
        }
        auto modelComp = entityRegistry.try_get<ModelComponent>(m_entityList[i].m_entityID);
        if(modelComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, modelComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + modelComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::GraphicsModelComponent;
        }
        auto shaderComp = entityRegistry.try_get<ShaderComponent>(m_entityList[i].m_entityID);
        if(shaderComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, shaderComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + shaderComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::GraphicsShaderComponent;
        }
        
        // PHYSICS components
        auto collisionShapeComp = entityRegistry.try_get<CollisionShapeComponent>(m_entityList[i].m_entityID);
        if(collisionShapeComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, collisionShapeComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + collisionShapeComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::PhysicsCollisionShapeComponent;
        }
        auto rigidBodyComp = entityRegistry.try_get<RigidBodyComponent>(m_entityList[i].m_entityID);
        if(rigidBodyComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, rigidBodyComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + rigidBodyComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::PhysicsRigidBodyComponent;
        }
        
        // SCRIPTING components
        auto luaComp = entityRegistry.try_get<LuaComponent>(m_entityList[i].m_entityID);
        if(luaComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, luaComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + luaComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::ScriptingLuaComponent;
        }
        
        // WORLD components
        auto objectMaterialComp = entityRegistry.try_get<ObjectMaterialComponent>(m_entityList[i].m_entityID);
        if(objectMaterialComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, objectMaterialComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + objectMaterialComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::WorldObjectMaterialComponent;
        }
        auto spatialComp = entityRegistry.try_get<SpatialComponent>(m_entityList[i].m_entityID);
        if(spatialComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, spatialComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + spatialComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::WorldSpatialComponent;
        }
        auto metadataComp = entityRegistry.try_get<MetadataComponent>(m_entityList[i].m_entityID);
        if(metadataComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, metadataComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + metadataComp->getName() + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::MetadataComponent));
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::WorldMetadataComponent;
        }
    }
}

void EditorWindow::updateAssetLists()
{
    // Get the entity registry from World Scene
    auto &entityRegistry = static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World))->getEntityRegistry();

    //	 ____________________________
    //	|							 |
    //	|	   TEXTURE ASSETS        |
    //	|____________________________|
    //
    // Clear texture asset array
    m_textureAssets.clear();

    // Go over each texture in the loaders texture pool
    const auto &texturePool = Loaders::texture2D().getObjectPool();
    for(decltype(texturePool.size()) i = 0, size = texturePool.size(); i < size; i++)
    {
        // Add the texture and name entry
        m_textureAssets.push_back(std::make_pair(texturePool[i], texturePool[i]->getFilename()));

        // Set the longest texture name from all loaded texture assets, required for setting popup sizes when showing the list of textures
        if(m_textureAssetLongestName.size() < texturePool[i]->getFilename().size())
            m_textureAssetLongestName = texturePool[i]->getFilename();
    }

    // Sort the texture list based on the texture name
    std::stable_sort(m_textureAssets.begin(), m_textureAssets.end(),
        [](const std::pair<const Texture2D *, std::string> &p_a, const std::pair<const Texture2D *, std::string> &p_b) -> bool { return p_a.second < p_b.second; });


    //	 ____________________________
    //	|							 |
    //	|	    MODEL ASSETS         |
    //	|____________________________|
    //
    // Clear model asset array
    m_modelAssets.clear();

    // Go over each model in the loaders model pool
    const auto &modelPool = Loaders::model().getObjectPool();
    for(decltype(modelPool.size()) i = 0, size = modelPool.size(); i < size; i++)
    {
        // Add the model and name entry
        m_modelAssets.push_back(std::make_pair(modelPool[i], modelPool[i]->getFilename()));

        // Set the longest model name from all loaded model assets, required for setting popup sizes when showing the list of models
        if(m_modelAssetLongestName.size() < modelPool[i]->getFilename().size())
            m_modelAssetLongestName = modelPool[i]->getFilename();
    }

    // Sort the model list based on the model name
    std::stable_sort(m_modelAssets.begin(), m_modelAssets.end(),
        [](const std::pair<const Model *, std::string> &p_a, const std::pair<const Model *, std::string> &p_b) -> bool { return p_a.second < p_b.second; });


    //	 ____________________________
    //	|							 |
    //	|	    SHADER ASSETS        |
    //	|____________________________|
    //
    // Clear shader asset array
    m_shaderAssets.clear();

    // Go over each shader in the loaders shader pool
    const auto &shaderPool = Loaders::shader().getObjectPool();
    for(decltype(shaderPool.size()) i = 0, size = shaderPool.size(); i < size; i++)
    {
        // Get the shader name
        std::string shaderName = shaderPool[i]->getCombinedFilename();

        // If the name was auto generated, set the shader name based on the types of shader present
        if(shaderPool[i]->isNameAutoGenerated())
        {
            shaderName = shaderPool[i]->getShaderFilename(ShaderType::ShaderType_Fragment);
            if(shaderName.empty())
            {
                shaderName = shaderPool[i]->getShaderFilename(ShaderType::ShaderType_Vertex);
                if(shaderName.empty())
                {
                    shaderName = shaderPool[i]->getShaderFilename(ShaderType::ShaderType_Compute);
                    if(shaderName.empty())
                    {
                        shaderName = shaderPool[i]->getShaderFilename(ShaderType::ShaderType_Geometry);
                        if(shaderName.empty())
                        {
                            shaderName = shaderPool[i]->getShaderFilename(ShaderType::ShaderType_TessControl);
                            if(shaderName.empty())
                            {
                                shaderName = shaderPool[i]->getShaderFilename(ShaderType::ShaderType_TessEvaluation);
                            }
                        }
                    }
                }
            }
        }

        // Remove the extension from the shader filename
        shaderName = Utilities::removeExtension(shaderName);

        // Add the shader and name entry
        m_shaderAssets.push_back(std::make_pair(shaderPool[i], shaderName));

        // Set the longest shader name from all loaded shader assets, required for setting popup sizes when showing the list of shaders
        if(m_shaderAssetLongestName.size() < shaderName.size())
            m_shaderAssetLongestName = shaderName;
    }

    // Sort the shader list based on the shader name
    std::stable_sort(m_shaderAssets.begin(), m_shaderAssets.end(),
        [](const std::pair<ShaderLoader::ShaderProgram *, std::string> &p_a, const std::pair<ShaderLoader::ShaderProgram *, std::string> &p_b) -> bool { return p_a.second < p_b.second; });
    

    //	 ____________________________
    //	|							 |
    //	|	  LUA SCRIPT ASSETS      |
    //	|____________________________|
    //
    // Clear lua script asset array
    m_luaScriptAssets.clear();

    auto luaComponentView = entityRegistry.view<LuaComponent>();
    for(auto entity : luaComponentView)
    {
        auto &luaComponent = luaComponentView.get<LuaComponent>(entity);

        // Add the entity and lua script filename entry
        m_luaScriptAssets.push_back(std::make_pair(entity, Utilities::stripFilename(luaComponent.getLuaScript()->getLuaScriptFilename())));
    }

    // Sort the lua script list based on the lua filename
    std::stable_sort(m_luaScriptAssets.begin(), m_luaScriptAssets.end(),
        [](const std::pair<EntityID, std::string> &p_a, const std::pair<EntityID, std::string> &p_b) -> bool { return p_a.second < p_b.second; });
}

void EditorWindow::generateNewMap(PropertySet &p_newSceneProperties, SceneData &p_sceneData)
{
    // Add load in background flag
    p_newSceneProperties.addProperty(Properties::LoadInBackground, p_sceneData.m_loadInBackground);

    // Add root property set game objects
    auto &gameObjects = p_newSceneProperties.addPropertySet(Properties::GameObject);

    // Create an array entry for root entity
    auto &rootObjectEntry = gameObjects.addPropertySet(Properties::ArrayEntry);
    rootObjectEntry.addProperty(Properties::PropertyID::Name, std::string("root"));
    rootObjectEntry.addProperty(Properties::PropertyID::ID, 0);
    rootObjectEntry.addProperty(Properties::PropertyID::Parent, 0);

    // Create sun directional light
    auto &dirLightObjectEntry = gameObjects.addPropertySet(Properties::ArrayEntry);
    dirLightObjectEntry.addProperty(Properties::PropertyID::Name, std::string("Sun"));
    dirLightObjectEntry.addProperty(Properties::PropertyID::ID, 1);
    dirLightObjectEntry.addProperty(Properties::PropertyID::Parent, 0);
    dirLightObjectEntry.addPropertySet(Properties::PropertyID::World).addPropertySet(Properties::PropertyID::SpatialComponent);
    auto &lightComponentEntry = dirLightObjectEntry.addPropertySet(Properties::PropertyID::Graphics).addPropertySet(Properties::PropertyID::LightComponent);
    lightComponentEntry.addProperty(Properties::PropertyID::Type, Properties::PropertyID::DirectionalLight);
    lightComponentEntry.addProperty(Properties::PropertyID::Intensity, 10.0f);

    auto &editorCameraObjectEntry = gameObjects.addPropertySet(Properties::ArrayEntry);
    editorCameraObjectEntry.addProperty(Properties::PropertyID::Name, std::string("EditorCamera"));
    editorCameraObjectEntry.addProperty(Properties::PropertyID::ID, 2000000000);
    editorCameraObjectEntry.addProperty(Properties::PropertyID::Parent, 0);
    editorCameraObjectEntry.addPropertySet(Properties::PropertyID::Graphics).addPropertySet(Properties::PropertyID::CameraComponent);
    auto &luaComponentEntry = editorCameraObjectEntry.addPropertySet(Properties::PropertyID::Script).addPropertySet(Properties::PropertyID::LuaComponent);
    luaComponentEntry.addProperty(Properties::PropertyID::Filename, std::string("Camera_free.lua"));
    luaComponentEntry.addProperty(Properties::PropertyID::PauseInEditor, false);
    editorCameraObjectEntry.addPropertySet(Properties::PropertyID::World).addPropertySet(Properties::PropertyID::SpatialComponent);

    // Add root property set for systems
    auto &rootSystemsPropertySet = p_newSceneProperties.addPropertySet(Properties::Systems);

    // Add audio banks
    if(!p_sceneData.m_audioBanks.empty())
    {
        auto &audioBanksPropertySet = rootSystemsPropertySet.addPropertySet(Properties::Audio).addPropertySet(Properties::Scene).addPropertySet(Properties::Banks);
        for(auto &bankFilename : p_sceneData.m_audioBanks)
        {
            if(!bankFilename.empty())
                audioBanksPropertySet.addPropertySet(Properties::ArrayEntry).addProperty(Properties::PropertyID::Filename, bankFilename);
        }
    }

    // Add graphics settings
    auto &graphicsScenePropertySet = rootSystemsPropertySet.addPropertySet(Properties::PropertyID::Graphics).addPropertySet(Properties::PropertyID::Scene);
    graphicsScenePropertySet.addProperty(Properties::PropertyID::CameraID, p_sceneData.m_activeCameraID);
    graphicsScenePropertySet.addProperty(Properties::PropertyID::StochasticSamplingSeamFix, p_sceneData.m_miscSceneData.m_stochasticSamplingSeamFix);

    // Add ambient intensity
    auto &ambientPropertySet = graphicsScenePropertySet.addPropertySet(Properties::AmbientIntensity);
    ambientPropertySet.addProperty(Properties::DirectionalLight, p_sceneData.m_miscSceneData.m_ambientIntensityDirectional);
    ambientPropertySet.addProperty(Properties::PointLight, p_sceneData.m_miscSceneData.m_ambientIntensityPoint);
    ambientPropertySet.addProperty(Properties::SpotLight, p_sceneData.m_miscSceneData.m_ambientIntensitySpot);

    // Add ambient occlusion settings
    auto &aoPropertySet = graphicsScenePropertySet.addPropertySet(Properties::PropertyID::AmbientOcclusion);
    switch(p_sceneData.m_aoData.m_aoType)
    {
        case AmbientOcclusionType::AmbientOcclusionType_SSAO:
            aoPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::SSAO);
            break;
        case AmbientOcclusionType::AmbientOcclusionType_HBAO:
            aoPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::HBAO);
            break;
        case AmbientOcclusionType::AmbientOcclusionType_None:
            aoPropertySet.addProperty(Properties::PropertyID::Type, Properties::PropertyID::None);
        default:
            break;
    }
    aoPropertySet.addProperty(Properties::PropertyID::Bias, p_sceneData.m_aoData.m_aoBias);
    aoPropertySet.addProperty(Properties::PropertyID::Radius, p_sceneData.m_aoData.m_aoRadius);
    aoPropertySet.addProperty(Properties::PropertyID::Intensity, p_sceneData.m_aoData.m_aoIntensity);
    aoPropertySet.addProperty(Properties::PropertyID::Directions, p_sceneData.m_aoData.m_aoNumOfDirections);
    aoPropertySet.addProperty(Properties::PropertyID::Samples, p_sceneData.m_aoData.m_aoNumOfSamples);
    aoPropertySet.addProperty(Properties::PropertyID::Steps, p_sceneData.m_aoData.m_aoNumOfSteps);
    aoPropertySet.addProperty(Properties::PropertyID::BlurSamples, p_sceneData.m_aoData.m_aoBlurNumOfSamples);
    aoPropertySet.addProperty(Properties::PropertyID::BlurSharpness, p_sceneData.m_aoData.m_aoBlurSharpness);

    // Add rendering passes
    RendererScene::exportRenderingPasses(graphicsScenePropertySet, p_sceneData.m_renderingPasses);

    // Add shadow mapping settings
    auto &shadowMappingPropertySet = graphicsScenePropertySet.addPropertySet(Properties::PropertyID::ShadowMapping);
    shadowMappingPropertySet.addProperty(Properties::PropertyID::PenumbraSize, p_sceneData.m_shadowMappingData.m_penumbraSize);
    shadowMappingPropertySet.addProperty(Properties::PropertyID::PenumbraScaleRange, p_sceneData.m_shadowMappingData.m_penumbraScaleRange);
    shadowMappingPropertySet.addProperty(Properties::PropertyID::Resolution, (int)p_sceneData.m_shadowMappingData.m_csmResolution);
    shadowMappingPropertySet.addProperty(Properties::PropertyID::ZClipping, p_sceneData.m_shadowMappingData.m_zClipping);
    shadowMappingPropertySet.addProperty(Properties::PropertyID::ZPlaneMultiplier, p_sceneData.m_shadowMappingData.m_csmCascadePlaneZMultiplier);
    shadowMappingPropertySet.addPropertySet(Properties::PCF).addProperty(Properties::PropertyID::Samples, (int)p_sceneData.m_shadowMappingData.m_numOfPCFSamples);
    auto &cascadesPropertySet = shadowMappingPropertySet.addPropertySet(Properties::PropertyID::Cascades);
    for(auto &cascade : p_sceneData.m_shadowMappingData.m_shadowCascadePlaneDistances)
    {
        auto &singleCascadePropertySet = cascadesPropertySet.addPropertySet(Properties::PropertyID::ArrayEntry);
        if(cascade.m_distanceIsDivider)
            singleCascadePropertySet.addProperty(Properties::PropertyID::Divider, cascade.m_cascadeFarDistance);
        else
            singleCascadePropertySet.addProperty(Properties::PropertyID::Distance, cascade.m_cascadeFarDistance);
        singleCascadePropertySet.addProperty(Properties::PropertyID::BiasMax, cascade.m_maxBias);
        singleCascadePropertySet.addProperty(Properties::PropertyID::PenumbraScale, cascade.m_penumbraScale);
    }
}
