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
	// A handle for buffer that holds data of lights
	struct LightUniformBuffer
	{
		LightUniformBuffer() : m_bufferType(BufferType_Uniform), m_bufferUsage(BufferUsageHint_Dynamic)
		{
			m_data = 0;
			m_size = 0;
			m_offset = 0;
			m_handle = 0;
			m_bindingIndex = 0;
		}

		int64_t m_size;
		unsigned int m_offset;
		unsigned int m_handle;
		unsigned int m_bindingIndex;
		void *m_data;

		const BufferType m_bufferType;
		const BufferUsageHint m_bufferUsage;
	};

	RendererFrontend() { }
	~RendererFrontend() { }

	ErrorCode init();

	// Renders a complete frame
	void renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime);

protected:
	inline void queueForDrawing(const RenderableObjectData &p_object, const unsigned int p_shaderHandle, const ShaderUniformUpdater &p_uniformUpdater, const Math::Mat4f &p_viewProjMatrix)
	{
		// Get the neccessary handles
		const unsigned int modelHandle = p_object.m_model.getHandle();

		// Calculare model-view-projection matrix
		const Math::Mat4f modelViewProjMatrix = p_viewProjMatrix * p_object.m_baseObjectData.m_modelMat;

		unsigned int materials[MaterialType_NumOfTypes];

		for(decltype(p_object.m_materials[0].size()) i = 0; i < MaterialType_NumOfTypes; i++)
		{
			//materials[i] = p_object.m_materials[i]
		}

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
					p_object.m_model[meshIndex].m_baseIndex,
					p_object.m_materials[MaterialType_Diffuse][meshIndex].getHandle(),
					p_object.m_materials[MaterialType_Normal][meshIndex].getHandle(),
					p_object.m_materials[MaterialType_Emissive][meshIndex].getHandle(),
					p_object.m_materials[MaterialType_Combined][meshIndex].getHandle())
				);
		}
	}
	inline void queueForDrawing(const unsigned int p_shaderHandle, const ShaderUniformUpdater &p_uniformUpdater, const Math::Mat4f &p_viewProjMatrix)
	{
		// Assign the object data that is later passed to the shaders
		const UniformObjectData objectData(p_viewProjMatrix,
										   p_viewProjMatrix,
										   Config::graphicsVar().height_scale,
										   Config::graphicsVar().alpha_threshold,
										   Config::graphicsVar().emissive_threshold,
										   Config::graphicsVar().texture_tiling_factor);

		// Calculate the sort key
		RendererBackend::DrawCommands::value_type::first_type sortKey = 0;

		m_screenSpaceDrawCommands.emplace_back(
			sortKey,
			RendererBackend::ScreenSpaceDrawCommand(
				p_uniformUpdater,
				objectData,
				p_shaderHandle)
			);
	}

	inline void queueForLoading(ShaderLoader::ShaderProgram &p_shader)
	{
		m_loadCommands.emplace_back(p_shader.m_shaderFilename,
									p_shader.m_programHandle,
									p_shader.getUniformUpdater(),
									p_shader.m_shaderSource, 
									p_shader.m_errorMessages);
	}
	inline void queueForLoading(ModelLoader::ModelHandle &p_model)
	{
		m_loadCommands.emplace_back(p_model.getFilename(),
									p_model.m_model->m_handle,
									p_model.m_model->m_buffers,
									p_model.m_model->m_numElements,
									p_model.m_model->m_bufferSize,
									p_model.m_model->getData());
	}
	inline void queueForLoading(TextureLoader2D::Texture2DHandle &p_texture)
	{
		m_loadCommands.emplace_back(p_texture.getFilename(),
									p_texture.getHandleRef(),
									p_texture.getTextureFormat(),
									p_texture.getMipmapLevel(),
									p_texture.getTextureWidth(),
									p_texture.getTextureHeight(),
									p_texture.getData());
	}
	inline void queueForLoading(TextureLoaderCubemap::TextureCubemapHandle p_texture)
	{
		m_loadCommands.emplace_back(
			p_texture.getHandleRef(),
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
	inline void queueForLoading(LightUniformBuffer &p_lightBuffer)
	{
		m_loadCommands.emplace_back(p_lightBuffer.m_handle,
			p_lightBuffer.m_bufferType,
			p_lightBuffer.m_bufferUsage,
			p_lightBuffer.m_bindingIndex,
			p_lightBuffer.m_size,
			p_lightBuffer.m_data);
	}

	inline void queueForUpdate(LightUniformBuffer &p_lightBuffer)
	{
		m_bufferUpdateCommands.emplace_back(p_lightBuffer.m_handle,
			p_lightBuffer.m_offset,
			p_lightBuffer.m_size,
			p_lightBuffer.m_data,
			BufferUpdateType::BufferUpdate_SubData,
			BufferType::BufferType_Uniform);
	}

	inline void passLoadCommandsToBackend()
	{
		// Pass the queued load commands to the backend to be loaded to GPU
		m_backend.processLoading(m_loadCommands, m_frameData);

		// Clear load commands, since they have already been passed to backend
		m_loadCommands.clear();
	}
	inline void passDrawCommandsToBackend()
	{
		// Pass the queued draw commands to the backend to be sent to GPU
		m_backend.processDrawing(m_drawCommands, m_frameData);

		// Clear draw commands
		m_drawCommands.clear();
	}
	inline void passScreenSpaceDrawCommandsToBackend()
	{
		// Pass the queued screen-space draw commands to the backend to be sent to GPU
		m_backend.processDrawing(m_screenSpaceDrawCommands, m_frameData);

		// Clear draw commands
		m_screenSpaceDrawCommands.clear();
	}
	inline void passUpdateCommandsToBackend()
	{
		// Pass the buffer update commands to the backend, so the buffers are updated on the GPU
		m_backend.processUpdate(m_bufferUpdateCommands, m_frameData);

		// Clear update commands, since they have already been passed to backend
		m_bufferUpdateCommands.clear();
	}
	//inline void pass

	//inline void 

	// Recalculates the projection matrix
	inline void updateProjectionMatrix()
	{
		m_frameData.m_projMatrix.perspective(Config::graphicsVar().fov, 
											 m_frameData.m_screenSize.x, 
											 m_frameData.m_screenSize.y, 
											 Config::graphicsVar().z_near, 
											 Config::graphicsVar().z_far);
	}

	// Renderer backend, serves as an interface layer to GPU
	RendererBackend m_backend;
	
	UniformFrameData m_frameData;
	
	// Renderer commands
	RendererBackend::DrawCommands m_drawCommands;
	RendererBackend::LoadCommands m_loadCommands;
	RendererBackend::BufferUpdateCommands m_bufferUpdateCommands;
	RendererBackend::ScreenSpaceDrawCommands m_screenSpaceDrawCommands;

	std::vector<RenderPass*> m_renderingPasses;

	Math::Mat4f m_viewProjMatrix;
};