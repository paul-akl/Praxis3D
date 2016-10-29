#pragma once

#include "ErrorCodes.h"
#include "ShaderLoader.h"
#include "ShaderUniforms.h"

class ShaderUniformUpdater
{
public:
	//ShaderUniformUpdater(unsigned int p_shaderHandle) : m_shaderHandle(p_shaderHandle) { }
	ShaderUniformUpdater(ShaderLoader::ShaderProgram &p_shader) : m_shader(p_shader)
	{
		m_numUpdatesPerFrame = 0;
		m_numUpdatesPerModel = 0;
		m_numUpdatesPerMesh = 0;
		m_numTextureUpdates = 0;
	}
	~ShaderUniformUpdater()
	{
		m_updatesPerFrame.clear();
		m_updatesPerModel.clear();
		m_updatesPerMesh.clear();
		m_textureUpdates.clear();
	}

	// Checks which uniforms are used in the shader, and generates a list of valid ones
	ErrorCode generateUpdateList();

	// Should be called once per frame
	const inline void updateFrame(RendererState &p_rendererState) const
	{
		for(decltype(m_updatesPerFrame.size()) i = 0, size = m_updatesPerFrame.size(); i < size; i++)
			m_updatesPerFrame[i]->update(p_rendererState);
	}
	// Should be called once per model
	const inline void updateModel(RendererState &p_rendererState) const
	{
		for(decltype(m_updatesPerModel.size()) i = 0, size = m_updatesPerModel.size(); i < size; i++)
			m_updatesPerModel[i]->update(p_rendererState);
	}
	// Should be called once per mesh
	const inline void updateMesh(RendererState &p_rendererState) const
	{
		for(decltype(m_updatesPerMesh.size()) i = 0, size = m_updatesPerMesh.size(); i < size; i++)
			m_updatesPerMesh[i]->update(p_rendererState);
	}
	// Should be called when texture handles need updating
	const inline void updateTextureUniforms(RendererState &p_rendererState) const
	{
		for(decltype(m_textureUpdates.size()) i = 0, size = m_textureUpdates.size(); i < size; i++)
			m_textureUpdates[i]->update(p_rendererState);
	}

	inline size_t getNumUpdatesPerFrame() const	{ return m_numUpdatesPerFrame;	}
	inline size_t getNumUpdatesPerModel() const { return m_numUpdatesPerModel;	}
	inline size_t getNumUpdatesPerMesh() const	{ return m_numUpdatesPerMesh;	}
	inline size_t getNumTextureUpdates() const	{ return m_numTextureUpdates;	}

private:
	ErrorCode generateTextureUpdateList();
	ErrorCode generatePerFrameList();
	ErrorCode generatePerModelList();
	ErrorCode generatePerMeshList();

	std::vector<BaseUniform*>	m_updatesPerFrame,
								m_updatesPerModel,
								m_updatesPerMesh,
								m_textureUpdates;

	size_t	m_numUpdatesPerFrame,
			m_numUpdatesPerModel,
			m_numUpdatesPerMesh,
			m_numTextureUpdates;

	unsigned int m_shaderHandle;
	ShaderLoader::ShaderProgram &m_shader;
};