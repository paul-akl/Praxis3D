#pragma once

#include "Systems/Base/Include/System.hpp"
#include "Systems/WorldSystem/Include/WorldScene.hpp"

class WorldSystem : public SystemBase
{
public:
	WorldSystem() 
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			m_worldScenes[i] = nullptr;

		m_systemName = GetString(Systems::World);
	}
	~WorldSystem() 
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			if(m_worldScenes[i] != nullptr)
				delete m_worldScenes[i];
	}

	ErrorCode init()
	{
		ErrorCode returnCode = ErrorCode::Success;

		ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_WorldSystem);

		return returnCode;
	}

	ErrorCode setup(const PropertySet &p_properties)
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}

	ErrorCode preload()
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}
	void loadInBackground() { }

	Systems::TypeID getSystemType() { return Systems::World; }

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
	{
		if(m_worldScenes[p_engineState] == nullptr)
		{
			// Create new scene
			m_worldScenes[p_engineState] = new WorldScene(this, p_sceneLoader);
			ErrorCode sceneError = m_worldScenes[p_engineState]->init();

			// Check if it initialized correctly (cannot continue without the scene)
			if(sceneError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(sceneError);
			}
			else
			{
				// Check for errors
				GLenum glError = glGetError();
			}
		}

		return m_worldScenes[p_engineState];
	}

	SystemScene *getScene(EngineStateType p_engineState) { return m_worldScenes[p_engineState]; }

	void deleteScene(EngineStateType p_engineState)
	{
		if(m_worldScenes[p_engineState] != nullptr)
		{
			// Shutdown the scene before destroying it
			m_worldScenes[p_engineState]->shutdown();

			delete m_worldScenes[p_engineState];
			m_worldScenes[p_engineState] = nullptr;
		}
	}

protected:
	WorldScene *m_worldScenes[EngineStateType::EngineStateType_NumOfTypes];
};