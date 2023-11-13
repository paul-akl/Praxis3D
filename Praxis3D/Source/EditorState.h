#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class EditorState : public EngineState
{
public:
	EditorState(Engine &p_engine);
	~EditorState();

	ErrorCode load();

	void update(Engine &p_engine);

	void activate();

	void deactivate();

private:
	EditorWindowSettings m_editorWindowSettings;
};