
#include "ErrorHandlerLocator.h"
#include "NullSystemObjects.h"
#include "ScriptScene.h"
#include "ScriptSystem.h"

ScriptSystem::ScriptSystem()
{
	for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
		m_scriptingScenes[i] = nullptr;

	m_systemName = GetString(Systems::Script);
}

ScriptSystem::~ScriptSystem()
{
	for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
		if(m_scriptingScenes[i] != nullptr)
			delete m_scriptingScenes[i];
}

ErrorCode ScriptSystem::init()
{
	ErrorCode returnCode = ErrorCode::Success;
	
	ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_Script);
	
	return returnCode;
}

ErrorCode ScriptSystem::setup(const PropertySet &p_properties)
{
	/*for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{

		}
	}*/

	return ErrorCode::Success;
}

void ScriptSystem::loadInBackground()
{
}

SystemScene *ScriptSystem::createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
{
	if(m_scriptingScenes[p_engineState] == nullptr)
	{
		// Create new scene
		m_scriptingScenes[p_engineState] = new ScriptScene(this, p_sceneLoader);
		ErrorCode sceneError = m_scriptingScenes[p_engineState]->init();

		// Check if it initialized correctly (cannot continue without the scene)
		if(sceneError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(sceneError);
		}
	}

	return m_scriptingScenes[p_engineState];
}

SystemScene *ScriptSystem::getScene(EngineStateType p_engineState)
{
	return m_scriptingScenes[p_engineState];
}

void ScriptSystem::deleteScene(EngineStateType p_engineState)
{
	if(m_scriptingScenes[p_engineState] != nullptr)
		delete m_scriptingScenes[p_engineState];
}
