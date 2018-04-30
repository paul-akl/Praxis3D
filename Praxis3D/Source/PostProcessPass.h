#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class PostProcessPass : public RenderPass
{
public:
	PostProcessPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer) { }

	~PostProcessPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Post Process Rendering Pass";

		// Create a property-set used to load blur vertical shaders
		PropertySet postProcessShaderProperties(Properties::Shaders);
		postProcessShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().postProcess_pass_vert_shader);
		postProcessShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().postProcess_pass_frag_shader);

		// Create shaders
		m_postProcessShader = Loaders::shader().load(postProcessShaderProperties);

		//		 _______________________________
		//		|	LOAD POST PROCESS SHADER	|
		//		|_______________________________|
		shaderError = m_postProcessShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)					// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_postProcessShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);

		// Bind input color texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GeometryBuffer::GBufferInputTexture);

		// Bind output color texture for writing to, so it can be used as an intermediate buffer between blur passes
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

		// Perform various visual effects in the post process shader
		m_renderer.queueForDrawing(m_postProcessShader->getShaderHandle(), m_postProcessShader->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	ShaderLoader::ShaderProgram	*m_postProcessShader;
};