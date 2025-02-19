#pragma once

#include "Systems/Base/Include/System.hpp"

class ScriptScene;

class ScriptSystem : public SystemBase
{
public:
	ScriptSystem();
	~ScriptSystem();

	ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	void loadInBackground();

	Systems::TypeID getSystemType() { return Systems::Script; }

	SystemScene *createScene(SceneLoader *p_sceneLoader, EngineStateType p_engineState);

	SystemScene *getScene(EngineStateType p_engineState);

	void deleteScene(EngineStateType p_engineState);
	
protected:
	ScriptScene *m_scriptingScenes[EngineStateType::EngineStateType_NumOfTypes];
};

