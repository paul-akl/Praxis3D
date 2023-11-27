#include <glm/gtc/type_ptr.hpp>
#include <ranges>

#include "EngineDefinitions.h"
#include "EditorWindow.h"
#include "imgui_internal.h"
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
}

ErrorCode EditorWindow::init()
{
    return ErrorCode::Success;
}

void EditorWindow::update(const float p_deltaTime)
{
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

    static float f = 0.0f;
    static int counter = 0;
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
        //colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
        //colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        //colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        //colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        //colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        //colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.122f, 0.122f, 0.122f, 1.0f);
        //colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        //colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        //colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        //colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        //colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        //colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        //colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        //colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.52f, 0.52f, 0.52f, 1.0f);
        //colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        //colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        //colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        //colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        //colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        //colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        //colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        //colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        //colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        //colors[ImGuiCol_Tab] = ImVec4(0.118f, 0.118f, 0.118f, 1.0f);
        colors[ImGuiCol_Tab] = ImVec4(0.122f, 0.122f, 0.122f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
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
    //m_imguiStyle = ImGui::GetStyle();
    //const float fontSize = ImGui::GetFontSize();
    //const ImVec2 openReloadButtonSize = ImVec2(fontSize, fontSize);
    ImVec2 mainMenuBarSize;
    ImGuiViewport *mainViewport = ImGui::GetMainViewport();

    //RendererScene *rendererScene = static_cast<RendererScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics));
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
        ImGui::BeginMainMenuBar();

        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New"))
            {
                // Set the new scene settings tab flag to be selected (bring to focus)
                m_newSceneSettingsTabFlags = ImGuiTabItemFlags_SetSelected;
                m_showNewMapWindow = true;
            }
            if(ImGui::MenuItem("Open...")) 
            {
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
            }
            ImGui::Separator();
            if(ImGui::MenuItem("Save")) 
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
            if(ImGui::MenuItem("Save as..."))
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
            if(ImGui::MenuItem("Reload scene")) 
            {
                // Send a notification to the engine to reload the current engine state
                m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneReload, EngineStateType::EngineStateType_Editor));
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Close editor"))
            {
                Config::m_engineVar.engineState = EngineStateType::EngineStateType_MainMenu;
            }

            if(ImGui::MenuItem("Exit"))
            {
                Config::m_engineVar.running = false;
            }

            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            if(ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if(ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if(ImGui::MenuItem("Cut", "CTRL+X")) {}
            if(ImGui::MenuItem("Copy", "CTRL+C")) {}
            if(ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }

        //ImGui::ArrowButton("##left", ImGuiDir_Left);

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

            // Draw ENABLE GUI SEQUENCE button
            if(ImGui::ImageButton("##GUISequenceButton",
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_GUISequence].getHandle(),
                m_playPauseButtonSize,
                ImVec2(0, 1),
                ImVec2(1, 0),
                m_GUISequenceEnabled ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
            {
                m_GUISequenceEnabled = !m_GUISequenceEnabled;

                // Tell the GUI scene to either enable or disable GUI Sequence components
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_EnableGUISequence, (void *)m_GUISequenceEnabled);
            }
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
                ImGui::SetTooltip(m_GUISequenceEnabled ? "Click to disable GUI Sequence components drawing on screen" : "Click to enable GUI Sequence components drawing on screen", ImGui::GetStyle().HoverDelayShort);
            ImGui::SameLine();

            // Draw ENABLE SCRIPTING button
            if(ImGui::ImageButton("##ScriptingEnableButton",
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_ScriptingEnable].getHandle(),
                m_playPauseButtonSize,
                ImVec2(0, 1),
                ImVec2(1, 0),
                m_LUAScriptingEnabled ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
            {
                m_LUAScriptingEnabled = !m_LUAScriptingEnabled;

                // Tell the Scripting scene to either enable or disable LUA components
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)m_LUAScriptingEnabled);
            }
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
                ImGui::SetTooltip(m_LUAScriptingEnabled ? "Click to disable LUA scripting components" : "Click to enable LUA scripting components", ImGui::GetStyle().HoverDelayShort);

            // Get the secondary menu bar style, required for retrieving size information
            ImGuiStyle &secondaryMenuBarStyle = ImGui::GetStyle();

            // Calculate the combined size of a single button (including the inner spacing)
            float playPauseButtonCombinedSize = m_playPauseButtonSize.x + secondaryMenuBarStyle.ItemInnerSpacing.x * 2.0f;

            // Calculate the offset of all buttons to the center
            float offsetToCenter = (playPauseButtonCombinedSize * 3) / 2;

            // Set the starting position of the buttons
            ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 - offsetToCenter);

            // Draw PLAY button
            ImGui::PushStyleColor(ImGuiCol_Button, m_sceneState == EditorSceneState::EditorSceneState_Play ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            if(ImGui::ImageButton("##PlayButton", 
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Play].getHandle(), 
                m_playPauseButtonSize,
                ImVec2(0, 1),
                ImVec2(1, 0),
                m_sceneState == EditorSceneState::EditorSceneState_Play ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
            {
                m_sceneState = EditorSceneState::EditorSceneState_Play;

                // Tell the Physics scene to run the simulation
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)true);
            }
            ImGui::PopStyleColor();

            // Draw PAUSE button
            ImGui::PushStyleColor(ImGuiCol_Button, m_sceneState == EditorSceneState::EditorSceneState_Pause ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::SameLine();
            if(ImGui::ImageButton("##PauseButton", 
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Pause].getHandle(), 
                m_playPauseButtonSize,
                ImVec2(0, 1),
                ImVec2(1, 0),
                m_sceneState == EditorSceneState::EditorSceneState_Pause ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
            {
                m_sceneState = EditorSceneState::EditorSceneState_Pause;

                // Tell the Physics scene to pause the simulation
                m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)false);
            }
            ImGui::PopStyleColor();

            // Draw RESTART button
            ImGui::SameLine();
            if(ImGui::ImageButton("##RestartButton", 
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Restart].getHandle(), 
                m_playPauseButtonSize,
                ImVec2(0, 1),
                ImVec2(1, 0)))
            {
                m_sceneState = EditorSceneState::EditorSceneState_Pause;
            }

            ImGui::PopStyleColor();

            ImGui::End();
        }
    }

    // Make the padding above tabs smaller
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 1));

    //	 ____________________________
    //	|							 |
    //	|	     LEFT WINDOW         |
    //	|____________________________|
    //
    {
        ImGui::SetNextWindowClass(&windowClassWithNoTabBar);

        ImGui::Begin("##LeftWindow", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        if(ImGui::BeginTabBar("##LeftWindowTabBar", ImGuiTabBarFlags_None))
        {
            if(ImGui::BeginTabItem("Hierarchy"))
            {
                // Set the indent spacing to a lower value, so more entries can be fit horizontally
                ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

                // Draw every entry from the hierarchy list
                for(decltype(m_entityHierarchy.size()) size = m_entityHierarchy.size(), i = 0; i < size; i++)
                {
                    drawEntityHierarchyEntry(m_entityHierarchy[i]);
                }

                // Pop indent spacing
                ImGui::PopStyleVar();

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

        ImGui::End();
    }

    //	 ____________________________
    //	|							 |
    //	|	     RIGHT WINDOW        |
    //	|____________________________|
    //
    {
        ImGui::SetNextWindowClass(&windowClassWithNoTabBar);

        ImGui::Begin("##RightWindow", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        if(ImGui::BeginTabBar("##RightWindowTabBar", ImGuiTabBarFlags_None))
        {
            if(ImGui::BeginTabItem("Inspector"))
            {
                if(m_selectedEntity)
                {
                    // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                    float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                    // WORLD COMPONENTS
                    auto *metadataComponent = entityRegistry.try_get<MetadataComponent>(m_selectedEntity.m_entityID);
                    if(metadataComponent != nullptr)
                    {
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::MetadataComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            // Get the current entity name
                            m_selectedEntity.m_componentData.m_name = metadataComponent->getName();

                            // Draw NAME
                            drawLeftAlignedLabelText("Name:", inputWidgetOffset);
                            if(ImGui::InputText("##NameStringInput", &m_selectedEntity.m_componentData.m_name, ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                // If the entity name was changed, send a notification to the Metadata Component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, metadataComponent, Systems::Changes::Generic::Name);
                            }
                        }
                    }

                    auto *spatialComponent = entityRegistry.try_get<SpatialComponent>(m_selectedEntity.m_entityID);
                    if(spatialComponent != nullptr)
                    {
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::SpatialComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            auto *rigidBodyComponent = entityRegistry.try_get<RigidBodyComponent>(m_selectedEntity.m_entityID);

                            // Get the current spatial data from the selected entity spatial component
                            m_selectedEntity.m_spatialDataManager = spatialComponent->getSpatialDataChangeManager();

                            // Draw POSITION
                            drawLeftAlignedLabelText("Position:", inputWidgetOffset);
                            if(ImGui::DragFloat3("##PositionDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_position), Config::GUIVar().editor_float_slider_speed))
                            {
                                // If the position vector was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                if(rigidBodyComponent != nullptr)
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::LocalPosition);
                                else
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalPosition);
                            }

                            // Draw ROTATION
                            // Make sure to get the current local rotation euler angles, as they are not automatically updated
                            m_selectedEntity.m_spatialDataManager.calculateLocalRotationEuler();
                            drawLeftAlignedLabelText("Rotation:", inputWidgetOffset);
                            if(ImGui::DragFloat3("##RotationDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_rotationEuler), Config::GUIVar().editor_float_slider_speed))
                            {
                                // If the rotation vector was changed, set the new rotation in the spatial data manager (so it can set the appropriate dirty flags internally)
                                m_selectedEntity.m_spatialDataManager.setLocalRotation(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_rotationEuler);
                                // Update the spatial data manager (so it updates the rotation quaternion internally)
                                m_selectedEntity.m_spatialDataManager.update();                             
                                
                                // If the rotation vector was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                if(rigidBodyComponent != nullptr)
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Spatial::LocalRotation);
                                else
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalRotation);
                            }

                            // Draw SCALE
                            drawLeftAlignedLabelText("Scale:", inputWidgetOffset);
                            if(ImGui::DragFloat3("##ScaleDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_scale), Config::GUIVar().editor_float_slider_speed, 0.01f, 10000.0f))
                            {
                                // If the scale vector was changed, send a notification to the spatial component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalScale);
                            }
                        }
                    }

                    auto *objectMaterialComponent = entityRegistry.try_get<ObjectMaterialComponent>(m_selectedEntity.m_entityID);
                    if(objectMaterialComponent != nullptr)
                    {
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::ObjectMaterialComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            // Get the current object material type from the selected entity Object Material Component
                            m_selectedEntity.m_objectMaterialType = objectMaterialComponent->getObjectMaterialType();
                            
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
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::CameraComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_active = cameraComponent->isObjectActive();

                            // Draw ACTIVE
                            drawLeftAlignedLabelText("Active:", inputWidgetOffset);
                            if(ImGui::Checkbox("##CameraActive", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_active))
                            {
                                // If the active flag was changed, send a notification to the Camera Component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, cameraComponent, Systems::Changes::Generic::Active);
                            }
                        }
                    }                    
                    auto *lightComponent = entityRegistry.try_get<LightComponent>(m_selectedEntity.m_entityID);
                    if(lightComponent != nullptr)
                    {
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::LightComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            const char *lightTypeStrings[] = { "null", "Directional", "Point", "Spot" };
                            m_selectedEntity.m_lightType = lightComponent->getLightType();

                            // Draw LIGHT TYPE
                            drawLeftAlignedLabelText("Light type:", inputWidgetOffset);
                            if(ImGui::Combo("##LightTypePicker", &m_selectedEntity.m_lightType, lightTypeStrings, 4))
                            {
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

                                        // Draw CUTOFF ANGLE
                                        drawLeftAlignedLabelText("Cutoff angle:", inputWidgetOffset);
                                        if(ImGui::DragFloat("##SpotLightCutoffAngleDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                        {
                                            // If the light cutoff angle was changed, send a notification to the Light Component
                                            m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, lightComponent, Systems::Changes::Graphics::CutoffAngle);
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }                    
                    auto *modelComponent = entityRegistry.try_get<ModelComponent>(m_selectedEntity.m_entityID);
                    if(modelComponent != nullptr)
                    {
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::ModelComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            bool modelComponentDataNeedsUpdating = false;

                            // If the model data was modified, send the new data to the ModelComponent
                            if(m_selectedEntity.m_modelDataModified)
                            {
                                m_systemScene->getSceneLoader()->getChangeController()->sendData(modelComponent, DataType::DataType_ModelsProperties, (void *)&m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties);
                                m_selectedEntity.m_modelDataModified = false;
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

                            // Go over each model
                            for(decltype(m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.size()) modelSize = m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models.size(), 
                                modelIndex = 0; modelIndex < modelSize; modelIndex++)
                            {
                                auto &modelEntry = m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties.m_models[modelIndex];

                                // Draw MODEL FILENAME
                                drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(2) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                                if(ImGui::InputText(("##" + Utilities::toString(modelIndex) + "ModelFileInput").c_str(), &modelEntry.m_modelName, ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                    // If the model filename was changed, set the modified flag
                                    m_selectedEntity.m_modelDataModified = true;
                                }

                                // Draw MODEL OPEN button
                                ImGui::SameLine(calcTextSizedButtonOffset(2));
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##ModelFileOpenButton", "Open a model file"))
                                {
                                    // Only open the file browser if it's not opened already
                                    if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                    {
                                        // Set the selected model filename handle
                                        m_selectedEntity.m_selectedModelName = &modelEntry.m_modelName;

                                        // Set the file browser activation to Model File
                                        m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_ModelFile;

                                        // Define file browser variables
                                        m_fileBrowserDialog.m_filter = "Model files (.obj .3ds .fbx){.obj,.3ds,.fbx},All files{.*}";
                                        m_fileBrowserDialog.m_title = "Open a model file";
                                        m_fileBrowserDialog.m_name = "OpenModelFileFileDialog";
                                        m_fileBrowserDialog.m_rootPath = Config::filepathVar().model_path;
                                        m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                                        // Tell the GUI scene to open the file browser
                                        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                    }
                                }

                                const std::string modelSelectionPopupName = "##" + Utilities::toString(modelIndex) + "ModelSelectionPopup";

                                // Draw OPEN ASSET LIST button
                                ImGui::SameLine(calcTextSizedButtonOffset(1));
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##" + Utilities::toString(modelIndex) + "ModelOpenAssetListButton", "Choose a model from the loaded assets"))
                                {
                                    // Open the pop-up with the model asset list
                                    ImGui::OpenPopup(modelSelectionPopupName.c_str());
                                }

                                // Draw MODEL RELOAD button
                                ImGui::SameLine(calcTextSizedButtonOffset(0));
                                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##" + Utilities::toString(modelIndex) + "ModelFileReloadButton", "Reload the model file"))
                                {
                                    // Set the modified flag
                                    m_selectedEntity.m_modelDataModified = true;
                                }

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

                                ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));
                                for(decltype(modelEntry.m_numOfMeshes) meshSize = modelEntry.m_numOfMeshes, meshIndex = 0; meshIndex < meshSize; meshIndex++)
                                {
                                    // Get the mesh name
                                    std::string meshName = modelEntry.m_meshNames[meshIndex];
                                    if(!meshName.empty())
                                        meshName = " (" + meshName + ")";

                                    // Draw MESH
                                    if(ImGui::TreeNodeEx(("Mesh " + Utilities::toString(meshIndex) + meshName + ":").c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) // ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf
                                    {
                                        ImGui::SeparatorText("Mesh settings:");

                                        // Draw HEIGHT SCALE
                                        drawLeftAlignedLabelText("Height scale:", inputWidgetOffset);
                                        if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "HeightScaleDrag").c_str(), &modelEntry.m_heightScale[meshIndex], Config::GUIVar().editor_float_slider_speed, 0.0f, 100000.0f))
                                        {
                                            // If the height scale was changed, set the modified flag
                                            m_selectedEntity.m_modelDataModified = true;
                                        }

                                        // Draw ALPHA THRESHOLD
                                        drawLeftAlignedLabelText("Alpha Threshold:", inputWidgetOffset);
                                        if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "AlphaThresholdDrag").c_str(), &modelEntry.m_alphaThreshold[meshIndex], Config::GUIVar().editor_float_slider_speed, 0.0f, 1.0f))
                                        {
                                            // If the alpha threshold was changed, set the modified flag
                                            m_selectedEntity.m_modelDataModified = true;
                                        }

                                        // Draw EMISSIVE INTENSITY
                                        drawLeftAlignedLabelText("Emissive intensity:", inputWidgetOffset);
                                        if(ImGui::DragFloat(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + "EmissiveIntensityDrag").c_str(), &modelEntry.m_emissiveIntensity[meshIndex], Config::GUIVar().editor_float_slider_speed, 0.0f, 100000.0f))
                                        {
                                            // If the emissive intensity was changed, set the modified flag
                                            m_selectedEntity.m_modelDataModified = true;
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

                                            //ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(20.0f, 4.0f));
                                            ImGui::SeparatorText(materialTypeName.c_str());

                                            // Draw TEXTURE FILENAME
                                            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(2) - inputWidgetOffset - m_imguiStyle.FramePadding.x + m_imguiStyle.IndentSpacing);
                                            if(ImGui::InputText(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureFilenameInput").c_str(), &modelEntry.m_meshMaterials[meshIndex][materialIndex], ImGuiInputTextFlags_EnterReturnsTrue))
                                            {
                                                // If the texture filename was changed, set the modified flag
                                                m_selectedEntity.m_modelDataModified = true;
                                            }

                                            // Draw TEXTURE OPEN button
                                            ImGui::SameLine(calcTextSizedButtonOffset(2) + m_imguiStyle.IndentSpacing);
                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureOpenButton", "Open a texture file"))
                                            {
                                                // Only open the file browser if it's not opened already
                                                if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                                {
                                                    // Set the selected texture filename handle
                                                    m_selectedEntity.m_selectedTextureName = &modelEntry.m_meshMaterials[meshIndex][materialIndex];

                                                    // Set the file browser activation to Texture File
                                                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_TextureFile;

                                                    // Define file browser variables
                                                    m_fileBrowserDialog.m_filter = "Texture files (.png .tga .tif .tiff .jpg .jpeg .bmp){.png,.tga,.tif,.tiff,.jpg,.jpeg,.bmp},All files{.*}";
                                                    m_fileBrowserDialog.m_title = "Open a texture file";
                                                    m_fileBrowserDialog.m_name = "OpenTextureFileFileDialog";
                                                    m_fileBrowserDialog.m_rootPath = Config::filepathVar().texture_path;
                                                    m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

                                                    // Tell the GUI scene to open the file browser
                                                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                                }
                                            }

                                            const std::string textureSelectionPopupName = "##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureSelectionPopup";

                                            // Draw OPEN ASSET LIST button
                                            ImGui::SameLine(calcTextSizedButtonOffset(1) + m_imguiStyle.IndentSpacing);
                                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenAssetList], "##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureOpenAssetListButton", "Choose a texture from the loaded assets"))
                                            {
                                                // Open the pop-up with the texture asset list
                                                ImGui::OpenPopup(textureSelectionPopupName.c_str());
                                            }

                                            // Draw TEXTURE RELOAD button
                                            ImGui::SameLine(calcTextSizedButtonOffset(0) + m_imguiStyle.IndentSpacing);
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
                                                            modelEntry.m_meshMaterials[meshIndex][materialIndex] = m_textureAssets[i].second;

                                                            // Set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;

                                                            ImGui::CloseCurrentPopup();
                                                        }

                                                        ImGui::SameLine();

                                                        // Draw TEXTURE NAME selection
                                                        // Set the text height to the texture image button height
                                                        if(ImGui::Selectable(m_textureAssets[i].second.c_str(), (modelEntry.m_meshMaterials[meshIndex][materialIndex] == m_textureAssets[i].second), 0, nameTextSize))
                                                        {
                                                            // Set the selected texture
                                                            modelEntry.m_meshMaterials[meshIndex][materialIndex] = m_textureAssets[i].second;

                                                            // Set the modified flag
                                                            m_selectedEntity.m_modelDataModified = true;
                                                        }
                                                    }
                                                }
                                                ImGui::PopStyleVar(2); //ImGuiStyleVar_FramePadding, ImGuiStyleVar_SelectableTextAlign
                                                ImGui::PopStyleColor(); //ImGuiCol_Button
                                                ImGui::EndPopup();
                                            }

                                            drawLeftAlignedLabelText("Texture scale:", inputWidgetOffset);
                                            if(ImGui::DragFloat2(("##" + Utilities::toString(modelIndex) + Utilities::toString(meshIndex) + Utilities::toString(materialIndex) + "TextureScaleDrag").c_str(), glm::value_ptr(modelEntry.m_meshMaterialsScale[meshIndex][materialIndex]), Config::GUIVar().editor_float_slider_speed))
                                            {
                                                // If the texture scale was changed, set the modified flag
                                                m_selectedEntity.m_modelDataModified = true;
                                            }
                                        }
                                        ImGui::SeparatorText("");
                                        ImGui::TreePop();
                                    }
                                }
                                ImGui::PopStyleVar(); // ImGuiStyleVar_SeparatorTextAlign
                            }
                        }
                    }

                    auto *shaderComponent = entityRegistry.try_get<ShaderComponent>(m_selectedEntity.m_entityID);
                    if(shaderComponent != nullptr)
                    {
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::ShaderComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {

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
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::RigidBodyComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            // Get the bullet physics rigid body object
                            auto rigidBody = rigidBodyComponent->getRigidBody();

                            // Get the current rigid body data
                            m_selectedEntity.m_collisionShapeType = rigidBodyComponent->getCollisionShapeType();
                            m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction = rigidBody->getFriction();
                            m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_mass = rigidBody->getMass();
                            m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_restitution = rigidBody->getRestitution();
                            m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_kinematic = rigidBody->isKinematicObject();

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

                            // Draw FRICTION
                            drawLeftAlignedLabelText("Friction:", inputWidgetOffset);
                            if(ImGui::DragFloat("##FrictionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction, Config::GUIVar().editor_float_slider_speed))
                            {
                                // If the friction was changed, send a notification to the Rigid Body Component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::Friction);
                            }

                            // Draw RESTITUTION
                            drawLeftAlignedLabelText("Restitution:", inputWidgetOffset);
                            if(ImGui::DragFloat("#RestitutionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_restitution, Config::GUIVar().editor_float_slider_speed))
                            {
                                // If the m_restitution was changed, send a notification to the Rigid Body Component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::Restitution);
                            }

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
                                    // Get the Bullet Physics collision shape
                                    auto collisionShape = rigidBodyComponent->getCollisionShapeBox();

                                    // Get the collision shape data
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize = Math::toGlmVec3(collisionShape->getImplicitShapeDimensions());

                                    // Draw BOX HALF EXTENTS
                                    drawLeftAlignedLabelText("Box half extents:", inputWidgetOffset);
                                    if(ImGui::DragFloat3("##BoxHalfExtentsDrag", glm::value_ptr(m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize), Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                    {
                                        // If the box half extents size vector was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
                                    }

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
                                    // Get the Bullet Physics collision shape
                                    auto collisionShape = rigidBodyComponent->getCollisionShapeSphere();

                                    // Get the collision shape data
                                    m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize = Math::toGlmVec3(collisionShape->getImplicitShapeDimensions());

                                    // Draw SPHERE RADIUS
                                    drawLeftAlignedLabelText("Sphere radius:", inputWidgetOffset);
                                    if(ImGui::DragFloat3("##SphereRadiusDrag", glm::value_ptr(m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize), Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                                    {
                                        // If the box half extents size vector was changed, send a notification to the Rigid Body Component
                                        m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
                                    }
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
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::SoundComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            // Get Sound Component data
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_active = soundComponent->isObjectActive();
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_volume = soundComponent->getVolume();
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_loop = soundComponent->getLoop();
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_spatialized = soundComponent->getSpatialized();
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_startPlaying = soundComponent->getStartPlaying();
                            m_selectedEntity.m_soundType = soundComponent->getSoundType();
                            m_selectedEntity.m_playing = soundComponent->getPlaying();

                            // If the sound filename was changed (by file browser), send a notification to the Sound Component
                            // Otherwise just get the current sound filename
                            if(m_selectedEntity.m_soundFilenameModified)
                            {
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::Filename);
                                m_selectedEntity.m_soundFilenameModified = false;
                            }
                            else
                                m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundFilename = soundComponent->getSoundFilename();

                            // Draw SOUND FILENAME
                            //drawLeftAlignedLabelText("Filename:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset - (openReloadButtonSize.x + m_imguiStyle.FramePadding.x * 2) * 2);
                            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                            if(ImGui::InputText("##SoundFilenameInput", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundFilename, ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                // If the sound filename was changed, send a notification to the Sound Component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::Filename);
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
                                    m_fileBrowserDialog.m_rootPath = Config::filepathVar().sound_path;
                                    m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_None;

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

                            // Draw VOLUME
                            drawLeftAlignedLabelText("Volume:", inputWidgetOffset);
                            if(ImGui::DragFloat("##SoundVolumeDrag", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_volume, Config::GUIVar().editor_float_slider_speed, 0.0f, 1.0f))
                            {
                                // If the sound volume was changed, send a notification to the Sound Component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, soundComponent, Systems::Changes::Audio::Volume);
                            }

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
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::SoundListenerComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID = soundListenerComponent->getListenerID();

                            // Draw SOUND LISTENER ID
                            drawLeftAlignedLabelText("Listener ID:", inputWidgetOffset);
                            ImGui::InputInt("##ListenerIDInput", &m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID);
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
                                if(!m_selectedEntity.m_luaVariables.empty() && ImGui::BeginChild("##LuaVariables", ImVec2(0, childWindowHeight), true, ImGuiWindowFlags_None))
                                {
                                    // Calculate item sizes and offsets
                                    //const ImVec2 deleteButtonSize = ImVec2(m_fontSize * ((float)m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry].getTextureWidth() / (float)m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry].getTextureHeight()), fontSize);
                                    //const ImVec2 addButtonSize = ImVec2(fontSize, fontSize);
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
                                        ImGui::SetNextItemWidth(itemSizes[2]);
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

                                    // Draw ADD button
                                    ImGui::SetCursorPosX(calcTextSizedButtonOffset(0) - m_imguiStyle.FramePadding.x);
                                    if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##AddLuaVariableButton", "Add a new Lua variable"))
                                    {
                                        m_selectedEntity.m_luaVariablesModified = true;
                                        m_selectedEntity.m_luaVariables.push_back(std::make_pair(std::string(), Property()));
                                    }

                                    ImGui::EndChild();
                                }
                                ImGui::PopStyleVar();
                            }
                        }
                    }
                    
                    // GUI COMPONENTS
                    auto *guiSequenceComponent = entityRegistry.try_get<GUISequenceComponent>(m_selectedEntity.m_entityID);
                    if(guiSequenceComponent != nullptr)
                    {
                        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::GUISequenceComponent), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            // Get the current GUI Sequence data
                            m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_staticSequence = guiSequenceComponent->isStaticSequence();

                            // Draw STATIC
                            drawLeftAlignedLabelText("Static sequence:", inputWidgetOffset);
                            if(ImGui::Checkbox("##StaticSequenceCheck", &m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_staticSequence))
                            {
                                // If the static sequence flag was changed, send a notification to the GUI Sequence Component
                                m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, guiSequenceComponent, Systems::Changes::GUI::StaticSequence);
                            }
                        }
                    }
                    
                }
                ImGui::EndTabItem();
            }
            
            if(ImGui::BeginTabItem("Scene settings"))
            {
                if(m_currentSceneData.m_modified)
                {
                    updateSceneData(m_currentSceneData);
                    m_currentSceneData.m_modified = false;
                }
                drawSceneData(m_currentSceneData, true);

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
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

                            ImGui::Text(m_textureAssets[i].first->getMipmapEnabled() ? "Mipmap enabled" : "Mipmap disabled");
                            if(m_textureAssets[i].first->getMipmapEnabled())
                                ImGui::Text(("Mipmap level: " + Utilities::toString(m_textureAssets[i].first->getMipmapLevel())).c_str());

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
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Models"))
                {
                    for(decltype(m_modelAssets.size()) i = 0, size = m_modelAssets.size(); i < size; i++)
                    {
                        ImGui::Text(m_modelAssets[i].second.c_str());
                    }
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Shaders"))
                {
                    auto contentRegionWidth = ImGui::GetContentRegionAvail().x;
                    auto singleWindowWidth = contentRegionWidth / 3.0f;

                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.26f, 0.26f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                    if(ImGui::BeginChild("##ShaderAssetsProgramSelection", ImVec2(contentRegionWidth * 0.25f, 0), true))
                    {
                        ImGui::SeparatorText("Shader programs:");
                        for(decltype(m_shaderAssets.size()) shaderIndex = 0, size = m_shaderAssets.size(); shaderIndex < size; shaderIndex++)
                        {
                            if(ImGui::Selectable(m_shaderAssets[shaderIndex].second.c_str(), m_selectedProgramShader == shaderIndex))
                            {
                                m_selectedProgramShader = (int)shaderIndex;
                                m_selectedShader = -1;
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    if(m_selectedProgramShader >= 0 && m_selectedProgramShader < m_shaderAssets.size())
                    {
                        if(ImGui::BeginChild("##ShaderAssetsShaderSelection", ImVec2(contentRegionWidth * 0.25f, 0), true))
                        {
                            ImGui::SeparatorText("Shaders:");
                            for(unsigned int shaderType = 0; shaderType < ShaderType::ShaderType_NumOfTypes; shaderType++)
                            {
                                if(!m_shaderAssets[m_selectedProgramShader].first->getShaderFilename(shaderType).empty())
                                {
                                    if(ImGui::Selectable(m_shaderAssets[m_selectedProgramShader].first->getShaderFilename(shaderType).c_str(), m_selectedShader == shaderType))
                                    {
                                        m_selectedShader = (int)shaderType;
                                    }
                                }
                            }
                        }
                        ImGui::EndChild();
                    }

                    ImGui::SameLine();

                    ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing

                    if(m_selectedShader >= 0 && m_selectedShader < ShaderType::ShaderType_NumOfTypes)
                    {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, m_imguiStyle.FramePadding.y));
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, m_imguiStyle.ItemSpacing.y));

                        if(ImGui::BeginChild("##ShaderAssetsSettings", ImVec2(contentRegionWidth * 0.5f, 0), true))
                        {
                            ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                            ImGui::SeparatorText("Shader settings:");

                            // Calculate widget offset used to draw a label on the left and a widget on the right (opposite of how ImGui draws it)
                            float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;                

                            // Calculate button width so they span across the whole window width
                            auto buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;

                            if(ImGui::Button("Open in text editor", ImVec2(buttonWidth, 0)))
                            {

                            }

                            ImGui::SameLine();

                            if(ImGui::Button("Open in file explorer", ImVec2(buttonWidth, 0)))
                            {
                                ShellExecuteA(NULL, "explore", (Filesystem::getCurrentDirectory() + "\\" + Config::filepathVar().shader_path + Utilities::stripFilePath(m_shaderAssets[m_selectedProgramShader].first->getShaderFilename(m_selectedShader))).c_str(), NULL, NULL, SW_SHOWDEFAULT);
                            }

                            ImGui::SameLine();

                            if(ImGui::Button("Reload shader program", ImVec2(buttonWidth, 0)))
                            {

                            }

                            ImGui::Separator();

                            // Draw SHADER FILENAME
                            auto shaderFilename = m_shaderAssets[m_selectedProgramShader].first->getShaderFilename(m_selectedShader);
                            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                            if(ImGui::InputText("##ShaderFilenameInput", &shaderFilename, ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                            }

                            // Draw OPEN button
                            ImGui::SameLine(calcTextSizedButtonOffset(1));
                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile], "##ShaderFileOpenFileButton", "Open a shader file"))
                            {
                            }

                            // Draw RELOAD button
                            ImGui::SameLine(calcTextSizedButtonOffset(0));
                            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##ShaderFileReloadButton", "Reload the shader"))
                            {
                            }

                            // Draw SHADER TYPE
                            auto shaderType = m_selectedShader;
                            drawLeftAlignedLabelText("Shader type:", inputWidgetOffset);
                            if(ImGui::Combo("##ShaderTypePicker", &shaderType, &m_shaderTypeStrings[0], (int)m_shaderTypeStrings.size()))
                            {
                            }

                            // Draw DEFAULT SHADER
                            auto defaultShader = m_shaderAssets[m_selectedProgramShader].first->isDefaultProgram();
                            drawLeftAlignedLabelText("Default shader:", inputWidgetOffset);
                            if(ImGui::Checkbox("##DefaultShaderCheck", &defaultShader))
                            {
                            }                           
                            
                            // Draw LOADED-TO-VIDEO-MEMORY
                            auto loadedToVideoMemory = m_shaderAssets[m_selectedProgramShader].first->isLoadedToVideoMemory();
                            drawLeftAlignedLabelText("Loaded to video memory:", inputWidgetOffset);
                            if(ImGui::Checkbox("##LoadedToVideoMemoryCheck", &loadedToVideoMemory))
                            {
                            }

                            // Draw UNIFORM UPDATER SETTINGS
                            if(ImGui::TreeNodeEx("Uniform updater settings:", ImGuiTreeNodeFlags_Framed)) // ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf
                            {
                                // Get uniform updater
                                auto const &uniformUpdater = m_shaderAssets[m_selectedProgramShader].first->getUniformUpdater();

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
                        else
                            ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding, ImGuiStyleVar_ItemSpacing
                        ImGui::EndChild();
                    }

                    ImGui::PopStyleVar(2); // ImGuiStyleVar_ChildBorderSize, ImGuiStyleVar_SeparatorTextAlign
                    ImGui::PopStyleColor(); // ImGuiCol_Border

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Scripts"))
                {
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
            ImGui::End();
        }
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
                    // Get window starting position and the size of available space inside the window
                    auto windowPosition = ImGui::GetCursorScreenPos();
                    auto contentRegionSize = ImGui::GetContentRegionAvail();

                    m_centerWindowSize.x = (int)contentRegionSize.x;
                    m_centerWindowSize.y = (int)contentRegionSize.y;

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
                        contentRegionSize,
                        ImVec2(0, 1),
                        ImVec2(1, 0)
                    );
                    ImGui::PopStyleVar();

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

                            //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));
                            //if(ImGui::BeginChild("##NewSceneSettingsWindow2"))
                            //{
                                drawSceneData(m_newSceneData);
                            //    ImGui::EndChild();
                            //}
                            //ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
                            ImGui::EndChild();
                        }
                        else
                        {
                            ImGui::PopStyleVar(); // ImGuiStyleVar_ChildBorderSize
                            ImGui::PopStyleColor(); // ImGuiCol_Border
                        }

                        ImGui::SameLine();

                        if(ImGui::BeginChild("##NewSceneButtonsWindow"))
                        {
                            const ImVec2 buttonSize(contentRegionSize.x * 0.2f, ImGui::GetFrameHeight() * 1.5f);
                            const float centerButtonOffset = (ImGui::GetContentRegionAvail().x / 2.0f) - (buttonSize.x / 2.0f);

                            //ImGui::NewLine();
                            ImGui::SetCursorPosX(centerButtonOffset);
                            if(ImGui::Button("Create scene", buttonSize))
                            {
                                PropertySet sceneProperties(Properties::Default);

                                generateNewMap(sceneProperties, m_newSceneData);

                                // Send a notification to the engine to reload the current engine state
                                m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneReload, EngineStateType::EngineStateType_Editor, sceneProperties));
                            }

                            ImGui::SetCursorPosX(centerButtonOffset);
                            if(ImGui::Button("Cancel", buttonSize))
                            {
                                m_newSceneData = SceneData();
                                m_showNewMapWindow = false;
                            }

                            ImGui::NewLine();
                            ImGui::SetCursorPosX(centerButtonOffset);
                            if(ImGui::Button("Reload to default", buttonSize))
                            {
                                m_newSceneData = SceneData();
                            }

                            ImGui::EndChild();
                        }

                        ImGui::EndTabItem();
                    }
                }

                ImGui::EndTabBar();
            }
            ImGui::End();
        }
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
                        const std::string currentDirectory = Filesystem::getCurrentDirectory() + "\\";

                        // Check if the selected file is within the current directory
                        if(m_fileBrowserDialog.m_filePath.rfind(currentDirectory, 0) == 0)
                        {
                            // Set the selected file path as a relative path from current directory
                            m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                            // If the Lua script filename was changed, send a notification to the LUA Component
                            m_selectedEntity.m_luaScriptFilenameModified = true;
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

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
                        m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneReload));
                    }

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
                        m_systemScene->getSceneLoader()->saveToFile(m_fileBrowserDialog.m_filePathName);
                    }

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
                            m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundFilename = m_fileBrowserDialog.m_filePathName.substr(currentDirectory.size());

                            // If the Lua script filename was changed, set a flag for it
                            m_selectedEntity.m_soundFilenameModified = true;
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

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
                                m_selectedEntity.m_modelDataModified = true;
                                m_selectedEntity.m_modelDataUpdateAfterLoading = true;
                            }
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

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
                                m_selectedEntity.m_modelDataModified = true;
                                m_selectedEntity.m_modelDataUpdateAfterLoading = true;
                            }
                        }
                        else
                            ErrHandlerLoc::get().log(ErrorCode::Editor_path_outside_current_dir, ErrorSource::Source_GUIEditor);
                    }

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

                    // Reset the file browser and mark the file browser as not opened
                    m_fileBrowserDialog.resetAll();
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
                }
            }
            break;
            
    }
}

void EditorWindow::activate()
{
}

void EditorWindow::deactivate()
{
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
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_pause_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_play_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_restart_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_gui_sequence_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_scripting_enabled_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_delete_entry_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_add_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_open_file_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_reload_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_open_asset_list_texture));
    m_buttonTextures.emplace_back(Loaders::texture2D().load(Config::GUIVar().editor_button_arrow_up_texture));

    assert(m_buttonTextures.size() == ButtonTextureType::ButtonTextureType_NumOfTypes && "m_buttonTextures array is different size than the number of button textures, in EditorWindow.cpp");

    // Load button textures to memory
    for(decltype(m_buttonTextures.size()) size = m_buttonTextures.size(), i = 0; i < size; i++)
        m_buttonTextures[i].loadToMemory();

    // Load button textures to GPU
    for(decltype(m_buttonTextures.size()) size = m_buttonTextures.size(), i = 0; i < size; i++)
        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_Texture2D, (void *)&m_buttonTextures[i]);

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

    // Calculate rendering passes window height and cap it to a max height value
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

    // Calculate rendering passes window height and cap it to a max height value
    float renderPassWindowHeight = (m_fontSize + m_imguiStyle.FramePadding.y * 2 + m_imguiStyle.ItemSpacing.y) * (p_sceneData.m_renderingPasses.size() + 2);
    renderPassWindowHeight = renderPassWindowHeight > Config::GUIVar().editor_render_pass_max_height ? Config::GUIVar().editor_render_pass_max_height : renderPassWindowHeight;

    if(ImGui::BeginChild("##RenderingPasses", ImVec2(0, renderPassWindowHeight), true))//, ImVec2(0, childWindowHeight), true, ImGuiWindowFlags_None)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
        ImGui::SeparatorText("Rendering passes:");
        ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextBorderSize

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
                }
            }

            // Draw RENDER PASSES
            ImGui::SameLine();
            ImGui::SetCursorPosX(inputWidgetOffset + (m_buttonSizedByFont.x + m_imguiStyle.FramePadding.x * 3) * 2);
            ImGui::SetNextItemWidth(calcTextSizedButtonOffset(1) - m_imguiStyle.FramePadding.x * 2);
            if(ImGui::Combo(("##RenderingPassCombo" + Utilities::toString(i)).c_str(), &renderType, &m_renderingPassesTypeText[0], (int)m_renderingPassesTypeText.size()))
            {
                p_sceneData.m_renderingPasses[i] = static_cast<RenderPassType>(renderType);
            }

            // Draw DELETE button
            ImGui::SameLine(calcTextSizedButtonOffset(1));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry], "##" + Utilities::toString(i) + "RenderingPassDeleteButton", "Remove this Render Pass entry"))
            {
                p_sceneData.m_renderingPasses.erase(p_sceneData.m_renderingPasses.begin() + i);
                size = p_sceneData.m_renderingPasses.size();
                i--;
            }

            // Draw ADD button
            ImGui::SameLine(calcTextSizedButtonOffset(0));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##" + Utilities::toString(i) + "RenderingPassAddButton", "Add a new Render Pass entry"))
            {
                p_sceneData.m_renderingPasses.insert(p_sceneData.m_renderingPasses.begin() + i + 1, RenderPassType::RenderPassType_AtmScattering);
                size = p_sceneData.m_renderingPasses.size();
            }
        }

        // Draw ADD button
        ImGui::SetCursorPosX(calcTextSizedButtonOffset(0));
        if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Add], "##RenderingPassAddAtEndButton", "Add a new Render Pass entry"))
        {
            p_sceneData.m_renderingPasses.push_back(RenderPassType::RenderPassType_AtmScattering);
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextAlign
    ImGui::PopStyleColor(); // ImGuiCol_Border
}

void EditorWindow::drawEntityHierarchyEntry(EntityHierarchyEntry &p_entityEntry)
{
    static ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGuiTreeNodeFlags flags = p_entityEntry.m_children.empty() ? baseNodeFlags | ImGuiTreeNodeFlags_Leaf : baseNodeFlags;

    if(ImGui::TreeNodeEx(p_entityEntry.m_combinedEntityIdAndName.c_str(), 
        m_selectedEntity.m_entityID == p_entityEntry.m_entityID ? flags | ImGuiTreeNodeFlags_Selected : flags))
    {
        if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            m_selectedEntity.setEntity(p_entityEntry.m_entityID);

        for(decltype(p_entityEntry.m_children.size()) size = p_entityEntry.m_children.size(), i = 0; i < size; i++)
        {
            drawEntityHierarchyEntry(p_entityEntry.m_children[i]);
        }

        ImGui::TreePop();
    }
}

void EditorWindow::updateSceneData(SceneData &p_sceneData)
{
    // Get the required system scenes
    const auto *audioScene = static_cast<AudioScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio));
    const auto *graphicsScene = static_cast<RendererScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics));
    const auto *physicsScene = static_cast<PhysicsScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics));

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
    entityRegistry.each([&](auto entity)
        {
            // Try to get the metadata component
            auto metadataComponent = entityRegistry.try_get<MetadataComponent>(entity);

            // If the metadata component is present, add it to the list
            if(metadataComponent != nullptr)
            {
                std::string entityIdPlusName = Utilities::toString(entity) + Config::componentVar().component_name_separator + metadataComponent->getName();
                m_entityList.emplace_back(entity, metadataComponent->getParentEntityID(), metadataComponent->getName(), entityIdPlusName);
            }
        });

    // Sort the list based on entity ID, so they are shown more conveniently
    std::sort(m_entityList.begin(), m_entityList.end(),
        [](const EntityListEntry &p_a, const EntityListEntry &p_b) -> bool { return p_a.m_entityID < p_b.m_entityID; });
}

void EditorWindow::updateHierarchyList()
{
    // Clear the old hierarchy list
    m_entityHierarchy.clear();

    // Define a root entity and a temp entity list
    EntityHierarchyEntry *rootEntity = nullptr;
    std::list<EntityListEntry> parentlessEntityList;

    // Find the root entity and add all non-root entities to the temp entity list
    for(decltype(m_entityList.size()) size = m_entityList.size(), i = 0; i < size; i++)
    {
        if(m_entityList[i].m_entityID == 0 && m_entityList[i].m_parentEntityID == 0)
        {
            m_entityHierarchy.emplace_back(m_entityList[i].m_entityID, m_entityList[i].m_parentEntityID, m_entityList[i].m_name, m_entityList[i].m_combinedEntityIdAndName, m_entityList[i].m_componentFlag);
            rootEntity = &m_entityHierarchy.front();
        }
        else
        {
            parentlessEntityList.push_back(m_entityList[i]);
        }
    }

    // Check if the root entity is present
    if(rootEntity != nullptr)
    {
        // Define a list that hold all the children that have been added during the last loop
        // and add the root entity to it
        std::vector<EntityHierarchyEntry*> newlyAddedChildren;
        newlyAddedChildren.push_back(rootEntity);

        // Continue the loop until there are no newly added children
        while(!newlyAddedChildren.empty())
        {
            // Save a copy of the children list that have been generated during the last loop
            // and clear the children list for the current loop
            auto newChildrenFromPreviousRun = newlyAddedChildren;
            newlyAddedChildren.clear();

            // Go over each new children from the previous loop
            for(decltype(newChildrenFromPreviousRun.size()) childrenSize = newChildrenFromPreviousRun.size(), childrenIndex = 0; childrenIndex < childrenSize; childrenIndex++)
            {
                // Go over each entity left, that haven't been added as children
                auto parentlessEntity = parentlessEntityList.begin();
                while(parentlessEntity != parentlessEntityList.end())
                {
                    // If the entity ID of the current child and the parent entity ID of the parent-less entity matches, add the parent-less entity as a child
                    // if it doesn't match, continue to the next entity
                    if(newChildrenFromPreviousRun[childrenIndex]->m_entityID == parentlessEntity->m_parentEntityID)
                    {
                        // Add the parent-less entity as a child
                        newChildrenFromPreviousRun[childrenIndex]->m_children.emplace_back(parentlessEntity->m_entityID, parentlessEntity->m_parentEntityID, parentlessEntity->m_name, parentlessEntity->m_combinedEntityIdAndName, parentlessEntity->m_componentFlag);

                        // Add the newly created child to the child list for the next loop
                        newlyAddedChildren.push_back(&newChildrenFromPreviousRun[childrenIndex]->m_children.back());

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
                std::cout << i.m_entityID << std::endl;
            }
        }
    }
    else
        std::cout << "NO ROOT ENTRY" << std::endl;

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
        //auto impactSoundComp = entityRegistry.try_get<ImpactSoundComponent>(m_entityList[i].m_entityID);
        //if(impactSoundComp != nullptr)
        //{
        //    m_componentList.emplace_back(m_entityList[i].m_entityID, impactSoundComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + impactSoundComp->getName());
        //    m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::AudioImpactSoundComponent;
        //}
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
        // Set the shader name based on the types of shader present
        std::string shaderName = shaderPool[i]->getShaderFilename(ShaderType::ShaderType_Fragment);
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

        // Remove the extension from the shader filename
        shaderName = Utilities::removeExtension(shaderName);

        // Add the shader and name entry
        m_shaderAssets.push_back(std::make_pair(shaderPool[i], shaderName));

        // Set the longest shader name from all loaded shader assets, required for setting popup sizes when showing the list of shaders
        if(m_shaderAssetLongestName.size() < shaderName.size())
            m_shaderAssetLongestName = shaderName;
    }
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
    editorCameraObjectEntry.addPropertySet(Properties::PropertyID::World).addPropertySet(Properties::PropertyID::SpatialComponent);
    editorCameraObjectEntry.addPropertySet(Properties::PropertyID::Script).addPropertySet(Properties::PropertyID::LuaComponent).addProperty(Properties::PropertyID::Filename, std::string("Camera_free_object_spawn.lua"));

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

    // Add rendering passes
    auto &graphicsScenePropertySet = rootSystemsPropertySet.addPropertySet(Properties::Graphics).addPropertySet(Properties::Scene);
    RendererScene::exportRenderingPasses(graphicsScenePropertySet, p_sceneData.m_renderingPasses);
}
