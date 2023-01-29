
#include "WorldScene.h"
#include "ComponentConstructorInfo.h"
#include "GUIHandlerLocator.h"
#include "GUIScene.h"
#include "NullSystemObjects.h"
#include "TaskManagerLocator.h"

GUIScene::GUIScene(SystemBase *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	 m_GUITask = nullptr;
}

GUIScene::~GUIScene()
{
	if(m_GUITask != nullptr)
		delete m_GUITask;
}

ErrorCode GUIScene::init()
{
	m_GUITask = new GUITask(this);

	return ErrorCode::Success;
}

ErrorCode GUIScene::setup(const PropertySet& p_properties)
{	
	// Get default object pool size
	int objectPoolSize = Config::objectPoolVar().object_pool_size;

	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ObjectPoolSize:
			objectPoolSize = p_properties[i].getInt();
			break;
		}
	}

	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Reserve every component type that belongs to this scene
	worldScene->reserve<GUISequenceComponent>(Config::objectPoolVar().gui_sequence_component_default_pool_size);

	return ErrorCode::Success;
}

void GUIScene::update(const float p_deltaTime)
{
	if(Config::GUIVar().gui_render)
	{
		// Get the world scene required for getting components
		WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

		// Set the beginning of the GUI frame
		GUIHandlerLocator::get().beginFrame();

		//static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;

		//Config::graphicsVar().current_resolution_x;
		//Config::graphicsVar().current_resolution_y;

		//ImVec2 exitButtonSize(100.0f, 25.0f);

		//ImGui::SetNextWindowPos(ImVec2((Config::graphicsVar().current_resolution_x / 2.0f) - (exitButtonSize.x / 2.0f), (Config::graphicsVar().current_resolution_y / 2.0f) - (exitButtonSize.y / 2.0f)));
		//ImGui::Begin("Example: Fullscreen window", 0, flags);

		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 255.0f, 0.0f, 255.0f));
		//ImGui::Text("Hello, world %d", 123);
		//ImGui::Button("Exit", exitButtonSize);
		//ImGui::PopStyleColor();

		//ImGui::End();

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
				// Update the object
				sequenceComponent.update(p_deltaTime);
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

std::vector<SystemObject*> GUIScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_guiComponents, p_startLoading);
}

SystemObject *GUIScene::createComponent(const EntityID p_entityID, const GUISequenceComponent::GUISequenceComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->getNullObject();

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

ErrorCode GUIScene::destroyObject(SystemObject *p_systemObject)
{	
	ErrorCode returnError = ErrorCode::Success;

	switch(p_systemObject->getObjectType())
	{
	case Properties::PropertyID::GUISequenceComponent:
		//m_sceneLoader->getChangeController()->removeObjectLink(p_systemObject);
		static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->removeComponent<GUISequenceComponent>(p_systemObject->getEntityID());
		break;

	default:
		// No object was found, return an appropriate error
		returnError = ErrorCode::Destroy_obj_not_found;
		break;
	}

	// If this point is reached, 
	return returnError;
}
