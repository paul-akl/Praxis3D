#pragma once

#include "Shaders/AtmosphericScattering/Include/AtmScatteringModel.hpp"
#include "Systems/RendererSystem/Include/GraphicsDataSets.hpp"
#include "Systems/RendererSystem/Passes/Include/RenderPassBase.hpp"

class AtmScatteringPass : public RenderPass
{
public:
	AtmScatteringPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_AtmScattering),
		m_useConstantSolarSpectrum(false),
		m_useOzone(true),
		m_useCombinedTextures(true),
		m_useHalfPrecision(true),
		m_luminanceType(Luminance::NONE),
		m_useWhiteBalance(true),
		m_sunAngularRadius(p_renderer.getFrameData().m_atmScatteringData.m_sunSize / 2.0),
		m_lengthUnitInMeters(1000.0),
		m_atmParamBuffer(BufferType_Uniform, BufferBindTarget_Uniform, BufferUsageHint_DynamicDraw),
		m_skyShader(nullptr),
		m_groundShader(nullptr)
	{
		m_sunSolidAngle = Math::PI_DOUBLE * m_sunAngularRadius * m_sunAngularRadius;

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

		const auto &rayleighDensity0 = p_renderer.getFrameData().m_atmScatteringData.m_rayleighDensity[0];
		const auto &rayleighDensity1 = p_renderer.getFrameData().m_atmScatteringData.m_rayleighDensity[1];

		const auto &mieDensity0 = p_renderer.getFrameData().m_atmScatteringData.m_mieDensity[0];
		const auto &mieDensity1 = p_renderer.getFrameData().m_atmScatteringData.m_mieDensity[1];

		const auto &absorptionDensity0 = p_renderer.getFrameData().m_atmScatteringData.m_absorptionDensity[0];
		const auto &absorptionDensity1 = p_renderer.getFrameData().m_atmScatteringData.m_absorptionDensity[1];

		m_atmosphereParam = AtmosphereParameters(
			p_renderer.getFrameData().m_atmScatteringData.m_sunIrradiance,
			(float)m_sunAngularRadius,
			DensityProfile(DensityProfLayer(rayleighDensity0.m_width, rayleighDensity0.m_expTerm, rayleighDensity0.m_expScale, rayleighDensity0.m_linearTerm, rayleighDensity0.m_constantTerm), 
				DensityProfLayer(rayleighDensity1.m_width, rayleighDensity1.m_expTerm, rayleighDensity1.m_expScale, rayleighDensity1.m_linearTerm, rayleighDensity1.m_constantTerm)),
			DensityProfile(DensityProfLayer(mieDensity0.m_width, mieDensity0.m_expTerm, mieDensity0.m_expScale, mieDensity0.m_linearTerm, mieDensity0.m_constantTerm),
				DensityProfLayer(mieDensity1.m_width, mieDensity1.m_expTerm, mieDensity1.m_expScale, mieDensity1.m_linearTerm, mieDensity1.m_constantTerm)),
			DensityProfile(DensityProfLayer(absorptionDensity0.m_width, absorptionDensity0.m_expTerm, absorptionDensity0.m_expScale, absorptionDensity0.m_linearTerm, absorptionDensity0.m_constantTerm),
				DensityProfLayer(absorptionDensity1.m_width, absorptionDensity1.m_expTerm, absorptionDensity1.m_expScale, absorptionDensity1.m_linearTerm, absorptionDensity1.m_constantTerm)),
			p_renderer.getFrameData().m_atmScatteringData.m_rayleighScattering,
			p_renderer.getFrameData().m_atmScatteringData.m_atmosphereBottomRadius,
			p_renderer.getFrameData().m_atmScatteringData.m_mieScattering,
			p_renderer.getFrameData().m_atmScatteringData.m_atmosphereTopRadius,
			p_renderer.getFrameData().m_atmScatteringData.m_mieExtinction,
			0.800000f,
			p_renderer.getFrameData().m_atmScatteringData.m_absorptionExtinction,
			-0.207912f,
			p_renderer.getFrameData().m_atmScatteringData.m_planetGroundColor);

		m_atmScatteringParam = AtmScatteringParameters(
			m_atmosphereParam,
			glm::vec3(1.0f, 1.0f, 1.0f),
			p_renderer.getFrameData().m_atmScatteringData.m_planetCenterPosition / (float)m_lengthUnitInMeters,
			glm::vec2(tan((float)m_sunAngularRadius), cos((float)m_sunAngularRadius)));

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
		initModel();

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
		m_renderer.m_frameData.m_atmScatteringDataChanged = false;

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_AtmScatteringPass);
			setInitialized(true);
		}
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_AtmScatteringPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glEnable(GL_DEPTH_TEST);

		if(p_sceneObjects.m_processDrawing)
		{
			if(m_renderer.getFrameData().m_atmScatteringDataChanged)
			{
				m_renderer.m_frameData.m_atmScatteringDataChanged = false;

				updateAtmScatteringData(m_renderer.getFrameData().m_atmScatteringData);

				// Set atmosphere parameters buffer size and data
				m_atmParamBuffer.m_size = sizeof(AtmScatteringParameters);
				m_atmParamBuffer.m_updateSize = sizeof(AtmScatteringParameters);
				m_atmParamBuffer.m_data = (void *)(&m_atmScatteringParam);

				// Update atmosphere parameters buffer on the GPU
				m_renderer.queueForUpdate(m_atmParamBuffer);
				m_renderer.passUpdateCommandsToBackend();
			}

			if(p_renderPassData.m_atmScatDoSkyPass)
			{
				glDepthFunc(GL_LEQUAL);

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
				m_renderer.queueForDrawing(m_skyShader->getShaderHandle(), m_skyShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);

			}
			else
			{
				glDepthFunc(GL_GREATER);

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

				m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);
				m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferPosition, GBufferTextureType::GBufferPosition);
				m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferNormal, GBufferTextureType::GBufferNormal);

				// Set atmosphere parameters buffer size and data
				//m_atmParamBuffer.m_updateSize = sizeof(AtmScatteringParameters);
				//m_atmParamBuffer.m_data = (void*)(&m_atmScatteringParam);
				//m_renderer.queueForUpdate(m_atmParamBuffer);

				// Pass update commands so they are executed 
				//m_renderer.passUpdateCommandsToBackend();

				// Perform various visual effects in the post process shader
				m_renderer.queueForDrawing(m_groundShader->getShaderHandle(), m_groundShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);

			}

			// Pass the draw command so it is executed
			m_renderer.passScreenSpaceDrawCommandsToBackend();
		}
		
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

	void initModel();
	void updateAtmScatteringData(const AtmosphericScatteringData &p_atmScatteringData);
	
	Luminance m_luminanceType;
	bool m_useConstantSolarSpectrum;
	bool m_useCombinedTextures;
	bool m_useHalfPrecision;
	bool m_useOzone;
	bool m_useWhiteBalance;

	std::unique_ptr<AtmScatteringModel> m_atmScatteringModel;

	double m_sunAngularRadius;
	double m_sunSolidAngle;
	double m_lengthUnitInMeters;

	std::string m_vertexShaderSource;

	ShaderLoader::ShaderProgram	*m_skyShader;
	ShaderLoader::ShaderProgram	*m_groundShader;

	RendererFrontend::ShaderBuffer m_atmParamBuffer;

	AtmosphereParameters m_atmosphereParam;
	AtmScatteringParameters m_atmScatteringParam;
};