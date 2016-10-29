#pragma once

#include "System.h"

class ScriptingScene;

class ScriptingTask : public SystemTask
{
	friend class ScriptingScene;
public:
	ScriptingTask(ScriptingScene *p_scriptingScene);
	~ScriptingTask();

	Systems::TypeID getSystemType() { return Systems::Scripting; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return false; }

protected:
	ScriptingScene *m_scriptingScene;
};

