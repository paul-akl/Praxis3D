#pragma once

#include "System.h"
#include "WorldScene.h"

class WorldSystem : public SystemBase
{
public:
	WorldSystem() 
	{ 
		m_worldScene = nullptr;
		m_systemName = GetString(Systems::World);
	}
	~WorldSystem() { }

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

	virtual ErrorCode preload()
	{
		ErrorCode returnCode = ErrorCode::Success;

		return returnCode;
	}
	void loadInBackground() { }

	Systems::TypeID getSystemType() { return Systems::World; }

	SystemScene *createScene(SceneLoader *p_sceneLoader)
	{
		if(m_worldScene == nullptr)
		{
			// Create new scene
			m_worldScene = new WorldScene(this, p_sceneLoader);
			ErrorCode sceneError = m_worldScene->init();

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

		return m_worldScene;
	}

	SystemScene *getScene() { return m_worldScene; }

protected:
	WorldScene *m_worldScene;
};