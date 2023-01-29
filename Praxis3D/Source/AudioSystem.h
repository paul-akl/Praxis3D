#pragma once

#include "AudioScene.h"
#include "ErrorHandlerLocator.h"
#include "System.h"

class AudioSystem : public SystemBase
{
public:
	AudioSystem()
	{
		m_audioScene = nullptr;
		m_systemName = GetString(Systems::Audio);
	}
	~AudioSystem() { }

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

	SystemScene *createScene(SceneLoader *p_sceneLoader)
	{
		if(m_audioScene == nullptr)
		{
			// Create new scene
			m_audioScene = new AudioScene(this, p_sceneLoader);
			ErrorCode sceneError = m_audioScene->init();

			// Check if it initialized correctly (cannot continue without the scene)
			if(sceneError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(sceneError, ErrorSource::Source_AudioScene);
			}
		}

		return m_audioScene;
	}

	SystemScene *getScene() { return m_audioScene; }

protected:
	AudioScene *m_audioScene;
};