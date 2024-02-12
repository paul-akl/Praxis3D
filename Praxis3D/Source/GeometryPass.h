#pragma once

#include "RenderPassBase.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_Geometry),
		m_shaderGeometry(nullptr),
		m_shaderGeometryStochastic(nullptr),
		m_shaderGeometryParallaxMap(nullptr),
		m_shaderGeometryStochasticParallaxMap(nullptr),
		m_noiseTexture(Loaders::texture2D().load(Config::filepathVar().engine_assets_path + Config::rendererVar().texture_repetition_noise_texture, MaterialType::MaterialType_Noise)),
		m_stochasticSamplingSeamFix(true) { }

	~GeometryPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Geometry Rendering Pass";
		m_stochasticSamplingSeamFix = m_renderer.getFrameData().m_miscSceneData.m_stochasticSamplingSeamFix;

		// Geometry pass shader
		{
			// Create a property-set used to load lighting shader
			PropertySet geomShaderProperties(Properties::Shaders);
			geomShaderProperties.addProperty(Properties::Name, std::string("geometryPass"));
			geomShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().geometry_pass_vert_shader);
			geomShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().geometry_pass_frag_shader);

			// Create the shader
			m_shaderGeometry = Loaders::shader().load(geomShaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = loadGeometryShaderToMemory(m_shaderGeometry))
				returnError = shaderError;
		}

		// Geometry pass with texture tiling shader
		{
			// Create a property-set used to load lighting shader
			PropertySet geomShaderProperties(Properties::Shaders);
			geomShaderProperties.addProperty(Properties::Name, std::string("geometryPass_Tiling"));
			geomShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().geometry_pass_vert_shader);
			geomShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().geometry_pass_frag_shader);

			// Create the shader
			m_shaderGeometryStochastic = Loaders::shader().load(geomShaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = loadGeometryStochasticShaderToMemory(m_shaderGeometryStochastic))
				returnError = shaderError;
		}

		// Geometry pass with texture repetition and parallax mapping shader
		{
			// Create a property-set used to load lighting shader
			PropertySet geomShaderProperties(Properties::Shaders);
			geomShaderProperties.addProperty(Properties::Name, std::string("geometryPass_TilingParallaxMap"));
			geomShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().geometry_pass_vert_shader);
			geomShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().geometry_pass_frag_shader);

			// Create the shader
			m_shaderGeometryStochasticParallaxMap = Loaders::shader().load(geomShaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = loadGeometryStochasticParallaxShaderToMemory(m_shaderGeometryStochasticParallaxMap))
				returnError = shaderError;
		}

		// Geometry pass with parallax mapping shader
		{
			// Create a property-set used to load lighting shader
			PropertySet geomShaderProperties(Properties::Shaders);
			geomShaderProperties.addProperty(Properties::Name, std::string("geometryPass_ParallaxMap"));
			geomShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().geometry_pass_vert_shader);
			geomShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().geometry_pass_frag_shader);

			// Create the shader
			m_shaderGeometryParallaxMap = Loaders::shader().load(geomShaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = loadGeometryParallaxShaderToMemory(m_shaderGeometryParallaxMap))
				returnError = shaderError;
		}

		if(returnError == ErrorCode::Success)
		{
			// Load noise texture to memory and to video memory
			m_noiseTexture.loadToMemory();
			m_renderer.queueForLoading(m_noiseTexture);

			// Log a successful initialization
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_GeometryPass);

			setInitialized(true);
		}
		else
		{
			// Log a failed initialization
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_GeometryPass);
		}

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		// Check if the texture repetition seam fix flag was changed, and reload the affected shaders if it did
		if(m_stochasticSamplingSeamFix != m_renderer.getFrameData().m_miscSceneData.m_stochasticSamplingSeamFix)
		{
			// Update the texture repetition seam fix flag
			m_stochasticSamplingSeamFix = m_renderer.getFrameData().m_miscSceneData.m_stochasticSamplingSeamFix;

			// Reset affected shaders loaded flags
			m_shaderGeometryStochastic->resetLoadedToVideoMemoryFlag();
			m_shaderGeometryStochasticParallaxMap->resetLoadedToVideoMemoryFlag();

			// Reload the affected shaders
			loadGeometryStochasticShaderToMemory(m_shaderGeometryStochastic);
			loadGeometryStochasticParallaxShaderToMemory(m_shaderGeometryStochasticParallaxMap);

			// Process shader load commands
			m_renderer.passLoadCommandsToBackend();
		}

		// Prepare the geometry buffer for a new frame and a geometry pass
		m_renderer.m_backend.getGeometryBuffer()->initFrame();

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);		// Enable depth testing, as this is much like a regular forward render pass
		glClear(GL_DEPTH_BUFFER_BIT);	// Make sure to clear the depth buffer for the new frame

		// Set depth test function
		glDepthFunc(Config::rendererVar().depth_test_func);
		//glDisable(GL_CULL_FACE);

		// Enable / disable face culling
		if(Config::rendererVar().face_culling)
		{
			glEnable(GL_CULL_FACE);

			// Set face culling mode
			glCullFace(Config::rendererVar().face_culling_mode);
		}
		else
			glDisable(GL_CULL_FACE);


		//glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);

		// Set input and output color maps for this frame
		p_renderPassData.setColorInputMap(GBufferTextureType::GBufferIntermediate);
		p_renderPassData.setColorOutputMap(GBufferTextureType::GBufferFinal);

		// Bind the geometry framebuffer to be used
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::GBufferFramebufferType::FramebufferGeometry);

		// Bind all the geometry buffer textures to be drawn to
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferPosition);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferDiffuse);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferNormal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferMatProperties);

		// Bind lens dirt texture
		glActiveTexture(GL_TEXTURE0 + MaterialType::MaterialType_Noise);
		glBindTexture(GL_TEXTURE_2D, m_noiseTexture.getHandle());

		// Initialize the geometry framebuffer for the geometry pass
		m_renderer.m_backend.getGeometryBuffer()->initGeometryPass();

		if(p_sceneObjects.m_processDrawing)
		{
			// Get most common (regular geometry pass) shader details
			auto geomShaderHandle = m_shaderGeometry->getShaderHandle();
			auto &geomUniformUpdater = m_shaderGeometry->getUniformUpdater();

			// Iterate over all objects to be rendered with geometry shader
			for(auto entity : p_sceneObjects.m_models)
			{
				ModelComponent &model = p_sceneObjects.m_models.get<ModelComponent>(entity);
				if(model.isObjectActive())
				{
					// Get the model data from the ModelComponent, that holds all the drawing data
					auto &modelData = model.getModelData();

					// Go over each model
					for(decltype(modelData.size()) modelIndex = 0, modelSize = modelData.size(); modelIndex < modelSize; modelIndex++)
					{
						// Go over each mesh
						for(decltype(modelData[modelIndex].m_model.getNumMeshes()) meshIndex = 0, meshSize = modelData[modelIndex].m_model.getNumMeshes(); meshIndex < meshSize; meshIndex++)
						{
							// Calculate model-view-projection matrix
							const glm::mat4 &modelMatrix = p_sceneObjects.m_models.get<SpatialComponent>(entity).getSpatialDataChangeManager().getWorldTransformWithScale();
							const glm::mat4 modelViewProjMatrix = m_renderer.m_viewProjMatrix * modelMatrix;

							// Only draw active meshes
							if(modelData[modelIndex].m_meshes[meshIndex].m_active)
							{
								// Choose a shader based on whether the texture repetition and parallax mapping are turned on for the given mesh
								if(modelData[modelIndex].m_meshes[meshIndex].m_stochasticSampling)
								{
									if(modelData[modelIndex].m_meshes[meshIndex].m_heightScale > 0.0f)
									{
										// TEXTURE REPETITION and PARALLAX MAPPING enabled
										m_renderer.queueForDrawing(modelData[modelIndex].m_model[meshIndex], modelData[modelIndex].m_meshes[meshIndex], modelData[modelIndex].m_model.getHandle(), m_shaderGeometryStochasticParallaxMap->getShaderHandle(), m_shaderGeometryStochasticParallaxMap->getUniformUpdater(), modelMatrix, modelViewProjMatrix);
									}
									else
									{
										// TEXTURE REPETITION enabled
										m_renderer.queueForDrawing(modelData[modelIndex].m_model[meshIndex], modelData[modelIndex].m_meshes[meshIndex], modelData[modelIndex].m_model.getHandle(), m_shaderGeometryStochastic->getShaderHandle(), m_shaderGeometryStochastic->getUniformUpdater(), modelMatrix, modelViewProjMatrix);
									}
								}
								else
								{
									if(modelData[modelIndex].m_meshes[meshIndex].m_heightScale > 0.0f)
									{
										// PARALLAX MAPPING enabled
										m_renderer.queueForDrawing(modelData[modelIndex].m_model[meshIndex], modelData[modelIndex].m_meshes[meshIndex], modelData[modelIndex].m_model.getHandle(), m_shaderGeometryParallaxMap->getShaderHandle(), m_shaderGeometryParallaxMap->getUniformUpdater(), modelMatrix, modelViewProjMatrix);
									}
									else
									{
										// Regular geometry pass
										m_renderer.queueForDrawing(modelData[modelIndex].m_model[meshIndex], modelData[modelIndex].m_meshes[meshIndex], modelData[modelIndex].m_model.getHandle(), geomShaderHandle, geomUniformUpdater, modelMatrix, modelViewProjMatrix);
									}
								}
							}
						}
					}
				}
			}

			// Iterate over all objects to be rendered with a custom shader
			for(auto entity : p_sceneObjects.m_modelsWithShaders)
			{
				ModelComponent &model = p_sceneObjects.m_modelsWithShaders.get<ModelComponent>(entity);
				if(model.isObjectActive())
				{
					SpatialComponent &spatialData = p_sceneObjects.m_modelsWithShaders.get<SpatialComponent>(entity);
					ShaderComponent &shader = p_sceneObjects.m_modelsWithShaders.get<ShaderComponent>(entity);
					if(shader.isLoadedToVideoMemory())
					{
						auto &modelData = model.getModelData();

						for(decltype(modelData.size()) i = 0, size = modelData.size(); i < size; i++)
						{
							m_renderer.queueForDrawing(modelData[i],
								shader.getShaderData()->m_shader.getShaderHandle(),
								shader.getShaderData()->m_shader.getUniformUpdater(),
								spatialData.getSpatialDataChangeManager().getWorldTransformWithScale(),
								m_renderer.m_viewProjMatrix);
						}
					}
				}
			}

			// Pass all the draw commands to be executed
			m_renderer.passDrawCommandsToBackend();
		}
	}

private:
	ErrorCode loadGeometryShaderToMemory(ShaderLoader::ShaderProgram *p_shader)
	{
		ErrorCode shaderError = p_shader->loadToMemory();

		if(shaderError == ErrorCode::Success)
		{
			// Set whether the normal texture compression is enabled in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_normalMapCompression, Config::textureVar().texture_normal_compression ? 1 : 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_normalMapCompression, ErrorSource::Source_GeometryPass);

			// Set the number of material types in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_numOfMaterialTypes, MaterialType::MaterialType_NumOfTypes); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfMaterialTypes, ErrorSource::Source_GeometryPass);

			// Disable stochastic sampling in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_stochasticSampling, 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_stochasticSampling, ErrorSource::Source_GeometryPass);

			// Disable parallax mapping in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_parallaxMapping, 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_parallaxMapping, ErrorSource::Source_GeometryPass);

			// Queue the shader to be loaded to GPU
			m_renderer.queueForLoading(*p_shader);
		}

		return shaderError;
	}
	ErrorCode loadGeometryStochasticShaderToMemory(ShaderLoader::ShaderProgram *p_shader)
	{
		ErrorCode shaderError = p_shader->loadToMemory();

		if(shaderError == ErrorCode::Success)
		{
			// Set whether the normal texture compression is enabled in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_normalMapCompression, Config::textureVar().texture_normal_compression ? 1 : 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_normalMapCompression, ErrorSource::Source_GeometryPass);

			// Set the number of material types in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_numOfMaterialTypes, MaterialType::MaterialType_NumOfTypes); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfMaterialTypes, ErrorSource::Source_GeometryPass);

			// Enable stochastic sampling in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_stochasticSampling, 1); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_stochasticSampling, ErrorSource::Source_GeometryPass);

			// Disable parallax mapping in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_parallaxMapping, 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_parallaxMapping, ErrorSource::Source_GeometryPass);

			// Set stochastic sampling mipmap seam fix flag in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_stochasticSamplingSeamFix, m_renderer.getFrameData().m_miscSceneData.m_stochasticSamplingSeamFix ? 1 : 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_stochasticSamplingSeamFix, ErrorSource::Source_GeometryPass);

			// Queue the shader to be loaded to GPU
			m_renderer.queueForLoading(*p_shader);
		}

		return shaderError;
	}
	ErrorCode loadGeometryParallaxShaderToMemory(ShaderLoader::ShaderProgram *p_shader)
	{
		ErrorCode shaderError = p_shader->loadToMemory();

		if(shaderError == ErrorCode::Success)
		{
			// Set whether the normal texture compression is enabled in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_normalMapCompression, Config::textureVar().texture_normal_compression ? 1 : 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_normalMapCompression, ErrorSource::Source_GeometryPass);

			// Set the number of material types in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_numOfMaterialTypes, MaterialType::MaterialType_NumOfTypes); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfMaterialTypes, ErrorSource::Source_GeometryPass);

			// Disable stochastic sampling in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_stochasticSampling, 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_stochasticSampling, ErrorSource::Source_GeometryPass);

			// Enable parallax mapping in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_parallaxMapping, 1); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_parallaxMapping, ErrorSource::Source_GeometryPass);

			// Set the parallax mapping method in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_parallaxMappingMethod, Config::rendererVar().parallax_mapping_method); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_parallaxMappingMethod, ErrorSource::Source_GeometryPass);

			// Queue the shader to be loaded to GPU
			m_renderer.queueForLoading(*p_shader);
		}

		return shaderError;
	}
	ErrorCode loadGeometryStochasticParallaxShaderToMemory(ShaderLoader::ShaderProgram *p_shader)
	{
		ErrorCode shaderError = p_shader->loadToMemory();

		if(shaderError == ErrorCode::Success)
		{
			// Set whether the normal texture compression is enabled in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_normalMapCompression, Config::textureVar().texture_normal_compression ? 1 : 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_normalMapCompression, ErrorSource::Source_GeometryPass);

			// Set the number of material types in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_numOfMaterialTypes, MaterialType::MaterialType_NumOfTypes); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfMaterialTypes, ErrorSource::Source_GeometryPass);

			// Enable stochastic sampling in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_stochasticSampling, 1); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_stochasticSampling, ErrorSource::Source_GeometryPass);

			// Enable parallax mapping in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(Config::shaderVar().define_parallaxMapping, 1); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_parallaxMapping, ErrorSource::Source_GeometryPass);

			// Set stochastic sampling mipmap seam fix flag in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_stochasticSamplingSeamFix, m_renderer.getFrameData().m_miscSceneData.m_stochasticSamplingSeamFix ? 1 : 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_stochasticSamplingSeamFix, ErrorSource::Source_GeometryPass);

			// Set the parallax mapping method in the shader
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_parallaxMappingMethod, Config::rendererVar().parallax_mapping_method); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_parallaxMappingMethod, ErrorSource::Source_GeometryPass);

			// Queue the shader to be loaded to GPU
			m_renderer.queueForLoading(*p_shader);
		}

		return shaderError;
	}

	ShaderLoader::ShaderProgram *m_shaderGeometry;
	ShaderLoader::ShaderProgram *m_shaderGeometryStochastic;
	ShaderLoader::ShaderProgram *m_shaderGeometryParallaxMap;
	ShaderLoader::ShaderProgram *m_shaderGeometryStochasticParallaxMap;

	TextureLoader2D::Texture2DHandle m_noiseTexture;

	bool m_stochasticSamplingSeamFix;
};