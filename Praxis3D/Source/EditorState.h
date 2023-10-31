#pragma once

#include "SceneLoader.h"
#include "EngineState.h"
#include "TaskManager.h"
#include "TaskScheduler.h"

class EditorState : public EngineState
{
public:
	EditorState();
	~EditorState();

	ErrorCode init(TaskManager *p_taskManager);
	void update(Engine &p_engine);

	void activate();

	void deactivate();

	const EngineStateType getEngineStateType() final { return EngineStateType::EngineStateType_Editor; }

private:
	EditorWindowSettings m_editorWindowSettings;
};