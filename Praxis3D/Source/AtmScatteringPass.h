#pragma once

#include "AtmScatteringModel.h"
#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class AtmScatteringPass : public RenderPass
{
public:
	AtmScatteringPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer),
		m_useConstantSolarSpectrum(false),
		m_useOzone(true),
		m_useCombinedTextures(true),
		m_useHalfPrecision(true),
		m_luminanceType(NONE),
		m_useWhiteBalance(true),
		kSunAngularRadius(0.00935 / 2.0),
		kLengthUnitInMeters(1000.0),
		m_atmParamBuffer(BufferType_Uniform, BufferUsageHint_DynamicDraw)
	{
		kSunSolidAngle = PI * kSunAngularRadius * kSunAngularRadius;

		m_vertexShaderSource = R"(
	#version 330
	uniform mat4 modelMat;
	uniform mat4 viewMat;
	layout(location = 0) in vec4 vertex;
	out vec3 view_ray;
	void main() {
		view_ray = (modelMat * vec4((viewMat * vertex).xyz, 0.0)).xyz;
	gl_Position = vertex;
	})";

		m_atmosphereParam = AtmosphereParameters(
			glm::vec3(1.474000f, 1.850400f, 1.911980f),
			0.004675f,
			DensityProfile(DensityProfLayer(0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f), DensityProfLayer(0.000000f, 1.000000f, -0.125000f, 0.000000f, 0.000000f)),
			DensityProfile(DensityProfLayer(0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f), DensityProfLayer(0.000000f, 1.000000f, -0.833333f, 0.000000f, 0.000000f)),
			DensityProfile(DensityProfLayer(25.000000f, 0.000000f, 0.000000f, 0.066667f, -0.666667f), DensityProfLayer(0.000000f, 0.000000f, 0.000000f, -0.066667f, 2.666667f)),
			glm::vec3(0.005802f, 0.013558f, 0.033100f),
			6360.000000f,
			glm::vec3(0.003996f, 0.003996f, 0.003996f),
			6420.000000f,
			glm::vec3(0.004440f, 0.004440f, 0.004440f),
			0.800000f,
			glm::vec3(0.000650f, 0.001881f, 0.000085f),
			-0.207912f,
			glm::vec3(0.100000f, 0.100000f, 0.100000f));

		m_atmScatteringParam = AtmScatteringParameters(
			m_atmosphereParam,
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(0.0f, -6360000.0f / 1000.0f, 0.0f),
			glm::vec2(tan(0.00935f / 2.0f), cos(0.00935f / 2.0f)));

	}

	~AtmScatteringPass()
	{
	}

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "Atmospheric Scattering Rendering Pass";
		
		// Initialize atmospheric scattering model
		InitModel();

		// Create a property-set used to load sky pass shaders
		PropertySet skyShaderProperties(Properties::Shaders);
		skyShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().atm_scattering_sky_vert_shader);
		skyShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().atm_scattering_sky_frag_shader);
		
		// Create a property-set used to load ground pass shaders
		PropertySet groundShaderProperties(Properties::Shaders);
		groundShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().atm_scattering_ground_vert_shader);
		groundShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().atm_scattering_ground_frag_shader);
		
		// Create shaders
		m_skyShader = Loaders::shader().load(skyShaderProperties);
		m_groundShader = Loaders::shader().load(groundShaderProperties);

		//		 _______________________________
		//		|	  ATMOSPHERIC SCATTERING 	|
		//		|________SKY PASS SHADER________|
		shaderError = m_skyShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)			// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_skyShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;
		
		//		 _______________________________
		//		|	  ATMOSPHERIC SCATTERING 	|
		//		|_______GROUND PASS SHADER______|
		shaderError = m_groundShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)				// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_groundShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;

		// Set atmosphere parameters buffer shader binding index
		m_atmParamBuffer.m_bindingIndex = UniformBufferBinding_AtmScatParam;

		// Set atmosphere parameters buffer size and data
		m_atmParamBuffer.m_size = sizeof(AtmScatteringParameters);
		m_atmParamBuffer.m_data = (void*)(&m_atmScatteringParam);

		// Queue atmosphere parameters buffer to be created
		m_renderer.queueForLoading(m_atmParamBuffer);

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_AtmScatteringPass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_AtmScatteringPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glEnable(GL_DEPTH_TEST);

		if(p_renderPassData.m_atmScatDoSkyPass)
		{
			//glDisable(GL_DEPTH_TEST);
			//glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			//glDepthMask(GL_FALSE);

			// Bind output color texture for writing to
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

			glUseProgram(m_skyShader->getShaderHandle());

			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_Irradiance);
			glBindTexture(GL_TEXTURE_2D, m_atmScatteringModel->getIrradianceTexture());
			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_Scattering);
			glBindTexture(GL_TEXTURE_3D, m_atmScatteringModel->getScatteringTexture());
			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_SingleMie);
			glBindTexture(GL_TEXTURE_3D, m_atmScatteringModel->getSingleMieTexture());
			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_Transmittance);
			glBindTexture(GL_TEXTURE_2D, m_atmScatteringModel->getTransmittanceTexture());

			// Set atmosphere parameters buffer size and data
			//m_atmParamBuffer.m_updateSize = sizeof(AtmScatteringParameters);
			//m_atmParamBuffer.m_data = (void*)(&m_atmScatteringParam);
			//m_renderer.queueForUpdate(m_atmParamBuffer);

			// Pass update commands so they are executed 
			//m_renderer.passUpdateCommandsToBackend();

			// Perform various visual effects in the post process shader
			m_renderer.queueForDrawing(m_skyShader->getShaderHandle(), m_skyShader->getUniformUpdater(), p_sceneObjects.m_camera.m_viewData.m_transformMat);
			
		}
		else
		{
			glDepthFunc(GL_GREATER);
			//glDisable(GL_DEPTH_TEST);
			//glEnable(GL_DEPTH_TEST);
			//glDepthFunc(GL_LEQUAL);
			//glDepthMask(GL_FALSE);

			// Bind output color texture for writing to
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

			glUseProgram(m_groundShader->getShaderHandle());

			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_Irradiance);
			glBindTexture(GL_TEXTURE_2D, m_atmScatteringModel->getIrradianceTexture());
			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_Scattering);
			glBindTexture(GL_TEXTURE_3D, m_atmScatteringModel->getScatteringTexture());
			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_SingleMie);
			glBindTexture(GL_TEXTURE_3D, m_atmScatteringModel->getSingleMieTexture());
			glActiveTexture(GL_TEXTURE0 + AtmScatteringTextureType::AtmScatteringTextureType_Transmittance);
			glBindTexture(GL_TEXTURE_2D, m_atmScatteringModel->getTransmittanceTexture());
			
			
			auto tex1 = m_atmScatteringModel->getIrradianceTexture();
			auto tex2 = m_atmScatteringModel->getScatteringTexture();
			auto tex3 = m_atmScatteringModel->getSingleMieTexture();
			auto tex4 = m_atmScatteringModel->getTransmittanceTexture();

			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GeometryBuffer::GBufferInputTexture);
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferPosition, GeometryBuffer::GBufferPosition);
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferNormal, GeometryBuffer::GBufferNormal);
			
			// Set atmosphere parameters buffer size and data
			//m_atmParamBuffer.m_updateSize = sizeof(AtmScatteringParameters);
			//m_atmParamBuffer.m_data = (void*)(&m_atmScatteringParam);
			//m_renderer.queueForUpdate(m_atmParamBuffer);

			// Pass update commands so they are executed 
			//m_renderer.passUpdateCommandsToBackend();

			// Perform various visual effects in the post process shader
			m_renderer.queueForDrawing(m_groundShader->getShaderHandle(), m_groundShader->getUniformUpdater(), p_sceneObjects.m_camera.m_viewData.m_transformMat);
		
		}
		
		// Pass the draw command so it is executed
		m_renderer.passScreenSpaceDrawCommandsToBackend();
		
		p_renderPassData.swapColorInputOutputMaps();

		p_renderPassData.m_atmScatDoSkyPass = !p_renderPassData.m_atmScatDoSkyPass;
	}

private:
	enum Luminance {
		// Render the spectral radiance at kLambdaR, kLambdaG, kLambdaB.
		NONE,
		// Render the sRGB luminance, using an approximate (on the fly) conversion
		// from 3 spectral radiance values only (see section 14.3 in <a href=
		// "https://arxiv.org/pdf/1612.04336.pdf">A Qualitative and Quantitative
		//  Evaluation of 8 Clear Sky Models</a>).
		APPROXIMATE,
		// Render the sRGB luminance, precomputed from 15 spectral radiance values
		// (see section 4.4 in <a href=
		// "http://www.oskee.wz.cz/stranka/uploads/SCCG10ElekKmoch.pdf">Real-time
		//  Spectral Scattering in Large-scale Natural Participating Media</a>).
		PRECOMPUTED
	};

	void InitModel();
	
	Luminance m_luminanceType;
	bool m_useConstantSolarSpectrum;
	bool m_useCombinedTextures;
	bool m_useHalfPrecision;
	bool m_useOzone;
	bool m_useWhiteBalance;

	std::unique_ptr<AtmScatteringModel> m_atmScatteringModel;

	double kSunAngularRadius;
	double kSunSolidAngle;
	double kLengthUnitInMeters;

	std::string m_vertexShaderSource;

	//glm::vec3 m_whitePoint;

	ShaderLoader::ShaderProgram	*m_skyShader;
	ShaderLoader::ShaderProgram	*m_groundShader;

	RendererFrontend::ShaderBuffer m_atmParamBuffer;

	AtmosphereParameters m_atmosphereParam;
	AtmScatteringParameters m_atmScatteringParam;
};