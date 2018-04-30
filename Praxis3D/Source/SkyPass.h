#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class SkyPass : public RenderPass
{
public:
	SkyPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer) { }

	~SkyPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Blur Rendering Pass";

		// Create a property-set used to load blur vertical shaders
		PropertySet skyPassShaderShaderProperties(Properties::Shaders);
		skyPassShaderShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().gaussian_blur_vertical_vert_shader);
		skyPassShaderShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().gaussian_blur_vertical_frag_shader);

		// Create shaders
		m_skyPassShader = Loaders::shader().load(skyPassShaderShaderProperties);

		//		 _______________________________
		//		|	  LOAD SKY PASS SHADER		|
		//		|_______________________________|
		shaderError = m_skyPassShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)					// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_skyPassShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;


		return returnError;
	}

	void update(const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);

		// Set the default framebuffer to be drawn to
		//m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);

		// Bind final texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferFinal, GeometryBuffer::GBufferFinal);

		// Bind intermediate texture for writing to, so HDR mapping is outputed to it
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferIntermediate);

		// Perform HDR mapping. Queue and render a full screen quad using an HDR pass shader
		m_renderer.queueForDrawing(m_skyPassShader->getShaderHandle(), m_skyPassShader->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();
	}

private:
	ShaderLoader::ShaderProgram	*m_skyPassShader;
};