#define IMGUI_DEFINE_MATH_OPERATORS

#include "Engine/Include/EngineDefinitions.hpp"
#include "Systems/GUISystem/Objects/Include/EditorWindow.hpp"

#include "imgui_internal.h"
#include "Systems/WorldSystem/Include/WorldScene.hpp"
#include "Systems/RendererSystem/Include/RendererScene.hpp"
#include "Definitions/Include/CommonDefinitions.hpp"

void EditorWindow::processFileBrowserResults()
{
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
