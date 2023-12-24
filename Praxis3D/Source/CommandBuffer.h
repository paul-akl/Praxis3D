#pragma once

#include "GraphicsDataSets.h"
#include "RendererFrontend.h"

class CommandBuffer
{
public:
	struct Command
	{
		Command(RendererBackend::ScreenSpaceDrawCommand &&p_screenSpaceCommand) : m_commandUnion(p_screenSpaceCommand), m_commandType(CommandType_ScreenSpaceDraw) { }
		Command(RendererBackend::DrawCommand &&p_drawCommand) : m_commandUnion(p_drawCommand), m_commandType(CommandType_Draw) { }
		Command(RendererBackend::BindCommand &&p_bindCommand) : m_commandUnion(p_bindCommand), m_commandType(CommandType_Bind) { }
		Command(RendererBackend::LoadCommand &&p_loadCommand) : m_commandUnion(p_loadCommand), m_commandType(CommandType_Load) { }

		union CommandUnion
		{
			CommandUnion(RendererBackend::ScreenSpaceDrawCommand &p_screenSpaceCommand) : m_screenSpaceCommand(p_screenSpaceCommand) { }
			CommandUnion(RendererBackend::DrawCommand &p_drawCommand) : m_drawCommand(p_drawCommand) { }
			CommandUnion(RendererBackend::BindCommand &p_bindCommand) : m_bindCommand(p_bindCommand) { }
			CommandUnion(RendererBackend::LoadCommand &p_loadCommand) : m_loadCommand(p_loadCommand) { }

			RendererBackend::ScreenSpaceDrawCommand m_screenSpaceCommand;
			RendererBackend::DrawCommand m_drawCommand;
			RendererBackend::BindCommand m_bindCommand;
			RendererBackend::LoadCommand m_loadCommand;
		};

		CommandUnion m_commandUnion;
		const CommandType m_commandType;
	};

	typedef std::vector<std::pair<int64_t, Command>> Commands;

	CommandBuffer() { }
	~CommandBuffer() { }

	inline void queueForDrawing(const RenderableObjectData &p_object, const unsigned int p_shaderHandle, const ShaderUniformUpdater &p_uniformUpdater, const glm::mat4 &p_viewProjMatrix)
	{
		// Get the neccessary handles
		const unsigned int modelHandle = p_object.m_model.getHandle();

		// Calculare model-view-projection matrix
		const glm::mat4 modelViewProjMatrix = p_viewProjMatrix * p_object.m_baseObjectData.m_modelMat;

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
			m_commands.emplace_back(std::make_pair(
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
					p_object.m_materials[MaterialType_Combined][meshIndex].getHandle(),
					p_object.m_baseObjectData.m_textureWrapMode))
			);
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

		m_commands.emplace_back(std::make_pair(
			sortKey,
			RendererBackend::ScreenSpaceDrawCommand(
				p_uniformUpdater,
				objectData,
				p_shaderHandle))
		);
	}

	inline void queueForLoading(ShaderLoader::ShaderProgram &p_shader)
	{
		// Calculate the sort key
		RendererBackend::DrawCommands::value_type::first_type sortKey = 0;

		m_commands.emplace_back(std::make_pair(
			sortKey,
			RendererBackend::LoadCommand(p_shader.m_shaderFilename,
										 p_shader.m_programHandle,
										 p_shader.getUniformUpdater(),
										 p_shader.m_shaderSource,
										 p_shader.m_errorMessages)));
	}
	inline void queueForLoading(ModelLoader::ModelHandle &p_model)
	{
		// Calculate the sort key
		RendererBackend::DrawCommands::value_type::first_type sortKey = 0;

		m_commands.emplace_back(std::make_pair(
			sortKey,
			RendererBackend::LoadCommand(p_model.getFilename(),
										 p_model.m_model->m_handle,
										 p_model.m_model->m_buffers,
										 p_model.m_model->m_numElements,
										 p_model.m_model->m_bufferSize,
										 p_model.m_model->getData())));
	}
	inline void queueForLoading(TextureLoader2D::Texture2DHandle &p_texture)
	{
		// Calculate the sort key
		RendererBackend::DrawCommands::value_type::first_type sortKey = 0;

		m_commands.emplace_back(std::make_pair(
			sortKey,
			RendererBackend::LoadCommand(p_texture.getFilename(),
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
										 p_texture.getData())));
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

	inline void queueForUpdate(RendererFrontend::ShaderBuffer &p_shaderBuffer)
	{
		/*/ Calculate the sort key
		RendererBackend::DrawCommands::value_type::first_type sortKey = 0;

		m_commands.emplace_back(std::make_pair(
			sortKey,
			RendererBackend::LoadCommand(p_shaderBuffer.m_handle,
										 p_shaderBuffer.m_bufferType,
										 p_shaderBuffer.m_bufferUsage,
										 p_shaderBuffer.m_bindingIndex,
										 p_shaderBuffer.m_size,
										 (void*)0)));*/
	}

	Commands &getCommands()	{ return m_commands; }

	void clearommands()	{ m_commands.clear(); }

private:
	Commands m_commands;
};