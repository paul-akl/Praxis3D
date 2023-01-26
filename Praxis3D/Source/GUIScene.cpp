
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
	decltype(m_GUIObjects.getPoolSize()) objectPoolSize = Config::objectPoolVar().object_pool_size;

	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::ObjectPoolSize:
			objectPoolSize = p_properties[i].getInt();
			break;
		}
	}

	// Initialize object pools
	m_GUIObjects.init(objectPoolSize);

	return ErrorCode::Success;
}

void GUIScene::update(const float p_deltaTime)
{
	// Get the world scene required for getting components
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Set the beginning of the GUI frame
	GUIHandlerLocator::get().beginFrame();

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

ErrorCode GUIScene::preload()
{
	// Load every GUI object. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), m_GUIObjects.getPoolSize(), size_t(1), [=](size_t i)
		{
			if(m_GUIObjects[i].allocated())
			{
				m_GUIObjects[i].getObject()->loadToMemory();
			}
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

ErrorCode GUIScene::destroyObject(SystemObject* p_systemObject)
{	
	// Check if object is valid and belongs to the GUI system
	if(p_systemObject != nullptr && p_systemObject->getSystemType() == Systems::GUI)
	{
		// Cast the system object to GUI object, as it belongs to the renderer scene
		GUIObject *objectToDestroy = static_cast<GUIObject *>(p_systemObject);

		// Try to destroy the object; return success if it succeeds
		if(removeObjectFromPool(*objectToDestroy))
			return ErrorCode::Success;
	}

	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}
