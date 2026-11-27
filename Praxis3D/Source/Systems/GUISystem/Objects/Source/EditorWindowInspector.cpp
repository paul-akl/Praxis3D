#define IMGUI_DEFINE_MATH_OPERATORS

#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"

#include "imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"

#include "Systems/AudioSystem/Components/Include/ImpactSoundComponent.hpp"
#include "Systems/AudioSystem/Components/Include/SoundComponent.hpp"
#include "Systems/AudioSystem/Components/Include/SoundListenerComponent.hpp"
#include "Systems/GUISystem/Components/Include/GUISequenceComponent.hpp"
#include "Systems/PhysicsSystem/Components/Include/CollisionShapeComponent.hpp"
#include "Systems/PhysicsSystem/Components/Include/RigidBodyComponent.hpp"
#include "Systems/RendererSystem/Components/Include/CameraComponent.hpp"
#include "Systems/RendererSystem/Components/Include/LightComponent.hpp"
#include "Systems/RendererSystem/Components/Include/ModelComponent.hpp"
#include "Systems/RendererSystem/Components/Include/ShaderComponent.hpp"
#include "Systems/ScriptingSystem/Components/Include/LuaComponent.hpp"
#include "Systems/WorldSystem/Components/Include/ObjectMaterialComponent.hpp"
#include "Systems/WorldSystem/Components/Include/SpatialComponent.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imspinner.h>

void EditorWindow::drawRightWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar)
{
    ImGui::SetNextWindowClass(&p_windowClassWithNoTabBar);
    if(ImGui::Begin("##RightWindow", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
    {
        if(ImGui::BeginTabBar("##RightWindowTabBar", ImGuiTabBarFlags_None))
        {
            drawInspectorTab();
            drawSceneSettingsTab();

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void EditorWindow::drawInspectorTab()
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
                drawMetadataInspector(inputWidgetOffset);
                drawSpatialInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);
                drawObjectMaterialInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);

                // GRAPHICS COMPONENTS
                drawCameraInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);
                drawLightInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);
                drawModelInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);
                drawShaderInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);

                // PHYSICS COMPONENTS
                drawCollisionShapeInspector();
                drawRigidBodyInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);

                // AUDIO COMPONENTS
                drawSoundInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);
                drawSoundListenerInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);

                // SCRIPTING COMPONENTS
                drawLuaInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);

                // GUI COMPONENTS
                drawGuiSequenceInspector(inputWidgetOffset, headerOffsetAfterDeleteButton);

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

                                sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_CreateComponent, (void *)newComponentInfo, false);
                            }
                            else
                                delete newComponentInfo;
                        }
                    }

                    ImGui::PopStyleVar(2); //ImGuiStyleVar_SelectableTextAlign, ImGuiStyleVar_FramePadding
                    ImGui::EndPopup();
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndTabItem();
    }
}

void EditorWindow::drawCollisionShapeInspector()
{
    auto &entityRegistry = getEntityRegistry();

    auto *collisionShapeComponent = entityRegistry.try_get<CollisionShapeComponent>(m_selectedEntity.m_entityID);
    if(collisionShapeComponent != nullptr)
    {
        if(ImGui::CollapsingHeader(GetString(Properties::PropertyID::CollisionShapeComponent), ImGuiTreeNodeFlags_DefaultOpen))
        {

        }
    }
}

void EditorWindow::drawMetadataInspector(const float inputWidgetOffset)
{
    auto &entityRegistry = getEntityRegistry();

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
                sendChangeTo(this, metadataComponent, Systems::Changes::World::PrefabName);
            else
                m_selectedEntity.m_componentData.m_prefab = metadataComponent->getPrefabName();

            const std::string parentSelectionPopupName = "##ParentSelectionPopup";

            // Draw NAME
            drawLeftAlignedLabelText("Name:", inputWidgetOffset);
            if(ImGui::InputText("##NameStringInput", &m_selectedEntity.m_componentData.m_name, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // If the prefab name was changed, send a notification to the Metadata Component
                sendChangeTo(this, metadataComponent, Systems::Changes::Generic::Name);
            }

            // Draw PREFAB
            if(!m_selectedEntity.m_componentData.m_prefab.empty())
            {
                drawLeftAlignedLabelText("Prefab:", inputWidgetOffset, calcTextSizedButtonOffset(2) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                if(ImGui::InputText("##PrefabStringInput", &m_selectedEntity.m_componentData.m_prefab, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    // If the prefab name was changed, send a notification to the Metadata Component
                    sendChangeTo(this, metadataComponent, Systems::Changes::World::PrefabName);
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
                        sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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
                    sendChangeTo(this, metadataComponent, Systems::Changes::World::PrefabName);
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
                            sendChangeTo(this, metadataComponent, Systems::Changes::World::PrefabName);
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
}

void EditorWindow::drawSpatialInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, spatialComponent, Systems::Changes::Generic::Active);
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
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                        else
                            sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
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
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                        else
                            sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::LocalTransformNoScale);
                    }
                    captureMouseWhileItemActive();

                    // Draw SCALE
                    drawLeftAlignedLabelText("Scale:", inputWidgetOffset);
                    if(ImGui::DragFloat3("##LocalScaleDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_scale), Config::GUIVar().editor_float_slider_speed, 0.01f, 10000.0f))
                    {
                        // If the scale vector was changed, send a notification to the spatial component
                        sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::LocalScale);
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
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Spatial::WorldTransformNoScale);
                        else
                            sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::WorldTransformNoScale);*/
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
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Spatial::WorldTransformNoScale);
                        else
                            sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::WorldTransformNoScale);*/
                    }

                    // Draw SCALE
                    drawLeftAlignedLabelText("Scale:", inputWidgetOffset);
                    if(ImGui::DragFloat3("##WorldScaleDrag", glm::value_ptr(m_selectedEntity.m_spatialDataManager.getLocalSpaceDataNonConst().m_spatialData.m_scale), Config::GUIVar().editor_float_slider_speed, 0.01f, 10000.0f))
                    {
                        // If the scale vector was changed, send a notification to the spatial component
                        //sendChangeTo(this, spatialComponent, Systems::Changes::Spatial::LocalScale);
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
    }
}

void EditorWindow::drawObjectMaterialInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, objectMaterialComponent, Systems::Changes::Generic::Active);
            }

            // Draw OBJECT MATERIAL TYPE
            drawLeftAlignedLabelText("Material type:", inputWidgetOffset);
            if(ImGui::Combo("##ObjectMaterialTypePicker", &m_selectedEntity.m_objectMaterialType, &m_physicalMaterialProperties[0], ObjectMaterialType::NumberOfMaterialTypes))
            {
                // If the object material type was changed, send a notification to the Object Material Component
                sendChangeTo(this, objectMaterialComponent, Systems::Changes::World::ObjectMaterialType);
            }
        }
    }
}

void EditorWindow::drawCameraInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, cameraComponent, Systems::Changes::Generic::Active);
            }                                

            // Draw CAMERA ID
            drawLeftAlignedLabelText("Camera ID:", inputWidgetOffset);
            if(ImGui::InputInt("##CameraIDInput", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_cameraID))
            {
                // If the camera ID was changed, send a notification to the Camera Component
                sendChangeTo(this, cameraComponent, Systems::Changes::Graphics::CameraID);
            }

            // Draw FOV
            drawLeftAlignedLabelText("FOV:", inputWidgetOffset);
            if(ImGui::DragFloat("##CameraFOVDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_fov, 1.0f, 1.0f, 180.0f, "%.0f"))
            {
                // If the FOV was changed, send a notification to the Camera Component
                sendChangeTo(this, cameraComponent, Systems::Changes::Graphics::FOV);
            }
            captureMouseWhileItemActive();

            // Draw FAR PLANE
            drawLeftAlignedLabelText("Far plane:", inputWidgetOffset);
            if(ImGui::DragFloat("##CameraFarPlaneDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zFar, 0.1f, 0.0f, 100000.0f, "%.5f"))
            {
                // If the far plane was changed, send a notification to the Camera Component
                sendChangeTo(this, cameraComponent, Systems::Changes::Graphics::ZFar);
            }
            captureMouseWhileItemActive();

            // Draw NEAR PLANE
            drawLeftAlignedLabelText("Near plane:", inputWidgetOffset);
            if(ImGui::DragFloat("##CameraNearPlaneDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_cameraConstructionInfo->m_zNear, 0.0001f, 0.0f, 100000.0f, "%.5f"))
            {
                // If the near plane was changed, send a notification to the Camera Component
                sendChangeTo(this, cameraComponent, Systems::Changes::Graphics::ZNear);
            }
            captureMouseWhileItemActive();
        }
    }
}

void EditorWindow::drawLightInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, lightComponent, Systems::Changes::Generic::Active);
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
                    sendChangeTo(this, lightComponent, Systems::Changes::Graphics::LightType);
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
                                sendChangeTo(this, lightComponent, Systems::Changes::Graphics::Color);
                            }

                            // Draw INTENSITY
                            drawLeftAlignedLabelText("Intensity:", inputWidgetOffset);
                            if(ImGui::DragFloat("##DirectionalLightIntensityDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                            {
                                // If the light intensity was changed, send a notification to the Light Component
                                sendChangeTo(this, lightComponent, Systems::Changes::Graphics::Intensity);
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
                                sendChangeTo(this, lightComponent, Systems::Changes::Graphics::Color);
                            }

                            // Draw INTENSITY
                            drawLeftAlignedLabelText("Intensity:", inputWidgetOffset);
                            if(ImGui::DragFloat("##PointLightIntensityDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                            {
                                // If the light intensity was changed, send a notification to the Light Component
                                sendChangeTo(this, lightComponent, Systems::Changes::Graphics::Intensity);
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
                                sendChangeTo(this, lightComponent, Systems::Changes::Graphics::Color);
                            }

                            // Draw INTENSITY
                            drawLeftAlignedLabelText("Intensity:", inputWidgetOffset);
                            if(ImGui::DragFloat("##SpotLightIntensityDrag", &m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_intensity, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
                            {
                                // If the light intensity was changed, send a notification to the Light Component
                                sendChangeTo(this, lightComponent, Systems::Changes::Graphics::Intensity);
                            }
                            captureMouseWhileItemActive();

                            // Draw CUTOFF ANGLE
                            auto cutoffAngleInDegrees = glm::degrees(m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle);
                            drawLeftAlignedLabelText("Cutoff angle:", inputWidgetOffset);
                            if(ImGui::DragFloat("##SpotLightCutoffAngleDrag", &cutoffAngleInDegrees, Config::GUIVar().editor_float_slider_speed, 0.0f, 360.0f))
                            {
                                m_selectedEntity.m_componentData.m_graphicsComponents.m_lightConstructionInfo->m_cutoffAngle = glm::radians(cutoffAngleInDegrees);

                                // If the light cutoff angle was changed, send a notification to the Light Component
                                sendChangeTo(this, lightComponent, Systems::Changes::Graphics::CutoffAngle);
                            }
                            captureMouseWhileItemActive();
                        }
                    }
                    break;
            }
        }
    }
}

void EditorWindow::drawModelInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                    sendChangeTo(this, modelComponent, Systems::Changes::Generic::Active);
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
                            sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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
                                            sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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

                    sendDataTo(modelComponent, DataType::DataType_ModelsProperties, (void *)&m_selectedEntity.m_componentData.m_graphicsComponents.m_modelConstructionInfo->m_modelsProperties);
                }
            }
            else
            {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 32.0f) / 2.0f);
                ImSpinner::SpinnerFadeDots("##ModelLoadingSpinner", 32.0f, 4.0f, ImSpinner::white, 16.0f, 8);
            }
        }
    }
}

void EditorWindow::drawShaderInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Graphics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, shaderComponent, Systems::Changes::Generic::Active);
            }
        }
    }

}

void EditorWindow::drawRigidBodyInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Physics), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Generic::Active);
            }

            // Draw KINEMATIC
            drawLeftAlignedLabelText("Kinematic:", inputWidgetOffset);
            if(ImGui::Checkbox("##KinematicCheck", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_kinematic))
            {
                // If the kinematic flag was changed, send a notification to the Rigid Body Component
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::Kinematic);
            }

            // Draw MASS
            drawLeftAlignedLabelText("Mass:", inputWidgetOffset);
            if(ImGui::DragFloat("##MassDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_mass, Config::GUIVar().editor_float_slider_speed, 0.0f, 10000.0f))
            {
                // If the mass was changed, send a notification to the Rigid Body Component
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::Mass);
            }
            captureMouseWhileItemActive();

            // Draw FRICTION
            drawLeftAlignedLabelText("Friction:", inputWidgetOffset);
            if(ImGui::DragFloat("##FrictionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_friction, Config::GUIVar().editor_float_slider_speed))
            {
                // If the friction was changed, send a notification to the Rigid Body Component
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::Friction);
            }
            captureMouseWhileItemActive();

            // Draw ROLLING FRICTION
            drawLeftAlignedLabelText("Rolling friction:", inputWidgetOffset);
            if(ImGui::DragFloat("##RollingFrictionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_rollingFriction, Config::GUIVar().editor_float_slider_speed))
            {
                // If the rolling friction was changed, send a notification to the Rigid Body Component
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::RollingFriction);
            }
            captureMouseWhileItemActive();

            // Draw SPINNING FRICTION
            drawLeftAlignedLabelText("Spinning friction:", inputWidgetOffset);
            if(ImGui::DragFloat("##SpinningFrictionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_spinningFriction, Config::GUIVar().editor_float_slider_speed))
            {
                // If the spinning friction was changed, send a notification to the Rigid Body Component
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::SpinningFriction);
            }
            captureMouseWhileItemActive();

            // Draw RESTITUTION
            drawLeftAlignedLabelText("Restitution:", inputWidgetOffset);
            if(ImGui::DragFloat("##RestitutionDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_restitution, Config::GUIVar().editor_float_slider_speed))
            {
                // If the m_restitution was changed, send a notification to the Rigid Body Component
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::Restitution);
            }
            captureMouseWhileItemActive();

            // Draw COLLISION SHAPE TYPE
            drawLeftAlignedLabelText("Collision shape:", inputWidgetOffset);
            if(ImGui::Combo("##CollisionShapePicker", &m_selectedEntity.m_collisionShapeType, &(rigidBodyComponent->getCollisionTypeText()[0]), RigidBodyComponent::CollisionShapeType::CollisionShapeType_NumOfTypes))
            {
                // If the collision shape type was changed, send a notification to the Rigid Body Component
                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeType);
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
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
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
                        // Get the collision shape data
                        m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize = rigidBodyComponent->getCollisionShapeSize();

                        // Draw CYLINDER RADIUS
                        drawLeftAlignedLabelText("Cylinder radius:", inputWidgetOffset);
                        if(ImGui::DragFloat("##CylinderRadiusDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.x, Config::GUIVar().editor_float_slider_speed))
                        {
                            m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.z = m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.x;

                            // If the cylinder half extents size vector was changed, send a notification to the Rigid Body Component
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
                        }
                        captureMouseWhileItemActive();

                        // Draw CYLINDER HEIGHT
                        drawLeftAlignedLabelText("Cylinder height:", inputWidgetOffset);
                        if(ImGui::DragFloat("##CylinderHeightDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.y, Config::GUIVar().editor_float_slider_speed))
                        {
                            // If the cylinder half extents size vector was changed, send a notification to the Rigid Body Component
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
                        }
                        captureMouseWhileItemActive();
                    }
                    break;
                case RigidBodyComponent::CollisionShapeType::CollisionShapeType_Sphere:
                    {
                        // Get the collision shape data
                        m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize = rigidBodyComponent->getCollisionShapeSize();

                        // Draw SPHERE RADIUS
                        drawLeftAlignedLabelText("Sphere radius:", inputWidgetOffset);
                        if(ImGui::DragFloat("##SphereRadiusDrag", &m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.x, Config::GUIVar().editor_float_slider_speed))
                        {
                            m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.y = m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.x;
                            m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.z = m_selectedEntity.m_componentData.m_physicsComponents.m_rigidBodyConstructionInfo->m_collisionShapeSize.x;

                            // If the sphere size vector was changed, send a notification to the Rigid Body Component
                            sendChangeTo(this, rigidBodyComponent, Systems::Changes::Physics::CollisionShapeSize);
                        }
                        captureMouseWhileItemActive();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void EditorWindow::drawSoundInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::SoundName);
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
                sendChangeTo(this, soundComponent, Systems::Changes::Generic::Active);
            }

            // Draw SOUND FILENAME
            drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
            if(ImGui::InputText("##SoundFilenameInput", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_soundName, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // If the sound filename was changed, send a notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::SoundName);
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
                    sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                }
            }

            // Draw RELOAD button
            ImGui::SameLine(calcTextSizedButtonOffset(0));
            if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##SoundFileReloadButton", "Reload the audio file"))
            {
                // Send a reload notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::Reload);
            }

            // Draw SOUND TYPE
            drawLeftAlignedLabelText("Sound type:", inputWidgetOffset);
            if(ImGui::Combo("##SoundTypePicker", &m_selectedEntity.m_soundType, &(soundComponent->getSoundTypeText()[0]), SoundComponent::SoundType_NumOfTypes))
            {
                // If the sound type was changed, send a notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::SoundType);
            }

            // Draw SOUND SOURCE TYPE
            drawLeftAlignedLabelText("Sound source type:", inputWidgetOffset);
            if(ImGui::Combo("##SoundSourceTypePicker", &m_selectedEntity.m_soundSourceType, &(soundComponent->getSoundSourceTypeText()[0]), SoundComponent::SoundSourceType_NumOfTypes))
            {
                // If the sound source type was changed, send a notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::SoundSourceType);
            }

            // Draw VOLUME
            drawLeftAlignedLabelText("Volume:", inputWidgetOffset);
            if(ImGui::DragFloat("##SoundVolumeDrag", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_volume, Config::GUIVar().editor_float_slider_speed, 0.0f, 1.0f))
            {
                // If the sound volume was changed, send a notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::Volume);
            }
            captureMouseWhileItemActive();

            // Draw LOOP
            drawLeftAlignedLabelText("Loop:", inputWidgetOffset);
            if(ImGui::Checkbox("##SoundLoopCheckbox", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_loop))
            {
                // If the loop flag was changed, send a notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::Loop);
            }

            // Draw SPATIALIZED
            drawLeftAlignedLabelText("Spatialized:", inputWidgetOffset);
            if(ImGui::Checkbox("##SoundSpatializedCheckbox", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_spatialized))
            {
                // If the spatialized flag was changed, send a notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::Spatialized);
            }

            // Draw START PLAYING
            drawLeftAlignedLabelText("Start playing:", inputWidgetOffset);
            if(ImGui::Checkbox("##SoundStartPlayingCheckbox", &m_selectedEntity.m_componentData.m_audioComponents.m_soundConstructionInfo->m_startPlaying))
            {
                // If the start-playing flag was changed, send a notification to the Sound Component
                sendChangeTo(this, soundComponent, Systems::Changes::Audio::StartPlaying);
            }
        }
    }
}

void EditorWindow::drawSoundListenerInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Audio), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, soundListenerComponent, Systems::Changes::Generic::Active);
            }

            // Draw SOUND LISTENER ID
            drawLeftAlignedLabelText("Listener ID:", inputWidgetOffset);
            if(ImGui::InputInt("##ListenerIDInput", &m_selectedEntity.m_componentData.m_audioComponents.m_soundListenerConstructionInfo->m_listenerID))
            {
                // If the sound listener ID was changed, send a notification to the Sound Listener Component
                sendChangeTo(this, soundListenerComponent, Systems::Changes::Audio::ListenerID);
            }
        }
    }
}

void EditorWindow::drawLuaInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::Script), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                    sendChangeTo(this, luaComponent, Systems::Changes::Script::Filename);
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
                    sendChangeTo(this, luaComponent, Systems::Changes::Generic::Active);
                }

                // Draw PAUSE IN EDITOR
                drawLeftAlignedLabelText("Pause in editor:", inputWidgetOffset);
                if(ImGui::Checkbox("##LUAPauseInEditorCheckbox", &m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_pauseInEditor))
                {
                    // If the pause-in-editor flag was changed, send a notification to the LUA Component
                    sendChangeTo(this, luaComponent, Systems::Changes::Script::PauseInEditor);
                }

                // Draw LUA FILENAME
                drawLeftAlignedLabelText("Filename:", inputWidgetOffset, calcTextSizedButtonOffset(1) - inputWidgetOffset - m_imguiStyle.FramePadding.x);
                if(ImGui::InputText("##LuaScriptFilenameInput", &m_selectedEntity.m_componentData.m_scriptComponents.m_luaConstructionInfo->m_luaScriptFilename, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    sendChangeTo(this, luaComponent, Systems::Changes::Script::Filename);
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
                        sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
                    }
                }

                // Draw RELOAD button
                ImGui::SameLine(calcTextSizedButtonOffset(0));
                if(drawTextSizedButton(m_buttonTextures[ButtonTextureType::ButtonTextureType_Reload], "##LuaScriptReloadButton", "Reload the Lua file script"))
                {
                    // Send a reload notification to the LUA Component
                    sendChangeTo(this, luaComponent, Systems::Changes::Script::Reload);
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
                    sendDataTo(luaComponent, DataType::DataType_LuaVariables, (void *)&m_selectedEntity.m_luaVariables, false);
                }

                ImGui::PopStyleVar();
            }
        }
    }
}

void EditorWindow::drawGuiSequenceInspector(const float inputWidgetOffset, const float headerOffsetAfterDeleteButton)
{
    auto &entityRegistry = getEntityRegistry();

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
            sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::GUI), DataType::DataType_DeleteComponent, (void *)deleteComponentData, false);
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
                sendChangeTo(this, guiSequenceComponent, Systems::Changes::Generic::Active);
            }

            // Draw STATIC
            drawLeftAlignedLabelText("Static sequence:", inputWidgetOffset);
            if(ImGui::Checkbox("##StaticSequenceCheck", &m_selectedEntity.m_componentData.m_guiComponents.m_guiSequenceConstructionInfo->m_staticSequence))
            {
                // If the static sequence flag was changed, send a notification to the GUI Sequence Component
                sendChangeTo(this, guiSequenceComponent, Systems::Changes::GUI::StaticSequence);
            }
        }
    }
}
