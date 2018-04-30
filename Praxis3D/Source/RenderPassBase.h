#pragma once

#include "CommandBuffer.h"
#include "Config.h"
#include "RendererFrontend.h"

// Used to share data between rendering passes
struct RenderPassData
{
	RenderPassData()
	{
		m_colorInputMap = GeometryBuffer::GBufferDiffuse;
		m_colorOutputMap = GeometryBuffer::GBufferFinal;
		m_emissiveInputMap = GeometryBuffer::GBufferEmissive;
	}

	inline void swapColorInputOutputMaps()
	{
		auto inputColorMap = getColorInputMap();
		setColorInputMap(getColorOutputMap());
		setColorOutputMap(inputColorMap);
	}

	// Setters
	inline void setColorInputMap(GeometryBuffer::GBufferTextureType p_inputColorMap)		{ m_colorInputMap = p_inputColorMap;		}
	inline void setColorOutputMap(GeometryBuffer::GBufferTextureType p_outputColorMap)		{ m_colorOutputMap = p_outputColorMap;		}
	inline void setEmissiveInputMap(GeometryBuffer::GBufferTextureType p_emissiveInputMap)	{ m_emissiveInputMap = p_emissiveInputMap;	}

	// Getters
	const inline GeometryBuffer::GBufferTextureType getColorInputMap() const	{ return m_colorInputMap;		}
	const inline GeometryBuffer::GBufferTextureType getColorOutputMap() const	{ return m_colorOutputMap;		}
	const inline GeometryBuffer::GBufferTextureType getEmissiveInputMap() const { return m_emissiveInputMap;	}

	// Remember which color maps to write to and read from, in different rendering passes.
	GeometryBuffer::GBufferTextureType	m_colorInputMap,
										m_colorOutputMap,
										m_emissiveInputMap;
};

class RenderPass
{
public:
	RenderPass(RendererFrontend &p_renderer) : m_renderer(p_renderer) { }
	virtual ~RenderPass() { }

	virtual ErrorCode init() = 0;

	virtual void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime) = 0;

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