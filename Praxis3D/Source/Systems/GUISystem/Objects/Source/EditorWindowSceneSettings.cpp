#define IMGUI_DEFINE_MATH_OPERATORS

#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"

#include "imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"

#include <glm/gtc/type_ptr.hpp>

void EditorWindow::drawSceneSettingsTab()
{
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
    sendChangeTo(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), Systems::Changes::Physics::Gravity);
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
        sendChangeTo(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeMaster);
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
        sendChangeTo(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeAmbient);
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
        sendChangeTo(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeMusic);
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
        sendChangeTo(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), Systems::Changes::Audio::VolumeSFX);
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
                sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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
    sendChangeTo(this, m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), Systems::Changes::Graphics::ActiveCameraID);
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
    sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_MiscSceneData, (void *)&p_sceneData.m_miscSceneData, false);
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
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_AmbientOcclusionData, (void *)&p_sceneData.m_aoData, false);
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
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_AtmScatteringData, (void *)&p_sceneData.m_atmScatteringData, false);
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
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_ShadowMappingData, (void *)&p_sceneData.m_shadowMappingData, false);
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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderingPasses, (void *)&p_sceneData.m_renderingPasses, false);
        p_sceneData.m_modified = true;
    }
}
ImGui::EndChild();

ImGui::PopStyleVar(); //ImGuiStyleVar_SeparatorTextAlign
ImGui::PopStyleColor(); // ImGuiCol_Border
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
    dirLightObjectEntry.addPropertySet(Properties::PropertyID::World).addPropertySet(Properties::PropertyID::SpatialComponent).addProperty(Properties::PropertyID::LocalRotation, glm::vec3(140.0f, 10.0f, 180.0f));
    auto &lightComponentEntry = dirLightObjectEntry.addPropertySet(Properties::PropertyID::Graphics).addPropertySet(Properties::PropertyID::LightComponent);
    lightComponentEntry.addProperty(Properties::PropertyID::Type, Properties::PropertyID::DirectionalLight);
    lightComponentEntry.addProperty(Properties::PropertyID::Intensity, 10.0f);

    // Create camera entity
    auto &editorCameraObjectEntry = gameObjects.addPropertySet(Properties::ArrayEntry);
    editorCameraObjectEntry.addProperty(Properties::PropertyID::Name, std::string("EditorCamera"));
    editorCameraObjectEntry.addProperty(Properties::PropertyID::ID, 20000000);
    editorCameraObjectEntry.addProperty(Properties::PropertyID::Parent, 0);
    editorCameraObjectEntry.addPropertySet(Properties::PropertyID::Graphics).addPropertySet(Properties::PropertyID::CameraComponent);
    auto &luaComponentEntry = editorCameraObjectEntry.addPropertySet(Properties::PropertyID::Script).addPropertySet(Properties::PropertyID::LuaComponent);
    luaComponentEntry.addProperty(Properties::PropertyID::Filename, std::string("Camera_free.lua"));
    luaComponentEntry.addProperty(Properties::PropertyID::PauseInEditor, false);
    auto &luaVariables = luaComponentEntry.addPropertySet(Properties::PropertyID::Variables);
    auto &cameraSpeed = luaVariables.addPropertySet(Properties::PropertyID::ArrayEntry);
    cameraSpeed.addProperty(Properties::PropertyID::Name, std::string("cameraSpeed"));
    cameraSpeed.addProperty(Properties::PropertyID::Value, 10.0f);
    auto &cameraSpeedMult = luaVariables.addPropertySet(Properties::PropertyID::ArrayEntry);
    cameraSpeedMult.addProperty(Properties::PropertyID::Name, std::string("cameraSpeedMultiplier"));
    cameraSpeedMult.addProperty(Properties::PropertyID::Value, 100.0f);
    editorCameraObjectEntry.addPropertySet(Properties::PropertyID::World).addPropertySet(Properties::PropertyID::SpatialComponent).addProperty(Properties::PropertyID::LocalPosition, glm::vec3(0.0f, 15.0f, 0.0f));

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

    // Add shadow mapping cascades
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
