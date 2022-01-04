#pragma once

#include "ErrorHandlerLocator.h"
#include "System.h"
#include "GUIScene.h"

class GUISystem : public SystemBase
{
public:
	GUISystem()
	{
		m_GUIScene = nullptr;
		m_systemName = GetString(Systems::GUI);
	}
	~GUISystem() { }

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

	SystemScene *createScene(SceneLoader *p_sceneLoader)
	{
		if (m_GUIScene == nullptr)
		{
			// Create new scene
			m_GUIScene = new GUIScene(this, p_sceneLoader);
			ErrorCode sceneError = m_GUIScene->init();

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

		return m_GUIScene;
	}

	SystemScene* getScene() { return m_GUIScene; }

protected:
	GUIScene* m_GUIScene;
};