#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <random>

#include "RenderPassBase.h"

class AmbientOcclusionPass : public RenderPass
{
public:
	AmbientOcclusionPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_AmbientOcclusion),
		m_hbaoPassShader(nullptr),
		m_ssaoPassShader(nullptr),
		m_hbaoBlurHorizontalShader(nullptr),
		m_hbaoBlurVerticalShader(nullptr),
		m_ssaoBlurShader(nullptr),
		m_ssaoSampleBuffer(BufferType_Uniform, BufferBindTarget_Uniform, BufferUsageHint_DynamicDraw),
		m_aoDataSetShaderBuffer(BufferType_Uniform, BufferBindTarget_Uniform, BufferUsageHint_DynamicDraw),
		m_ssaoNoiseTexture(Loaders::texture2D().create("SSAONoiseTexture", 4, 4, TextureFormat_RGB, TextureDataFormat_RGB32F, TextureDataType_Float)),
		m_hbaoNoiseTexture(Loaders::texture2D().create("HBAONoiseTexture", 4, 4, TextureFormat_RGBA, TextureDataFormat_RGBA16F, TextureDataType_Float))
	{

	}

	~AmbientOcclusionPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Ambient Occlusion Rendering Pass";

		// Create the HBAO pass shader
		{
			// Create a property-set used to load the shader
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().hbao_pass_frag_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().hbao_pass_vert_shader);

			// Create the shader
			m_hbaoPassShader = Loaders::shader().load(shaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_hbaoPassShader->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_hbaoPassShader);
			}
			else
				returnError = shaderError;
		}

		// Create the SSAO pass shader
		{
			// Create a property-set used to load the shader
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().ssao_pass_frag_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().ssao_pass_vert_shader);

			// Create the shader
			m_ssaoPassShader = Loaders::shader().load(shaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_ssaoPassShader->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_ssaoPassShader);
			}
			else
				returnError = shaderError;
		}

		// Create the HBAO horizontal blur shader
		{
			// Create a property-set used to load the shader
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().hbao_blur_horizontal_frag_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().hbao_blur_horizontal_vert_shader);

			// Create the shader
			m_hbaoBlurHorizontalShader = Loaders::shader().load(shaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_hbaoBlurHorizontalShader->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_hbaoBlurHorizontalShader);
			}
			else
				returnError = shaderError;
		}

		// Create the HBAO vertical blur shader
		{
			// Create a property-set used to load the shader
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().hbao_blur_vertical_frag_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().hbao_blur_vertical_vert_shader);

			// Create the shader
			m_hbaoBlurVerticalShader = Loaders::shader().load(shaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_hbaoBlurVerticalShader->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_hbaoBlurVerticalShader);
			}
			else
				returnError = shaderError;
		}

		// Create the SSAO blur shader
		{
			// Create a property-set used to load the shader
			PropertySet shaderProperties(Properties::Shaders);
			shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().ssao_blur_frag_shader);
			shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().ssao_blur_vert_shader);

			// Create the shader
			m_ssaoBlurShader = Loaders::shader().load(shaderProperties);

			// Load the shader to memory
			if(ErrorCode shaderError = m_ssaoBlurShader->loadToMemory(); shaderError == ErrorCode::Success)
			{
				// Queue the shader to be loaded to GPU
				m_renderer.queueForLoading(*m_ssaoBlurShader);
			}
			else
				returnError = shaderError;
		}

		// Set AO data
		updateAODataSet();

		// Set data for AO buffer
		m_aoDataSetShaderBuffer.m_bindingIndex = UniformBufferBinding::UniformBufferBinding_AODataSet;
		m_aoDataSetShaderBuffer.m_size = sizeof(AODataSet);
		m_aoDataSetShaderBuffer.m_data = (void *)&m_aoDataSet;

		// Queue the HBAO buffer for loading
		m_renderer.queueForLoading(m_aoDataSetShaderBuffer);

		// Generate and load the SSAO sample buffer
		generateSSAOsampleBuffer(m_ssaoSampleBuffer);
		m_renderer.queueForLoading(m_ssaoSampleBuffer);

		// Generate and load the SSAO noise texture
		generateSSAOnoiseData(m_ssaoNoiseTexture);
		m_renderer.queueForLoading(m_ssaoNoiseTexture);

		// Generate and load the HBAO noise texture
		generateHBAOnoiseData(m_hbaoNoiseTexture);
		m_renderer.queueForLoading(m_hbaoNoiseTexture);

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_SSAOPass);
			setInitialized(true);
		}
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_SSAOPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);

		// If the ambient occlusion data has changed, update it
		if(m_aoData != m_renderer.getFrameData().m_aoData)
		{
			m_aoData = m_renderer.getFrameData().m_aoData;
			updateAODataSet();
			m_aoDataSetShaderBuffer.m_updateSize = sizeof(AODataSet);
			m_renderer.queueForUpdate(m_aoDataSetShaderBuffer);
			m_renderer.passUpdateCommandsToBackend();
		}

		switch(m_aoData.m_aoType)
		{
			case AmbientOcclusionType_None:
				return;
				break;
			case AmbientOcclusionType_SSAO:
				{
					//	 ____________________________
					//	|							 |
					//	|		   AO PASS           |
					//	|____________________________|
					//

					// Bind the noise texture
					m_renderer.m_backend.bindTextureForReadering(MaterialType::MaterialType_Noise, m_ssaoNoiseTexture);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferPosition, GBufferTextureType::GBufferPosition);
					m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferNormal, GBufferTextureType::GBufferNormal);
					m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferMatProperties, GBufferTextureType::GBufferMatProperties);

					m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferFinal);

					// Queue and render a full screen quad using a final pass shader
					m_renderer.queueForDrawing(m_ssaoPassShader->getShaderHandle(), m_ssaoPassShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
					m_renderer.passScreenSpaceDrawCommandsToBackend();

					//	 ____________________________
					//	|							 |
					//	|		  BLUR PASS          |
					//	|____________________________|
					//

					m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferFinal, GBufferTextureType::GBufferFinal);
					m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferMatProperties);

					// Queue and render a full screen quad using a final pass shader
					m_renderer.queueForDrawing(m_ssaoBlurShader->getShaderHandle(), m_ssaoBlurShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
					m_renderer.passScreenSpaceDrawCommandsToBackend();
				}
				break;
			case AmbientOcclusionType_HBAO:
				{
					//	 ____________________________
					//	|							 |
					//	|		   AO PASS           |
					//	|____________________________|
					//

					// Bind the noise texture
					m_renderer.m_backend.bindTextureForReadering(MaterialType::MaterialType_Noise, m_hbaoNoiseTexture);

					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferPosition, GBufferTextureType::GBufferPosition);
					m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferNormal, GBufferTextureType::GBufferNormal);

					m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferFinal);

					// Queue and render a full screen quad using a final pass shader
					m_renderer.queueForDrawing(m_hbaoPassShader->getShaderHandle(), m_hbaoPassShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
					m_renderer.passScreenSpaceDrawCommandsToBackend();

					//	 ____________________________
					//	|							 |
					//	|		  BLUR PASS          |
					//	|____________________________|
					//
					// Horizontal blur
					{
						m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferFinal, GBufferTextureType::GBufferInputTexture);
						m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferIntermediate);

						// Queue and render a full screen quad using a final pass shader
						m_renderer.queueForDrawing(m_hbaoBlurHorizontalShader->getShaderHandle(), m_hbaoBlurHorizontalShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
						m_renderer.passScreenSpaceDrawCommandsToBackend();
					}

					// Vertical blur
					{
						m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferIntermediate, GBufferTextureType::GBufferInputTexture);
						m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferMatProperties, GBufferTextureType::GBufferMatProperties);
						m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferMatProperties);

						// Queue and render a full screen quad using a final pass shader
						m_renderer.queueForDrawing(m_hbaoBlurVerticalShader->getShaderHandle(), m_hbaoBlurVerticalShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
						m_renderer.passScreenSpaceDrawCommandsToBackend();
					}
				}
				break;
		}
	}

private:
	float ssaoLerp(float p_a, float p_b, float p_f)
	{
		return p_a + p_f * (p_b - p_a);
	}

	void generateHBAOnoiseData(TextureLoader2D::Texture2DHandle &p_noiseTexture)
	{
		// Create a Mersenne Twister pseudo-random generator
		std::mt19937 rmt;

		unsigned int numberOfRandomElements = m_aoData.m_aoNumOfSteps * m_aoData.m_aoNumOfSteps;

		// Reserve enough space for all the pixels in advance
		m_hbaoNoise.reserve(numberOfRandomElements); //HBAO_RANDOM_ELEMENTS

		// Go over each pixel
		for(unsigned int i = 0; i < numberOfRandomElements; i++) //HBAO_RANDOM_ELEMENTS
		{
			// Get random numbers
			float randomNumber1 = static_cast<float>(rmt()) / 4294967296.0f;
			float randomNumber2 = static_cast<float>(rmt()) / 4294967296.0f;

			// Use random rotation angles in ( 0 <-> 2PI / NUM_DIRECTIONS )
			float randomAngle = glm::two_pi<float>() * randomNumber1 / m_aoData.m_aoNumOfDirections;

			// Set the pixel values
			m_hbaoNoise.push_back(glm::vec4(cosf(randomAngle), sinf(randomAngle), randomNumber2, 0.0f));
		}

		// Set the pixel data of the texture
		p_noiseTexture.setPixelData(&m_hbaoNoise[0]);

		// Set texture parameters
		p_noiseTexture.setMagnificationFilterType(TextureFilterType::TextureFilterType_Nearest);
		p_noiseTexture.setMinificationFilterType(TextureFilterType::TextureFilterType_Nearest);
	}
	void generateSSAOnoiseData(TextureLoader2D::Texture2DHandle &p_noiseTexture)
	{
		// Create the number distribution of floats between 0.0 and 1.0
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;

		unsigned int numberOfRandomElements = m_aoData.m_aoNumOfSteps * m_aoData.m_aoNumOfSteps;

		// Go over each pixel
		for(unsigned int i = 0; i < numberOfRandomElements; i++)
		{
			// Rotate around z-axis (in tangent space)
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);

			// Set the pixel values
			m_ssaoNoise.push_back(noise);
		}

		// Set the data for the noise texture
		p_noiseTexture.setPixelData((void *)&m_ssaoNoise[0]);

		// Set texture parameters
		p_noiseTexture.setMagnificationFilterType(TextureFilterType::TextureFilterType_Nearest);
		p_noiseTexture.setMinificationFilterType(TextureFilterType::TextureFilterType_Nearest);
	}
	void generateSSAOsampleBuffer(RendererFrontend::ShaderBuffer &p_sampleBuffer)
	{
		// Create the number distribution of floats between 0.0 and 1.0
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;

		// Go over each sample
		for(int i = 0; i < m_aoData.m_aoNumOfSamples; ++i)
		{
			// Generate sample
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			// Scale samples so they're more aligned to the center of the kernel
			float scale = float(i) / (float)m_aoData.m_aoNumOfSamples;
			scale = ssaoLerp(0.1f, 1.0f, scale * scale);
			sample *= scale;

			// Set the sample data
			m_ssaoKernel.push_back(glm::vec4(sample, 0.0f));
		}

		// Set the data for the sample buffer
		p_sampleBuffer.m_bindingIndex = UniformBufferBinding::UniformBufferBinding_SSAOSampleBuffer;
		p_sampleBuffer.m_size = sizeof(glm::vec4) * m_aoData.m_aoNumOfSamples;
		p_sampleBuffer.m_data = &m_ssaoKernel[0];
	}

	void updateAODataSet()
	{
		float projScale = float(m_renderer.getFrameData().m_screenSize.y) / (tanf(Config::graphicsVar().fov * 0.5f) * 2.0f);

		m_aoDataSet.m_RadiusToScreen = m_aoData.m_aoRadius * 0.5f * projScale;
		m_aoDataSet.m_radius = m_aoData.m_aoRadius;
		m_aoDataSet.m_NegInvR2 = -1.0f / (m_aoDataSet.m_radius * m_aoDataSet.m_radius);
		m_aoDataSet.m_NDotVBias = std::min(std::max(0.0f, m_aoData.m_aoBias), 1.0f);

		// Resolution
		m_aoDataSet.m_InvFullResolution = glm::vec2(1.0f / float(m_renderer.getFrameData().m_screenSize.x), 1.0f / float(m_renderer.getFrameData().m_screenSize.y));

		m_aoDataSet.m_AOMultiplier = 1.0f / (1.0f - m_aoDataSet.m_NDotVBias);
		m_aoDataSet.m_PowExponent = std::max(m_renderer.getFrameData().m_aoData.m_aoIntensity, 0.0f);

		m_aoDataSet.m_bias = m_aoData.m_aoBias;
		m_aoDataSet.m_numOfDirections = m_aoData.m_aoNumOfDirections;
		m_aoDataSet.m_numOfSamples = m_aoData.m_aoNumOfSamples;
		m_aoDataSet.m_numOfSteps = m_aoData.m_aoNumOfSteps;
	}

	AODataSet m_aoDataSet;
	AmbientOcclusionData m_aoData;

	// AO pass shaders
	ShaderLoader::ShaderProgram *m_hbaoPassShader;
	ShaderLoader::ShaderProgram *m_ssaoPassShader;

	// Blur shaders
	ShaderLoader::ShaderProgram *m_hbaoBlurHorizontalShader;
	ShaderLoader::ShaderProgram *m_hbaoBlurVerticalShader;
	ShaderLoader::ShaderProgram *m_ssaoBlurShader;

	// AO buffers
	RendererFrontend::ShaderBuffer m_ssaoSampleBuffer;
	RendererFrontend::ShaderBuffer m_aoDataSetShaderBuffer;

	// Noise textures
	TextureLoader2D::Texture2DHandle m_ssaoNoiseTexture;
	TextureLoader2D::Texture2DHandle m_hbaoNoiseTexture;

	// AO generated data
	std::vector<glm::vec3> m_ssaoNoise;
	std::vector<glm::vec4> m_ssaoKernel;
	std::vector<glm::vec4> m_hbaoNoise;
};