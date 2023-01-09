#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class LenseFlarePass : public RenderPass
{
public:
	LenseFlarePass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer), 
		m_lensFlareParamBuffer(BufferType_Uniform, BufferUsageHint_DynamicDraw),
		m_lensFlareGhostGradient(Loaders::texture2D().load(Config::rendererVar().lens_flare_ghost_gradient_texture))
	{
		m_lensFlareParam.m_lensFlaireDownsample = Config::graphicsVar().lens_flare_downsample;
		m_lensFlareParam.m_chromaticAberration = Config::graphicsVar().lens_flare_chrom_abberration;
		m_lensFlareParam.m_ghostCount = Config::graphicsVar().lens_flare_ghost_count;
		m_lensFlareParam.m_ghostSpacing  = Config::graphicsVar().lens_flare_ghost_spacing;
		m_lensFlareParam.m_ghostThreshold = Config::graphicsVar().lens_flare_ghost_threshold;
		m_lensFlareParam.m_haloRadius = Config::graphicsVar().lens_flare_halo_radius;
		m_lensFlareParam.m_haloThickness = Config::graphicsVar().lens_flare_halo_thickness;
		m_lensFlareParam.m_haloThreshold = Config::graphicsVar().lens_flare_halo_threshold;

		if(Config::windowVar().fullscreen)
		{
			m_currentScreenRes.x = Config::windowVar().window_size_fullscreen_x;
			m_currentScreenRes.y = Config::windowVar().window_size_fullscreen_y;
		}
		else
		{
			m_currentScreenRes.x = Config::windowVar().window_size_windowed_x;
			m_currentScreenRes.y = Config::windowVar().window_size_windowed_y;
		}

		m_lenseFlareShader = nullptr;
	}

	~LenseFlarePass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Lense Flare Rendering Pass";
		
		// Set buffer values
		m_diffuseAndOutputBuffers.resize(2);
		m_diffuseAndOutputBuffers[0] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferDiffuse);
		m_diffuseAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferFinal);

		// Create a property-set used to load blur vertical shaders
		PropertySet shaderProperties(Properties::Shaders);
		shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().lense_flare_pass_vert_shader);
		shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().lense_flare_pass_frag_shader);

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
		
		// Set lens flare parameters buffer shader binding index
		m_lensFlareParamBuffer.m_bindingIndex = UniformBufferBinding_LensFlareParam;

		// Set lens flare parameters buffer size and data
		m_lensFlareParamBuffer.m_size = sizeof(LensFlareParameters);
		m_lensFlareParamBuffer.m_data = (void*)(&m_lensFlareParam);

		// Queue lens flare parameters buffer to be created
		m_renderer.queueForLoading(m_lensFlareParamBuffer);

		// Create textures for lens flare effect and load them to memory
		m_lensFlareGhostGradient.loadToMemory();
		
		// Queue lens flare textures for loading to GPU
		m_renderer.queueForLoading(m_lensFlareGhostGradient);
		
		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_LensFlarePass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_LensFlarePass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);

		// Calculate the apsect ratio for the halo
		if(m_currentScreenRes.x != Config::graphicsVar().current_resolution_x || m_currentScreenRes.y != Config::graphicsVar().current_resolution_y)
		{
			m_currentScreenRes.x = Config::graphicsVar().current_resolution_x;
			m_currentScreenRes.y = Config::graphicsVar().current_resolution_y;

			// Set lens flare parameters buffer size and data
			m_lensFlareParamBuffer.m_size = sizeof(LensFlareParameters);
			m_lensFlareParamBuffer.m_data = (void *)(&m_lensFlareParam);

			// Queue lens flare parameters buffer to be created
			m_renderer.queueForLoading(m_lensFlareParamBuffer);
		}

		// Set the output buffer
		m_diffuseAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(p_renderPassData.getColorOutputMap());

		// Bind input color texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GeometryBuffer::GBufferInputTexture);
		//m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferEmissive, GeometryBuffer::GBufferEmissive);
		
		// Bind textures for writing
		//m_renderer.m_backend.getGeometryBuffer()->bindBuffersForWriting(m_diffuseAndOutputBuffers);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferDiffuse);

		// Bind lens flare textures
		glActiveTexture(GL_TEXTURE0 + LensFlareTextureType::LensFlareTextureType_GhostGradient);
		glBindTexture(GL_TEXTURE_2D, m_lensFlareGhostGradient.getHandle());

		// Perform various visual effects in the post process shader
		m_renderer.queueForDrawing(m_lenseFlareShader->getShaderHandle(), m_lenseFlareShader->getUniformUpdater(), p_sceneObjects.m_camera.m_spatialData.m_transformMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		p_renderPassData.setBlurInputMap(GeometryBuffer::GBufferDiffuse);
		p_renderPassData.setBlurOutputMap(GeometryBuffer::GBufferDiffuse);
		p_renderPassData.setIntermediateMap(p_renderPassData.getColorOutputMap());
		p_renderPassData.m_blurDoBlending = false;
		p_renderPassData.m_numOfBlurPasses = Config::graphicsVar().lens_flare_blur_passes;
	}

private:
	// Buffer handles used for binding
	std::vector<GeometryBuffer::GBufferTexture> m_diffuseAndOutputBuffers;

	ShaderLoader::ShaderProgram	*m_lenseFlareShader;

	RendererFrontend::ShaderBuffer m_lensFlareParamBuffer;

	LensFlareParameters m_lensFlareParam;

	TextureLoader2D::Texture2DHandle m_lensFlareGhostGradient;

	glm::ivec2 m_currentScreenRes;
};