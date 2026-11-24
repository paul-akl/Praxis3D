#include "ServiceLocators/Include/ClockLocator.hpp"
#include "Engine/States/Include/EditorState.hpp"
#include "Systems/GUISystem/Include/GUISystem.hpp"
#include "Systems/PhysicsSystem/Include/PhysicsSystem.hpp"
#include "Common/Include/PropertySet.hpp"
#include "Systems/ScriptingSystem/Include/ScriptSystem.hpp"
#include "Systems/RendererSystem/Include/RendererSystem.hpp"
#include "Systems/WorldSystem/Include/WorldSystem.hpp"

EditorState::EditorState(Engine &p_engine) : EngineState(p_engine, EngineStateType::EngineStateType_Editor)
{
	m_sceneFilename = Config::gameplayVar().play_map;
}

EditorState::~EditorState()
{
}

ErrorCode EditorState::load()
{
	ErrorCode returnError = ErrorCode::Success;

	if(m_initialized && !m_loaded)
	{
		// Load the scene map, and log an error if it wasn't successful
		returnError = m_sceneLoader.loadFromFile(m_sceneFilename);
		if(returnError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);
		}
		else
		{
			// Tell the GUI scene to create the editor window
			m_sceneLoader.getChangeController()->sendData(static_cast<GUIScene *>(m_sceneLoader.getSystemScene(Systems::GUI)), DataType::DataType_EditorWindow, (void *)&m_editorWindowSettings);
			m_loaded = true;
		}
	}

	return returnError;
}

ErrorCode EditorState::load(const PropertySet &p_sceneProperty)
{
	ErrorCode returnError = ErrorCode::Success;

	if(m_initialized && !m_loaded && p_sceneProperty)
	{
		// Load the scene map, and log an error if it wasn't successful
		returnError = m_sceneLoader.loadFromProperties(p_sceneProperty);
		if(returnError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(returnError, ErrorSource::Source_SceneLoader);
		}
		else
		{
			// Tell the GUI scene to create the editor window
			m_sceneLoader.getChangeController()->sendData(static_cast<GUIScene *>(m_sceneLoader.getSystemScene(Systems::GUI)), DataType::DataType_EditorWindow, (void *)&m_editorWindowSettings);
			m_loaded = true;
		}
	}

	return returnError;
}

void EditorState::update(Engine &p_engine)
{
	m_scheduler->execute<>(std::function<void(SystemTask *, float)>(&SystemTask::update), ClockLocator::get().getDeltaSecondsF());

	m_objectChangeController->distributeChanges();
	m_sceneChangeController->distributeChanges();

	updateSceneLoadingStatus();
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
