#pragma once

#include "Loaders/Include/SceneLoader.hpp"
#include "Engine/States/Include/EngineState.hpp"
#include "Multithreading/Include/TaskManager.hpp"
#include "Multithreading/Include/TaskScheduler.hpp"

class EditorState : public EngineState
{
public:
	EditorState(Engine &p_engine);
	~EditorState();

	ErrorCode load();

	ErrorCode load(const PropertySet &p_sceneProperty);

	void update(Engine &p_engine);

	void activate();

	void deactivate();

private:
	EditorWindowSettings m_editorWindowSettings;
};