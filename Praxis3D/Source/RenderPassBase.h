#pragma once

#include "CommandBuffer.h"
#include "Config.h"
#include "RendererFrontend.h"

class RenderPass
{
public:
	RenderPass(RendererFrontend &p_renderer) : m_renderer(p_renderer) { }

	virtual ErrorCode init() = 0;

	virtual void update(const SceneObjects &p_sceneObjects, const float p_deltaTime) = 0;

	inline CommandBuffer::Commands &getCommands() { return m_commandBuffer.getCommands(); }

	inline void setID(unsigned int p_ID) { m_ID = p_ID; }

	inline const std::string &getName() const { return m_name; }
	inline unsigned int getID() const { return m_ID; }

protected:
	unsigned int m_ID;
	std::string m_name;

	RendererFrontend &m_renderer;
	CommandBuffer m_commandBuffer;
};