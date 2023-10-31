#pragma once

#include "RenderPassBase.h"

class LuminancePass : public RenderPass
{
public:
	LuminancePass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer),
		m_histogramBuffer(BufferType_ElementArray, BufferBindTarget_ShaderStorage, BufferUsageHint_DynamicDraw),
		m_luminanceAverageTexture(Loaders::texture2D().create("LuminanceTexture", 1, 1, TextureFormat_Red, TextureDataFormat_R16F, TextureDataType_Float)),
		m_luminanceHistogramShader(nullptr),
		m_luminanceAverageShader(nullptr),
		m_tonemappingShader(nullptr) { }

	~LuminancePass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Luminance Pass";

		// Create a property-set used to load luminance histogram compute shader
		PropertySet luminanceHistogramProperty(Properties::Shaders);
		luminanceHistogramProperty.addProperty(Properties::ComputeShader, Config::rendererVar().luminance_histogram_comp_shader);

		// Create a property-set used to load luminance average compute shader
		PropertySet luminanceAverageProperty(Properties::Shaders);
		luminanceAverageProperty.addProperty(Properties::ComputeShader, Config::rendererVar().luminance_average_comp_shader);

		// Create a property-set used to load tonemapping shader
		PropertySet tonemappingProperty(Properties::Shaders);
		tonemappingProperty.addProperty(Properties::VertexShader, Config::rendererVar().tonemapping_vert_shader);
		tonemappingProperty.addProperty(Properties::FragmentShader, Config::rendererVar().tonemapping_frag_shader);

		// Create shaders
		m_luminanceHistogramShader = Loaders::shader().load(luminanceHistogramProperty);
		m_luminanceAverageShader = Loaders::shader().load(luminanceAverageProperty);
		m_tonemappingShader = Loaders::shader().load(tonemappingProperty);

		// Load shaders to memory
		ErrorCode luminanceHistogramError = m_luminanceHistogramShader->loadToMemory();
		ErrorCode luminanceAverageError = m_luminanceAverageShader->loadToMemory();
		ErrorCode tonemappingError = m_tonemappingShader->loadToMemory();

		// Queue the shaders to be loaded to GPU
		if(luminanceHistogramError == ErrorCode::Success)
			m_renderer.queueForLoading(*m_luminanceHistogramShader);
		else
			returnError = luminanceHistogramError;

		if(luminanceAverageError == ErrorCode::Success)
			m_renderer.queueForLoading(*m_luminanceAverageShader);
		else
			returnError = luminanceAverageError;

		if(tonemappingError == ErrorCode::Success)
			m_renderer.queueForLoading(*m_tonemappingShader);
		else
			returnError = tonemappingError;

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			// Set histogram buffer values
			m_histogramBuffer.m_bindingIndex = 1;
			m_histogramBuffer.m_size = sizeof(uint32_t) * 256;

			// Queue histogram buffer and luminance texture to be created in video memory
			m_renderer.queueForLoading(m_histogramBuffer);
			m_renderer.queueForLoading(m_luminanceAverageTexture);

			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_LuminancePass);
		}
		else
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_LuminancePass);
		}

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		// Get the screen image buffer size
		const unsigned int imageWidth = m_renderer.m_backend.getGeometryBuffer()->getBufferWidth();
		const unsigned int imageHeight = m_renderer.m_backend.getGeometryBuffer()->getBufferHeight();

		// Calculate the number of groups that the compute shader should execute
		unsigned int groupsX = static_cast<uint32_t>(glm::ceil(imageWidth / 16.0f));
		unsigned int groupsY = static_cast<uint32_t>(glm::ceil(imageHeight / 16.0f));

		m_renderer.m_backend.getGeometryBuffer()->bindBufferToImageUnitForReading(GBufferTextureType::GBufferFinal, GBufferTextureType::GBufferInputTexture, 0);

		m_renderer.queueForDrawing(m_luminanceHistogramShader->getShaderHandle(), m_luminanceHistogramShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix, groupsX, groupsY, 1, MemoryBarrierType::MemoryBarrierType_ShaderStorageBarrier);
		m_renderer.passComputeDispatchCommandsToBackend();

		glBindImageTexture(0, m_luminanceAverageTexture.getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F);

		m_renderer.queueForDrawing(m_luminanceAverageShader->getShaderHandle(), m_luminanceAverageShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix, 1, 1, 1, MemoryBarrierType::MemoryBarrierType_ShaderStorageBarrier);
		m_renderer.passComputeDispatchCommandsToBackend();

		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0 + LuminanceTextureType::LensFlareTextureType_AverageLuminance);
		glBindTexture(GL_TEXTURE_2D, m_luminanceAverageTexture.getHandle());

		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);

		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

		// Queue and render a full screen quad using a final pass shader
		m_renderer.queueForDrawing(m_tonemappingShader->getShaderHandle(), m_tonemappingShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
		m_renderer.passScreenSpaceDrawCommandsToBackend();

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	ShaderLoader::ShaderProgram *m_luminanceHistogramShader;
	ShaderLoader::ShaderProgram *m_luminanceAverageShader;
	ShaderLoader::ShaderProgram *m_tonemappingShader;

	RendererFrontend::ShaderBuffer m_histogramBuffer;

	TextureLoader2D::Texture2DHandle m_luminanceAverageTexture;
};