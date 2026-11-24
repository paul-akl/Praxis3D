#pragma once

#include "Systems/RendererSystem/Include/GraphicsDataSets.hpp"
#include "Systems/RendererSystem/Passes/Include/RenderPassBase.hpp"

class BloomCompositePass : public RenderPass
{
public:
	BloomCompositePass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_BloomComposite)
	{
		m_bloomCompositeShader = nullptr;
	}

	~BloomCompositePass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Bloom Composite Rendering Pass";
		
		// Create a property-set used to load blur vertical shaders
		PropertySet shaderProperties(Properties::Shaders);
		shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().bloom_composite_pass_vert_shader);
		shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().bloom_composite_pass_frag_shader);

		// Create shaders
		m_bloomCompositeShader = Loaders::shader().load(shaderProperties);

		//		 _______________________________
		//		|	LOAD POST PROCESS SHADER	|
		//		|_______________________________|
		shaderError = m_bloomCompositeShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)						// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_bloomCompositeShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;
		
		if(returnError == ErrorCode::Success)
			setInitialized(true);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		if(p_sceneObjects.m_processDrawing)
		{
			glDisable(GL_DEPTH_TEST);

			// Bind input color texture for reading so it can be accessed in the shaders
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferEmissive, GBufferTextureType::GBufferEmissive);

			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

			// Perform various visual effects in the post process shader
			m_renderer.queueForDrawing(m_bloomCompositeShader->getShaderHandle(), m_bloomCompositeShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
			m_renderer.passScreenSpaceDrawCommandsToBackend();
		}

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	ShaderLoader::ShaderProgram	*m_bloomCompositeShader;
};