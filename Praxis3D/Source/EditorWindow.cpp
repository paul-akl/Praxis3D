#include <glm/gtc/type_ptr.hpp>
#include <ranges>

#include "EngineDefinitions.h"
#include "EditorWindow.h"
#include "imgui_internal.h"
#include "RendererScene.h"
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

    ImVec2 playPauseButtonSize(30.0f, 30.0f);
    ImVec2 mainMenuBarSize;
    ImGuiViewport *mainViewport = ImGui::GetMainViewport();

    //RendererScene *rendererScene = static_cast<RendererScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics));
    auto *rendererScene = m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics);

    WorldScene *worldScene = static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World));
    auto &entityRegistry = worldScene->getEntityRegistry();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::PopStyleColor();

    ImGuiStyle &style = ImGui::GetStyle();

    style.TabRounding = 0.0f;

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

                    // Tell the GUI scene to open the file browser
                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                }
            }
            ImGui::Separator();
            if(ImGui::MenuItem("Save")) {}
            if(ImGui::MenuItem("Save as..."))
            {
                // Only open the file browser if it's not opened already
                if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                {
                    // Set the file browser activation to Save Scene
                    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_SaveScene;

                    // Define file browser variables
                    m_fileBrowserDialog.m_definedFilename = m_systemScene->getSceneLoader()->getSceneFilename();
                    m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_ConfirmOverwrite;
                    m_fileBrowserDialog.m_filter = ".pmap,.*";
                    m_fileBrowserDialog.m_title = "Save scene";
                    m_fileBrowserDialog.m_name = "SaveSceneFileDialog";
                    m_fileBrowserDialog.m_rootPath = Config::filepathVar().map_path;
                    
                    // Tell the GUI scene to open the file browser
                    m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                }
            }
            if(ImGui::MenuItem("Reload scene")) 
            {
                // Send a notification to the engine to reload the current engine state
                m_systemScene->getSceneLoader()->getChangeController()->sendEngineChange(EngineChangeData(EngineChangeType::EngineChangeType_SceneReload));
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
                playPauseButtonSize,
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
                playPauseButtonSize,
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
            //ImGui::SameLine();

            // Get the secondary menu bar style, required for retrieving size information
            ImGuiStyle &secondaryMenuBarStyle = ImGui::GetStyle();

            // Calculate the combined size of a single button (including the inner spacing)
            float playPauseButtonCombinedSize = playPauseButtonSize.x + secondaryMenuBarStyle.ItemInnerSpacing.x * 2.0f;

            // Calculate the offset of all buttons to the center
            float offsetToCenter = (playPauseButtonCombinedSize * 3) / 2;

            // Set the starting position of the buttons
            ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 - offsetToCenter);

            // Draw PLAY button
            ImGui::PushStyleColor(ImGuiCol_Button, m_sceneState == EditorSceneState::EditorSceneState_Play ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            if(ImGui::ImageButton("##PlayButton", 
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Play].getHandle(), 
                playPauseButtonSize,
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
                playPauseButtonSize,
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
                playPauseButtonSize,
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

        if(ImGui::BeginTabBar("##LeftWindowTabBar", ImGuiTabBarFlags_None))
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
                                // Send a notification to the spatial component about the change in rotation
                                //m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, spatialComponent, Systems::Changes::Spatial::LocalRotation);                                
                                
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
                            //modelComponent->
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

                                const float fontSize = ImGui::GetFontSize(); 
                                const ImVec2 openReloadButtonSize = ImVec2(fontSize, fontSize);

                                // Draw LUA FILENAME
                                drawLeftAlignedLabelText("Filename:", inputWidgetOffset, ImGui::GetContentRegionAvail().x - inputWidgetOffset - (openReloadButtonSize.x + style.FramePadding.x * 2) * 2);
                                if(ImGui::InputText("##LuaScriptFilenameInput", &m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename, ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, luaComponent, Systems::Changes::Script::Filename);
                                }

                                // Draw OPEN button
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - style.FramePadding.x - (openReloadButtonSize.x + style.FramePadding.x) * 2);
                                if(ImGui::ImageButton("##LuaScriptOpenFileButton",
                                    (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_OpenFile].getHandle(),
                                    openReloadButtonSize,
                                    ImVec2(0, 1),
                                    ImVec2(1, 0),
                                    ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
                                {
                                    // Only open the file browser if it's not opened already
                                    if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
                                    {
                                        // Set the file browser activation to Lua Script
                                        m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_LuaScript;

                                        // Define file browser variables
                                        m_fileBrowserDialog.m_filter = ".lua,.*";
                                        m_fileBrowserDialog.m_title = "Open LUA script file";
                                        m_fileBrowserDialog.m_name = "OpenLuaScriptFileDialog";
                                        m_fileBrowserDialog.m_rootPath = Config::filepathVar().script_path;

                                        // Tell the GUI scene to open the file browser
                                        m_systemScene->getSceneLoader()->getChangeController()->sendData(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                                    }
                                }
                                if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
                                    ImGui::SetTooltip("Open a Lua file script", ImGui::GetStyle().HoverDelayShort);

                                // Draw RELOAD button
                                ImGui::SameLine(ImGui::GetWindowWidth() - openReloadButtonSize.x - style.FramePadding.x * 2);
                                if(ImGui::ImageButton("##LuaScriptOpenFileButton",
                                    (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload].getHandle(),
                                    openReloadButtonSize,
                                    ImVec2(0, 1),
                                    ImVec2(1, 0),
                                    ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
                                {
                                    // Send a reload notification to the LUA Component
                                    m_systemScene->getSceneLoader()->getChangeController()->sendChange(this, luaComponent, Systems::Changes::Script::Reload);
                                }
                                if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
                                    ImGui::SetTooltip("Reload the Lua file script", ImGui::GetStyle().HoverDelayShort);

                                // Update lua variables from the LUA Component only if the previous variables haven't been modified
                                if(!m_selectedEntity.m_luaVariablesModified)
                                    m_selectedEntity.m_luaVariables = luaScript->getLuaVariables();

                                // Calculate lua variables window height and cap it to a max height value
                                float childWindowHeight = (fontSize + style.FramePadding.y * 2 + style.ItemSpacing.y) * (m_selectedEntity.m_luaVariables.size() + 2);
                                childWindowHeight = childWindowHeight > Config::GUIVar().editor_lua_variables_max_height ? Config::GUIVar().editor_lua_variables_max_height : childWindowHeight;

                                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
                                if(!m_selectedEntity.m_luaVariables.empty() && ImGui::BeginChild("##LuaVariables", ImVec2(0, childWindowHeight), true, ImGuiWindowFlags_None))
                                {
                                    // Calculate item sizes and offsets
                                    const ImVec2 deleteButtonSize = ImVec2(fontSize * ((float)m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry].getTextureWidth() / (float)m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry].getTextureHeight()), fontSize);
                                    const ImVec2 addButtonSize = ImVec2(fontSize, fontSize);
                                    const float itemSpacing = style.ItemInnerSpacing.x;
                                    const float windowWidth = ImGui::GetContentRegionAvail().x;
                                    const float itemSpace = windowWidth - (itemSpacing * 3) - deleteButtonSize.x - style.FramePadding.x * 2;
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
                                        if(ImGui::Combo((widgetName + "LuaVariableTypeCombo").c_str(), &variableType, &m_luaVariableTypeStrings[0], m_luaVariableTypeStrings.size()))
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
                                        ImGui::SameLine(windowWidth - deleteButtonSize.x - style.FramePadding.x * 2);
                                        if(ImGui::ImageButton((widgetName + "LuaVariablesDeleteButton").c_str(),
                                            (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_DeleteEntry].getHandle(),
                                            deleteButtonSize,
                                            ImVec2(0, 1),
                                            ImVec2(1, 0),
                                            ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
                                        {
                                            m_selectedEntity.m_luaVariablesModified = true;
                                            m_selectedEntity.m_luaVariables.erase(m_selectedEntity.m_luaVariables.begin() + i);
                                            size = m_selectedEntity.m_luaVariables.size();
                                            i--;
                                        }
                                        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
                                            ImGui::SetTooltip("Delete Lua variable", ImGui::GetStyle().HoverDelayShort);

                                    }

                                    // Draw ADD button
                                    ImGui::SetCursorPosX(windowWidth - addButtonSize.x - style.FramePadding.x * 2);
                                    if(ImGui::ImageButton("##AddLuaVariableButton",
                                        (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Add].getHandle(),
                                        addButtonSize,
                                        ImVec2(0, 1),
                                        ImVec2(1, 0),
                                        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
                                    {
                                        m_selectedEntity.m_luaVariablesModified = true;
                                        m_selectedEntity.m_luaVariables.push_back(std::make_pair(std::string(), Property()));
                                    }
                                    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
                                        ImGui::SetTooltip("Add a new Lua variable", ImGui::GetStyle().HoverDelayShort);

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
                if(ImGui::BeginTabItem("Assets"))
                {
                    auto texturePool = Loaders::texture2D().getObjectPool();

                    // Draw each texture in the 2D texture loader pool
                    for(decltype(texturePool.size()) i = 0, size = texturePool.size(); i < size; i++)
                    {
                        // Set each entry to be drawn on the same line (except the first entry)
                        if(i > 0)
                            ImGui::SameLine();

                        // Draw the texture
                        if(ImGui::ImageButton((ImTextureID)texturePool[i]->getHandle(), ImVec2(60.0f, 60.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 0))
                        {
                            m_selectedTexture = texturePool[i];

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
                            ImGui::PopStyleVar();
                            ImGui::PopStyleVar();
                            ImGui::Separator();

                            ImGui::Text(("Filename: " + texturePool[i]->getFilename()).c_str());
                            ImGui::Text(("Size: " + Utilities::toString(texturePool[i]->getTextureWidth()) + "x" + Utilities::toString(texturePool[i]->getTextureHeight())).c_str());

                            ImGui::Text(("Texture format: " + getTextureFormatString(texturePool[i]->getTextureFormat())).c_str());
                            ImGui::Text(("Texture data type: " + getTextureDataTypeString(texturePool[i]->getTextureDataType())).c_str());
                            ImGui::Text(("Texture data format: " + getTextureDataFormat(texturePool[i]->getTextureDataFormat())).c_str());

                            ImGui::Text(texturePool[i]->getMipmapEnabled() ? "Mipmap enabled" : "Mipmap disabled");
                            if(texturePool[i]->getMipmapEnabled())
                                ImGui::Text(("Mipmap level: " + Utilities::toString(texturePool[i]->getMipmapLevel())).c_str());

                            ImGui::Separator();

                            ImGui::EndTooltip();
                        }
                    }

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

                    // Fill the whole window with the rendered scene
                    ImGui::GetWindowDrawList()->AddImage(
                        (void *)sceneRenderBufferHandle,
                        windowPosition,
                        ImVec2(windowPosition.x + contentRegionSize.x, windowPosition.y + contentRegionSize.y),
                        ImVec2(0, 1),
                        ImVec2(1, 0)
                    );

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
                                        //if(ImGui::BeginTabItem("Scene viewport"))
                                        //{
                                        //    ImGui::EndTabItem();
                                        //}
                                        if(ImGui::BeginTabItem("Texture inspector"))
                                        {
                                            ImVec2 textureSize = ImVec2(m_selectedTexture->getTextureWidth(), m_selectedTexture->getTextureHeight());

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
                        //m_systemScene->getSceneLoader()->loadFromFile(m_fileBrowserDialog.m_filename);
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
