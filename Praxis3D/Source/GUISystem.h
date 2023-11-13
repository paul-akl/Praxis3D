#pragma once

#include "ErrorHandlerLocator.h"
#include "System.h"
#include "GUIScene.h"

class GUISystem : public SystemBase
{
public:
	GUISystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			m_GUIScenes[i] = nullptr;

		m_systemName = GetString(Systems::GUI);
	}
	~GUISystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			if(m_GUIScenes[i] != nullptr)
				delete m_GUIScenes[i];
	}

	ErrorCode init()
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}

	ErrorCode setup(const PropertySet& p_properties)
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}

	virtual ErrorCode preload()
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}
	void loadInBackground() { }

	Systems::TypeID getSystemType() { return Systems::GUI; }

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
	{
		if(m_GUIScenes[p_engineState] == nullptr)
		{
			// Create new scene
			m_GUIScenes[p_engineState] = new GUIScene(this, p_sceneLoader);
			ErrorCode sceneError = m_GUIScenes[p_engineState]->init();

			// Check if it initialized correctly (cannot continue without the scene)
			if (sceneError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(sceneError);
			}
			else
			{
				// Check for errors
				GLenum glError = glGetError();
			}
		}

		return m_GUIScenes[p_engineState];
	}

	SystemScene* getScene(EngineStateType p_engineState) { return m_GUIScenes[p_engineState]; }

	void deleteScene(EngineStateType p_engineState)
	{
		if(m_GUIScenes[p_engineState] != nullptr)
		{
			delete m_GUIScenes[p_engineState];
			m_GUIScenes[p_engineState] = nullptr;
		}
	}

protected:
	GUIScene *m_GUIScenes[EngineStateType::EngineStateType_NumOfTypes];
};