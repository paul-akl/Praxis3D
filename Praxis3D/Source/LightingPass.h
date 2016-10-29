#pragma once

#include "RenderPassBase.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(RendererFrontend &p_renderer) : RenderPass(p_renderer) { }

	ErrorCode init()
	{
		ErrorCode returnError;

		m_pointLightBuffer.m_bindingIndex = LightBufferBinding_PointLight;
		m_spotLightBuffer.m_bindingIndex = LightBufferBinding_SpotLight;

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

		return returnError;
	}

	void update(const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferPosition, GeometryBuffer::GBufferPosition);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferDiffuse, GeometryBuffer::GBufferDiffuse);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferNormal, GeometryBuffer::GBufferNormal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferMatProperties, GeometryBuffer::GBufferMatProperties);

		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferFinal);
	}

	std::string getName() { return "Lighting Rendering Pass"; }

private:
	ShaderLoader::ShaderProgram	*m_shaderLightPass;

	// Light buffers
	RendererFrontend::LightUniformBuffer m_pointLightBuffer, 
										 m_spotLightBuffer;
};