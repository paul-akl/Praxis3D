#pragma once

#include "AudioScene.h"
#include "ErrorHandlerLocator.h"
#include "System.h"

class AudioSystem : public SystemBase
{
public:
	AudioSystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			m_audioScenes[i] = nullptr;

		m_systemName = GetString(Systems::Audio);
	}
	~AudioSystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			if(m_audioScenes[i] != nullptr)
				delete m_audioScenes[i];
	}

	ErrorCode init()
	{
		ErrorCode returnCode = ErrorCode::Success;

		ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_AudioSystem);

		return returnCode;
	}

	ErrorCode setup(const PropertySet &p_properties)
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

	Systems::TypeID getSystemType() { return Systems::Audio; }

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
	{
		if(m_audioScenes[p_engineState] == nullptr)
		{
			// Create new scene
			m_audioScenes[p_engineState] = new AudioScene(this, p_sceneLoader);
			ErrorCode sceneError = m_audioScenes[p_engineState]->init();

			// Check if it initialized correctly (cannot continue without the scene)
			if(sceneError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(sceneError, ErrorSource::Source_AudioScene);
				delete m_audioScenes[p_engineState];
				m_audioScenes[p_engineState] = nullptr;
			}
		}

		return m_audioScenes[p_engineState];
	}

	SystemScene *getScene(EngineStateType p_engineState) { return m_audioScenes[p_engineState]; }

	void deleteScene(EngineStateType p_engineState)
	{
		if(m_audioScenes[p_engineState] != nullptr)
			delete m_audioScenes[p_engineState];
	}

protected:
	AudioScene *m_audioScenes[EngineStateType::EngineStateType_NumOfTypes];
};