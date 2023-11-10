#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class FinalPass : public RenderPass
{
public:
	FinalPass(RendererFrontend &p_renderer) : 
		RenderPass(p_renderer, RenderPassType::RenderPassType_Final) { }

	~FinalPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Failure;

		m_name = "Final Pass";

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
		
		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_FinalPass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_FinalPass);

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

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_DEPTH_TEST);
		
		if(Config::graphicsVar().bloom_enabled)
		{
			// Generate mipmaps for the final buffer, for use in tone mapping
			//m_renderer.m_backend.getGeometryBuffer()->generateMipmap(GeometryBuffer::GBufferFinal);
		}

		// Bind final texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferEmissive, GBufferTextureType::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);

		// If the render to texture flag is set, draw the scene to an FBO instead of the screen (i.e. so that it can be rendered in a GUI window during editor state)
		if(p_renderPassData.m_renderFinalToTexture)
		{
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferEmissive);
		}
		else
		{
			// Set the default framebuffer to be drawn to
			m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);
		}

		// Queue and render a full screen quad using a final pass shader
		m_renderer.queueForDrawing(m_shaderFinalPass->getShaderHandle(), m_shaderFinalPass->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

#endif // SETTING_USE_BLIT_FRAMEBUFFER
	}

private:
	ShaderLoader::ShaderProgram *m_shaderFinalPass;
};