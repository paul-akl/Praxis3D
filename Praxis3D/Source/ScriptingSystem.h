#pragma once

#include "System.h"

class ScriptingScene;

class ScriptingSystem : public SystemBase
{
public:
	ScriptingSystem();
	virtual ~ScriptingSystem();

	virtual ErrorCode init();

	ErrorCode setup(const PropertySet &p_properties);

	//virtual ErrorCode preload();

	void loadInBackground();

	virtual std::string getName() { return GetString(Systems::Scripting); }
	Systems::TypeID getSystemType() { return Systems::Scripting; }

	virtual SystemScene *createScene(SceneLoader *p_sceneLoader);

	virtual SystemScene *getScene();
	
protected:
	ScriptingScene *m_scriptingScene;
};

