#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class FinalPass : public RenderPass
{
public:
	FinalPass(RendererFrontend &p_renderer) : 
		RenderPass(p_renderer, RenderPassType::RenderPassType_Final),
		m_shaderFinalPass(nullptr)
	{
		updateFxaaSettings();
	}

	~FinalPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Final Pass";

		// Load final pass shader
		{
			// Create a property-set used to load final pass shader
			PropertySet finalShaderProperties(Properties::Shaders);
			finalShaderProperties.addProperty(Properties::Name, std::string("final_pass"));
			finalShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().final_pass_vert_shader);
			finalShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().final_pass_frag_shader);

			// Create the shader
			m_shaderFinalPass = Loaders::shader().load(finalShaderProperties);

			updateFxaaSettings();

			// Load the shader to memory
			if(ErrorCode shaderError = loadFinalPassShaderToMemory(m_shaderFinalPass))
				returnError = shaderError;
		}

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_FinalPass);
			setInitialized(true);
		}
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

		// If any of the FXAA settings variables have changed, reload the Final Pass shader
		if(!checkFxaaSettingsUpToDate())
		{
			// Update the FXAA settings
			updateFxaaSettings();

			// Reset the loaded flag of the final pass shader, so it can be reloaded to memory
			m_shaderFinalPass->resetLoadedToVideoMemoryFlag();

			// Reload the final pass shader to memory
			loadFinalPassShaderToMemory(m_shaderFinalPass);

			// Process shader load command
			m_renderer.passLoadCommandsToBackend();
		}

		//glDepthFunc(GL_LEQUAL);
		glDisable(GL_DEPTH_TEST);
		
		// Bind final texture for reading so it can be accessed in the shaders
		//m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferEmissive, GBufferTextureType::GBufferEmissive);
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
	ErrorCode loadFinalPassShaderToMemory(ShaderLoader::ShaderProgram *p_shader)
	{
		ErrorCode shaderError = p_shader->loadToMemory();

		if(shaderError == ErrorCode::Success)
		{
			// Enable or disable the FXAA
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_fxaa, m_fxaaEnabled ? 1 : 0); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_fxaa, ErrorSource::Source_FinalPass);

			// Set the minimum FXAA edge threshold
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_fxaa_edge_threshold_min, m_fxaaEdgeThresholdMin); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_fxaa_edge_threshold_min, ErrorSource::Source_FinalPass);

			// Set the maximum FXAA edge threshold
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_fxaa_edge_threshold_max, m_fxaaEdgeThresholdMax); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_fxaa_edge_threshold_max, ErrorSource::Source_FinalPass);

			// Set the number of FXAA iterations
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_fxaa_iterations, m_fxaaIterations); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_fxaa_iterations, ErrorSource::Source_FinalPass);

			// Set the FXAA sub-pixel quality
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_fxaa_subpixel_quality, m_fxaaSubpixelQuality); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_fxaa_subpixel_quality, ErrorSource::Source_FinalPass);

			// Queue the shader to be loaded to GPU
			m_renderer.queueForLoading(*p_shader);
		}

		return shaderError;
	}

	// Returns false if any of the FXAA settings have changed
	bool checkFxaaSettingsUpToDate()
	{
		return	m_fxaaEnabled == Config::rendererVar().fxaa_enabled &&
				m_fxaaIterations == Config::rendererVar().fxaa_iterations &&
				m_fxaaEdgeThresholdMin == Config::rendererVar().fxaa_edge_threshold_min &&
				m_fxaaEdgeThresholdMax == Config::rendererVar().fxaa_edge_threshold_max &&
				m_fxaaSubpixelQuality == Config::rendererVar().fxaa_edge_subpixel_quality;
	}

	// Update the FXAA settings to the current ones (from Config)
	void updateFxaaSettings()
	{
		m_fxaaEnabled = Config::rendererVar().fxaa_enabled;
		m_fxaaIterations = Config::rendererVar().fxaa_iterations;
		m_fxaaEdgeThresholdMin = Config::rendererVar().fxaa_edge_threshold_min;
		m_fxaaEdgeThresholdMax = Config::rendererVar().fxaa_edge_threshold_max;
		m_fxaaSubpixelQuality = Config::rendererVar().fxaa_edge_subpixel_quality;
	}

	ShaderLoader::ShaderProgram *m_shaderFinalPass;

	// FXAA settings
	bool m_fxaaEnabled;
	int m_fxaaIterations;
	float m_fxaaEdgeThresholdMin;
	float m_fxaaEdgeThresholdMax;
	float m_fxaaSubpixelQuality;
};