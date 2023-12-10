
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include "WorldScene.h"
#include "ComponentConstructorInfo.h"
#include "EditorWindow.h"
#include "GUIHandlerLocator.h"
#include "GUIScene.h"
#include "NullSystemObjects.h"
#include "TaskManagerLocator.h"

GUIScene::GUIScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::GUI)
{
	 m_GUITask = nullptr;
	 m_editorWindow = nullptr;
	 m_GUISequenceEnabled = true;
}

GUIScene::~GUIScene()
{
	if(m_GUITask != nullptr)
		delete m_GUITask;

	if(m_editorWindow != nullptr)
		delete m_editorWindow;
}

ErrorCode GUIScene::init()
{
	m_GUITask = new GUITask(this);

	// Set the text color of directories in the file browser
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(Config::GUIVar().gui_file_dialog_dir_color_R, Config::GUIVar().gui_file_dialog_dir_color_G, Config::GUIVar().gui_file_dialog_dir_color_B, 1.0f));
	/* Some other color values for directories that were tested. Left here so that they may be used again
	ImVec4(0.843f, 0.729f, 0.49f, 1.0f)
	ImVec4(0.745f, 0.482f, 0.176f, 1.0f)
	ImVec4(0.843f, 0.682f, 0.361f, 1.0f)
	ImVec4(0.808f, 0.498f, 0.306f, 1.0f)*/

	return ErrorCode::Success;
}

ErrorCode GUIScene::setup(const PropertySet& p_properties)
{
	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the property set containing object pool size
	auto &objectPoolSizeProperty = p_properties.getPropertySetByID(Properties::ObjectPoolSize);

	// Reserve every component type that belongs to this scene (and set the minimum number of objects based on default config)
	worldScene->reserve<GUISequenceComponent>(std::max(Config::objectPoolVar().gui_sequence_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::GUISequenceComponent).getInt()));

	return ErrorCode::Success;
}

void GUIScene::exportSetup(PropertySet &p_propertySet)
{
	// Get the world scene required for getting the pool sizes
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Add object pool sizes
	auto &objectPoolSizePropertySet = p_propertySet.addPropertySet(Properties::ObjectPoolSize);
	objectPoolSizePropertySet.addProperty(Properties::GUISequenceComponent, (int)worldScene->getPoolSize<GUISequenceComponent>());
}

void GUIScene::activate()
{
	if(m_editorWindow != nullptr)
		m_editorWindow->activate();
}

void GUIScene::deactivate()
{
	if(m_editorWindow != nullptr)
		m_editorWindow->deactivate();
}

void GUIScene::update(const float p_deltaTime)
{
	if(Config::GUIVar().gui_render)
	{
		// Get the world scene required for getting components
		WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

		// Set the beginning of the GUI frame
		GUIHandlerLocator::get().beginFrame();

		//	 ____________________________
		//	|							 |
		//	|	 FILE BROWSER DIALOGS	 |
		//	|____________________________|
		//
		if(!m_fileBrowserDialogs.empty())
		{
			// Get the first added file browser dialog
			auto *browserDialog = m_fileBrowserDialogs.front();

			// Check if its pointer is valid
			if(browserDialog != nullptr)
			{
				// Only open the current dialog once
				if(!browserDialog->m_opened)
				{
					browserDialog->m_opened = true;
					ImGuiFileDialog::Instance()->OpenDialog(
						browserDialog->m_name.c_str(), 
						browserDialog->m_title.c_str(), 
						browserDialog->m_filter.c_str(), 
						browserDialog->m_rootPath.c_str(), 
						browserDialog->m_definedFilename.c_str(), 
						browserDialog->m_numOfSelectableFiles, 
						nullptr, 
						browserDialog->m_flags);
				}

				// Display the dialog every frame until it is completed
				if(ImGuiFileDialog::Instance()->Display(browserDialog->m_name, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse, ImVec2(Config::GUIVar().gui_file_dialog_min_size_x, Config::GUIVar().gui_file_dialog_min_size_y)))
				{
					// If the dialog completed successfully (i.e. files were selected), get the selection data
					if(ImGuiFileDialog::Instance()->IsOk())
					{
						browserDialog->m_filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
						browserDialog->m_filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
						browserDialog->m_filename = ImGuiFileDialog::Instance()->GetCurrentFileName();
						browserDialog->m_success = true;
					}

					// Close the dialog and remove it from the queue
					ImGuiFileDialog::Instance()->Close();
					browserDialog->m_closed = true;
					m_fileBrowserDialogs.pop();
				}
			}
			else // Remove dialog of invalid pointer
				m_fileBrowserDialogs.pop();
		}

		//	 ____________________________
		//	|							 |
		//	|		EDITOR WINDOW		 |
		//	|____________________________|
		//
		if(m_editorWindow != nullptr)
		{
			m_editorWindow->update(p_deltaTime);
		}

		//	 ____________________________
		//	|							 |
		//	|	GUI SEQUENCE COMPONENTS	 |
		//	|____________________________|
		//
		auto sequenceView = worldScene->getEntityRegistry().view<GUISequenceComponent>();
		for(auto entity : sequenceView)
		{
			GUISequenceComponent &sequenceComponent = sequenceView.get<GUISequenceComponent>(entity);

			// Check if the script object is enabled
			if(sequenceComponent.isObjectActive())
			{
				// If GUI Sequence objects are enabled, update the object
				// otherwise just clear the sequence without drawing it
				if(m_GUISequenceEnabled)
					sequenceComponent.update(p_deltaTime);
				else
					sequenceComponent.clearSequence();
			}
		}

		// Set the end of the GUI frame
		GUIHandlerLocator::get().endFrame();
	}
}

ErrorCode GUIScene::preload()
{
	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();
	auto sequenceView = entityRegistry.view<GUISequenceComponent>();

	std::vector<SystemObject*> componentsToLoad;
	for(auto entity : sequenceView)
	{
		componentsToLoad.push_back(&sequenceView.get<GUISequenceComponent>(entity));
	}

	// Load every object to memory. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), componentsToLoad.size(), size_t(1), [=](size_t i)
		{
			componentsToLoad[i]->loadToMemory();
		});

	return ErrorCode::Success;
}

void GUIScene::loadInBackground()
{
}

std::vector<SystemObject *> GUIScene::getComponents(const EntityID p_entityID)
{
	std::vector<SystemObject *> returnVector;

	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	auto *guiSequenceComponent = entityRegistry.try_get<GUISequenceComponent>(p_entityID);
	if(guiSequenceComponent != nullptr)
		returnVector.push_back(guiSequenceComponent);

	return returnVector;
}

std::vector<SystemObject*> GUIScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_guiComponents, p_startLoading);
}

void GUIScene::exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	exportComponents(p_entityID, p_constructionInfo.m_guiComponents);
}

void GUIScene::exportComponents(const EntityID p_entityID, GUIComponentsConstructionInfo &p_constructionInfo)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Export GUISequenceComponent
	auto *guiSequenceComponent = entityRegistry.try_get<GUISequenceComponent>(p_entityID);
	if(guiSequenceComponent != nullptr)
	{
		if(p_constructionInfo.m_guiSequenceConstructionInfo == nullptr)
			p_constructionInfo.m_guiSequenceConstructionInfo = new GUISequenceComponent::GUISequenceComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_guiSequenceConstructionInfo, *guiSequenceComponent);
	}
}

SystemObject *GUIScene::createComponent(const EntityID p_entityID, const GUISequenceComponent::GUISequenceComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	auto &component = worldScene->addComponent<GUISequenceComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	// Try to initialize the component
	auto componentInitError = component.init();
	if(componentInitError == ErrorCode::Success)
	{
		component.m_objectType = Properties::PropertyID::GUISequenceComponent;
		component.setActive(p_constructionInfo.m_active);

		returnObject = &component;
	}
	else // Remove the component if it failed to initialize
	{
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_GUISequenceComponent, component.getName());
		worldScene->removeComponent<GUISequenceComponent>(p_entityID);
	}

	return returnObject;
}

void GUIScene::releaseObject(SystemObject *p_systemObject)
{
	switch(p_systemObject->getObjectType())
	{
		case Properties::PropertyID::GUISequenceComponent:
			{
				auto *component = static_cast<GUISequenceComponent *>(p_systemObject);

				// Nothing to release
			}
			break;
	}
}

ErrorCode GUIScene::destroyObject(SystemObject *p_systemObject)
{	
	ErrorCode returnError = ErrorCode::Success;

	// Get the world scene required for deleting components
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	switch(p_systemObject->getObjectType())
	{
	case Properties::PropertyID::GUISequenceComponent:
		{
			// Delete component
			worldScene->removeComponent<GUISequenceComponent>(p_systemObject->getEntityID());
		}
		break;

	default:
		{
			// No object was found, return an appropriate error
			returnError = ErrorCode::Destroy_obj_not_found;
		}
		break;
	}

	// If this point is reached, 
	return returnError;
}

void GUIScene::receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving)
{
	switch(p_dataType)
	{
		case DataType::DataType_DeleteComponent:
			{
				// Get the world scene required for getting the entity registry and deleting components
				WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

				// Get the entity registry 
				auto &entityRegistry = worldScene->getEntityRegistry();

				// Get entity and component data
				auto const *componentData = static_cast<EntityAndComponent *>(p_data);

				// Delete the component based on its type
				switch(componentData->m_componentType)
				{
					case ComponentType::ComponentType_GUISequenceComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<GUISequenceComponent>(componentData->m_entityID);
							if(component != nullptr)
								if(auto error = destroyObject(component); error != ErrorCode::Success)
									ErrHandlerLoc::get().log(error, component->getName(), ErrorSource::Source_GUI);
						}
						break;
				}

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentData;
			}
			break;
		case DataType_EnableGUISequence:
			{
				setGUISequenceEnabled(static_cast<bool>(p_data));
			}
			break;

		case DataType_FileBrowserDialog:
			{
				// Cast the sent data into the intended type and add it to the file-browser dialog queue
				m_fileBrowserDialogs.push(static_cast<FileBrowserDialog *>(p_data));
			}
			break;

		case DataType_EditorWindow:
			{
				// Cast the sent data into the intended type
				EditorWindowSettings *editorWindowSettings = static_cast<EditorWindowSettings *>(p_data);

				// If the editor window should be enabled
				if(editorWindowSettings->m_enabled)
				{
					// If the editor window doesn't exist, create it
					if(m_editorWindow == nullptr)
					{
						m_editorWindow = new EditorWindow(this, Config::GUIVar().gui_editor_window_name, 0);
						m_editorWindow->init();
					}

					// Send the settings to the editor window
					m_editorWindow->setup(*editorWindowSettings);
					m_editorWindow->activate();

					GUIHandlerLocator::get().enableDocking();
				}
				else // If the editor should be disabled
				{
					// If the editor window exist, delete it
					if(m_editorWindow != nullptr)
					{
						delete m_editorWindow;
						m_editorWindow = nullptr;
					}
				}

				// Delete the received data if it has been marked for deletion (ownership transfered upon receiving)
				if(p_deleteAfterReceiving)
					delete editorWindowSettings;
			}
			break;

		default:
			assert(p_deleteAfterReceiving == true && "Memory leak - unhandled orphaned void data pointer in receiveData");
			break;
	}
}