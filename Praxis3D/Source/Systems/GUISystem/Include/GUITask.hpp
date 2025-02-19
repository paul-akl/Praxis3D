#pragma once

#include "Systems/Base/Include/System.hpp"

class GUIScene;

class GUITask : public SystemTask
{
	friend class GUIScene;
public:
	GUITask(GUIScene* p_GUIScene);
	~GUITask();

	Systems::TypeID getSystemType() { return Systems::GUI; }

	void update(const float p_deltaTime);

	bool isPrimaryThreadOnly() { return false; }

private:
	GUIScene* m_GUIScene;
};