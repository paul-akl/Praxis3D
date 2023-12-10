#pragma once

#include "ErrorHandlerLocator.h"
#include "System.h"
#include "PhysicsScene.h"

class PhysicsSystem : public SystemBase
{
public:
	PhysicsSystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			m_physicsScenes[i] = nullptr;

		m_systemName = GetString(Systems::Physics);
	}
	~PhysicsSystem()
	{
		for(unsigned int i = 0; i < EngineStateType::EngineStateType_NumOfTypes; i++)
			if(m_physicsScenes[i] != nullptr)
				delete m_physicsScenes[i];
	}

	ErrorCode init()
	{
		ErrorCode returnCode = ErrorCode::Success;

		ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_Physics);

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

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState)
	{
		if(m_physicsScenes[p_engineState] == nullptr)
		{
			// Create new scene
			m_physicsScenes[p_engineState] = new PhysicsScene(this, p_sceneLoader);
			ErrorCode sceneError = m_physicsScenes[p_engineState]->init();

			// Check if it initialized correctly (cannot continue without the scene)
			if(sceneError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(sceneError);
			}
		}

		return m_physicsScenes[p_engineState];
	}

	SystemScene *getScene(EngineStateType p_engineState) { return m_physicsScenes[p_engineState]; }

	void deleteScene(EngineStateType p_engineState)
	{
		if(m_physicsScenes[p_engineState] != nullptr)
		{
			// Shutdown the scene before destroying it
			m_physicsScenes[p_engineState]->shutdown();

			delete m_physicsScenes[p_engineState];
			m_physicsScenes[p_engineState] = nullptr;
		}
	}

protected:
	PhysicsScene *m_physicsScenes[EngineStateType::EngineStateType_NumOfTypes];
};