#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class TonemappingPass : public RenderPass
{
public:
	TonemappingPass(RendererFrontend &p_renderer) : 
		RenderPass(p_renderer, RenderPassType::RenderPassType_Tonemapping),
		m_shaderTonemappingPass(nullptr)
	{
		m_tonemappingMethod = 0;
	}

	~TonemappingPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Tonemapping Pass";

		m_tonemappingMethod = Config::graphicsVar().tonemap_method;

		// Load tonemapping pass shader
		{
			// Create a property-set used to load final pass shader
			PropertySet tonemappingShaderProperties(Properties::Shaders);
			tonemappingShaderProperties.addProperty(Properties::Name, std::string("tonemapping_pass"));
			tonemappingShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().tonemapping_frag_shader);
			tonemappingShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().tonemapping_vert_shader);

			// Create the shader
			m_shaderTonemappingPass = Loaders::shader().load(tonemappingShaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = loadTonemappingPassShaderToMemory(m_shaderTonemappingPass))
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
		// If tonemapping method has changed, reload the Tonemapping Pass shader
		if(m_tonemappingMethod != Config::graphicsVar().tonemap_method)
		{
			m_tonemappingMethod = Config::graphicsVar().tonemap_method;

			// Reset the loaded flag of the shader, so it can be reloaded to memory
			m_shaderTonemappingPass->resetLoadedToVideoMemoryFlag();

			// Reload the shader to memory
			loadTonemappingPassShaderToMemory(m_shaderTonemappingPass);

			// Process shader load command
			m_renderer.passLoadCommandsToBackend();
		}

		glDisable(GL_DEPTH_TEST);

		// Bind input texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);

		// Bind output texture for drawing to
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());
		
		// Queue and render a full screen quad using a final pass shader
		m_renderer.queueForDrawing(m_shaderTonemappingPass->getShaderHandle(), m_shaderTonemappingPass->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	ErrorCode loadTonemappingPassShaderToMemory(ShaderLoader::ShaderProgram *p_shader)
	{
		ErrorCode shaderError = p_shader->loadToMemory();

		if(shaderError == ErrorCode::Success)
		{
			// Set the tonemapping method
			if(ErrorCode shaderVariableError = p_shader->setDefineValue(ShaderType::ShaderType_Fragment, Config::shaderVar().define_tonemappingMethod, m_tonemappingMethod); shaderVariableError != ErrorCode::Success)
				ErrHandlerLoc::get().log(shaderVariableError, Config::shaderVar().define_tonemappingMethod, ErrorSource::Source_FinalPass);

			// Queue the shader to be loaded to GPU
			m_renderer.queueForLoading(*p_shader);
		}

		return shaderError;
	}

	ShaderLoader::ShaderProgram *m_shaderTonemappingPass;

	// Tonemapping settings
	int m_tonemappingMethod;
};