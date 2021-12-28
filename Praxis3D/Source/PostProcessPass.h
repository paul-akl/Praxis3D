#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class PostProcessPass : public RenderPass
{
public:
	PostProcessPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer), 
		m_lensFlareParamBuffer(BufferType_Uniform, BufferUsageHint_DynamicDraw),
		m_lensFlareGhostGradient(Loaders::texture2D().load(Config::rendererVar().lens_flare_ghost_gradient_texture))
	{
		m_lensFlareParam.m_lensFlaireDownsample = 0.0f;
		m_lensFlareParam.m_chromaticAberration = 0.01f;
		m_lensFlareParam.m_ghostCount = 4;
		m_lensFlareParam.m_ghostSpacing = 0.1f;
		m_lensFlareParam.m_ghostThreshold = 2.0f;
		m_lensFlareParam.m_haloRadius = 0.6f;
		m_lensFlareParam.m_haloThickness = 0.1f;
		m_lensFlareParam.m_haloThreshold = 2.0f;
		m_lensFlareParam.m_haloAspectRatio = 1.0f;
	}

	~PostProcessPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Post Process Rendering Pass";
		
		// Set buffer values
		m_diffuseAndOutputBuffers.resize(2);
		m_diffuseAndOutputBuffers[0] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferDiffuse);
		m_diffuseAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferFinal);

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

		//Loaders::texture2D().load(m_materialNames.m_materials[matType][i].m_filename, static_cast<MaterialType>(matType), false)
		
		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_PostProcessPass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_PostProcessPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);
		
		// Set the output buffer
		m_diffuseAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(p_renderPassData.getColorOutputMap());

		// Bind input color texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GeometryBuffer::GBufferInputTexture);
		//m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferEmissive, GeometryBuffer::GBufferEmissive);
		
		// Bind textures for writing
		m_renderer.m_backend.getGeometryBuffer()->bindBuffersForWriting(m_diffuseAndOutputBuffers);

		// Bind lens flare textures
		glActiveTexture(GL_TEXTURE0 + LensFlareTextureType::LensFlareTextureType_GhostGradient);
		glBindTexture(GL_TEXTURE_2D, m_lensFlareGhostGradient.getHandle());

		// Perform various visual effects in the post process shader
		m_renderer.queueForDrawing(m_postProcessShader->getShaderHandle(), m_postProcessShader->getUniformUpdater(), p_sceneObjects.m_camera.m_viewData.m_transformMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	// Buffer handles used for binding
	std::vector<GeometryBuffer::GBufferTexture> m_diffuseAndOutputBuffers;

	ShaderLoader::ShaderProgram	*m_postProcessShader;

	RendererFrontend::ShaderBuffer m_lensFlareParamBuffer;

	LensFlareParameters m_lensFlareParam;

	TextureLoader2D::Texture2DHandle m_lensFlareGhostGradient;
};