#define IMGUI_DEFINE_MATH_OPERATORS

#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"

#include "imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"
#include "Shaders/Include/ShaderUniformUpdater.hpp"
#include "Systems/RendererSystem/Components/Include/ModelComponent.hpp"
#include "Systems/ScriptingSystem/Components/Include/LuaComponent.hpp"

#include <Windows.h>

void EditorWindow::drawBottomWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar)
{
            ImGui::SetNextWindowClass(&p_windowClassWithNoTabBar);
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
                            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);

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
                                float inputWidgetOffset = ImGui::GetCursorPosX() + ImGui::CalcTextSize("Updates per frame:").x + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::GetStyle().IndentSpacing;

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
                                    sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
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
                                        sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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
                                                    sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
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
                                    sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
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
                                if(ImGui::Selectable((m_luaScriptAssets[i].second + " (" + Utilities::toString(m_luaScriptAssets[i].first) + ")").c_str(), m_selectedLuaScript != NULL_ENTITY_ID ? m_luaScriptAssets[i].first == m_selectedLuaScript : false))
                                {
                                    m_selectedLuaScript = m_luaScriptAssets[i].first;
                                }
                            }
                        }
                        ImGui::EndChild();

                        auto &entityRegistry = getEntityRegistry();
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
                                    //sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadShader, (void *)m_selectedProgram);
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
