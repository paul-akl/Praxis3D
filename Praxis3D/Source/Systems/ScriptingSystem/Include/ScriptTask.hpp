#pragma once

#include "Systems/Base/Include/System.hpp"

class ScriptScene;

class ScriptTask : public SystemTask
{
	friend class ScriptScene;
public:
	ScriptTask(ScriptScene *p_scriptingScene);
	~ScriptTask();

	Systems::TypeID getSystemType() { return Systems::Script; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return false; }

protected:
	ScriptScene *m_scriptingScene;
};

