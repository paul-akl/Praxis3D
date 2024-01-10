#pragma once

#include "ErrorCodes.h"
#include "ShaderLoader.h"
#include "ShaderUniforms.h"

//class BaseUniform;
//class BaseUniformBlock;

// Automatically generates arrays of available uniforms in the specific shader
// All uniforms are updates through this class
// Should only be accessible by the renderer backend, since this class deals directly with GPU
class ShaderUniformUpdater
{
	friend class RendererBackend;
	friend class LightingPass;
public:
	ShaderUniformUpdater(ShaderLoader::ShaderProgram &p_shader) : m_shader(p_shader)
	{
		m_numUpdatesPerFrame = 0;
		m_numUpdatesPerModel = 0;
		m_numUpdatesPerMesh = 0;
		m_numTextureUpdates = 0;
		m_numUniformBlockUpdates = 0;
		m_numSSBBBlockUpdates = 0;
		m_shaderHandle = 0;
	}
	~ShaderUniformUpdater()
	{
		m_updatesPerFrame.clear();
		m_updatesPerModel.clear();
		m_updatesPerMesh.clear();
		m_textureUpdates.clear();
		m_uniformBlockUpdates.clear();
		m_SSBBlockUpdates.clear();
	}

	inline size_t getNumUpdatesPerFrame() const	{ return m_numUpdatesPerFrame;		}
	inline size_t getNumUpdatesPerModel() const { return m_numUpdatesPerModel;		}
	inline size_t getNumUpdatesPerMesh() const	{ return m_numUpdatesPerMesh;		}
	inline size_t getNumTextureUpdates() const	{ return m_numTextureUpdates;		}
	inline size_t getNumUniformBlocks() const	{ return m_numUniformBlockUpdates;	}
	inline size_t getNumSSBBufferBlocks() const	{ return m_numSSBBBlockUpdates;		}

	const inline std::vector<BaseUniform *> &getPerFrameUniforms() const		{ return m_updatesPerFrame; }
	const inline std::vector<BaseUniform *> &getPerModelUniforms() const		{ return m_updatesPerModel; }
	const inline std::vector<BaseUniform *> &getPerMeshUniforms() const			{ return m_updatesPerMesh;	}
	const inline std::vector<BaseUniform *> &getTextureUpdateUniforms() const	{ return m_textureUpdates;	}

	const inline std::vector<BaseUniformBlock *> &getUniformBlocks() const		{ return m_uniformBlockUpdates; }
	const inline std::vector<BaseShaderStorageBlock *> &getSSBblocks() const	{ return m_SSBBlockUpdates;		}

private:
	// Clears all the internal arrays that are populated by generateUpdateList()
	const inline void clearUpdateList()
	{
		// Delete all uniforms
		for(auto *uniform : m_updatesPerFrame)
			delete uniform;
		for(auto *uniform : m_updatesPerModel)
			delete uniform;
		for(auto *uniform : m_updatesPerMesh)
			delete uniform;
		for(auto *uniform : m_textureUpdates)
			delete uniform;
		for(auto *uniform : m_uniformBlockUpdates)
			delete uniform;
		for(auto *uniform : m_SSBBlockUpdates)
			delete uniform;

		// Clear arrays
		m_updatesPerFrame.clear();
		m_updatesPerModel.clear();
		m_updatesPerMesh.clear();
		m_textureUpdates.clear();
		m_uniformBlockUpdates.clear();
		m_SSBBlockUpdates.clear();
	}

	// Checks which uniforms are used in the shader, and generates a list of valid ones
	ErrorCode generateUpdateList();

	// Should be called once per frame
	const inline void updateFrame(const UniformData &p_uniformData = m_defaultUniformData) const
	{
		for(decltype(m_numUpdatesPerFrame) i = 0; i < m_numUpdatesPerFrame; i++)
			m_updatesPerFrame[i]->update(p_uniformData);
	}

	// Should be called once per model
	const inline void updateModel(const UniformData &p_uniformData = m_defaultUniformData) const
	{
		for(decltype(m_numUpdatesPerModel) i = 0; i < m_numUpdatesPerModel; i++)
			m_updatesPerModel[i]->update(p_uniformData);
	}

	// Should be called once per mesh
	const inline void updateMesh(const UniformData &p_uniformData = m_defaultUniformData) const
	{
		for(decltype(m_numUpdatesPerMesh) i = 0; i < m_numUpdatesPerMesh; i++)
			m_updatesPerMesh[i]->update(p_uniformData);
	}

	// Should be called when texture handles need updating
	const inline void updateTextureUniforms(const UniformData &p_uniformData = m_defaultUniformData) const
	{
		for(decltype(m_numTextureUpdates) i = 0; i < m_numTextureUpdates; i++)
			m_textureUpdates[i]->update(p_uniformData);
	}

	// Should never be called, unless a binding point for a uniform block has changed
	// (Does not update the data in the uniform block, only updates the binding point)
	const inline void updateBlockBindingPoints(const UniformData &p_uniformData = m_defaultUniformData) const
	{
		for(decltype(m_numUniformBlockUpdates) i = 0; i < m_numUniformBlockUpdates; i++)
			m_uniformBlockUpdates[i]->update(p_uniformData);
	}

	// Should never be called, unless a binding point for a uniform block has changed
	// (Does not update the data in the uniform block, only updates the binding point)
	const inline void updateSSBBindingPoints(const UniformData &p_uniformData = m_defaultUniformData) const
	{
		for(decltype(m_numSSBBBlockUpdates) i = 0; i < m_numSSBBBlockUpdates; i++)
			m_SSBBlockUpdates[i]->update(p_uniformData);
	}

	ErrorCode generateTextureUpdateList();
	ErrorCode generatePerFrameList();
	ErrorCode generatePerModelList();
	ErrorCode generatePerMeshList();
	ErrorCode generateUniformBlockList();
	ErrorCode generateSSBBlockList();

	std::vector<BaseUniform*>	m_updatesPerFrame,
								m_updatesPerModel,
								m_updatesPerMesh,
								m_textureUpdates;

	std::vector<BaseUniformBlock*> m_uniformBlockUpdates;

	std::vector<BaseShaderStorageBlock*> m_SSBBlockUpdates;

	size_t	m_numUpdatesPerFrame,
			m_numUpdatesPerModel,
			m_numUpdatesPerMesh,
			m_numTextureUpdates,
			m_numUniformBlockUpdates,
			m_numSSBBBlockUpdates;

	unsigned int m_shaderHandle;
	ShaderLoader::ShaderProgram &m_shader;

	// (Quick, but safe hack) Used to enable calling uniform updates without any arguments
	const static UniformObjectData m_defaultObjectData;
	const static UniformFrameData m_defaultFrameData;
	const static UniformData m_defaultUniformData;
};