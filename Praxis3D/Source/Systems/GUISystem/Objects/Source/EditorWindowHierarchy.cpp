#define IMGUI_DEFINE_MATH_OPERATORS

#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"

#include "imgui_internal.h"
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"
#include "imgui/imgui_stdlib.h"

#include "Systems/PhysicsSystem/Components/Include/CollisionShapeComponent.hpp"
#include "Systems/PhysicsSystem/Components/Include/RigidBodyComponent.hpp"

void EditorWindow::drawLeftWindow(const ImGuiWindowClass &p_windowClassWithNoTabBar)
{
            auto &entityRegistry = getEntityRegistry();
            ImGui::SetNextWindowClass(&p_windowClassWithNoTabBar);
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
                                                sendChangeTo(this, lightComponent, Systems::Changes::Generic::Active);
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
                                                sendChangeTo(this, modelComponent, Systems::Changes::Generic::Active);
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
                                                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Generic::Active);
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
                                                sendChangeTo(this, lightComponent, Systems::Changes::Generic::Active);
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
                                                sendChangeTo(this, modelComponent, Systems::Changes::Generic::Active);
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
                                                sendChangeTo(this, rigidBodyComponent, Systems::Changes::Generic::Active);
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
                                    m_newEntityConstructionInfo->m_id = findNextAvailableEntityID(m_newEntityConstructionInfo->m_parent);

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
                                            sendDataTo(m_systemScene, DataType::DataType_FileBrowserDialog, (void *)&m_fileBrowserDialog);
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

                                        sendDataTo(m_systemScene->getSceneLoader()->getSystemScene(Systems::World), DataType::DataType_CreateEntity, (void *)m_newEntityConstructionInfo, false);

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
                auto parentlessEntity = parentlessEntityList.begin();
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
