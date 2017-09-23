#pragma once

#include "RenderPassBase.h"

class FinalPass : public RenderPass
{
public:
	FinalPass(RendererFrontend &p_renderer) : RenderPass(p_renderer) { }

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

	void update(const SceneObjects &p_sceneObjects, const float p_deltaTime)
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

		// Bind final framebuffer
		//glActiveTexture(GL_TEXTURE0 + GeometryBuffer::GBufferFinal);
		//glBindTexture(GL_TEXTURE_2D, GeometryBuffer::GBufferFinal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferFinal);

		// Set the default framebuffer to be drawn to
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);


#endif // SETTING_USE_BLIT_FRAMEBUFFER

		m_renderer.queueForDrawing(m_shaderFinalPass->getShaderHandle(), m_shaderFinalPass->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);

		m_renderer.passScreenSpaceDrawCommandsToBackend();
	}

private:
	ShaderLoader::ShaderProgram	*m_shaderFinalPass;
};