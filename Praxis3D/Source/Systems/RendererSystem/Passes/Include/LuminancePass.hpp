#pragma once

#include "Systems/RendererSystem/Passes/Include/RenderPassBase.hpp"

class LuminancePass : public RenderPass
{
public:
	LuminancePass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_Luminance),
		m_histogramBuffer(BufferType_ElementArray, BufferBindTarget_ShaderStorage, BufferUsageHint_DynamicDraw),
		m_luminanceAverageTexture(Loaders::texture2D().create("LuminanceAverageTexture", 1, 1, TextureFormat_Red, TextureDataFormat_R16F, TextureDataType_Float)),
		m_luminanceHistogramShader(nullptr),
		m_luminanceAverageShader(nullptr),
		m_exposureAdaptationShader(nullptr),
		m_passthroughShader(nullptr) { }

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

		// Create a property-set used to load exposure adaptation shader
		PropertySet exposureAdaptationProperty(Properties::Shaders);
		exposureAdaptationProperty.addProperty(Properties::Name, std::string("exposureAdaptation"));
		exposureAdaptationProperty.addProperty(Properties::FragmentShader, Config::rendererVar().exposure_adaptation_frag_shader);
		exposureAdaptationProperty.addProperty(Properties::VertexShader, Config::rendererVar().exposure_adaptation_vert_shader);

		// Create a property-set used to load pass-through shader
		PropertySet passthroughProperty(Properties::Shaders);
		passthroughProperty.addProperty(Properties::Name, std::string("luminancePassthrough"));
		passthroughProperty.addProperty(Properties::FragmentShader, std::string("passthrough.frag"));
		passthroughProperty.addProperty(Properties::VertexShader, std::string("passthrough.vert"));

		// Create shaders
		m_luminanceHistogramShader = Loaders::shader().load(luminanceHistogramProperty);
		m_luminanceAverageShader = Loaders::shader().load(luminanceAverageProperty);
		m_exposureAdaptationShader = Loaders::shader().load(exposureAdaptationProperty);
		m_passthroughShader = Loaders::shader().load(passthroughProperty);

		// Load shaders to memory
		ErrorCode luminanceHistogramError = m_luminanceHistogramShader->loadToMemory();
		ErrorCode luminanceAverageError = m_luminanceAverageShader->loadToMemory();
		ErrorCode tonemappingError = m_exposureAdaptationShader->loadToMemory();
		ErrorCode passthroughError = m_passthroughShader->loadToMemory();

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
			m_renderer.queueForLoading(*m_exposureAdaptationShader);
		else
			returnError = tonemappingError;

		if(passthroughError == ErrorCode::Success)
			m_renderer.queueForLoading(*m_passthroughShader);
		else
			returnError = passthroughError;

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			// Set histogram buffer values
			m_histogramBuffer.m_bindingIndex = 1;
			m_histogramBuffer.m_size = sizeof(uint32_t) * 256;

			// Queue histogram buffer and luminance texture to be created in video memory
			m_renderer.queueForLoading(m_histogramBuffer);
			m_renderer.queueForLoading(m_luminanceAverageTexture);

			setInitialized(true);

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
		if(p_sceneObjects.m_processDrawing)
		{
			// Get the screen image buffer size
			const unsigned int imageWidth = m_renderer.m_backend.getGeometryBuffer()->getBufferWidth();
			const unsigned int imageHeight = m_renderer.m_backend.getGeometryBuffer()->getBufferHeight();

			// Calculate the number of groups that the compute shader should execute
			unsigned int groupsX = static_cast<uint32_t>(glm::ceil(imageWidth / 16.0f));
			unsigned int groupsY = static_cast<uint32_t>(glm::ceil(imageHeight / 16.0f));

			/*
				Calculate luminance histogram
			*/
			m_renderer.m_backend.getGeometryBuffer()->bindBufferToImageUnitForReading(GBufferTextureType::GBufferFinal, GBufferTextureType::GBufferInputTexture, 0);

			m_renderer.queueForDrawing(m_luminanceHistogramShader->getShaderHandle(), m_luminanceHistogramShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix, groupsX, groupsY, 1, MemoryBarrierType::MemoryBarrierType_ShaderStorageBarrier);
			m_renderer.passComputeDispatchCommandsToBackend();

			/*
				Calculate average luminance
			*/
			glBindImageTexture(0, m_luminanceAverageTexture.getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F);

			m_renderer.queueForDrawing(m_luminanceAverageShader->getShaderHandle(), m_luminanceAverageShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix, 1, 1, 1, MemoryBarrierType::MemoryBarrierType_ShaderStorageBarrier);
			m_renderer.passComputeDispatchCommandsToBackend();

			/*
				Adjust exposure based on average luminance
			*/
			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0 + LuminanceTextureType::LensFlareTextureType_AverageLuminance);
			glBindTexture(GL_TEXTURE_2D, m_luminanceAverageTexture.getHandle());

			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GBufferTextureType::GBufferInputTexture);
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(p_renderPassData.getColorOutputMap());

			// Queue and render a full screen quad using a exposure adaptation pass shader
			m_renderer.queueForDrawing(m_exposureAdaptationShader->getShaderHandle(), m_exposureAdaptationShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
			m_renderer.passScreenSpaceDrawCommandsToBackend();

			/*
				Copy color from input color buffer to final buffer
			*/
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorOutputMap(), GBufferTextureType::GBufferInputTexture);
			m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GBufferTextureType::GBufferFinal);

			// Queue and render a full screen quad using a pass-through pass shader
			m_renderer.queueForDrawing(m_passthroughShader->getShaderHandle(), m_passthroughShader->getUniformUpdater(), p_sceneObjects.m_cameraViewMatrix);
			m_renderer.passScreenSpaceDrawCommandsToBackend();

			//p_renderPassData.swapColorInputOutputMaps();
		}

	}

private:
	ShaderLoader::ShaderProgram *m_luminanceHistogramShader;
	ShaderLoader::ShaderProgram *m_luminanceAverageShader;
	ShaderLoader::ShaderProgram *m_exposureAdaptationShader;
	ShaderLoader::ShaderProgram *m_passthroughShader;

	RendererFrontend::ShaderBuffer m_histogramBuffer;

	TextureLoader2D::Texture2DHandle m_luminanceAverageTexture;
};