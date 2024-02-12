#pragma once

#include "RenderPassBase.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(RendererFrontend &p_renderer) : 
		RenderPass(p_renderer, RenderPassType::RenderPassType_Lighting),
		m_pointLightBuffer(BufferType_Uniform, BufferBindTarget_Uniform, BufferUsageHint_DynamicDraw),
		m_spotLightBuffer(BufferType_Uniform, BufferBindTarget_Uniform, BufferUsageHint_DynamicDraw),
		m_shaderLightPass(nullptr),
		m_shaderLightCSMPass(nullptr),
		m_maxNumPointLights(decltype(m_pointLights.size())(Config::rendererVar().max_num_point_lights)),
		m_maxNumSpotLights(decltype(m_spotLights.size())(Config::rendererVar().max_num_spot_lights)),
		m_numOfCascades(0),
		m_numOfPCFSamples(1),
		m_shadowMappingEnabled(false) { }

	~LightingPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Lighting Rendering Pass";

		// Set lightbuffer values
		m_pointLightBuffer.m_bindingIndex = UniformBufferBinding_PointLights;
		m_spotLightBuffer.m_bindingIndex = UniformBufferBinding_SpotLights;

		// Set the light buffer sizes
		m_pointLightBuffer.m_size = sizeof(PointLightDataSet) * m_maxNumPointLights;
		m_spotLightBuffer.m_size = sizeof(SpotLightDataSet) * m_maxNumSpotLights;
		m_pointLights.reserve(m_maxNumPointLights);
		m_spotLights.reserve(m_maxNumSpotLights);

		// Set buffer values
		m_emissiveAndOutputBuffers.resize(2);
		m_emissiveAndOutputBuffers[0] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GBufferTextureType::GBufferEmissive);
		m_emissiveAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GBufferTextureType::GBufferFinal);

		// Get the current shadow mapping data
		const auto &shadowMappingData = m_renderer.m_frameData.m_shadowMappingData;

		// Get the current number of shadow cascades
		m_numOfCascades = (unsigned int)shadowMappingData.m_shadowCascadePlaneDistances.size();
		m_numOfPCFSamples = shadowMappingData.m_numOfPCFSamples;
		m_shadowMappingEnabled = shadowMappingData.m_shadowMappingEnabled;

		// Load lighting pass shader
		{
			// Create a property-set used to load lighting shader
			PropertySet lightShaderProperties(Properties::Shaders);

			lightShaderProperties.addProperty(Properties::Name, std::string("lightPass"));
			lightShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().light_pass_vert_shader);
			lightShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().light_pass_frag_shader);

			// Create the shader
			m_shaderLightPass = Loaders::shader().load(lightShaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_shaderLightPass->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Set the max number of point lights
				if(ErrorCode shaderVariableError = m_shaderLightPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_maxNumOfPointLights, m_maxNumPointLights); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_maxNumOfPointLights, ErrorSource::Source_LightingPass);

				// Set the max number of spot lights
				if(ErrorCode shaderVariableError = m_shaderLightPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_maxNumOfSpotLights, m_maxNumSpotLights); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_maxNumOfSpotLights, ErrorSource::Source_LightingPass);

				// Disable shadow mapping
				if(ErrorCode shaderVariableError = m_shaderLightPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_shadowMapping, 0); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_shadowMapping, ErrorSource::Source_LightingPass);

				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_shaderLightPass);
			}
			else
				returnError = shaderError;
		}

		// Load lighting with cascaded shadow mapping pass shader
		{
			// Create a property-set used to load lighting shader
			PropertySet lightShaderProperties(Properties::Shaders);

			lightShaderProperties.addProperty(Properties::Name, std::string("lightPass_CSM"));
			lightShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().light_pass_vert_shader);
			lightShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().light_pass_frag_shader);

			// Create the shader
			m_shaderLightCSMPass = Loaders::shader().load(lightShaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_shaderLightCSMPass->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Set the max number of point lights
				if(ErrorCode shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_maxNumOfPointLights, m_maxNumPointLights); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_maxNumOfPointLights, ErrorSource::Source_LightingPass);

				// Set the max number of spot lights
				if(ErrorCode shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_maxNumOfSpotLights, m_maxNumSpotLights); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_maxNumOfSpotLights, ErrorSource::Source_LightingPass);

				// Enable shadow mapping
				if(ErrorCode shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_shadowMapping, 1); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_shadowMapping, ErrorSource::Source_LightingPass);

				// Set the number of cascades
				if(m_numOfCascades > 0)
					if(ErrorCode shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_numOfCascades, m_numOfCascades); shaderVariableError != ErrorCode::Success)
						ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfCascades, ErrorSource::Source_LightingPass);

				// Set the number of PCF samples
				if(ErrorCode shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_numOfPCFSamples, m_numOfPCFSamples); shaderVariableError != ErrorCode::Success)
					ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfPCFSamples, ErrorSource::Source_LightingPass);

				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_shaderLightCSMPass);
			}
			else
				returnError = shaderError;
		}

		// Queue light buffers to be created
		m_renderer.queueForLoading(m_pointLightBuffer);
		m_renderer.queueForLoading(m_spotLightBuffer);

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_LightingPass);
			setInitialized(true);
		}
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_LightingPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		//glDisable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);
		glDepthMask(GL_FALSE);

		// Clear light arrays from previous frame
		m_pointLights.clear();
		m_spotLights.clear();
		m_directionalLight.clear();

		// Iterate over all objects to be rendered with geometry shader
		for(auto entity : p_sceneObjects.m_lights)
		{
			LightComponent &lightComponent = p_sceneObjects.m_lights.get<LightComponent>(entity);

			// Check if the light is enabled
			if(lightComponent.isObjectActive())
			{
				SpatialComponent &spatialComponent = p_sceneObjects.m_lights.get<SpatialComponent>(entity);

				// Add the light data to the corresponding array, based on the light type
				switch(lightComponent.getLightType())
				{
				case LightComponent::LightComponentType_point:
				{
					if(m_pointLights.size() < m_maxNumPointLights)
					{
						// Update position of the light data set
						PointLightDataSet *lightDataSet = lightComponent.getPointLight();
						lightDataSet->m_position = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[3];

						m_pointLights.push_back(*lightDataSet);
					}
				}
				break;

				case LightComponent::LightComponentType_spot:
				{
					if(m_spotLights.size() < m_maxNumSpotLights)
					{
						// Update position and rotation of the light data set
						SpotLightDataSet *lightDataSet = lightComponent.getSpotLight();
						lightDataSet->m_position = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[3];
						lightDataSet->m_direction = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[2];

						m_spotLights.push_back(*lightDataSet);
					}
				}
				break;

				case LightComponent::LightComponentType_directional:
				{
					DirectionalLightDataSet *lightDataSet = lightComponent.getDirectionalLight();
					lightDataSet->m_direction = spatialComponent.getSpatialDataChangeManager().getWorldTransform()[2];

					m_directionalLight = *lightDataSet;
				}
				break;
				}
			}
		}

		// Set the directional light data so it can be sent to the shader
		m_renderer.m_frameData.m_directionalLight = m_directionalLight;

		// Set number of lights so they can be send to the shader
		m_renderer.m_frameData.m_numPointLights = (decltype(m_renderer.m_frameData.m_numPointLights))m_pointLights.size();
		m_renderer.m_frameData.m_numSpotLights = (decltype(m_renderer.m_frameData.m_numSpotLights))m_spotLights.size();

		// Setup point light buffer values
		m_pointLightBuffer.m_updateSize = sizeof(PointLightDataSet) * m_pointLights.size();
		m_pointLightBuffer.m_data = (void*)m_pointLights.data();

		// Setup spot light buffer values
		m_spotLightBuffer.m_updateSize = sizeof(SpotLightDataSet) * m_spotLights.size();
		m_spotLightBuffer.m_data = (void*)m_spotLights.data();

		// Queue light buffer updates (so that new values that were just setup are sent to the GPU)
		m_renderer.queueForUpdate(m_pointLightBuffer);
		m_renderer.queueForUpdate(m_spotLightBuffer);

		// Pass update commands so they are executed 
		m_renderer.passUpdateCommandsToBackend();

		// Bind textures for reading
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferPosition, GBufferTextureType::GBufferPosition);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferDiffuse, GBufferTextureType::GBufferDiffuse);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferNormal, GBufferTextureType::GBufferNormal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferEmissive, GBufferTextureType::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferMatProperties, GBufferTextureType::GBufferMatProperties);

		// Get the current shadow mapping data
		const auto &shadowMappingData = m_renderer.m_frameData.m_shadowMappingData;

		ShaderLoader::ShaderProgram *lightPassShader = m_shaderLightCSMPass;

		if(p_sceneObjects.m_processDrawing && shadowMappingData.m_shadowMappingEnabled && !shadowMappingData.m_shadowCascadePlaneDistances.empty())
		{
			// If the number of shadow cascades has changed, set the new define inside the shader source and recompile the shader
			if(m_numOfCascades != (unsigned int)shadowMappingData.m_shadowCascadePlaneDistances.size() || m_numOfPCFSamples != shadowMappingData.m_numOfPCFSamples)
			{
				m_numOfCascades = (unsigned int)shadowMappingData.m_shadowCascadePlaneDistances.size();
				m_numOfPCFSamples = shadowMappingData.m_numOfPCFSamples;

				if(m_numOfCascades > 0)
				{
					// Enable shadow mapping
					ErrorCode shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_shadowMapping, 1); 
					
					if(shaderVariableError == ErrorCode::Success)
					{
						shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_numOfCascades, m_numOfCascades);

						if(shaderVariableError == ErrorCode::Success)
						{
							shaderVariableError = m_shaderLightCSMPass->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_numOfPCFSamples, m_numOfPCFSamples);

							if(shaderVariableError == ErrorCode::Success)
							{
								// Queue the shader to be loaded to GPU
								m_shaderLightCSMPass->resetLoadedToVideoMemoryFlag();
								m_renderer.queueForLoading(*m_shaderLightCSMPass);
								m_renderer.passLoadCommandsToBackend();

								// Make sure to update uniform block bindings, since light CSM shader uses CSM data set uniform that is owned by the shadow map pass, and might be created later
								m_shaderLightCSMPass->m_uniformUpdater->updateBlockBindingPoints();
							}
							else
								ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfPCFSamples, ErrorSource::Source_LightingPass);
						}
						else
							ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_numOfCascades, ErrorSource::Source_LightingPass);
					}
					else
						ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_shadowMapping, ErrorSource::Source_LightingPass);
				}
			}

			m_renderer.m_backend.getCSMFramebuffer()->bindBufferForReading(CSMBufferTextureType::CSMBufferTextureType_CSMDepthMap, CSMBufferTextureType::CSMBufferTextureType_CSMDepthMap);

			m_shaderLightCSMPass->m_uniformUpdater->updateBlockBindingPoints();
		}
		else
			lightPassShader = m_shaderLightPass;

		// Bind texture for writing
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

		// Queue the screen space triangle, using lighting shader, to be drawn
		m_renderer.queueForDrawing(lightPassShader->getShaderHandle(), lightPassShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);


		// Pass the draw command so it is executed
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	ShaderLoader::ShaderProgram *m_shaderLightPass;
	ShaderLoader::ShaderProgram	*m_shaderLightCSMPass;

	// Buffer handles used for binding
	std::vector<GeometryBuffer::GBufferTexture> m_emissiveAndOutputBuffers;

	// Light buffers
	RendererFrontend::ShaderBuffer	m_pointLightBuffer, 
									m_spotLightBuffer;

	DirectionalLightDataSet m_directionalLight;
	std::vector<PointLightDataSet> m_pointLights;
	std::vector<SpotLightDataSet> m_spotLights;

	decltype(m_pointLights.size()) m_maxNumPointLights;
	decltype(m_spotLights.size()) m_maxNumSpotLights;

	unsigned int m_numOfCascades;
	unsigned int m_numOfPCFSamples;
	bool m_shadowMappingEnabled;
};