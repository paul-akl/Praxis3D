#pragma once

#include "Systems/Base/Include/System.hpp"

class RenderTaskBase : public SystemTask
{
public:
	RenderTaskBase(SystemScene *p_rendererScene) : SystemTask(p_rendererScene) { }
	virtual ~RenderTaskBase() { }

	BitMask getSystemType() { return Systems::Graphics; }

	virtual void update(float p_deltaTime) = 0;

	bool isPrimaryThreadOnly() { return false; }
};

