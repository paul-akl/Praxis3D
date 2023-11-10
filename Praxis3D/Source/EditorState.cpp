#include "ClockLocator.h"
#include "EditorState.h"
#include "GUISystem.h"
#include "PhysicsSystem.h"
#include "PropertySet.h"
#include "ScriptSystem.h"
#include "RendererSystem.h"
#include "WorldSystem.h"

EditorState::EditorState(Engine &p_engine) : EngineState(p_engine, EngineStateType::EngineStateType_Editor)
{
}

EditorState::~EditorState()
{
}

ErrorCode EditorState::init(TaskManager *p_taskManager)
{
	ErrorCode returnError = EngineState::init(p_taskManager);

	if(returnError == ErrorCode::Success)
	{
		// Load the default map, and log an error if it wasn't successful
		returnError = m_sceneLoader.loadFromFile(Config::gameplayVar().play_map);
		
		if(returnError != ErrorCode::Success)
			ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);

		m_sceneLoader.getChangeController()->sendData(static_cast<GUIScene *>(m_sceneLoader.getSystemScene(Systems::GUI)), DataType::DataType_EditorWindow, (void *)&m_editorWindowSettings);

		m_initialized = true;
	}

	return returnError;
}

void EditorState::update(Engine &p_engine)
{
	m_scheduler->execute<>(std::function<void(SystemTask *, float)>(&SystemTask::update), ClockLocator::get().getDeltaSecondsF());

	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();
}

void EditorState::activate()
{
	EngineState::activate();
	//Config::m_engineVar.editorState = true;
}

void EditorState::deactivate()
{
	EngineState::deactivate();
	//Config::m_engineVar.editorState = false;
}
