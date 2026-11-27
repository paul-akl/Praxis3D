
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>

#define IMSPINNER_DEMO
#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "imspinner.h"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Shaders/Include/ShaderUniformUpdater.hpp"
#include "Systems/WorldSystem/Include/WorldScene.hpp"

// Include every component
#include "Systems/AudioSystem/Components/Include/ImpactSoundComponent.hpp"
#include "Systems/AudioSystem/Components/Include/SoundComponent.hpp"
#include "Systems/AudioSystem/Components/Include/SoundListenerComponent.hpp"
#include "Systems/GUISystem/Components/Include/GUISequenceComponent.hpp"
#include "Systems/PhysicsSystem/Components/Include/CollisionEventComponent.hpp"
#include "Systems/PhysicsSystem/Components/Include/CollisionShapeComponent.hpp"
#include "Systems/PhysicsSystem/Components/Include/RigidBodyComponent.hpp"
#include "Systems/RendererSystem/Components/Include/CameraComponent.hpp"
#include "Systems/RendererSystem/Components/Include/LightComponent.hpp"
#include "Systems/RendererSystem/Components/Include/ModelComponent.hpp"
#include "Systems/RendererSystem/Components/Include/ShaderComponent.hpp"
#include "Systems/ScriptingSystem/Components/Include/LuaComponent.hpp"
#include "Systems/WorldSystem/Components/Include/ObjectMaterialComponent.hpp"
#include "Systems/WorldSystem/Components/Include/SpatialComponent.hpp"

#include <tex_inspect_opengl.h>
#include <imgui_tex_inspect.h>
#include <imgui/imgui_stdlib.h>

EditorWindow::EditorWindow(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id) : 
    SystemObject(p_systemScene, p_name, Properties::PropertyID::EditorWindow, p_entityID),
    m_selectedEntity(*this),
    m_imguiStyle(ImGui::GetStyle()),
    m_playPauseButtonSize(Config::GUIVar().editor_play_button_size, Config::GUIVar().editor_play_button_size)
{
    m_GUISequenceEnabled = false;
    m_LUAScriptingEnabled = false;
    m_translateGuizmoEnabled = true;
    m_rotateGuizmoEnabled = false;
    m_showNewMapWindow = false;
    m_showImGuiDemoWindow = false;
    m_showImspinnerDemoWindow = false;
    m_fullscreen = false;
    m_enlargedSceneViewport = false;
    m_showingExitDialog = false;
    m_activatedMainMenuButton = MainMenuButtonType::MainMenuButtonType_None;
    m_sceneState = EditorSceneState::EditorSceneState_Pause;
    m_centerWindowSize = glm::ivec2(0);
    m_pickingRequestPixel = glm::ivec2(0);
    m_activeItemID = -1;

    m_selectedTexture = nullptr;
    m_textureInspectorTabFlags = 0;

    m_buttonMaterialType = MaterialType::MaterialType_Diffuse;
    m_colorEditFlags = ImGuiColorEditFlags_Float;
    m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
    m_previouslyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_None;
    m_fileBrowserDialog.m_name = "EditorFileBrowserDialog";

    m_selectedLuaScript = NULL_ENTITY_ID;
    m_selectedModel = nullptr;
    m_selectedProgram = nullptr;
    m_selectedShaderType = -1;

    m_nextEntityIDToSelect = NULL_ENTITY_ID;
    m_nextEntityToSelect = nullptr;
    m_pendingEntityToSelect = false;

    m_newEntityConstructionInfo = nullptr;
    m_openNewEntityPopup = false;
    m_openEntityRightClickOptionsPopup = false;
    m_duplicateParent = false;

    resetActivateAllComponentFlags();

    m_newSceneSettingsTabFlags = 0;

    m_fontSize = ImGui::GetFontSize();
    m_buttonSizedByFont = ImVec2(m_fontSize, m_fontSize);
    m_assetSelectionPopupImageSize = ImVec2(m_fontSize, m_fontSize) * Config::GUIVar().editor_asset_selection_button_size_multiplier;
    m_textureAssetImageSize = ImVec2(Config::GUIVar().editor_asset_texture_button_size_x, Config::GUIVar().editor_asset_texture_button_size_y);

    m_buttonBackgroundEnabled = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
    m_buttonBackgroundDisabled = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    m_mousePositionOnNewEntity = ImVec2(0.0f, 0.0f);
    m_newEntityWindowInitialized = false;
    m_sceneViewportPosition = ImVec2(0.0f, 0.0f);
    m_sceneViewportSize = ImVec2(0.0f, 0.0f);
    m_synchronizeTextureScale = true;
    m_synchronizeTextureFraming = true;
    m_2DTextureScale = true;
    m_2DTextureFraming = false;

    m_numOfLogs = 0;

    for(unsigned int i = 0; i < ObjectMaterialType::NumberOfMaterialTypes; i++)
        m_physicalMaterialProperties.push_back(GetString(static_cast<ObjectMaterialType>(i)));

    for(unsigned int i = 0; i < RenderPassType::RenderPassType_NumOfTypes; i++)
        m_renderingPassesTypeText.push_back(GetString(static_cast<RenderPassType>(i)));

    m_entityRightClickOptionsPopup = "EntityRightClickOptionsPopup";

    m_antialiasingTypeText = { "None", "MSAA", "FXAA" };
    m_ambientOcclusionTypeText = { "None", "SSAO", "HBAO" };
    m_cascadeDistanceTypeText = { "Units", "Divider" };
    m_luaVariableTypeStrings = { "null", "bool", "int", "float", "double", "vec2i", "vec2f", "vec3f", "vec4f", "string", "propertyID" };
    m_shaderTypeStrings = { "Compute", "Fragment", "Geometry", "Vertex", "Tessellation control", "Tessellation evaluation" };
    m_tonemappingMethodText = { "None", "Simple reinhard", "Reinhard with white point", "Filmic tonemapping", "Uncharted 2", "Unreal 3", "ACES", "Lottes", "Uchimura" };
    m_textEditorLanguageTypeText = { "Text", "C++", "C", "Cs", "Python", "Lua", "Json", "SQL", "AngelScript", "GLSL", "HLSL" };
    m_textureWrapModeStrings = { "Clamp to border", "Clamp to edge", "Mirrored clamp to edge", "Mirrored repeat", "Repeat" };
    m_textureWrapModeTypes = { TextureWrapType::TextureWrapType_ClampToBorder, TextureWrapType::TextureWrapType_ClampToEdge, TextureWrapType::TextureWrapType_MirroredClampToEdge, TextureWrapType::TextureWrapType_MirroredRepeat, TextureWrapType::TextureWrapType_Repeat };

    m_currentlyActiveTextEditor = nullptr;

    TextEditor::SetDefaultPalette(TextEditor::PaletteId::Dark);
}

const glm::vec3 &EditorWindow::getVec3(const Observer *p_observer, BitMask p_changedBits)	const 
{ 
    if((p_changedBits & Systems::Changes::Type::Spatial) != 0)
    {
        return m_selectedEntity.m_spatialDataManager.getVec3(p_observer, p_changedBits);
    }
    else
    {
        switch(p_changedBits)
        {
            case Systems::Changes::Graphics::Color:
                {
                    switch(m_selectedEntity.m_lightType)
                    {
                        case LightComponent::LightComponentType::LightComponentType_directional:
                            return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color;
                            break;
                        case LightComponent::LightComponentType::LightComponentType_point:
                            return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color;
                            break;
                        case LightComponent::LightComponentType::LightComponentType_spot:
                            return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_color;
                            break;
                    }
                }
                break;
            case Systems::Changes::Physics::CollisionShapeSize:
                {
                    return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize;
                }
                break;
            case Systems::Changes::Physics::Gravity:
                {
                    return m_currentSceneData.m_gravity;
                }
                break;
        }
    }

    return NullObjects::NullVec3f;
}

const bool EditorWindow::getBool(const Observer *p_observer, BitMask p_changedBits) const 
{ 
    switch(p_changedBits)
    {
        case Systems::Changes::Generic::Active:
            {
                switch(p_observer->getObjectType())
                {
                    case Properties::PropertyID::SoundComponent:
                        return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_active;
                    case Properties::PropertyID::SoundListenerComponent:
                        return m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_active;

                    case Properties::PropertyID::CameraComponent:
                        return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_active;
                    case Properties::PropertyID::LightComponent:
                    case Properties::PropertyID::DirectionalLight:
                    case Properties::PropertyID::PointLight:
                    case Properties::PropertyID::SpotLight:
                        return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_active;
                    case Properties::PropertyID::ModelComponent:
                        return m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_active;
                    case Properties::PropertyID::ShaderComponent:
                        return m_selectedEntity.m_componentData.m_graphicsComponents.m_shaderConstructionInfo->m_active;

                    case Properties::PropertyID::GUISequenceComponent:
                        return m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_active;

                    case Properties::PropertyID::RigidBodyComponent:
                        return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_active;

                    case Properties::PropertyID::LuaComponent:
                        return m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_active;

                    case Properties::PropertyID::ObjectMaterialComponent:
                        return m_selectedEntity.m_componentData.m_worldComponents.m_objectMaterialConstructionInfo->m_active;
                    case Properties::PropertyID::SpatialComponent:
                        return m_selectedEntity.m_componentData.m_worldComponents.m_spatialConstructionInfo->m_active;
                    default:
                        return true;
                }
            }
            break;

        case Systems::Changes::Audio::Loop:
            {
                return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_loop;
            }
            break;

        case Systems::Changes::Audio::Spatialized:
            {
                return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_spatialized;
            }
            break;

        case Systems::Changes::Audio::StartPlaying:
            {
                return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_startPlaying;
            }
            break;

        case Systems::Changes::Physics::Kinematic:
            {
                return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_kinematic;
            }
            break;

        case Systems::Changes::GUI::StaticSequence:
            {
                return m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_staticSequence;
            }
            break;

        case Systems::Changes::Script::PauseInEditor:
            {
                return m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_pauseInEditor;
            }
            break;
    }

    return NullObjects::NullBool; 
}

const int EditorWindow::getInt(const Observer *p_observer, BitMask p_changedBits) const 
{ 
    switch(p_changedBits)
    {
    case Systems::Changes::Audio::ListenerID:
        {
            if(m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID >= 0)
                return m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID;
        }
        break;

    case Systems::Changes::Graphics::ActiveCameraID:
        {
            return m_currentSceneData.m_activeCameraID;
        }
        break;

    case Systems::Changes::Graphics::CameraID:
        {
            return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_cameraID;
        }
        break;

    case Systems::Changes::Physics::CollisionShapeType:
        {
            if(m_selectedEntity.m_collisionShapeType >= 0 && m_selectedEntity.m_collisionShapeType < RigidBodyComponent::CollisionShapeType::CollisionShapeType_NumOfTypes)
                return m_selectedEntity.m_collisionShapeType;
        }
        break;
    }

    return NullObjects::NullInt; 
}

const unsigned int EditorWindow::getUnsignedInt(const Observer *p_observer, BitMask p_changedBits) const
{
    switch(p_changedBits)
    {
        case Systems::Changes::Audio::SoundType:
            {
                if(m_selectedEntity.m_soundType >= 0 && m_selectedEntity.m_soundType < SoundComponent::SoundType_NumOfTypes)
                    return (unsigned int)m_selectedEntity.m_soundType;
            }
            break;

        case Systems::Changes::Audio::SoundSourceType:
            {
                if(m_selectedEntity.m_soundSourceType >= 0 && m_selectedEntity.m_soundSourceType < SoundComponent::SoundSourceType_NumOfTypes)
                    return (unsigned int)m_selectedEntity.m_soundSourceType;
            }
            break;

        case Systems::Changes::Graphics::LightType:
            {
                if(m_selectedEntity.m_lightType >= 0 && m_selectedEntity.m_lightType < LightComponent::LightComponentType::LightComponentType_spot + 1)
                    return (unsigned int)m_selectedEntity.m_lightType;
            }
            break;

        case Systems::Changes::Physics::CollisionShapeType:
            {
                if(m_selectedEntity.m_collisionShapeType >= 0 && m_selectedEntity.m_collisionShapeType < RigidBodyComponent::CollisionShapeType::CollisionShapeType_NumOfTypes)
                    return (unsigned int)m_selectedEntity.m_collisionShapeType;
            }
            break;

        case Systems::Changes::World::ObjectMaterialType:
            {
                if(m_selectedEntity.m_objectMaterialType >= 0 && m_selectedEntity.m_objectMaterialType < ObjectMaterialType::NumberOfMaterialTypes)
                    return (unsigned int)m_selectedEntity.m_objectMaterialType;
            }
            break;
    }

    return NullObjects::NullUnsignedInt;
}

const float EditorWindow::getFloat(const Observer *p_observer, BitMask p_changedBits) const
{
    switch(p_changedBits)
    {
        case Systems::Changes::Audio::Volume:
            {
                return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_volume;
            }
            break;

        case Systems::Changes::Audio::VolumeAmbient:
            {
                return m_currentSceneData.m_volume[AudioBusType::AudioBusType_Ambient];
            }
            break;

        case Systems::Changes::Audio::VolumeMaster:
            {
                return m_currentSceneData.m_volume[AudioBusType::AudioBusType_Master];
            }
            break;

        case Systems::Changes::Audio::VolumeMusic:
            {
                return m_currentSceneData.m_volume[AudioBusType::AudioBusType_Music];
            }
            break;

        case Systems::Changes::Audio::VolumeSFX:
            {
                return m_currentSceneData.m_volume[AudioBusType::AudioBusType_SFX];
            }
            break;

        case Systems::Changes::Graphics::CutoffAngle:
            {
                if(m_selectedEntity.m_lightType == LightComponent::LightComponentType::LightComponentType_spot)
                    return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle;
            }
            break;

        case Systems::Changes::Graphics::FOV:
            {
                return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_fov;
            }
            break;

        case Systems::Changes::Graphics::Intensity:
            {
                switch(m_selectedEntity.m_lightType)
                {
                    case LightComponent::LightComponentType::LightComponentType_directional:
                    case LightComponent::LightComponentType::LightComponentType_point:
                    case LightComponent::LightComponentType::LightComponentType_spot:
                        return m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity;
                        break;
                }
            }
            break;

        case Systems::Changes::Graphics::ZFar:
            {
                return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zFar;
            }
            break;

        case Systems::Changes::Graphics::ZNear:
            {
                return m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zNear;
            }
            break;

        case Systems::Changes::Physics::Friction:
            {
                return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction;
            }
            break;

        case Systems::Changes::Physics::RollingFriction:
            {
                return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_rollingFriction;
            }
            break;

        case Systems::Changes::Physics::SpinningFriction:
            {
                return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_spinningFriction;
            }
            break;

        case Systems::Changes::Physics::Mass:
            {
                return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_mass;
            }
            break;

        case Systems::Changes::Physics::Restitution:
            {
                return m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_restitution;
            }
            break;
    }

    return NullObjects::NullFloat;
}

const std::string &EditorWindow::getString(const Observer *p_observer, BitMask p_changedBits)	const 
{
    switch(p_changedBits)
    {
        case Systems::Changes::Generic::Name:
            {
                return m_selectedEntity.m_componentData.m_name;
            }
            break;

        case Systems::Changes::Audio::SoundName:
            {
                return m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundName;
            }
            break;

        case Systems::Changes::Script::Filename:
            {
                return m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename;
            }
            break;

        case Systems::Changes::World::PrefabName:
            {
                return m_selectedEntity.m_componentData.m_prefab;
            }
            break;
    }

    return NullObjects::NullString; 
}

EditorWindow::~EditorWindow()
{
    // If engine is still running
    if(Config::engineVar().running == true)
    {
        // Tell the renderer to draw the scene to the screen
        //sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTexture, (void *)false);
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

    // If the renderer completed a picking request, select the picked entity
    updatePickedEntitySelection();

    // Clear the entity and component creation / deletion pools left from the previous frame
    clearPerFramePools();

    if(m_showImGuiDemoWindow)
        ImGui::ShowDemoWindow();

    applyEditorStyle();

    //	 ____________________________
    //	|							 |
    //	|	    MAIN MENU BAR	     |
    //	|____________________________|
    //
    drawMainMenuBar();

    //	 ____________________________
    //	|							 |
    //	|	  SECONDARY MENU BAR	 |
    //	|____________________________|
    //
    drawSecondaryMenuBar(ImGui::GetMainViewport());

    // Process the pressed main menu buttons
    processMainMenuButton(m_activatedMainMenuButton);

    // Make the padding above tabs smaller
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 1));

    ImGuiWindowClass windowClassWithNoTabBar;
    windowClassWithNoTabBar.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    //	 ____________________________
    //	|							 |
    //	|	     LEFT WINDOW         |
    //	|____________________________|
    //
    drawLeftWindow(windowClassWithNoTabBar);

    //	 ____________________________
    //	|							 |
    //	|	     RIGHT WINDOW        |
    //	|____________________________|
    //
    drawRightWindow(windowClassWithNoTabBar);

    //	 ____________________________
    //	|							 |
    //	|	    BOTTOM WINDOW        |
    //	|____________________________|
    //
    drawBottomWindow(windowClassWithNoTabBar);

    //	 ____________________________
    //	|							 |
    //	|	    CENTER WINDOW        |
    //	|____________________________|
    //
    drawCenterWindow(windowClassWithNoTabBar);

    // Pop window padding style variable
    ImGui::PopStyleVar();

    // Handle the file browser dialog
    processFileBrowserResults();
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
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_LoadTexture2D, (void *)&m_buttonTextures[i]);

    // Tell the renderer to draw the scene to a texture instead of the screen
    sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_RenderToTexture, (void *)true);

    // If the GUI Sequence Enabled flag is set to false, tell the GUI scene to disable GUI Sequence components
    if(!m_GUISequenceEnabled)
        sendDataTo(m_systemScene, DataType::DataType_EnableGUISequence, (void *)m_GUISequenceEnabled);

    // If the LUA Scripting flag is set to false, tell the Scripting scene to disable LUA components
    if(!m_LUAScriptingEnabled)
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_EnableLuaScripting, (void *)m_LUAScriptingEnabled);

    // If the scene is paused, tell the Physics scene to pause the simulation
    if(m_sceneState == EditorSceneState::EditorSceneState_Pause)
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_SimulationActive, (void *)false);
}

void EditorWindow::applyEditorStyle()
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

    m_imguiStyle.TabRounding = 0.0f;
}

void EditorWindow::updatePickedEntitySelection()
{
    // If the renderer completed a picking request, select the picked entity (or clear the
    // selection if nothing was picked) on this frame
    {
        EntityID pickedEntityID = NULL_ENTITY_ID;
        if(getRendererScene()->consumePickedEntity(pickedEntityID))
        {
            m_nextEntityIDToSelect = pickedEntityID;
            m_pendingEntityToSelect = true;
        }
    }

    // If there is a pending entity to select, iterate over each entity and find the one that needs to be selected
    if(m_pendingEntityToSelect)
    {
        if(m_nextEntityToSelect != nullptr)
        {
            m_nextEntityIDToSelect = m_nextEntityToSelect->m_id;
            m_nextEntityToSelect = nullptr;
        }

        // A null entity ID means that nothing was picked, so clear the current selection
        if(m_nextEntityIDToSelect == NULL_ENTITY_ID)
            m_selectedEntity.unselect();
        else
            for(decltype(m_entityList.size()) size = m_entityList.size(), i = 0; i < size; i++)
                if(m_entityList[i].m_entityID == m_nextEntityIDToSelect)
                    m_selectedEntity.setEntity(m_entityList[i].m_entityID);

        m_pendingEntityToSelect = false;
    }
}

void EditorWindow::clearPerFramePools()
{
    // Clear the entity and component creation / deletion pools left from the previous frame
    clearEntityAndComponentPool();
    clearConstructionInfoPool();
    resetActivateAllComponentFlags();
}

WorldScene *EditorWindow::getWorldScene() const
{
    return static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World));
}

RendererScene *EditorWindow::getRendererScene() const
{
    return static_cast<RendererScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics));
}

entt::basic_registry<EntityID> &EditorWindow::getEntityRegistry() const
{
    return static_cast<WorldScene *>(m_systemScene->getSceneLoader()->getSystemScene(Systems::World))->getEntityRegistry();
}

void EditorWindow::sendDataTo(SystemScene *p_observer, const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
    m_systemScene->getSceneLoader()->getChangeController()->sendData(p_observer, p_dataType, p_data, p_deleteAfterReceiving);
}

void EditorWindow::sendDataTo(SystemObject *p_observer, const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
    m_systemScene->getSceneLoader()->getChangeController()->sendData(p_observer, p_dataType, p_data, p_deleteAfterReceiving);
}

EntityID EditorWindow::findNextAvailableEntityID(const EntityID p_startAfterEntityID)
{
    EntityID newEntityID = p_startAfterEntityID + 1;
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

    return newEntityID;
}

std::string EditorWindow::makeUniqueEntityName(const std::string &p_baseName, const int p_startIndex)
{
    WorldScene *worldScene = getWorldScene();

    std::string baseEntityName = p_baseName;

    // If the entity name ends with ' 1', replace the existing number
    if(baseEntityName.size() > 2 && baseEntityName[baseEntityName.size() - 1] == '1' && baseEntityName[baseEntityName.size() - 2] == ' ')
    {
        baseEntityName.pop_back();  // Remove '1'
        baseEntityName.pop_back();  // Remove ' '
    }

    int newEntityNameCount = p_startIndex;
    std::string newEntityName = baseEntityName + " " + Utilities::toString(newEntityNameCount);

    // Keep increasing the number at the end until there is no other entity with the same name
    while(worldScene->getEntity(newEntityName) != NULL_ENTITY_ID)
    {
        newEntityNameCount++;
        newEntityName = baseEntityName + " " + Utilities::toString(newEntityNameCount);
    }

    return newEntityName;
}

void EditorWindow::drawExportPrefabFileBrowser()
{
    // Only open the file browser if it's not opened already
    if(m_currentlyOpenedFileBrowser == FileBrowserActivated::FileBrowserActivated_None)
    {
        // Set the file browser activation to Save Scene
        m_currentlyOpenedFileBrowser = FileBrowserActivated::FileBrowserActivated_SavePrefabFile;

        // Define file browser variables
        m_fileBrowserDialog.m_definedFilename = m_selectedEntity.m_componentData.m_name + ".prefab";
        m_fileBrowserDialog.m_filter = ".prefab,.*";
        m_fileBrowserDialog.m_title = "Save prefab";
        m_fileBrowserDialog.m_name = "SavePrefabFileDialog";
        m_fileBrowserDialog.m_rootPath = Config::filepathVar().prefab_path;
        m_fileBrowserDialog.m_flags = FileBrowserDialog::FileBrowserDialogFlags::FileBrowserDialogFlags_ConfirmOverwrite;

        // Tell the GUI scene to open the file browser
        sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
    }
}

void EditorWindow::duplicateEntity(const EntityID p_entityID)
{
    // Construction info for the new entity
    ComponentsConstructionInfo *newEntityConstructionInfo = new ComponentsConstructionInfo();

    // Get the construction info of the current entity
    WorldScene *worldScene = getWorldScene();
    worldScene->exportEntity(p_entityID, *newEntityConstructionInfo);

    // Set the new entity name by adding a count at the end and checking if an entity of the same name doesn't exist
    newEntityConstructionInfo->m_name = makeUniqueEntityName(newEntityConstructionInfo->m_name, 2);

    // Assign a next available entity ID (start the available ID search from the next ID after the parent)
    newEntityConstructionInfo->m_id = findNextAvailableEntityID(newEntityConstructionInfo->m_parent);

    sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_CreateEntity, (void *)newEntityConstructionInfo, false);

    // Make the new entity be selected next frame
    m_nextEntityIDToSelect = newEntityConstructionInfo->m_id;
    m_pendingEntityToSelect = true;

    // Add the new entity construction info to the pool (so it will be deleted the next frame)
    m_componentConstructionInfoPool.push_back(newEntityConstructionInfo);
}

void EditorWindow::deleteEntityAndChildren(const EntityID p_entityID)
{
    // Get entity children
    std::vector<EntityID> childrenEntityIDs;
    if(p_entityID != 0)
        childrenEntityIDs.push_back(p_entityID);
    if(auto *entityListEntry = getEntityListEntry(p_entityID); entityListEntry != nullptr)
        getEntityChildren(childrenEntityIDs, *entityListEntry);

    // Do not allow the deletion of root entity
    for(decltype(childrenEntityIDs.size()) i = 0, size = childrenEntityIDs.size(); i < size; i++)
    {
        // Create a container with the entity ID and the component type, add it to the pool (so it can be deleted next frame) and send a Delete Entity change with the attached container
        EntityAndComponent *deleteComponentData = new EntityAndComponent(childrenEntityIDs[i], ComponentType::ComponentType_Entity);
        m_entityAndComponentPool.push_back(deleteComponentData);
        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_DeleteEntity, (void *)deleteComponentData, false);
    }
}

EditorWindow::EntityListEntry *EditorWindow::getEntityListEntry(const EntityID p_entityID)
{
    for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size; i++)
    {
        if(m_entityList[i].m_entityID == p_entityID)
        {
            return &m_entityList[i];
        }
    }

    return nullptr;
}

void EditorWindow::getEntityChildren(std::vector<EntityID> &p_children, const EntityListEntry &p_entityHierarchyEntry)
{
    for(decltype(m_entityList.size()) i = 0, size = m_entityList.size(); i < size; i++)
    {
        if(m_entityList[i].m_parentEntityID == p_entityHierarchyEntry.m_entityID)
        {
            p_children.push_back(m_entityList[i].m_entityID);

            // Do not process the root node as it will cause an infinite loop
            if(p_entityHierarchyEntry.m_entityID != 0)
                getEntityChildren(p_children, m_entityList[i]);
        }
    }
}

std::string EditorWindow::getTextureFormatString(const TextureFormat p_textureFormat) const
{
    switch(p_textureFormat)
    {
    case TextureFormat_Red:
        return "Red";
        break;
    case TextureFormat_Green:
        return "Green";
        break;
    case TextureFormat_Blue:
        return "Blue";
        break;
    case TextureFormat_Alpha:
        return "Alpha";
        break;
    case TextureFormat_R:
        return "R";
        break;
    case TextureFormat_RG:
        return "RG";
        break;
    case TextureFormat_RGB:
        return "RGB";
        break;
    case TextureFormat_RGBA:
        return "RGBA";
        break;
    }
    return "";
}

std::string EditorWindow::getTextureDataTypeString(const TextureDataType p_textureDataType) const
{
    switch(p_textureDataType)
    {
        case TextureDataType::TextureDataType_Short:
            return "Short";
            break;
        case TextureDataType::TextureDataType_Float:
            return "Float";
            break;
        case TextureDataType::TextureDataType_Int:
            return "Integer";
            break;
        case TextureDataType::TextureDataType_UnsignedByte:
            return "Unsigned byte";
            break;
    }
    return "";
}

std::string EditorWindow::getTextureDataFormat(const TextureDataFormat p_textureDataFormat) const
{
    switch(p_textureDataFormat)
    {
        case TextureDataFormat::TextureDataFormat_R8:
            return "R8";
            break;
        case TextureDataFormat::TextureDataFormat_R16:
            return "R16";
            break;
        case TextureDataFormat::TextureDataFormat_R16F:
            return "R16F";
            break;
        case TextureDataFormat::TextureDataFormat_R32F:
            return "R32F";
            break;
        case TextureDataFormat::TextureDataFormat_RG8:
            return "RG8";
            break;
        case TextureDataFormat::TextureDataFormat_RG16:
            return "RG16";
            break;
        case TextureDataFormat::TextureDataFormat_RG16F:
            return "RG16F";
            break;
        case TextureDataFormat::TextureDataFormat_RG32F:
            return "RG32F";
            break;
        case TextureDataFormat::TextureDataFormat_RGB8:
            return "RGB8";
            break;
        case TextureDataFormat::TextureDataFormat_RGB16:
            return "RGB16";
            break;
        case TextureDataFormat::TextureDataFormat_RGB16F:
            return "RGB16F";
            break;
        case TextureDataFormat::TextureDataFormat_RGB32F:
            return "RGB32F";
            break;
        case TextureDataFormat::TextureDataFormat_RGBA8:
            return "RGBA8";
            break;
        case TextureDataFormat::TextureDataFormat_RGBA16:
            return "RGBA16";
            break;
        case TextureDataFormat::TextureDataFormat_RGBA16SN:
            return "RGBA16 SNORM";
            break;
        case TextureDataFormat::TextureDataFormat_RGBA16F:
            return "RGBA16F";
            break;
        case TextureDataFormat::TextureDataFormat_RGBA32F:
            return "RGBA32F";
            break;
        case TextureDataFormat::TextureDataFormat_R16I:
            return "R16I";
            break;
        case TextureDataFormat::TextureDataFormat_R32I:
            return "R32I";
            break;
        case TextureDataFormat::TextureDataFormat_R16UI:
            return "R16UI";
            break;
        case TextureDataFormat::TextureDataFormat_R32UI:
            return "R32UI";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_RGB:
            return "COMPRESSED RGB";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_RGBA:
            return "COMPRESSED RGBA";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGB:
            return "DXT1 RGB (BC1)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGBA:
            return "DXT1 RGBA (BC1)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT3_RGBA:
            return "DXT3 RGBA (BC2)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT5_RGBA:
            return "DXT5 RGBA (BC3)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC1_R:
            return "RGTC1 R (BC4)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC2_RG:
            return "RGTC2 RG (BC5)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_BPTC_RGBA:
            return "BPTC RGBA (BC7)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_R:
            return "EAC R11 (ETC2)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_RG:
            return "EAC RG11 (ETC2)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGB:
            return "ETC2 RGB8 (ETC2)";
            break;
        case TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGBA:
            return "ETC2 RGBA8 (ETC2)";
            break;
    }
    return "";
}
