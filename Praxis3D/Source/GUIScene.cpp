
#include "WorldScene.h"
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

SystemObject *GUIScene::createComponent(const EntityID &p_entityID, const std::string &p_entityName, const PropertySet &p_properties)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene()->createObject(p_properties);

	// Check if property set node is present
	if(p_properties)
	{
		// Get the world scene required for attaching components to the entity
		WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

		switch(p_properties.getPropertyID())
		{
			case Properties::PropertyID::Sequence:
			{
				auto &component = worldScene->addComponent<GUISequenceComponent>(p_entityID, this, p_entityName + Config::componentVar().component_name_separator + GetString(Properties::PropertyID::Sequence), p_entityID);

				// Try to initialize the component
				auto componentInitError = component.init();
				if(componentInitError == ErrorCode::Success)
				{
					// Try to import the component
					auto const &componentImportError = component.importObject(p_properties);

					// Remove the component if it failed to import
					if(componentImportError != ErrorCode::Success)
					{
						ErrHandlerLoc().get().log(componentImportError, ErrorSource::Source_GUISequenceComponent, component.getName());
						worldScene->removeComponent<GUISequenceComponent>(p_entityID);
					}
					else
						returnObject = &component;
				}
				else // Remove the component if it failed to initialize
				{
					ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_GUISequenceComponent, component.getName());
					worldScene->removeComponent<GUISequenceComponent>(p_entityID);
				}
			}
			break;
		}
	}

	return returnObject;
}

SystemObject* GUIScene::createObject(const PropertySet& p_properties)
{
	// Check if property set node is present
	if(p_properties)
	{
		// Check if the GUI property is present
		auto &GUIProperty = p_properties.getPropertySetByID(Properties::GUI);
		if(GUIProperty)
		{
			// Get the object name
			auto &nameProperty = p_properties.getPropertyByID(Properties::Name);

			// Find a place for the new object in the pool
			auto GUIObjectFromPool = m_GUIObjects.newObject();

			// Check if the pool wasn't full
			if(GUIObjectFromPool != nullptr)
			{
				std::string name;

				// If the name property is missing, generate a unique name based on the object's index in the pool
				if(nameProperty)
					name = nameProperty.getString() + " (" + GetString(Properties::GUIObject) + ")";
				else
					name = GetString(Properties::GUIObject) + Utilities::toString(GUIObjectFromPool->getIndex());

				// Construct the GUIObject
				GUIObjectFromPool->construct(this, name);
				auto newGUIObject = GUIObjectFromPool->getObject();

				// Start importing the newly created object in a background thread
				newGUIObject->importObject(GUIProperty);

				return newGUIObject;
			}
			else
			{
				ErrHandlerLoc::get().log(ErrorCode::ObjectPool_full, ErrorSource::Source_GUIObject, "Failed to add GUIObject - \'" + nameProperty.getString() + "\'");
			}
		}
	}

	// If valid type was not specified, or object creation failed, return a null object instead
	return g_nullSystemBase.getScene()->createObject(p_properties);
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
