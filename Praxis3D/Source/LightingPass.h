#pragma once

#include "RenderPassBase.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(RendererFrontend &p_renderer) : 
		RenderPass(p_renderer), 
		m_pointLightBuffer(BufferType_Uniform, BufferUsageHint_DynamicDraw),
		m_spotLightBuffer(BufferType_Uniform, BufferUsageHint_DynamicDraw) { }

	~LightingPass() { }

	ErrorCode init()
	{
		ErrorCode returnError;

		m_name = "Lighting Rendering Pass";

		// Set lightbuffer values
		m_pointLightBuffer.m_bindingIndex = UniformBufferBinding_PointLights;
		m_spotLightBuffer.m_bindingIndex = UniformBufferBinding_SpotLights;

		// Set the light buffer sizes
		m_pointLightBuffer.m_size = sizeof(PointLightDataSet) * Config::graphicsVar().max_num_point_lights;
		m_spotLightBuffer.m_size = sizeof(SpotLightDataSet) * Config::graphicsVar().max_num_spot_lights;
		
		// Set buffer values
		m_emissiveAndOutputBuffers.resize(2);
		m_emissiveAndOutputBuffers[0] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferEmissive);
		m_emissiveAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferFinal);

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

		// Setup point light buffer values
		m_pointLightBuffer.m_updateSize = sizeof(PointLightDataSet) * p_sceneObjects.m_pointLights.size();
		m_pointLightBuffer.m_data = (void*)p_sceneObjects.m_pointLights.data();

		// Setup spot light buffer values
		m_spotLightBuffer.m_updateSize = sizeof(SpotLightDataSet) * p_sceneObjects.m_spotLights.size();
		m_spotLightBuffer.m_data = (void*)p_sceneObjects.m_spotLights.data();

		// Bind textures for reading
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferPosition, GeometryBuffer::GBufferPosition);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferDiffuse, GeometryBuffer::GBufferDiffuse);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferNormal, GeometryBuffer::GBufferNormal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferEmissive, GeometryBuffer::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferMatProperties, GeometryBuffer::GBufferMatProperties);

		// Bind texture for writing
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

		// Queue light buffer updates (so that new values that were just setup are sent to the GPU)
		m_renderer.queueForUpdate(m_pointLightBuffer);
		m_renderer.queueForUpdate(m_spotLightBuffer);

		// Pass update commands so they are executed 
		m_renderer.passUpdateCommandsToBackend();

		// Queue the screen space triangle, using lighting shader, to be drawned
		m_renderer.queueForDrawing(m_shaderLightPass->getShaderHandle(), m_shaderLightPass->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);

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
};