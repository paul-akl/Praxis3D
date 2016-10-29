#pragma once

#include "Config.h"
#include "RendererBackend.h"
#include "RendererScene.h"

class RenderPass;

class RendererFrontend
{
	friend class GeometryPass;
	friend class LightingPass;
	friend class FinalPass;
	friend class ReflectionPass;
public:
	RendererFrontend() { }
	~RendererFrontend() { }

	ErrorCode init();

	// Renders a complete frame
	void renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime);

protected:
	struct LightUniformBuffer
	{
		LightUniformBuffer() : m_bufferType(BufferType_Uniform), m_bufferUsage(BufferUsageHint_Dynamic)
		{ 
			m_size = 0;
			m_handle = 0;
			m_bindingIndex = 0;
		}

		int64_t m_size;
		unsigned int m_handle;
		unsigned int m_bindingIndex;

		const BufferType m_bufferType;
		const BufferUsageHint m_bufferUsage;
	};

	inline void queueForDrawing(const RenderableObjectData &p_object, const unsigned int p_shaderHandle, const ShaderUniformUpdater &p_uniformUpdater, const Math::Mat4f &p_viewProjMatrix)
	{
		// Get the neccessary handles
		const unsigned int modelHandle = p_object.m_model.getHandle();

		// Calculare model-view-projection matrix
		const Math::Mat4f modelViewProjMatrix = p_viewProjMatrix * p_object.m_baseObjectData.m_modelMat;

		// Assign the object data that is later passed to the shaders
		const UniformObjectData objectData(p_object.m_baseObjectData.m_modelMat,
										   modelViewProjMatrix,
										   p_object.m_baseObjectData.m_heightScale,
										   p_object.m_baseObjectData.m_alphaThreshold,
										   p_object.m_baseObjectData.m_emissiveThreshold,
										   p_object.m_baseObjectData.m_textureTilingFactor);

		// Calculate the sort key
		RendererBackend::DrawCommands::value_type::first_type sortKey = 0;

		// Add a draw command for each mesh, using the same object data
		for(decltype(p_object.m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_object.m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
		{
			m_drawCommands.emplace_back(
				sortKey,
				RendererBackend::DrawCommand(
					p_uniformUpdater,
					objectData,
					p_shaderHandle,
					modelHandle,
					p_object.m_model[meshIndex].m_numIndices,
					p_object.m_model[meshIndex].m_baseVertex,
					p_object.m_model[meshIndex].m_baseIndex)
				);
		}
	}

	inline void queueForLoading(ShaderLoader::ShaderProgram &p_shader)
	{
		m_loadCommands.emplace_back(p_shader.m_programHandle, 
									p_shader.getUniformUpdater(),
									p_shader.m_shaderSource, 
									p_shader.m_errorMessages);
	}
	inline void queueForLoading(ModelLoader::ModelHandle &p_model)
	{
		m_loadCommands.emplace_back(p_model.m_model->m_handle,
									p_model.m_model->m_buffers,
									p_model.m_model->m_numElements,
									p_model.m_model->m_bufferSize,
									p_model.m_model->getData());
	}
	inline void queueForLoading(TextureLoader2D::Texture2DHandle &p_texture)
	{
		m_loadCommands.emplace_back(p_texture.getHandleRef(),
									 p_texture.getTextureFormat(),
									 p_texture.getMipmapLevel(),
									 p_texture.getTextureWidth(),
									 p_texture.getTextureHeight(),
									 p_texture.getData());
	}
	inline void queueForLoading(RenderableObjectData &p_objectData)
	{
		// Load model
		queueForLoading(p_objectData.m_model);

		// Load shader only if it is custom
		if(!p_objectData.m_shader->isDefaultProgram())
			queueForLoading(*p_objectData.m_shader);

		// Load all materials
		for(decltype(p_objectData.m_numMaterials) i = 0; i < p_objectData.m_numMaterials; i++)
			for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
				queueForLoading(p_objectData.m_materials[matType][i]);
	}

	inline void queueForUpdate(LightUniformBuffer &p_lightBuffer)
	{
		m_loadCommands.emplace_back(p_lightBuffer.m_handle,
									p_lightBuffer.m_bufferType,
									p_lightBuffer.m_bufferUsage,
									p_lightBuffer.m_bindingIndex,
									p_lightBuffer.m_size,
									(void*)0);
	}

	inline void passDrawCommandsToBackend()
	{
		// Pass the queued load commands to the backend to be loaded to GPU
		m_backend.processLoading(m_loadCommands, m_frameData);

		// Clear load commands, since they have already been passed to backend
		m_loadCommands.clear();
	}

	// Recalculates the projection matrix
	inline void updateProjectionMatrix()
	{
		m_frameData.m_projMatrix.perspective(Config::graphicsVar().fov, 
											 m_frameData.m_screenSize.x, 
											 m_frameData.m_screenSize.y, 
											 Config::graphicsVar().z_near, 
											 Config::graphicsVar().z_far);
	}

	// Renderer backend, servers as an interface layer to GPU
	RendererBackend m_backend;
	
	UniformFrameData m_frameData;

	RendererBackend::DrawCommands m_drawCommands;

	RendererBackend::LoadCommands m_loadCommands;

	std::vector<RenderPass*> m_renderingPasses;

	Math::Mat4f m_viewProjMatrix;
};