#pragma once

#include "GraphicsDataSets.h"
#include "RenderPassBase.h"

class HdrMappingPass : public RenderPass
{
public:
	HdrMappingPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer),
		m_HDRSSBuffer(BufferType_ShaderStorage, BufferUsageHint_DynamicCopy) { }

	~HdrMappingPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;
		ErrorCode shaderError;

		m_name = "HDR Mapping Pass";

		// Set HDR buffer binding index
		m_HDRSSBuffer.m_bindingIndex = SSBOBinding_HDR;

		// Assign HDR buffer data
		m_HDRSSBuffer.m_data = &m_HDRDataSet;

		// Set the HDR buffer size
		m_HDRSSBuffer.m_size = sizeof(HDRDataSet);

		// Set buffer values
		m_emissiveAndOutputBuffers.resize(2);
		m_emissiveAndOutputBuffers[0] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferDiffuse);
		m_emissiveAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(GeometryBuffer::GBufferFinal);

		// Create a property-set used to load blur vertical shaders
		PropertySet hdrMappingShaderProperties(Properties::Shaders);
		hdrMappingShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().hdr_mapping_pass_vert_shader);
		hdrMappingShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().hdr_mapping_pass_frag_shader);

		// Create shaders
		m_hdrMappingShader = Loaders::shader().load(hdrMappingShaderProperties);

		//		 _______________________________
		//		| LOAD HDR MAPPING PASS SHADER	|
		//		|_______________________________|
		shaderError = m_hdrMappingShader->loadToMemory();		// Load shader to memory
		if(shaderError == ErrorCode::Success)					// Check if shader was loaded successfully
			m_renderer.queueForLoading(*m_hdrMappingShader);	// Queue the shader to be loaded to GPU
		else
			returnError = shaderError;

		// Queue HDR buffer to be created
		m_renderer.queueForLoading(m_HDRSSBuffer);
		
		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_HdrMappingPass);
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_HdrMappingPass);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDisable(GL_DEPTH_TEST);

		if(Config::graphicsVar().eye_adaption)
		{
			// Generate mipmaps for the final buffer, for use in tone mapping
			m_renderer.m_backend.getGeometryBuffer()->generateMipmap(GeometryBuffer::GBufferFinal);
		}

		p_renderPassData.setEmissiveInputMap(GeometryBuffer::GBufferDiffuse);

		m_emissiveAndOutputBuffers[0] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(p_renderPassData.getEmissiveInputMap());
		m_emissiveAndOutputBuffers[1] = m_renderer.m_backend.getGeometryBuffer()->getBufferLocation(p_renderPassData.getColorOutputMap());
		
		// Set the default framebuffer to be drawn to
		//m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferDefault);
		
		// Bind final texture for reading so it can be accessed in the shaders
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferFinal, GeometryBuffer::GBufferFinal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(GeometryBuffer::GBufferEmissive, GeometryBuffer::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForReading(p_renderPassData.getColorInputMap(), GeometryBuffer::GBufferInputTexture);

		// Bind intermediate texture for writing to, so HDR mapping is outputed to it
		//m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferEmissive);
		//m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferIntermediate);

		// Bind textures for writing
		m_renderer.m_backend.getGeometryBuffer()->bindBuffersForWriting(m_emissiveAndOutputBuffers);

		// Perform HDR mapping. Queue and render a full screen quad using an HDR pass shader
		m_renderer.queueForDrawing(m_hdrMappingShader->getShaderHandle(), m_hdrMappingShader->getUniformUpdater(), p_sceneObjects.m_camera->getBaseObjectData().m_modelMat);
		m_renderer.passScreenSpaceDrawCommandsToBackend();
		
		p_renderPassData.setBlurInputMap(p_renderPassData.getEmissiveInputMap());
		p_renderPassData.setBlurOutputMap(GeometryBuffer::GBufferEmissive);
		p_renderPassData.setIntermediateMap(p_renderPassData.getColorInputMap());
		p_renderPassData.m_blurDoBlending = false;
		p_renderPassData.m_numOfBlurPasses = Config::graphicsVar().bloom_blur_passes;

		p_renderPassData.swapColorInputOutputMaps();
	}

private:
	// Buffer handles used for binding
	std::vector<GeometryBuffer::GBufferTexture> m_emissiveAndOutputBuffers;

	// HDR shader
	ShaderLoader::ShaderProgram	*m_hdrMappingShader;

	// HDR shader storage buffer
	RendererFrontend::ShaderBuffer m_HDRSSBuffer;

	// HDR shader storage buffer data set
	HDRDataSet m_HDRDataSet;
};