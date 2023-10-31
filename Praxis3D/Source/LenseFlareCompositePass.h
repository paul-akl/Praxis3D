#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class LenseFlareCompositePass : public RenderPass
{
public:
	LenseFlareCompositePass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer), 
		m_lensFlareDirt(Loaders::texture2D().load(Config::rendererVar().lens_flare_dirt_texture)),
		m_lenseFlareStarburst(Loaders::texture2D().load(Config::rendererVar().lens_flare_starburst_texture))
	{
		m_lenseFlareShader = nullptr;
	}

	~LenseFlareCompositePass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError = ErrorCode::Success;

		m_name = "Lense Flare Composite Rendering Pass";
		
		// Set buffer values
		m_diffuseAndOutputBuffers.resize(2);
		m_diffuseAndOutputBuffers[0] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GBufferTextureType::GBufferDiffuse);
		m_diffuseAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GBufferTextureType::GBufferFinal);

		// Create a property-set used to load blur vertical shaders
		PropertySet shaderProperties(Properties::Shaders);
		shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().lense_flare_comp_pass_vert_shader);
		shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().lense_flare_comp_pass_frag_shader);

		// Create shaders
		m_lenseFlareShader = Loaders::shader().load(shaderProperties);

		//		 _______________________________
		//		|	 LOAD LENSE FLARE SHADER	|
		//		|_______________________________|
		shaderError = m_lenseFlareShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)					// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_lenseFlareShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;
		
		// Create textures for lens flare effect and load them to memory
		m_lensFlareDirt.loadToMemory();
		m_lenseFlareStarburst.loadToMemory();
		
		// Queue lens flare textures for loading to GPU
		m_renderer.queueForLoading(m_lensFlareDirt);
		m_renderer.queueForLoading(m_lenseFlareStarburst);

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_LensFlareCompositePass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_LensFlareCompositePass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);
		
		// Set the output buffer
		//m_diffuseAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(p_renderPassData.getColorOutputMap());

		// Bind input color texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferDiffuse, GBufferTextureType::GBufferDiffuse);
		
		// Bind textures for writing
		//m_renderer.m_backend.getGeometryBuffer()->bindBuffersForWriting(m_diffuseAndOutputBuffers);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

		// Bind lens flare textures
		glActiveTexture(GL_TEXTURE0 + LensFlareTextureType::LensFlareTextureType_LenseDirt);
		glBindTexture(GL_TEXTURE_2D, m_lensFlareDirt.getHandle());
		glActiveTexture(GL_TEXTURE0 + LensFlareTextureType::LensFlareTextureType_Starburst);
		glBindTexture(GL_TEXTURE_2D, m_lenseFlareStarburst.getHandle());

		// Perform various visual effects in the post process shader
		m_renderer.queueForDrawing(m_lenseFlareShader->getShaderHandle(), m_lenseFlareShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
		m_renderer.passScreenSpaceDrawCommandsToBackend();
		
		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	// Buffer handles used for binding
	std::vector<GeometryBuffer::GBufferTexture> m_diffuseAndOutputBuffers;

	ShaderLoader::ShaderProgram	*m_lenseFlareShader;

	TextureLoader2D::Texture2DHandle	m_lensFlareDirt,
										m_lenseFlareStarburst;
};