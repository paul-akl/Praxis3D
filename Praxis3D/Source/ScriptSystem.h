#pragma once

#include "System.h"

class ScriptScene;

class ScriptSystem : public SystemBase
{
public:
	ScriptSystem();
	virtual ~ScriptSystem();

	virtual ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	//virtual ErrorCode preload();

	void loadInBackground();

	Systems::TypeID getSystemType() { return Systems::Script; }

	virtual SystemScene *createScene(SceneLoader *p_sceneLoader);

	virtual SystemScene *getScene();
	
protected:
	ScriptScene *m_scriptingScene;
};

