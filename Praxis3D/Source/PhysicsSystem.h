#pragma once

#include "ErrorHandlerLocator.h"
#include "System.h"
#include "PhysicsScene.h"

class PhysicsSystem : public SystemBase
{
public:
	PhysicsSystem()
	{
		m_physicsScene = nullptr;
		m_systemName = GetString(Systems::Physics);
	}
	~PhysicsSystem() { }

	ErrorCode init()
	{
		ErrorCode returnCode = ErrorCode::Success;

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

	Systems::TypeID getSystemType() { return Systems::Physics; }

	SystemScene *createScene(SceneLoader *p_sceneLoader)
	{
		if(m_physicsScene == nullptr)
		{
			// Create new scene
			m_physicsScene = new PhysicsScene(this, p_sceneLoader);
			ErrorCode sceneError = m_physicsScene->init();

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

		return m_physicsScene;
	}

	SystemScene *getScene() { return m_physicsScene; }

protected:
	PhysicsScene *m_physicsScene;
};