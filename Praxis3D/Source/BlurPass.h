#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class BlurPass : public RenderPass
{
public:
	BlurPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer) { }

	~BlurPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Blur Rendering Pass";
		
		// Create a property-set used to load blur vertical shaders
		PropertySet blurVerticalShaderProperties(Properties::Shaders);
		blurVerticalShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().gaussian_blur_vertical_vert_shader);
		blurVerticalShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().gaussian_blur_vertical_frag_shader);

		// Create a property-set used to load blur horizontal shaders
		PropertySet blurHorizontalShaderProperties(Properties::Shaders);
		blurHorizontalShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().gaussian_blur_horizontal_vert_shader);
		blurHorizontalShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().gaussian_blur_horizontal_frag_shader);

		// Create shaders
		m_blurVerticalShader = Loaders::shader().load(blurVerticalShaderProperties);
		m_blurHorizontalShader = Loaders::shader().load(blurHorizontalShaderProperties);

		//		 _______________________________
		//		|	LOAD BLUR VERTICAL SHADER	|
		//		|_______________________________|
		shaderError = m_blurVerticalShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)					// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_blurVerticalShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;

		//		 _______________________________
		//		|	LOAD BLUR HORIZONTAL SHADER	|
		//		|_______________________________|
		shaderError = m_blurHorizontalShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)						// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_blurHorizontalShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;
		
		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_BlurPass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_BlurPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);
		
		// Set the default framebuffer to be drawn to
		//m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);
		
		// Bind emissive texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getBlurInputMap(), GeometryBuffer::GBufferInputTexture);

		// Bind blur texture for writing to, so it can be used as an intermediate buffer between blur passes
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getIntermediateMap());

		// Perform verical blur. Queue and render a full screen quad using a vertical blur shader
		m_renderer.queueForDrawing(m_blurVerticalShader->getShaderHandle(), m_blurVerticalShader->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		for(decltype(p_renderPassData.m_numOfBlurPasses) i = 0; i < p_renderPassData.m_numOfBlurPasses; i++)
		{
			// Bind intermediate blur texture for reading so it can be accessed in the shaders
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getIntermediateMap(), GeometryBuffer::GBufferInputTexture);

			// Bind emissive texture for writing to, so the second pass populates it with the final blur result
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getBlurInputMap());

			// Perform horizontal blur. Queue and render a full screen quad using a horizontal blur shader
			m_renderer.queueForDrawing(m_blurHorizontalShader->getShaderHandle(), m_blurHorizontalShader->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
			m_renderer.passScreenSpaceDrawCommandsToBackend();

			// Bind emissive texture for reading so it can be accessed in the shaders
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getBlurInputMap(), GeometryBuffer::GBufferInputTexture);

			// Bind blur texture for writing to, so it can be used as an intermediate buffer between blur passes
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getIntermediateMap());

			// Perform verical blur. Queue and render a full screen quad using a vertical blur shader
			m_renderer.queueForDrawing(m_blurVerticalShader->getShaderHandle(), m_blurVerticalShader->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
			m_renderer.passScreenSpaceDrawCommandsToBackend();
		}

		// If blending should be enabled
		if(p_renderPassData.m_blurDoBlending)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		}

		// Bind intermediate blur texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getIntermediateMap(), GeometryBuffer::GBufferInputTexture);
		
		// Bind emissive texture for writing to, so the second pass populates it with the final blur result
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getBlurOutputMap());

		// Perform horizontal blur. Queue and render a full screen quad using a horizontal blur shader
		m_renderer.queueForDrawing(m_blurHorizontalShader->getShaderHandle(), m_blurHorizontalShader->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		// If blending should be enabled
		if(p_renderPassData.m_blurDoBlending)
			glDisable(GL_BLEND);
	}

private:
	ShaderLoader::ShaderProgram	*m_blurVerticalShader,
								*m_blurHorizontalShader;
};