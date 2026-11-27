#define IMSPINNER_DEMO
#define IMGUI_DEFINE_MATH_OPERATORS

#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"

#include "imgui_internal.h"
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"

#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_tex_inspect.h>
#include <tex_inspect_opengl.h>
#include <imspinner.h>

void EditorWindow::drawCenterWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar)
{
            auto &entityRegistry = getEntityRegistry();
            auto *rendererScene = getRendererScene();

            ImGui::SetNextWindowClass(&p_windowClassWithNoTabBar);
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
                        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTextureResolution, (void *)&m_centerWindowSize);

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
                                        sendChangeTo(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                    else
                                        sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                }

                                // Draw ROTATE MANIPULATION ImGuizmo
                                if(m_rotateGuizmoEnabled && ImGuizmo::Manipulate(glm::value_ptr(rendererScene->getViewMatrix()[0]), glm::value_ptr(rendererScene->getProjectionMatrix()[0]), ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(worldMatrix[0])))
                                {
                                    // Get the local transform from the world matrix by multiplying it with the inverse of the parent transform
                                    m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_transformMatNoScale = glm::inverse(spatialComponent->getSpatialDataChangeManager().getParentTransform()) * worldMatrix;

                                    // If the model transform matrix was changed, send a notification to the either the Spatial Component or Rigid Body Component (if the Rigid Body Component is present, it takes control over the spatial data)
                                    if(rigidBodyComponent != nullptr)
                                        sendChangeTo(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                    else
                                        sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                                }
                            }
                            else
                                ImGuizmo::Enable(false);
                        }

                        //	 ____________________________
                        //	|							 |
                        //	|	   OBJECT PICKING        |
                        //	|____________________________|
                        //
                        // Query the gizmo after it has been drawn, so that dragging a gizmo handle
                        // never triggers a selection change
                        if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
                        {
                            const ImVec2 mousePosition = ImGui::GetIO().MousePos;

                            // Convert the mouse position into the render target's pixel space.
                            // The render target's origin is in the bottom-left corner, while the
                            // ImGui coordinates start in the top-left corner of the viewport
                            const int pixelX = (int)(mousePosition.x - m_sceneViewportPosition.x);
                            const int pixelYFromTop = (int)(mousePosition.y - m_sceneViewportPosition.y);

                            if(pixelX >= 0 && pixelYFromTop >= 0 && pixelX < m_centerWindowSize.x && pixelYFromTop < m_centerWindowSize.y)
                            {
                                m_pickingRequestPixel = glm::ivec2(pixelX, m_centerWindowSize.y - 1 - pixelYFromTop);

                                // Tell the graphics scene to determine which entity is under the mouse
                                sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_PickingRequest, (void *)&m_pickingRequestPixel, false);
                            }
                        }

                        // If the mouse is hovering over the viewport, stop capturing it, so the engine can handle mouse events
                        if(ImGui::IsItemHovered())
                        {
                            ImGui::CaptureMouseFromApp(false);
                        }

                        // If the viewport is focused, stop capturing key presses, so the engine can handle keyboard events
                        if(ImGui::IsItemFocused())
                        {
                            ImGui::CaptureKeyboardFromApp(false);
                        }

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
                            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_GUIPassFunctors, (void *)&functorSequence);

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
                            if(ImGui::BeginChild("##NewSceneSettingsWindow", ImVec2(contentRegionSize.x * 0.6f, contentRegionSize.y - m_imguiStyle.FramePadding.y), true))
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
