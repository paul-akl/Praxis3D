#pragma once

#include "Config.h"
#include "GUIHandler.h"
#include "RendererBackend.h"
#include "RendererScene.h"

class RenderPass;
struct RenderPassData;

class RendererFrontend
{
	friend class AtmScatteringPass;
	friend class BloomCompositePass;
	friend class BloomPass;
	friend class BlurPass;
	friend class GeometryPass;
	friend class GUIPass;
	friend class HdrMappingPass;
	friend class LenseFlareCompositePass;
	friend class LenseFlarePass;
	friend class LightingPass;
	friend class LuminancePass;
	friend class PostProcessPass;
	friend class FinalPass;
	friend class ReflectionPass;
	friend class SkyPass;
public:
	// A handle for a uniform or shader storage buffer
	struct ShaderBuffer
	{
		ShaderBuffer(const BufferType p_bufferType, const BufferBindTarget p_bufferBindTarget, const BufferUsageHint p_usageHint) : m_bufferType(p_bufferType), m_bufferBindTarget(p_bufferBindTarget), m_bufferUsage(p_usageHint)
		{
			m_data = 0;
			m_size = 0;
			m_updateSize = 0;
			m_offset = 0;
			m_handle = 0;
			m_bindingIndex = 0;
		}

		int64_t m_size;
		int64_t m_updateSize;
		unsigned int m_offset;
		unsigned int m_handle;
		unsigned int m_bindingIndex;
		void *m_data;

		const BufferType m_bufferType;
		const BufferBindTarget m_bufferBindTarget;
		const BufferUsageHint m_bufferUsage;
	};

	RendererFrontend();
	~RendererFrontend();

	ErrorCode init();

	const bool getRenderFinalToTexture() const;

	void setGUIPassFunctorSequence(FunctorSequence *p_GUIPassFunctorSequence);
	void setRenderFinalToTexture(const bool p_renderToTexture);
	void setRenderToTextureResolution(const glm::ivec2 p_renderToTextureResolution);
	void setRenderingPasses(const RenderingPasses &p_renderingPasses);

	const RenderingPasses getRenderingPasses();

	// Renders a complete frame
	void renderFrame(SceneObjects &p_sceneObjects, const float p_deltaTime);

	unsigned int getFramebufferTextureHandle(GBufferTextureType p_bufferType) const;

	const inline UniformFrameData &getFrameData() const { return m_frameData; }
	
protected:
	inline void queueForDrawing(const ModelData &p_modelData, const unsigned int p_shaderHandle, const ShaderUniformUpdater &p_uniformUpdater, const glm::mat4 &p_modelMatrix, const glm::mat4 &p_viewProjMatrix)
	{
		// Get the necessary handles
		const unsigned int modelHandle = p_modelData.m_model.getHandle();

		// Calculate model-view-projection matrix
		const glm::mat4 modelViewProjMatrix = p_viewProjMatrix * p_modelMatrix;

		// Calculate the sort key
		RendererBackend::DrawCommands::value_type::first_type sortKey = 0;
		
		// Add a draw command for each mesh, using the same object data
		for(decltype(p_modelData.m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_modelData.m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
		{
			if(p_modelData.m_meshes[meshIndex].m_active)
			{
				// TODO: per-texture material parameters
				// Assign the object data that is later passed to the shaders
				const UniformObjectData objectData(p_modelMatrix,
					modelViewProjMatrix,
					p_modelData.m_meshes[meshIndex].m_heightScale,
					p_modelData.m_meshes[meshIndex].m_alphaThreshold,
					p_modelData.m_meshes[meshIndex].m_emissiveIntensity,
					p_modelData.m_meshes[meshIndex].m_materials[MaterialType::MaterialType_Diffuse].m_textureScale.x);

				m_drawCommands.emplace_back(
					sortKey,
					RendererBackend::DrawCommand(
						p_uniformUpdater,
						objectData,
						p_shaderHandle,
						modelHandle,
						p_modelData.m_model[meshIndex].m_numIndices,
						p_modelData.m_model[meshIndex].m_baseVertex,
						p_modelData.m_model[meshIndex].m_baseIndex,
						p_modelData.m_meshes[meshIndex].m_materials[MaterialType::MaterialType_Diffuse].m_texture.getHandle(),
						p_modelData.m_meshes[meshIndex].m_materials[MaterialType::MaterialType_Normal].m_texture.getHandle(),
						p_modelData.m_meshes[meshIndex].m_materials[MaterialType::MaterialType_Emissive].m_texture.getHandle(),
						p_modelData.m_meshes[meshIndex].m_materials[MaterialType::MaterialType_Combined].m_texture.getHandle(),
						p_modelData.m_meshes[meshIndex].m_textureWrapMode)
				);
			}
		}
	}
	inline void queueForDrawing(const unsigned int p_shaderHandle, const ShaderUniformUpdater &p_uniformUpdater, const glm::mat4 &p_viewProjMatrix)
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
	inline void queueForDrawing(const unsigned int p_shaderHandle, const ShaderUniformUpdater &p_uniformUpdater, const glm::mat4 &p_viewProjMatrix, const unsigned int p_numOfGroupsX, const unsigned int p_numOfGroupsY, const unsigned int p_numOfGroupsZ, const MemoryBarrierType p_memoryBarrier)
	{
		// Assign the object data that is later passed to the shaders
		const UniformObjectData objectData(p_viewProjMatrix,
										   p_viewProjMatrix,
										   Config::graphicsVar().height_scale,
										   Config::graphicsVar().alpha_threshold,
										   Config::graphicsVar().emissive_threshold,
										   Config::graphicsVar().texture_tiling_factor);

		m_computeDispatchCommands.emplace_back(p_numOfGroupsX,
											   p_numOfGroupsY,
											   p_numOfGroupsZ, 
											   p_memoryBarrier,
											   p_uniformUpdater,
											   objectData,
											   p_shaderHandle);
	}

	inline void queueForLoading(ShaderLoader::ShaderProgram &p_shader)
	{
		if(!p_shader.m_loadedToVideoMemory)
		{
			m_loadCommands.emplace_back(p_shader.m_shaderFilename,
										p_shader.m_programHandle,
										p_shader.getUniformUpdater(),
										p_shader.m_shaderSource,
										p_shader.m_errorMessages);

			p_shader.m_loadedToVideoMemory = true;
		}
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
									p_texture.getTextureDataFormat(),
									p_texture.getTextureDataType(),
									p_texture.getEnableMipmap(),
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
			{
				// Check if the material hasn't been loaded already
				if(!p_objectData.m_materials[matType][i].isLoadedToVideoMemory())
				{
					// Send the material to be loaded
					queueForLoading(p_objectData.m_materials[matType][i]);

					// Set the material as already loaded, since it has been queued for loading already
					p_objectData.m_materials[matType][i].setLoadedToVideoMemory(true);
				}
			}
	}
	inline void queueForLoading(ShaderBuffer &p_shaderBuffer)
	{
		m_loadCommands.emplace_back(p_shaderBuffer.m_handle,
			p_shaderBuffer.m_bufferType,
			p_shaderBuffer.m_bufferBindTarget,
			p_shaderBuffer.m_bufferUsage,
			p_shaderBuffer.m_bindingIndex,
			p_shaderBuffer.m_size,
			p_shaderBuffer.m_data);
	}

	inline void queueForUnloading(ShaderLoader::ShaderProgram &p_shader)
	{
		m_unloadCommands.emplace_back(std::make_pair(UnloadObjectType::UnloadObjectType_Shader, p_shader.getShaderHandle()));
	}
	inline void queueForUnloading(ModelLoader::ModelHandle &p_model)
	{
		m_unloadCommands.emplace_back(std::make_pair(UnloadObjectType::UnloadObjectType_VAO, p_model.getHandle()));

		for(unsigned int i = 0; i < ModelBuffer_NumAllTypes; i++)
			m_unloadCommands.emplace_back(std::make_pair(UnloadObjectType::UnloadObjectType_Buffer, p_model.m_model->m_buffers[i]));

	}
	inline void queueForUnloading(TextureLoader2D::Texture2DHandle &p_texture)
	{
		m_unloadCommands.emplace_back(std::make_pair(UnloadObjectType::UnloadObjectType_Texture, p_texture.getHandle()));
	}
	inline void queueForUnloading(TextureLoaderCubemap::TextureCubemapHandle p_texture)
	{
		m_unloadCommands.emplace_back(std::make_pair(UnloadObjectType::UnloadObjectType_Texture, p_texture.getHandle()));
	}

	inline void queueForUpdate(ShaderBuffer &p_shaderBuffer)
	{
		m_bufferUpdateCommands.emplace_back(p_shaderBuffer.m_handle,
			p_shaderBuffer.m_offset,
			p_shaderBuffer.m_updateSize,
			p_shaderBuffer.m_data,
			p_shaderBuffer.m_updateSize == p_shaderBuffer.m_size ?	// If update size is the same as buffer size
			BufferUpdateType::BufferUpdate_Data :					// Update the whole buffer
			BufferUpdateType::BufferUpdate_SubData,					// Otherwise update only part of the data
			p_shaderBuffer.m_bufferType);
	}

	inline void passLoadCommandsToBackend()
	{
		// Pass the queued load commands to the backend to be loaded to GPU
		m_backend.processLoading(m_loadCommands, m_frameData);

		// Clear load commands, since they have already been passed to backend
		m_loadCommands.clear();
	}
	inline void passUnloadCommandsToBackend()
	{
		// Pass the queued unload commands to the backend to be released from GPU memory
		m_backend.processUnloading(m_unloadCommands);

		// Clear unload commands, since they have already been passed to backend
		m_unloadCommands.clear();
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
	inline void passComputeDispatchCommandsToBackend()
	{
		// Pass the queued compute shader dispatch commands to the backend to be sent to GPU
		m_backend.processDrawing(m_computeDispatchCommands, m_frameData);

		// Clear compute dispatch commands
		m_computeDispatchCommands.clear();
	}
	inline void passUpdateCommandsToBackend()
	{
		// Pass the buffer update commands to the backend, so the buffers are updated on the GPU
		m_backend.processUpdate(m_bufferUpdateCommands, m_frameData);

		// Clear update commands, since they have already been passed to backend
		m_bufferUpdateCommands.clear();
	}

	// Recalculates the projection matrix
	inline void updateProjectionMatrix()
	{
		//m_frameData.m_projMatrix = glm::perspective(
		//	Config::graphicsVar().fov, 
		//	(float) m_frameData.m_screenSize.x / (float) m_frameData.m_screenSize.y, 
		//	Config::graphicsVar().z_near,
		//	Config::graphicsVar().z_far);

		m_frameData.m_projMatrix = glm::perspectiveFov(
			glm::radians(Config::graphicsVar().fov),
			(float)m_frameData.m_screenSize.x,
			(float)m_frameData.m_screenSize.y,
			Config::graphicsVar().z_near,
			Config::graphicsVar().z_far);

		m_frameData.m_atmScatProjMatrix = Math::perspectiveRadian(Config::graphicsVar().fov,
																m_frameData.m_screenSize.x,
																m_frameData.m_screenSize.y);

		//m_frameData.m_atmScatProjMatrix.perspectiveRadian(	Config::graphicsVar().fov,
		//													m_frameData.m_screenSize.x,
		//													m_frameData.m_screenSize.y);
	}

	bool m_renderingPassesSet;
	bool m_guiRenderWasEnabled;
	float m_zFar;
	float m_zNear;

	// Renderer backend, serves as an interface layer to GPU
	RendererBackend m_backend;
	
	// Holds information about the current frame, that is used in the shaders
	UniformFrameData m_frameData;
	
	// Renderer commands
	RendererBackend::DrawCommands m_drawCommands;
	RendererBackend::LoadCommands m_loadCommands;
	RendererBackend::UnloadCommands m_unloadCommands;
	RendererBackend::BufferUpdateCommands m_bufferUpdateCommands;
	RendererBackend::ScreenSpaceDrawCommands m_screenSpaceDrawCommands;
	RendererBackend::ComputeDispatchCommands m_computeDispatchCommands;

	// Holds info used between rendering passes
	RenderPassData *m_renderPassData;

	// View - projection matrix
	glm::mat4 m_viewProjMatrix;
	
	// An array of all active rendering passes
	std::vector<RenderPass*> m_activeRenderPasses;
	RenderPass* m_allRenderPasses[RenderPassType::RenderPassType_NumOfTypes];
};