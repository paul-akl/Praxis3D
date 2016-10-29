#pragma once

#include "Config.h"
#include "RendererBackend.h"
#include "RendererScene.h"

class RendererFrontend
{
public:
	RendererFrontend() { }
	~RendererFrontend() { }

	ErrorCode init();

	void renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime);

	void queueForDrawing(const RenderableObjectData &p_object);

protected:
	//RendererBackend::RenderableObjects m_objects;
};