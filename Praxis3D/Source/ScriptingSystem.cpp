
#include "ErrorHandlerLocator.h"
#include "NullSystemObjects.h"
#include "ScriptingScene.h"
#include "ScriptingSystem.h"

ScriptingSystem::ScriptingSystem()
{
	m_scriptingScene = nullptr;
}

ScriptingSystem::~ScriptingSystem()
{

}

ErrorCode ScriptingSystem::init()
{
	ErrorCode returnCode = ErrorCode::Success;
	
	ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Scripting, "Scripting system has been initialized");
	
	return returnCode;
}

ErrorCode ScriptingSystem::setup(const PropertySet &p_properties)
{
	/*for(decltype(p_properties.getNumProperties()) i = 0, size = p_properties.getNumProperties(); i < size; i++)
	{
		switch(p_properties[i].getPropertyID())
		{

		}
	}*/

	return ErrorCode::Success;
}

void ScriptingSystem::loadInBackground()
{
}

SystemScene *ScriptingSystem::createScene(SceneLoader *p_sceneLoader)
{
	if(m_scriptingScene == nullptr)
	{
		// Create new scene
		m_scriptingScene = new ScriptingScene(this, p_sceneLoader);
		ErrorCode sceneError = m_scriptingScene->init();

		// Check if it initialized correctly (cannot continue without the scene)
		if(sceneError != ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(sceneError);
		}
	}

	return m_scriptingScene;
}

SystemScene *ScriptingSystem::getScene()
{
	return m_scriptingScene;
}
