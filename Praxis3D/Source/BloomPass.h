#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class BloomPass : public RenderPass
{
public:
	BloomPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer),
		m_lensDirtTexture(Loaders::texture2D().load(Config::rendererVar().lens_flare_dirt_texture)),
		m_bloomDownscaleShader(nullptr),
		m_bloomUpscaleShader(nullptr) { }

	~BloomPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Failure;

		m_name = "Bloom Pass";
		
		// Create a property-set used to load bloom downscale compute shader
		PropertySet bloomDownscaleProperty(Properties::Shaders);
		bloomDownscaleProperty.addProperty(Properties::ComputeShader, Config::rendererVar().bloom_downscale_comp_shader);

		// Create a property-set used to load bloom upscale compute shader
		PropertySet bloomUpscaleProperty(Properties::Shaders);
		bloomUpscaleProperty.addProperty(Properties::ComputeShader, Config::rendererVar().bloom_upscale_comp_shader);

		// Create shaders
		m_bloomDownscaleShader = Loaders::shader().load(bloomDownscaleProperty);
		m_bloomUpscaleShader = Loaders::shader().load(bloomUpscaleProperty);

		// Load shaders to memory
		ErrorCode downscaleError = m_bloomDownscaleShader->loadToMemory();
		ErrorCode upscaleError = m_bloomUpscaleShader->loadToMemory();

		// Queue the shaders to be loaded to GPU
		if(downscaleError == ErrorCode::Success)
			m_renderer.queueForLoading(*m_bloomDownscaleShader);
		if(upscaleError == ErrorCode::Success)
			m_renderer.queueForLoading(*m_bloomUpscaleShader);

		// Check for errors and log either a successful or a failed initialization
		if(downscaleError == ErrorCode::Success && upscaleError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_BloomPass);
			returnError = ErrorCode::Success;
		}
		else
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_BloomPass);

			// Assign the error code of the failed shader
			if(downscaleError != ErrorCode::Success)
				returnError = downscaleError;
			else
				returnError = upscaleError;
		}

		m_bloomTreshold = glm::vec4(Config::graphicsVar().bloom_threshold, Config::graphicsVar().bloom_threshold - Config::graphicsVar().bloom_knee, 2.0f * Config::graphicsVar().bloom_knee, 0.25f * Config::graphicsVar().bloom_knee);

		// Load lens dirt texture to memory and to video memory
		m_lensDirtTexture.loadToMemory();
		m_renderer.queueForLoading(m_lensDirtTexture);

		return returnError;
	}

	// Calculates the maximum mipmap levels based on the image size and bloom mipmap, bloom downscale limits 
	unsigned int calculateMipmapLevels(unsigned int p_width, unsigned int p_height)
	{
		unsigned int width = p_width / 2;
		unsigned int height = p_height / 2;
		unsigned int mipLevels = 1;

		unsigned int mipmapLimit = (unsigned int)Config::graphicsVar().bloom_mipmap_limit;
		unsigned int downscaleLimit = (unsigned int)Config::graphicsVar().bloom_downscale_limit;

		for(unsigned int i = 0; i < mipmapLimit; i++)
		{
			width = width / 2;
			height = height / 2;

			if(width < downscaleLimit || height < downscaleLimit) break;

			mipLevels++;
		}

		return mipLevels + 1;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		// Assign the bloom threshold value so it can be sent to the shader
		m_renderer.m_frameData.m_bloomTreshold = m_bloomTreshold;

		// Get the screen image buffer size
		const unsigned int imageWidth = m_renderer.m_backend.getGeometryBuffer()->getBufferWidth();
		const unsigned int imageHeight = m_renderer.m_backend.getGeometryBuffer()->getBufferHeight();

		// Calculate mipmap size and level
		glm::uvec2 mipmapSize = glm::uvec2(imageWidth / 2, imageHeight / 2);
		unsigned int mipmapLevels = calculateMipmapLevels(imageWidth, imageHeight);

		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferFinal, GBufferTextureType::GBufferInputTexture);

		// Bloom downscaling
		for(unsigned int i = 0; i < mipmapLevels - 1; i++)
		{
			// Assign the texel size and mipmap level so it can be sent to the shader
			m_renderer.m_frameData.m_texelSize = 1.0f / glm::vec2(mipmapSize);
			m_renderer.m_frameData.m_mipLevel = i;

			// Bind the corresponding mipmap level of the image buffer
			m_renderer.m_backend.getGeometryBuffer()->bindBufferToImageUnitForWriting(GBufferTextureType::GBufferFinal, 0, i + 1);

			const unsigned int groupX = (unsigned int)glm::ceil(mipmapSize.x / 8.0);
			const unsigned int groupY = (unsigned int)glm::ceil(mipmapSize.y / 8.0);

			// Dispatch the compute shader
			m_renderer.queueForDrawing(m_bloomDownscaleShader->getShaderHandle(), m_bloomDownscaleShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix, groupX, groupY, 1, MemoryBarrierType::MemoryBarrierType_AccessAndFetchBarrier);
			m_renderer.passComputeDispatchCommandsToBackend();
			
			// Half the mipmap size as we go up the mipmap levels
			mipmapSize = mipmapSize / 2u;
		}

		// Bind lens dirt texture
		glActiveTexture(GL_TEXTURE0 + LensFlareTextureType::LensFlareTextureType_LenseDirt);
		glBindTexture(GL_TEXTURE_2D, m_lensDirtTexture.getHandle());

		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GBufferTextureType::GBufferFinal, GBufferTextureType::GBufferInputTexture);

		// Bloom upscaling
		for(unsigned int i = mipmapLevels - 1; i >= 1; i--)
		{
			// Recalculate the mipmap size as we go down the mipmap levels
			mipmapSize.x = (unsigned int)glm::max(1.0, glm::floor(imageWidth / glm::pow(2u, i - 1.0)));
			mipmapSize.y = (unsigned int)glm::max(1.0, glm::floor(imageHeight / glm::pow(2u, i - 1.0)));

			// Assign the texel size and mipmap level so it can be sent to the shader
			m_renderer.m_frameData.m_texelSize = 1.0f / glm::vec2(mipmapSize);
			m_renderer.m_frameData.m_mipLevel = i;

			// Bind the corresponding mipmap level of the image buffer
			m_renderer.m_backend.getGeometryBuffer()->bindBufferToImageUnitForWriting(GBufferTextureType::GBufferFinal, 0, i - 1);

			const unsigned int groupX = (unsigned int)glm::ceil(mipmapSize.x / 8.0);
			const unsigned int groupY = (unsigned int)glm::ceil(mipmapSize.y / 8.0);

			// Dispatch the compute shader
			m_renderer.queueForDrawing(m_bloomUpscaleShader->getShaderHandle(), m_bloomUpscaleShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix, groupX, groupY, 1, MemoryBarrierType::MemoryBarrierType_AccessAndFetchBarrier);
			m_renderer.passComputeDispatchCommandsToBackend();
		}
	}

private:
	ShaderLoader::ShaderProgram *m_bloomDownscaleShader;
	ShaderLoader::ShaderProgram *m_bloomUpscaleShader;

	TextureLoader2D::Texture2DHandle m_lensDirtTexture;

	glm::vec4 m_bloomTreshold;
};