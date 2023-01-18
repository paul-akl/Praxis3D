
#include "ErrorHandlerLocator.h"
#include "NullSystemObjects.h"
#include "ScriptScene.h"
#include "ScriptSystem.h"

ScriptSystem::ScriptSystem()
{
	m_scriptingScene = nullptr;
	m_systemName = GetString(Systems::Script);
}

ScriptSystem::~ScriptSystem()
{

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

SystemScene *ScriptSystem::createScene(SceneLoader *p_sceneLoader)
{
	if(m_scriptingScene == nullptr)
	{
		// Create new scene
		m_scriptingScene = new ScriptScene(this, p_sceneLoader);
		ErrorCode sceneError = m_scriptingScene->init();

		// Check if it initialized correctly (cannot continue without the scene)
		if(sceneError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(sceneError);
		}
	}

	return m_scriptingScene;
}

SystemScene *ScriptSystem::getScene()
{
	return m_scriptingScene;
}
