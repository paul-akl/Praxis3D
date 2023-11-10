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
		m_maxNumPointLights(decltype(m_pointLights.size())(Config::graphicsVar().max_num_point_lights)),
		m_maxNumSpotLights(decltype(m_spotLights.size())(Config::graphicsVar().max_num_spot_lights)) { }

	~LightingPass() { }

	ErrorCode init()
	{
		ErrorCode returnError;

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

		// Create a property-set used to load lighting shader
		PropertySet lightShaderProperties(Properties::Shaders);
		lightShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().light_pass_vert_shader);
		lightShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().light_pass_frag_shader);

		// Create shaders
		m_shaderLightPass = Loaders::shader().load(lightShaderProperties);

		// Load shaders to memory
		returnError = m_shaderLightPass->loadToMemory();

		if(returnError == ErrorCode::Success)
		{
			// Queue the shaders to be loaded to GPU
			m_renderer.queueForLoading(*m_shaderLightPass);
		}

		// Queue light buffers to be created
		m_renderer.queueForLoading(m_pointLightBuffer);
		m_renderer.queueForLoading(m_spotLightBuffer);
		
		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_LightingPass);
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

		// Bind textures for reading
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferPosition, GBufferTextureType::GBufferPosition);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferDiffuse, GBufferTextureType::GBufferDiffuse);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferNormal, GBufferTextureType::GBufferNormal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferEmissive, GBufferTextureType::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferMatProperties, GBufferTextureType::GBufferMatProperties);

		// Bind texture for writing
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

		// Queue light buffer updates (so that new values that were just setup are sent to the GPU)
		m_renderer.queueForUpdate(m_pointLightBuffer);
		m_renderer.queueForUpdate(m_spotLightBuffer);

		// Pass update commands so they are executed 
		m_renderer.passUpdateCommandsToBackend();

		// Queue the screen space triangle, using lighting shader, to be drawn
		m_renderer.queueForDrawing(m_shaderLightPass->getShaderHandle(), m_shaderLightPass->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);

		// Pass the draw command so it is executed
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	ShaderLoader::ShaderProgram	*m_shaderLightPass;

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
};