#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class SkyPass : public RenderPass
{
public:
	SkyPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_NumOfTypes) { }

	~SkyPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Sky Rendering Pass";

		// Create a property-set used to load blur vertical shaders
		PropertySet skyPassShaderShaderProperties(Properties::Shaders);
		skyPassShaderShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().atm_scattering_sky_vert_shader);
		skyPassShaderShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().atm_scattering_sky_frag_shader);

		// Create shaders
		m_skyPassShader = Loaders::shader().load(skyPassShaderShaderProperties);

		//		 _______________________________
		//		|	  LOAD SKY PASS SHADER		|
		//		|_______________________________|
		shaderError = m_skyPassShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)				// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_skyPassShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_SkyPass);
			setInitialized(true);
		}
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_SkyPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		//glDisable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LEQUAL);
		//glDepthMask(GL_FALSE);

		// Bind input color texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);

		// Bind output color texture for writing to, so it can be used as an intermediate buffer between blur passes
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

		// Perform various visual effects in the post process shader
		m_renderer.queueForDrawing(m_skyPassShader->getShaderHandle(), m_skyPassShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
		
		// Pass the draw command so it is executed
		m_renderer.passScreenSpaceDrawCommandsToBackend();
	}

private:
	ShaderLoader::ShaderProgram	*m_skyPassShader;
};