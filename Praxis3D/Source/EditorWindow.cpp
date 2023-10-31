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

ErrorCode EditorWindow::init()
{
    return ErrorCode::Success;
}

void EditorWindow::update(const float p_deltaTime)
{
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

    //ImGui::DockSpaceOverViewport(mainViewport, ImGuiDockNodeFlags_PassthruCentralNode);

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
            if(ImGui::MenuItem("Open...")) {}
            ImGui::Separator();
            if(ImGui::MenuItem("Save")) {}
            if(ImGui::MenuItem("Save as...")) {}
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
            ImGui::SameLine();

            // Get the secondary menu bar style, required for retrieving size information
            ImGuiStyle &secondaryMenuBarStyle = ImGui::GetStyle();

            // Calculate the combined size of a single button (including the inner spacing)
            float playPauseButtonCombinedSize = playPauseButtonSize.x + secondaryMenuBarStyle.ItemInnerSpacing.x * 2.0f;

            // Calculate the offset of all buttons to the center
            float offsetToCenter = ((ImGui::GetContentRegionAvail().x - (playPauseButtonCombinedSize * 3.0f)) * 0.5f) + secondaryMenuBarStyle.FramePadding.x;

            // Set the starting position of the buttons
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetToCenter);

            // Draw PLAY button
            if(ImGui::ImageButton("##PlayButton", 
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Play].getHandle(), 
                playPauseButtonSize,
                ImVec2(0, 1),
                ImVec2(1, 0),
                m_sceneState == EditorSceneState::EditorSceneState_Play ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
            {
                m_sceneState = EditorSceneState::EditorSceneState_Play;
            }

            // Draw PAUSE button
            ImGui::SameLine();
            if(ImGui::ImageButton("##PauseButton", 
                (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_Pause].getHandle(), 
                playPauseButtonSize,
                ImVec2(0, 1),
                ImVec2(1, 0),
                m_sceneState == EditorSceneState::EditorSceneState_Pause ? ImVec4(0.26f, 0.26f, 0.26f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
            {
                m_sceneState = EditorSceneState::EditorSceneState_Pause;
            }

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
                    if(ImGui::Selectable(m_entityList[i].m_combinedEntityIdAndName.c_str(), m_entityList[i].m_entityID == m_selectedEntity))
                    {
                        m_selectedEntity = m_entityList[i].m_entityID;
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
                    if(ImGui::Selectable(m_componentList[i].m_combinedEntityIdAndName.c_str(), m_componentList[i].m_entityID == m_selectedEntity))
                    {
                        m_selectedEntity = m_componentList[i].m_entityID;
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
                if(m_selectedEntity != NULL_ENTITY_ID)
                {
                    MetadataComponent *metadataComponent = entityRegistry.try_get<MetadataComponent>(m_selectedEntity);

                    if(metadataComponent != nullptr)
                    {
                        std::string test = metadataComponent->getName();
                        char str0[128]; 
                        std::strcpy(str0, metadataComponent->getName().c_str());

                        float inputTextOffset = ImGui::GetCursorPosX() + ImGui::CalcItemWidth() * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x;

                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Name:");
                        ImGui::SameLine();
                        ImGui::SetCursorPosX(inputTextOffset);
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("", str0, IM_ARRAYSIZE(str0));

                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Position:");
                        ImGui::SameLine();
                        ImGui::SetCursorPosX(inputTextOffset);
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("", str0, IM_ARRAYSIZE(str0));

                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Rotation:");
                        ImGui::SameLine();
                        ImGui::SetCursorPosX(inputTextOffset);
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
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

        ImGui::Begin("##BottomWindow", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);

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
                        ImGui::Text("");
                        ImGui::SeparatorText("Click to open in Texture Inspector");
                        ImGui::Text("");
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

    //ImGui::Begin("Simple Texture Inspector");
    ////ImGuiTexInspect::BeginInspectorPanel("Inspector", (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_ScriptingEnable].getHandle(), ImVec2(200.0f, 200.0f), 0, ImGuiTexInspect::SizeIncludingBorder(ImGui::GetContentRegionAvail()));
    ////ImGuiTexInspect::BeginInspectorPanel("Inspector", (ImTextureID)m_buttonTextures[ButtonTextureType::ButtonTextureType_ScriptingEnable].getHandle(), ImVec2(200.0f, 200.0f), 0);//, ImGuiTexInspect::SizeIncludingBorder(ImGui::GetContentRegionAvail()));
    //ImGuiTexInspect::BeginInspectorPanel("Inspector", (ImTextureID)Loaders::texture2D().getDefaultTexture().getHandle(), ImVec2(200.0f, 200.0f), 0);//, ImGuiTexInspect::SizeIncludingBorder(ImGui::GetContentRegionAvail()));
    //ImGuiTexInspect::DrawAnnotations(ImGuiTexInspect::ValueText(ImGuiTexInspect::ValueText::Floats));
    //ImGuiTexInspect::EndInspectorPanel();
    //ImGui::End();
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
}

void EditorWindow::drawEntityHierarchyEntry(EntityHierarchyEntry &p_entityEntry)
{
    static ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGuiTreeNodeFlags flags = p_entityEntry.m_children.empty() ? baseNodeFlags | ImGuiTreeNodeFlags_Leaf : baseNodeFlags;

    if(ImGui::TreeNodeEx(p_entityEntry.m_combinedEntityIdAndName.c_str(), 
        m_selectedEntity == p_entityEntry.m_entityID ? flags | ImGuiTreeNodeFlags_Selected : flags))
    {
        if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            m_selectedEntity = p_entityEntry.m_entityID;

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
        auto impactSoundComp = entityRegistry.try_get<ImpactSoundComponent>(m_entityList[i].m_entityID);
        if(impactSoundComp != nullptr)
        {
            m_componentList.emplace_back(m_entityList[i].m_entityID, impactSoundComp->getName(), Utilities::toString(m_entityList[i].m_entityID) + Config::componentVar().component_name_separator + impactSoundComp->getName());
            m_entityList[i].m_componentFlag |= Systems::AllComponentTypes::AudioImpactSoundComponent;
        }
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
