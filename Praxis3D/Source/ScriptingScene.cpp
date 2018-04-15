
#include <functional>

#include "NullSystemObjects.h"
#include "ScriptingSystem.h"
#include "ScriptingScene.h"
#include "TaskManagerLocator.h"

ScriptingScene::ScriptingScene(ScriptingSystem *p_system, SceneLoader *p_sceneLoader) : SystemScene(p_system, p_sceneLoader)
{
	m_scriptingTask = nullptr;
}

ScriptingScene::~ScriptingScene()
{
}

ErrorCode ScriptingScene::init()
{
	m_scriptingTask = new ScriptingTask(this);
	return ErrorCode::Success;
}

ErrorCode ScriptingScene::setup(const PropertySet &p_properties)
{
	/*for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{

		}
	}*/

	return ErrorCode::Success;
}

void ScriptingScene::update(const float p_deltaTime)
{
	for(decltype(m_scriptObjects.size()) i = 0, size = m_scriptObjects.size(); i < size; i++)
	{
		m_scriptObjects[i]->update(p_deltaTime);
	}
}

void ScriptingScene::loadInBackground()
{
	// Iterate over script objects and start loading them in background
	for(decltype(m_scriptObjects.size()) i = 0, size = m_scriptObjects.size(); i < size; i++)
	{
		TaskManagerLocator::get().startBackgroundThread(std::bind(&BaseScriptObject::loadToMemory, m_scriptObjects[i]));
	}
}

PropertySet ScriptingScene::exportObject()
{
	// Create the root property set
	PropertySet propertySet(Properties::Scripting);

	// Add root property set for scene values
	auto &scene = propertySet.addPropertySet(Properties::Scene);

	// Add root property set for all the objects
	auto &objects = propertySet.addPropertySet(Properties::Objects);

	// Add script object property sets
	for(decltype(m_scriptObjects.size()) i = 0, size = m_scriptObjects.size(); i < size; i++)
		objects.addPropertySet(m_scriptObjects[i]->exportObject());

	return propertySet;
}

ErrorCode ScriptingScene::preload()
{	
	// Load every script object. It still works in parallel, however,
	// it returns only when all objects have finished loading (simulating sequential call)
	TaskManagerLocator::get().parallelFor(size_t(0), m_scriptObjects.size(), size_t(1), [=](size_t i)
	{
		m_scriptObjects[i]->loadToMemory();
	});

	return ErrorCode::Success;
}

SystemObject *ScriptingScene::createObject(const PropertySet &p_properties)
{
	//  Get certain object properties
	auto const &type = p_properties.getPropertyByID(Properties::Type).getID();

	SystemObject *newObject = nullptr;

	// Create the object by it's type
	switch(type)
	{
	case Properties::FreeCamera:
		newObject = loadFreeCamera(p_properties);
		break;
	case Properties::DebugUIScript:
		newObject = loadDebugUI(p_properties);
		break;
	case Properties::DebugMoveScript:
		newObject = loadDebugMove(p_properties);
		break;
	case Properties::DebugRotateScript:
		newObject = loadDebugRotate(p_properties);
		break;
	case Properties::SolarTimeScript:
		newObject = loadSolarTime(p_properties);
		break;
	case Properties::WorldEditScript:
		newObject = loadWorldEdit(p_properties);
		break;
	}

	// If the object creation was successful, return the new object
	if(newObject != nullptr)
		return newObject;

	// If valid type was not specified, or object creation failed, return a null object instead
	return g_nullSystemBase.getScene()->createObject(p_properties);
}

ErrorCode ScriptingScene::destroyObject(SystemObject *p_systemObject)
{
	if(p_systemObject != nullptr)
	{
		// Iterate over all elements and match the pointer address
		for(decltype(m_scriptObjects.size()) i = 0, size = m_scriptObjects.size(); i < size; i++)
		{
			// If pointer addresses match, remove it and return success
			if(m_scriptObjects[i] == p_systemObject)
			{
				std::swap(m_scriptObjects[i], m_scriptObjects.back());
				m_scriptObjects.pop_back();

				return ErrorCode::Success;
			}
		}
	}

	// If this point is reached, no object was found, return an appropriate error
	return ErrorCode::Destroy_obj_not_found;
}

void ScriptingScene::changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
{
}

FreeCamera *ScriptingScene::loadFreeCamera(const PropertySet & p_properties)
{
	FreeCamera *freeCamera = new FreeCamera(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Get key-bindings from the property
	const auto &keybindings = p_properties.getPropertySetByID(Properties::Keybindings);

	// Declare keys
	Scancode forwardKey;
	Scancode backwardKey;
	Scancode leftStrafeKey;
	Scancode rightStrafeKey;
	Scancode sprintKey;

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
		case Properties::Position:
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

	m_scriptObjects.push_back(freeCamera);

	return freeCamera;
}

DebugUIScript *ScriptingScene::loadDebugUI(const PropertySet &p_properties)
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

	m_scriptObjects.push_back(debugUI);

	return debugUI;
}

DebugMoveScript *ScriptingScene::loadDebugMove(const PropertySet &p_properties)
{
	DebugMoveScript *debugMove = new DebugMoveScript(this, p_properties.getPropertyByID(Properties::Name).getString());

	// Load property data
	for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{
		case Properties::Position:
			debugMove->setOriginPosition(p_properties[i].getVec3f());
			break;
		case Properties::Radius:
			debugMove->setRadius(p_properties[i].getFloat());
			break;
		case Properties::Rotation:
			debugMove->setMovementAxis(p_properties[i].getVec3f());
			break;
		case Properties::Speed:
			debugMove->setMovementSpeed(p_properties[i].getFloat());
			break;
		}
	}

	m_scriptObjects.push_back(debugMove);

	return debugMove;
}

DebugRotateScript *ScriptingScene::loadDebugRotate(const PropertySet &p_properties)
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
		case Properties::Rotation:
			debugRotate->setRotation(p_properties[i].getVec3f());
			break;
		case Properties::Speed:
			debugRotate->setMovementSpeed(p_properties[i].getFloat());
			break;
		}
	}

	m_scriptObjects.push_back(debugRotate);

	return debugRotate;
}

SolarTimeScript *ScriptingScene::loadSolarTime(const PropertySet & p_properties)
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
		case Properties::Latitude:
			solarTimeScript->setLatitude(p_properties[i].getFloat());
			break;
		case Properties::Longitude:
			solarTimeScript->setLongitude(p_properties[i].getFloat());
			break;
		case Properties::DayOfYear:
			solarTimeScript->setDayOfYear(p_properties[i].getInt());
			break;
		case Properties::TimeMultiplier:
			solarTimeScript->setTimeMultiplier(p_properties[i].getFloat());
			break;
		case Properties::OffsetPosition:
			solarTimeScript->setOffsetPosition(p_properties[i].getFloat());
			break;
		}
	}

	m_scriptObjects.push_back(solarTimeScript);

	return solarTimeScript;
}

WorldEditScript *ScriptingScene::loadWorldEdit(const PropertySet & p_properties)
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
	if(worldEditor->m_movementSpeed == 0.0f)
		worldEditor->setSpeed(Config::gameplayVar().camera_freelook_speed);

	if(worldEditor->m_fasterMovementSpeed == 0.0f)
		worldEditor->setFasterSpeed(worldEditor->m_movementSpeed * 10.0f);

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

	m_scriptObjects.push_back(worldEditor);

	return worldEditor;
}
