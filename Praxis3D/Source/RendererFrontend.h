#pragma once

#include "Config.h"
#include "GUIHandler.h"
#include "RendererBackend.h"
#include "RendererScene.h"

class RenderPass;
struct RenderPassData;

class RendererFrontend
{
	friend class AmbientOcclusionPass;
	friend class AtmScatteringPass;
	friend class BloomCompositePass;
	friend class BloomPass;
	friend class BlurPass;
	friend class FinalPass;
	friend class GeometryPass;
	friend class GUIPass;
	friend class HdrMappingPass;
	friend class LenseFlareCompositePass;
	friend class LenseFlarePass;
	friend class LightingPass;
	friend class LuminancePass;
	friend class PostProcessPass;
	friend class ReflectionPass;
	friend class ShadowMappingPass;
	friend class SkyPass;
	friend class TonemappingPass;
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
	void setAtmScatteringData(const AtmosphericScatteringData &p_atmScatteringData) { m_frameData.m_atmScatteringData = p_atmScatteringData; m_frameData.m_atmScatteringDataChanged = true; }
	void setAmbientOcclusionData(const AmbientOcclusionData &p_aoData) { m_frameData.m_aoData = p_aoData; }
	void setMiscSceneData(const MiscSceneData &p_miscSceneData) { m_frameData.m_miscSceneData = p_miscSceneData; }
	void setShadowMappingData(const ShadowMappingData &p_shadowMappingData) { m_frameData.m_shadowMappingData = p_shadowMappingData; }

	const RenderingPasses getRenderingPasses();
	const ShadowMappingData &getShadowMappingData() const { return m_frameData.m_shadowMappingData; }

	// Renders a complete frame
	void renderFrame(SceneObjects &p_sceneObjects, const float p_deltaTime);

	unsigned int getFramebufferTextureHandle(GBufferTextureType p_bufferType) const;

	const inline UniformFrameData &getFrameData() const { return m_frameData; }
	
protected:
	inline void queueForDrawing(const Model::Mesh &p_mesh, const MeshData &p_meshData, const uint32_t p_modelHandle, const uint32_t p_shaderHandle, ShaderUniformUpdater &p_uniformUpdater, const DrawCommandTextureBinding p_textureBindingType, const FaceCullingSettings p_faceCulling, const glm::mat4 &p_modelMatrix, const glm::mat4 &p_modelViewProjMatrix)
	{
		// Calculate the sort key, by combining lower 16 bits of shader handle and model handle
		// sortkey = 64 bits total
		// 64-48 bits = shader handle
		// 48-32 bits = model handle
		RendererBackend::DrawCommands::value_type::first_type sortKey = ((p_shaderHandle & 0x0000ffff) << 16) | (p_modelHandle & 0x0000ffff);
		sortKey = sortKey << 32;

		// TODO: per-texture material parameters
		// Assign the object data that is later passed to the shaders
		const UniformObjectData objectData(p_modelMatrix,
			p_modelViewProjMatrix,
			p_meshData.m_heightScale,
			p_meshData.m_alphaThreshold,
			p_meshData.m_emissiveIntensity,
			p_meshData.m_materialData.m_parameters[MaterialType::MaterialType_Diffuse].m_scale.x,
			p_meshData.m_textureRepetitionScale);

		m_drawCommands.emplace_back(
			sortKey,
			RendererBackend::DrawCommand(
				p_uniformUpdater,
				objectData,
				p_shaderHandle,
				p_modelHandle,
				p_mesh.m_numIndices,
				p_mesh.m_baseVertex,
				p_mesh.m_baseIndex,
				p_meshData.m_materials[MaterialType::MaterialType_Diffuse].getHandle(),
				p_meshData.m_materials[MaterialType::MaterialType_Normal].getHandle(),
				p_meshData.m_materials[MaterialType::MaterialType_Emissive].getHandle(),
				p_meshData.m_materials[MaterialType::MaterialType_Combined].getHandle(),
				p_faceCulling,
				p_meshData.m_materialData,
				p_textureBindingType,
				p_meshData.m_textureWrapMode)
		);
	}
	inline void queueForDrawing(const ModelData &p_modelData, const unsigned int p_shaderHandle, ShaderUniformUpdater &p_uniformUpdater, const DrawCommandTextureBinding p_textureBindingType, const glm::mat4 &p_modelMatrix, const glm::mat4 &p_viewProjMatrix)
	{
		// Get the necessary handles
		const unsigned int modelHandle = p_modelData.m_model.getHandle();

		// Calculate model-view-projection matrix
		const glm::mat4 modelViewProjMatrix = p_viewProjMatrix * p_modelMatrix;
				
		// Add a draw command for each mesh, using the same object data
		for(decltype(p_modelData.m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_modelData.m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
		{
			if(p_modelData.m_meshes[meshIndex].m_active)
			{
				queueForDrawing(p_modelData.m_model[meshIndex], p_modelData.m_meshes[meshIndex], modelHandle, p_shaderHandle, p_uniformUpdater, p_textureBindingType, p_modelData.m_drawFaceCulling, p_modelMatrix, modelViewProjMatrix);
			}
		}
	}
	inline void queueForDrawing(const unsigned int p_shaderHandle, ShaderUniformUpdater &p_uniformUpdater, const glm::mat4 &p_viewProjMatrix)
	{
		// Assign the object data that is later passed to the shaders
		const UniformObjectData objectData(p_viewProjMatrix,
										   p_viewProjMatrix,
										   Config::graphicsVar().height_scale,
										   Config::graphicsVar().alpha_threshold,
										   Config::graphicsVar().emissive_threshold,
										   Config::graphicsVar().texture_tiling_factor,
										   Config::graphicsVar().stochastic_sampling_scale);

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
	inline void queueForDrawing(const unsigned int p_shaderHandle, ShaderUniformUpdater &p_uniformUpdater, const glm::mat4 &p_viewProjMatrix, const unsigned int p_numOfGroupsX, const unsigned int p_numOfGroupsY, const unsigned int p_numOfGroupsZ, const MemoryBarrierType p_memoryBarrier)
	{
		// Assign the object data that is later passed to the shaders
		const UniformObjectData objectData(p_viewProjMatrix,
										   p_viewProjMatrix,
										   Config::graphicsVar().height_scale,
										   Config::graphicsVar().alpha_threshold,
										   Config::graphicsVar().emissive_threshold,
										   Config::graphicsVar().texture_tiling_factor,
										   Config::graphicsVar().stochastic_sampling_scale);

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
			p_texture.getMagnificationFilterType(),
			p_texture.getMinificationFilterType(),
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

	inline void queueForUnloading(const UnloadableObjectsContainer &p_unloadableObject)
	{
		m_unloadCommands.emplace_back(std::make_pair(p_unloadableObject.m_objectType, p_unloadableObject.m_handle));
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
		std::sort(m_drawCommands.begin(), m_drawCommands.end(), [](const auto &p_left, const auto &p_right) { return p_left.first < p_right.first; });

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
		m_frameData.m_projMatrix = glm::perspectiveFov(
			glm::radians(m_frameData.m_fov),
			(float)m_frameData.m_screenSize.x,
			(float)m_frameData.m_screenSize.y,
			m_frameData.m_zNear,
			m_frameData.m_zFar);

		m_frameData.m_atmScatProjMatrix = Math::perspectiveRadian(m_frameData.m_fov,
																m_frameData.m_screenSize.x,
																m_frameData.m_screenSize.y);
	}

	// Sets the needed flag for the given anti-aliasing method
	inline void setAntialiasingMethod(const AntiAliasingType p_antialiasingType)
	{
		switch(p_antialiasingType)
		{
			case AntiAliasingType_None:
				Config::m_rendererVar.fxaa_enabled = false;
				Config::m_rendererVar.msaa_enabled = false;
				break;
			case AntiAliasingType_MSAA:
				Config::m_rendererVar.fxaa_enabled = false;
				Config::m_rendererVar.msaa_enabled = true;
				break;
			case AntiAliasingType_FXAA:
				Config::m_rendererVar.fxaa_enabled = true;
				Config::m_rendererVar.msaa_enabled = false;
				break;
		}
	}

	bool m_renderingPassesSet;
	bool m_guiRenderWasEnabled;
	AntiAliasingType m_antialiasingType;

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