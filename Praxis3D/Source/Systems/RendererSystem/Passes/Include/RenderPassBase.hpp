#pragma once

#include "Systems/RendererSystem/Include/CommandBuffer.hpp"
#include "Loaders/Include/Config.hpp"
#include "Systems/RendererSystem/Include/RendererFrontend.hpp"

// Used to share data between rendering passes
struct RenderPassData
{
	RenderPassData()
	{
		m_colorInputMap = GBufferTextureType::GBufferDiffuse;
		m_colorOutputMap = GBufferTextureType::GBufferFinal;
		m_emissiveInputMap = GBufferTextureType::GBufferEmissive;

		m_blurInputMap = GBufferTextureType::GBufferEmissive;
		m_blurOutputMap = GBufferTextureType::GBufferEmissive;
		m_blurBlendingMap = GBufferTextureType::GBufferFinal;

		m_intermediateMap = GBufferTextureType::GBufferIntermediate;

		m_GUIPassFunctorSequence = nullptr;

		m_numOfBlurPasses = 1;

		m_atmScatDoSkyPass = true;
		m_blurDoBlending = false;
		m_renderFinalToTexture = false;
	}

	inline void swapColorInputOutputMaps()
	{
		auto inputColorMap = getColorInputMap();
		setColorInputMap(getColorOutputMap());
		setColorOutputMap(inputColorMap);
	}

	// Setters
	inline void setColorInputMap(GBufferTextureType p_inputColorMap)			{ m_colorInputMap = p_inputColorMap;			}
	inline void setColorOutputMap(GBufferTextureType p_outputColorMap)			{ m_colorOutputMap = p_outputColorMap;			}
	inline void setEmissiveInputMap(GBufferTextureType p_emissiveInputMap)		{ m_emissiveInputMap = p_emissiveInputMap;		}
	inline void setBlurInputMap(GBufferTextureType p_blurInputMap)				{ m_blurInputMap = p_blurInputMap;				}
	inline void setBlurOutputMap(GBufferTextureType p_blurOutputMap)			{ m_blurOutputMap = p_blurOutputMap;			}
	inline void setBlurBlendingMap(GBufferTextureType p_blurBlendingMap)		{ m_blurBlendingMap = p_blurBlendingMap;		}
	inline void setIntermediateMap(GBufferTextureType p_intermediateMap)		{ m_intermediateMap = p_intermediateMap;		}
	inline void setRenderFinalToTexture(const bool p_renderToTexture)			{ m_renderFinalToTexture = p_renderToTexture;	}
	inline void setGUIPassFunctorSequence(FunctorSequence *p_functorSequence)	{ m_GUIPassFunctorSequence = p_functorSequence; }

	// Getters
	const inline GBufferTextureType getColorInputMap() const	{ return m_colorInputMap;			}
	const inline GBufferTextureType getColorOutputMap() const	{ return m_colorOutputMap;			}
	const inline GBufferTextureType getEmissiveInputMap() const { return m_emissiveInputMap;		}
	const inline GBufferTextureType getBlurInputMap() const		{ return m_blurInputMap;			}
	const inline GBufferTextureType getBlurOutputMap() const	{ return m_blurOutputMap;			}
	const inline GBufferTextureType getBlurBlendingMap() const	{ return m_blurBlendingMap;			}
	const inline GBufferTextureType getIntermediateMap() const	{ return m_intermediateMap;			}
	const inline bool getRenderFinalToTexture() const			{ return m_renderFinalToTexture;	}
	inline FunctorSequence *getGUIPassFunctorSequence() 
	{ 
		FunctorSequence *returnFunctorSequence = m_GUIPassFunctorSequence;
		m_GUIPassFunctorSequence = nullptr; 
		return returnFunctorSequence;
	}

	// Remember which color maps to write to and read from, in different rendering passes.
	GBufferTextureType	m_colorInputMap,
						m_colorOutputMap,
						m_emissiveInputMap,
						m_blurInputMap,
						m_blurOutputMap,
						m_blurBlendingMap,
						m_intermediateMap;

	// Used to pass GUI calls to the GUI pass that require to be called from the main (graphics) thread
	FunctorSequence *m_GUIPassFunctorSequence;

	unsigned int m_numOfBlurPasses;
	bool m_atmScatDoSkyPass;
	bool m_blurDoBlending;
	bool m_renderFinalToTexture;
};

class RenderPass
{
public:
	RenderPass(RendererFrontend &p_renderer, RenderPassType p_renderPassType) : m_renderer(p_renderer), m_renderPassType(p_renderPassType), m_ID(0), m_initialized(false) { }
	virtual ~RenderPass() { }

	virtual ErrorCode init() = 0;

	virtual void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime) = 0;

	inline CommandBuffer::Commands &getCommands() { return m_commandBuffer.getCommands(); }

	inline void setID(unsigned int p_ID) { m_ID = p_ID; }

	inline const std::string &getName() const { return m_name; }
	inline unsigned int getID() const { return m_ID; }
	RenderPassType getRenderPassType() const { return m_renderPassType; }
	inline bool isInitialized() const { return m_initialized; }

protected:
	inline void setInitialized(const bool p_initialized) { m_initialized = p_initialized; }

	// Calculates the maximum mipmap levels based on the image size and mipmap / downscale limits 
	inline unsigned int calculateMipmapLevels(const unsigned int p_width, const unsigned int p_height, const unsigned int p_mipmapLimit, const unsigned int p_downscaleLimit) const
	{
		unsigned int width = p_width / 2;
		unsigned int height = p_height / 2;
		unsigned int mipLevels = 1;

		for(unsigned int i = 0; i < p_mipmapLimit; i++)
		{
			width = width / 2;
			height = height / 2;

			if(width < p_downscaleLimit || height < p_downscaleLimit) break;

			mipLevels++;
		}

		return mipLevels + 1;
	}

	unsigned int m_ID;
	bool m_initialized;
	RenderPassType m_renderPassType;
	std::string m_name;

	RendererFrontend &m_renderer;
	CommandBuffer m_commandBuffer;
};