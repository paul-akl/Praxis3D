
#include <functional>

#include "ComponentConstructorInfo.h"
#include "NullSystemObjects.h"
#include "ScriptSystem.h"
#include "ScriptScene.h"
#include "TaskManagerLocator.h"
#include "WorldScene.h"

ScriptScene::ScriptScene(ScriptSystem *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader, Properties::PropertyID::Script)
{
	m_scriptingTask = nullptr;
	m_luaScriptsEnabled = true;
}

ScriptScene::~ScriptScene()
{
	if(m_scriptingTask != nullptr)
		delete m_scriptingTask;
}

ErrorCode ScriptScene::init()
{
	m_scriptingTask = new ScriptTask(this);

	return ErrorCode::Success;
}

ErrorCode ScriptScene::setup(const PropertySet &p_properties)
{
	// Get the world scene required for reserving the component pools
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the property set containing object pool size
	auto &objectPoolSizeProperty = p_properties.getPropertySetByID(Properties::ObjectPoolSize);

	// Reserve every component type that belongs to this scene (and set the minimum number of objects based on default config)
	worldScene->reserve<LuaComponent>(std::max(Config::objectPoolVar().lua_component_default_pool_size, objectPoolSizeProperty.getPropertyByID(Properties::LuaComponent).getInt()));

	return ErrorCode::Success;
}

void ScriptScene::exportSetup(PropertySet &p_propertySet)
{
	// Get the world scene required for getting the pool sizes
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Add object pool sizes
	auto &objectPoolSizePropertySet = p_propertySet.addPropertySet(Properties::ObjectPoolSize);
	objectPoolSizePropertySet.addProperty(Properties::LuaComponent, (int)worldScene->getPoolSize<LuaComponent>());
}

void ScriptScene::update(const float p_deltaTime)
{
	// Get the world scene required for getting components
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	if(m_luaScriptsEnabled)
	{
		//	 ____________________________
		//	|							 |
		//	| LUA AND SPATIAL COMPONENTS |
		//	|____________________________|
		//
		auto luaAndSpatialView = worldScene->getEntityRegistry().view<LuaComponent, SpatialComponent>();
		for(auto entity : luaAndSpatialView)
		{
			LuaComponent &luaComponent = luaAndSpatialView.get<LuaComponent>(entity);
			SpatialComponent &spatialComponent = luaAndSpatialView.get<SpatialComponent>(entity);

			// Check if the script object is enabled
			if(luaComponent.isObjectActive())
			{
				// Update the object
				luaComponent.update(p_deltaTime, spatialComponent);
			}
		}

		//	 ____________________________
		//	|							 |
		//	|		LUA COMPONENTS		 |
		//	|____________________________|
		//
		auto luaOnlyView = worldScene->getEntityRegistry().view<LuaComponent>(entt::exclude<SpatialComponent>);
		for(auto entity : luaOnlyView)
		{
			LuaComponent &luaComponent = luaOnlyView.get<LuaComponent>(entity);

			// Check if the script object is enabled
			if(luaComponent.isObjectActive())
			{
				// Update the object
				luaComponent.update(p_deltaTime);
			}
		}
	}
}

void ScriptScene::loadInBackground()
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	auto luaView = worldScene->getEntityRegistry().view<LuaComponent>();
	for(auto entity : luaView)
	{
		auto &component = luaView.get<LuaComponent>(entity);

		TaskManagerLocator::get().startBackgroundThread(std::bind(&LuaComponent::loadToMemory, &component));
	}
}

ErrorCode ScriptScene::preload()
{	
	// Get the world scene required for getting components
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	auto LuaComponentView = worldScene->getEntityRegistry().view<LuaComponent>();

	for(auto entity : LuaComponentView)
	{
		LuaComponentView.get<LuaComponent>(entity).loadToMemory();
	}

	return ErrorCode::Success;
}

std::vector<SystemObject *> ScriptScene::getComponents(const EntityID p_entityID)
{
	std::vector<SystemObject *> returnVector;

	// Get the entity registry 
	auto &entityRegistry = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->getEntityRegistry();

	auto *luaComponent = entityRegistry.try_get<LuaComponent>(p_entityID);
	if(luaComponent != nullptr)
		returnVector.push_back(luaComponent);

	return returnVector;
}

std::vector<SystemObject*> ScriptScene::createComponents(const EntityID p_entityID, const ComponentsConstructionInfo &p_constructionInfo, const bool p_startLoading)
{
	return createComponents(p_entityID, p_constructionInfo.m_scriptComponents, p_startLoading);
}

void ScriptScene::exportComponents(const EntityID p_entityID, ComponentsConstructionInfo &p_constructionInfo)
{
	exportComponents(p_entityID, p_constructionInfo.m_scriptComponents);
}

void ScriptScene::exportComponents(const EntityID p_entityID, ScriptComponentsConstructionInfo &p_constructionInfo)
{
	// Get the world scene required for getting the entity registry
	WorldScene *worldScene = static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World));

	// Get the entity registry 
	auto &entityRegistry = worldScene->getEntityRegistry();

	// Export LuaComponent
	auto *luaComponent = entityRegistry.try_get<LuaComponent>(p_entityID);
	if(luaComponent != nullptr)
	{
		if(p_constructionInfo.m_luaConstructionInfo == nullptr)
			p_constructionInfo.m_luaConstructionInfo = new LuaComponent::LuaComponentConstructionInfo();

		exportComponent(*p_constructionInfo.m_luaConstructionInfo, *luaComponent);
	}
}

SystemObject *ScriptScene::createComponent(const EntityID &p_entityID, const LuaComponent::LuaComponentConstructionInfo &p_constructionInfo, const bool p_startLoading)
{	
	// If valid type was not specified, or object creation failed, return a null object instead
	SystemObject *returnObject = g_nullSystemBase.getScene(EngineStateType::EngineStateType_Default)->getNullObject();

	// Get the world scene required for attaching components to the entity
	WorldScene *worldScene = static_cast<WorldScene*>(m_sceneLoader->getSystemScene(Systems::World));

	auto &component = worldScene->addComponent<LuaComponent>(p_entityID, this, p_constructionInfo.m_name, p_entityID);

	// Try to initialize the lua component
	auto componentInitError = component.init();
	if(componentInitError == ErrorCode::Success)
	{
		if(!p_constructionInfo.m_luaScriptFilename.empty())
		{
			std::string luaFilename = Config::filepathVar().script_path + p_constructionInfo.m_luaScriptFilename;
			if(Filesystem::exists(luaFilename))
			{
				component.m_luaScript->setScriptFilename(luaFilename);

				if(!p_constructionInfo.m_variables.empty())
					component.m_luaScript->setVariables(p_constructionInfo.m_variables);

				component.m_objectType = Properties::PropertyID::LuaComponent;
				component.m_setActiveAfterLoading = p_constructionInfo.m_active;
				component.setActive(false);
				component.setLoadedToMemory(false);

				returnObject = &component;
				//ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_LuaComponent, component.getName() + " - Script loaded");
			}
			else
			{
				ErrHandlerLoc().get().log(ErrorCode::File_not_found, component.getName(), ErrorSource::Source_LuaComponent);
				worldScene->removeComponent<LuaComponent>(p_entityID);
			}
		}
		else
		{
			ErrHandlerLoc().get().log(ErrorCode::Property_no_filename, component.getName(), ErrorSource::Source_LuaComponent);
			worldScene->removeComponent<LuaComponent>(p_entityID);
		}
	}
	else // Remove the component if it failed to initialize
	{
		ErrHandlerLoc().get().log(componentInitError, ErrorSource::Source_LuaComponent, component.getName());
		worldScene->removeComponent<LuaComponent>(p_entityID);
	}

	return returnObject;
}

ErrorCode ScriptScene::destroyObject(SystemObject *p_systemObject)
{
	ErrorCode returnError = ErrorCode::Success;

	switch(p_systemObject->getObjectType())
	{
	case Properties::PropertyID::LuaComponent:
		//m_sceneLoader->getChangeController()->removeObjectLink(p_systemObject);
		static_cast<WorldScene *>(m_sceneLoader->getSystemScene(Systems::World))->removeComponent<LuaComponent>(p_systemObject->getEntityID());
		break;

	default:
		// No object was found, return an appropriate error
		returnError = ErrorCode::Destroy_obj_not_found;
		break;
	}

	// If this point is reached, 
	return returnError;
}

void ScriptScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
}

void ScriptScene::receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving = false)
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
					case ComponentType::ComponentType_LuaComponent:
						{
							// Check if the component exists
							auto *component = entityRegistry.try_get<LuaComponent>(componentData->m_entityID);
							if(component != nullptr)
							{
								// Delete component
								worldScene->removeComponent<LuaComponent>(componentData->m_entityID);
							}
						}
						break;
				}

				// Delete the sent data if the ownership of it was transfered
				if(p_deleteAfterReceiving)
					delete componentData;
			}
			break;

	case DataType_EnableLuaScripting:
		m_luaScriptsEnabled = static_cast<bool>(p_data);
		break;

	default:
		assert(p_deleteAfterReceiving == true && "Memory leak - unhandled orphaned void data pointer in receiveData");
		break;
	}
}

FreeCamera *ScriptScene::loadFreeCamera(const PropertySet & p_properties)
{
	FreeCamera *freeCamera = new FreeCamera(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Get key-bindings from the property
	const auto &keybindings = p_properties.getPropertySetByID(Properties::Keybindings);

	// Declare keys
	Scancode forwardKey = Key_Invalid;
	Scancode backwardKey = Key_Invalid;
	Scancode leftStrafeKey = Key_Invalid;
	Scancode rightStrafeKey = Key_Invalid;
	Scancode sprintKey = Key_Invalid;

	// Check if key-bindings in the property are present
	if(keybindings.getPropertyID() != Properties::Null)
	{
		forwardKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::ForwardKey).getInt());
		backwardKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::BackwardKey).getInt());
		leftStrafeKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::LeftStrafeKey).getInt());
		rightStrafeKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::RightStrafeKey).getInt());
		sprintKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::SprintKey).getInt());
	}

	// Check validity of each key, use a default value in case they are invalid
	if(forwardKey == Scancode::Key_Invalid)
		forwardKey = static_cast<Scancode>(Config::inputVar().forward_key);
	if(backwardKey == Scancode::Key_Invalid)
		backwardKey = static_cast<Scancode>(Config::inputVar().backward_key);
	if(leftStrafeKey == Scancode::Key_Invalid)
		leftStrafeKey = static_cast<Scancode>(Config::inputVar().left_strafe_key);
	if(rightStrafeKey == Scancode::Key_Invalid)
		rightStrafeKey = static_cast<Scancode>(Config::inputVar().right_strafe_key);
	if(sprintKey == Scancode::Key_Invalid)
		sprintKey = static_cast<Scancode>(Config::inputVar().sprint_key);

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::LocalPosition:
			freeCamera->setPosition(p_properties[i].getVec3f());
			break;
		case Properties::Angle:
			freeCamera->setAngles(p_properties[i].getVec2f());
			break;
		case Properties::LowerLimit:
			freeCamera->setLowerLimit(p_properties[i].getFloat());
			break;
		case Properties::Speed:
			freeCamera->setSpeed(p_properties[i].getFloat());
			break;
		case Properties::SprintSpeed:
			freeCamera->setFasterSpeed(p_properties[i].getFloat());
			break;
		case Properties::UpperLimit:
			freeCamera->setUpperLimit(p_properties[i].getFloat());
			break;
		}
	}

	// Check if the movement speeds were set
	if(freeCamera->m_speed == 0.0f)
		freeCamera->m_speed = Config::gameplayVar().camera_freelook_speed;

	if(freeCamera->m_fasterSpeed == 0.0f)
		freeCamera->m_fasterSpeed = freeCamera->m_speed * 10.0f;

	// Set the keys of the object
	freeCamera->setForwardKey(forwardKey);
	freeCamera->setBackwardKey(backwardKey);
	freeCamera->setStrafeLeftKey(leftStrafeKey);
	freeCamera->setStrafeRightKey(rightStrafeKey);
	freeCamera->setSprintKey(sprintKey);

	//m_scriptObjects.push_back(freeCamera);

	return freeCamera;
}

DebugUIScript *ScriptScene::loadDebugUI(const PropertySet &p_properties)
{
	DebugUIScript *debugUI = new DebugUIScript(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Get key-bindings from the property
	const auto &keybindings = p_properties.getPropertySetByID(Properties::Keybindings);

	// Declare keys
	Scancode closeWindowKey;
	Scancode fullscreenKey;
	Scancode mouseCaptureKey;
	Scancode verticalSyncKey;

	// Check if key-bindings in the property are present
	if(keybindings.getPropertyID() != Properties::Null)
	{
		closeWindowKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::CloseKey).getInt());
		fullscreenKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::DebugFullscreenKey).getInt());
		mouseCaptureKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::DebugCaptureMouseKey).getInt());
		verticalSyncKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::DebugVertSyncKey).getInt());

		// Check validity of each key, use a default value in case they are invalid
		if(closeWindowKey == Scancode::Key_Invalid)
			closeWindowKey = Utilities::toScancode(Config::inputVar().close_window_key);
		if(fullscreenKey == Scancode::Key_Invalid)
			fullscreenKey = Utilities::toScancode(Config::inputVar().fullscreen_key);
		if(mouseCaptureKey == Scancode::Key_Invalid)
			mouseCaptureKey = Utilities::toScancode(Config::inputVar().clip_mouse_key);
		if(verticalSyncKey == Scancode::Key_Invalid)
			verticalSyncKey = Utilities::toScancode(Config::inputVar().vsync_key);
	}
	else
	{
		closeWindowKey = Utilities::toScancode(Config::inputVar().close_window_key);
		fullscreenKey = Utilities::toScancode(Config::inputVar().fullscreen_key);
		mouseCaptureKey = Utilities::toScancode(Config::inputVar().clip_mouse_key);
		verticalSyncKey = Utilities::toScancode(Config::inputVar().vsync_key);
	}

	debugUI->setCloseWindowKey(closeWindowKey);
	debugUI->setFullscreenKey(fullscreenKey);
	debugUI->setMouseCaptureKey(mouseCaptureKey);
	debugUI->setVerticalSyncKey(verticalSyncKey);

	//m_scriptObjects.push_back(debugUI);

	return debugUI;
}

DebugMoveScript *ScriptScene::loadDebugMove(const PropertySet &p_properties)
{
	DebugMoveScript *debugMove = new DebugMoveScript(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::LocalPosition:
			debugMove->setOriginPosition(p_properties[i].getVec3f());
			break;
		case Properties::Radius:
			debugMove->setRadius(p_properties[i].getFloat());
			break;
		case Properties::LocalRotation:
			debugMove->setMovementAxis(p_properties[i].getVec3f());
			break;
		case Properties::Speed:
			debugMove->setMovementSpeed(p_properties[i].getFloat());
			break;
		}
	}

	//m_scriptObjects.push_back(debugMove);

	return debugMove;
}

DebugRotateScript *ScriptScene::loadDebugRotate(const PropertySet &p_properties)
{
	DebugRotateScript *debugRotate = new DebugRotateScript(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Load property data
	for (decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch (p_properties[i].getPropertyID())
		{
		case Properties::Axis:
			debugRotate->setRotationAxis(p_properties[i].getVec3f());
			break;
		case Properties::LocalRotation:
			debugRotate->setRotation(p_properties[i].getVec3f());
			break;
		case Properties::Speed:
			debugRotate->setMovementSpeed(p_properties[i].getFloat());
			break;
		}
	}

	//m_scriptObjects.push_back(debugRotate);

	return debugRotate;
}

SolarTimeScript *ScriptScene::loadSolarTime(const PropertySet & p_properties)
{
	SolarTimeScript *solarTimeScript = new SolarTimeScript(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Hours:
			solarTimeScript->setHours(p_properties[i].getInt());
			break;
		case Properties::Minutes:
			solarTimeScript->setMinutes(p_properties[i].getInt());
			break;
		case Properties::Seconds:
			solarTimeScript->setSeconds(p_properties[i].getFloat());
			break;
		case Properties::Year:
			solarTimeScript->setYear(p_properties[i].getInt());
			break;
		case Properties::Month:
			solarTimeScript->setMonth(p_properties[i].getInt());
			break;
		case Properties::Day:
			solarTimeScript->setDay(p_properties[i].getInt());
			break;
		case Properties::TimeZone:
			solarTimeScript->setTimeZone(p_properties[i].getInt());
			break;
		case Properties::Latitude:
			solarTimeScript->setLatitude(p_properties[i].getFloat());
			break;
		case Properties::Longitude:
			solarTimeScript->setLongitude(p_properties[i].getFloat());
			break;
		case Properties::TimeMultiplier:
			solarTimeScript->setTimeMultiplier(p_properties[i].getFloat());
			break;
		case Properties::OffsetPosition:
			solarTimeScript->setOffsetPosition(p_properties[i].getFloat());
			break;
		}
	}

	//m_scriptObjects.push_back(solarTimeScript);

	return solarTimeScript;
}

SunScript *ScriptScene::loadSun(const PropertySet & p_properties)
{
	SunScript *sunScript = new SunScript(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Azimuth:
			sunScript->setAzimuthAngle(p_properties[i].getFloat());
			break;
		case Properties::Zenith:
			sunScript->setZenithAngle(p_properties[i].getFloat());
			break;
		}
	}

	//m_scriptObjects.push_back(sunScript);

	return sunScript;
}

WorldEditScript *ScriptScene::loadWorldEdit(const PropertySet & p_properties)
{
	WorldEditScript *worldEditor = new WorldEditScript(this, p_properties.getPropertyByID(Properties::Name).getString(), m_sceneLoader);

	// Get key-bindings from the property
	const auto &keybindings = p_properties.getPropertySetByID(Properties::Keybindings);

	// Declare keys
	Scancode upKey;
	Scancode downKey;
	Scancode leftKey;
	Scancode rightKey;
	Scancode forwardKey;
	Scancode backwardKey;
	Scancode nextKey;
	Scancode previousKey;
	Scancode fasterMovementKey;
	Scancode centerKey;
	Scancode saveKey;
	Scancode modifierKey;

	// Check if key-bindings in the property are present
	if(keybindings.getPropertyID() != Properties::Null)
	{
		upKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::UpKey).getInt());
		downKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::DownKey).getInt());
		leftKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::LeftKey).getInt());
		rightKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::RightKey).getInt());
		forwardKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::ForwardKey).getInt());
		backwardKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::BackwardKey).getInt());
		nextKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::NextKey).getInt());
		previousKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::PreviousKey).getInt());
		fasterMovementKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::SprintKey).getInt());
		centerKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::CenterKey).getInt());
		saveKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::SaveKey).getInt());
		modifierKey = Utilities::toScancode(keybindings.getPropertyByID(Properties::ModifierKey).getInt());

		// Check validity of each key, use a default value in case they are invalid
		if(upKey == Scancode::Key_Invalid)
			upKey = static_cast<Scancode>(Config::inputVar().up_editor_key);
		if(downKey == Scancode::Key_Invalid)
			downKey = static_cast<Scancode>(Config::inputVar().down_editor_key);
		if(leftKey == Scancode::Key_Invalid)
			leftKey = static_cast<Scancode>(Config::inputVar().left_editor_key);
		if(rightKey == Scancode::Key_Invalid)
			rightKey = static_cast<Scancode>(Config::inputVar().right_editor_key);
		if(forwardKey == Scancode::Key_Invalid)
			forwardKey = static_cast<Scancode>(Config::inputVar().forward_key);
		if(backwardKey == Scancode::Key_Invalid)
			backwardKey = static_cast<Scancode>(Config::inputVar().backward_key);
		if(nextKey == Scancode::Key_Invalid)
			nextKey = static_cast<Scancode>(Config::inputVar().next_editor_key);
		if(previousKey == Scancode::Key_Invalid)
			previousKey = static_cast<Scancode>(Config::inputVar().previous_editor_key);
		if(fasterMovementKey == Scancode::Key_Invalid)
			fasterMovementKey = static_cast<Scancode>(Config::inputVar().sprint_key);
		if(centerKey == Scancode::Key_Invalid)
			centerKey = static_cast<Scancode>(Config::inputVar().center_key);
		if(saveKey == Scancode::Key_Invalid)
			saveKey = static_cast<Scancode>(Config::inputVar().save_editor_key);
		if(modifierKey == Scancode::Key_Invalid)
			modifierKey = static_cast<Scancode>(Config::inputVar().modifier_editor_key);
	}
	else
	{
		// Set keys to default, if they key-bindings were not present
		upKey = static_cast<Scancode>(Config::inputVar().up_editor_key);
		downKey = static_cast<Scancode>(Config::inputVar().down_editor_key);
		leftKey = static_cast<Scancode>(Config::inputVar().left_editor_key);
		rightKey = static_cast<Scancode>(Config::inputVar().right_editor_key);
		forwardKey = static_cast<Scancode>(Config::inputVar().forward_key);
		backwardKey = static_cast<Scancode>(Config::inputVar().backward_key);
		nextKey = static_cast<Scancode>(Config::inputVar().next_editor_key);
		previousKey = static_cast<Scancode>(Config::inputVar().previous_editor_key);
		fasterMovementKey = static_cast<Scancode>(Config::inputVar().sprint_key);
		centerKey = static_cast<Scancode>(Config::inputVar().center_key);
		saveKey = static_cast<Scancode>(Config::inputVar().save_editor_key);
		modifierKey = static_cast<Scancode>(Config::inputVar().modifier_editor_key);
	}
	
	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Speed:
			worldEditor->setSpeed(p_properties[i].getFloat());
			break;
		case Properties::SprintSpeed:
			worldEditor->setFasterSpeed(p_properties[i].getFloat());
			break;
		}
	}

	// Check if the movement speeds were set
	//if(worldEditor->m_movementSpeed == 0.0f)
	//	worldEditor->setSpeed(Config::gameplayVar().camera_freelook_speed);

	//if(worldEditor->m_fasterMovementSpeed == 0.0f)
	//	worldEditor->setFasterSpeed(worldEditor->m_movementSpeed * 10.0f);

	// Set the keys of the object
	worldEditor->setUpKey(upKey);
	worldEditor->setDownKey(downKey);
	worldEditor->setLeftKey(leftKey);
	worldEditor->setRightKey(rightKey);
	worldEditor->setForwardKey(forwardKey);
	worldEditor->setBackwardKey(backwardKey);
	worldEditor->setNextKey(nextKey);
	worldEditor->setPreviousKey(previousKey);
	worldEditor->setFasterMovementKey(fasterMovementKey);
	worldEditor->setCenterKey(centerKey);
	worldEditor->setSaveKey(saveKey);
	worldEditor->setModifierKey(modifierKey);

	//m_scriptObjects.push_back(worldEditor);

	return worldEditor;
}
