#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class FinalPass : public RenderPass
{
public:
	FinalPass(RendererFrontend &p_renderer) : 
		RenderPass(p_renderer) { }

	~FinalPass() { }

	ErrorCode init()
	{
		ErrorCode returnError;

		m_name = "Final Rendering Pass";
		
		// Create a property-set used to load geometry shader
		PropertySet finalShaderProperties(Properties::Shaders);
		finalShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().final_pass_vert_shader);
		finalShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().final_pass_frag_shader);

		// Create shaders
		m_shaderFinalPass = Loaders::shader().load(finalShaderProperties);

		// Load shaders to memory
		returnError = m_shaderFinalPass->loadToMemory();

		if(returnError == ErrorCode::Success)
		{
			// Queue the shaders to be loaded to GPU
			m_renderer.queueForLoading(*m_shaderFinalPass);
		}
		
		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
#ifdef SETTING_USE_BLIT_FRAMEBUFFER

		// Set the default framebuffer to blit pixels to
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);

		// Set final framebuffer to blit pixels from
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForReading(GeometryBuffer::FramebufferGeometry);

		// Bind final framebuffer
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferFinal, GeometryBuffer::GBufferFinal);

#else

		// Bind final framebuffer
		//m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferFinal, GeometryBuffer::GBufferFinal);

		glDisable(GL_DEPTH_TEST);
		
		if(Config::graphicsVar().eye_adaption)
		{
			// Generate mipmaps for the final buffer, for use in tone mapping
			m_renderer.m_backend.getGeometryBuffer()->generateMipmap(GeometryBuffer::GBufferFinal);
		}

		// Bind final texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferEmissive, GeometryBuffer::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GeometryBuffer::GBufferInputTexture);
		//m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferNormal, GeometryBuffer::GBufferInputTexture);

		// Set the default framebuffer to be drawn to
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);

		// Queue and render a full screen quad using a final pass shader
		m_renderer.queueForDrawing(m_shaderFinalPass->getShaderHandle(), m_shaderFinalPass->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

#endif // SETTING_USE_BLIT_FRAMEBUFFER
	}

private:
	ShaderLoader::ShaderProgram	*m_shaderFinalPass;
};