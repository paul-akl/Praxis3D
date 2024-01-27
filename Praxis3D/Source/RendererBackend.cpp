#include "RendererBackend.h"

RendererBackend::SingleTriangle RendererBackend::m_fullscreenTriangle;

RendererBackend::RendererBackend()
{
	m_gbuffer = nullptr;
	m_csmBuffer = nullptr;
}

RendererBackend::~RendererBackend()
{
	if(m_gbuffer != nullptr)
		delete m_gbuffer;
	if(m_csmBuffer != nullptr)
		delete m_csmBuffer;
}

ErrorCode RendererBackend::init(const UniformFrameData &p_frameData)
{
	ErrorCode returnCode = ErrorCode::Success;

	// Clear the default framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Initialize gbuffer and check if the gbuffer initialization was successful
	if(ErrHandlerLoc::get().ifSuccessful(createFramebuffer(FramebufferType::FramebufferType_GBuffer, p_frameData), returnCode))
	{
		// Load fullscreen triangle (used to render post-processing effects)
		m_fullscreenTriangle.load();

		// Enable / disable face culling
		if(Config::rendererVar().face_culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		// Enable / disable depth test
		if(Config::rendererVar().depth_test)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		// Set depth test function
		glDepthFunc(Config::rendererVar().depth_test_func);
	}
	return returnCode;
}

ErrorCode RendererBackend::createFramebuffer(const FramebufferType p_frambufferType, const UniformFrameData &p_frameData)
{
	ErrorCode returnError = ErrorCode::Success;

	switch(p_frambufferType)
	{
		case FramebufferType_GBuffer:
			{
				// Make sure there is only one gbuffer
				if(m_gbuffer != nullptr)
					delete m_gbuffer;

				// Initialize the gbuffer
				m_gbuffer = new GeometryBuffer(p_frameData);

				// Check if the gbuffer initialization was successful
				returnError = m_gbuffer->init(p_frameData);
			}
			break;

		case FramebufferType_CSMBuffer:
			{
				// Make sure there is only one csm buffer
				if(m_csmBuffer != nullptr)
					delete m_csmBuffer;

				// Initialize the csm buffer
				m_csmBuffer = new CSMFramebuffer(p_frameData);

				// Check if the csm buffer initialization was successful
				returnError = m_csmBuffer->init(p_frameData);
			}
			break;
	}

	return returnError;
}

void RendererBackend::processUpdate(const BufferUpdateCommands &p_updateCommands, const UniformFrameData &p_frameData)
{
	for(decltype(p_updateCommands.size()) i = 0, size = p_updateCommands.size(); i < size; i++)
	{
		processCommand(p_updateCommands[i], p_frameData);
	}
}

void RendererBackend::processLoading(LoadCommands &p_loadCommands, const UniformFrameData &p_frameData)
{
	for(decltype(p_loadCommands.size()) i = 0, size = p_loadCommands.size(); i < size; i++)
	{
		processCommand(p_loadCommands[i], p_frameData);
	}
}

void RendererBackend::processUnloading(UnloadCommands &p_unloadCommands)
{
	// Declare an array for each type of unloadable object
	std::vector<unsigned int> unloadArrays[UnloadObjectType::UnloadObjectType_NumOfTypes];

	// Go over each unload command and put them inside the corresponding array
	for(decltype(p_unloadCommands.size()) i = 0, size = p_unloadCommands.size(); i < size; i++)
		unloadArrays[p_unloadCommands[i].first].push_back(p_unloadCommands[i].second);

	// Go over each type of unloadable object arrays, and if it contains any elements, pass the whole array to be unloaded in a batch
	for(unsigned int i = 0; i < UnloadObjectType::UnloadObjectType_NumOfTypes; i++)
		if(!unloadArrays[i].empty())
			processCommand(static_cast<UnloadObjectType>(i), (int)unloadArrays[i].size(), unloadArrays[i].data());
}

void RendererBackend::processDrawing(const DrawCommands &p_drawCommands, const UniformFrameData &p_frameData)
{
	resetVAO();

	for(decltype(p_drawCommands.size()) i = 0, size = p_drawCommands.size(); i < size; i++)
	{
		// Get uniform data
		const UniformObjectData &uniformObjectData = p_drawCommands[i].second.m_uniformObjectData;

		// Get various handles
		const auto shaderHandle = p_drawCommands[i].second.m_shaderHandle;
		const auto &uniformUpdater = p_drawCommands[i].second.m_uniformUpdater;

		// Bind the shader
		bindShader(shaderHandle);

		// Update shader uniforms
		textureUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		frameUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		modelUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		meshUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);

		// Bind VAO
		bindVAO(p_drawCommands[i].second.m_modelHandle);

		// Bind textures
		glActiveTexture(GL_TEXTURE0 + MaterialType_Diffuse);
		glBindTexture(GL_TEXTURE_2D, p_drawCommands[i].second.m_matDiffuse);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p_drawCommands[i].second.m_matWrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p_drawCommands[i].second.m_matWrapMode);

		glActiveTexture(GL_TEXTURE0 + MaterialType_Normal);
		glBindTexture(GL_TEXTURE_2D, p_drawCommands[i].second.m_matNormal);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p_drawCommands[i].second.m_matWrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p_drawCommands[i].second.m_matWrapMode);

		glActiveTexture(GL_TEXTURE0 + MaterialType_Combined);
		glBindTexture(GL_TEXTURE_2D, p_drawCommands[i].second.m_matCombined);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p_drawCommands[i].second.m_matWrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p_drawCommands[i].second.m_matWrapMode);

		glActiveTexture(GL_TEXTURE0 + MaterialType_Emissive);
		glBindTexture(GL_TEXTURE_2D, p_drawCommands[i].second.m_matEmissive);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p_drawCommands[i].second.m_matWrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p_drawCommands[i].second.m_matWrapMode);
		
		// Draw the geometry
		glDrawElementsBaseVertex(GL_TRIANGLES,
								 p_drawCommands[i].second.m_numIndices,
								 GL_UNSIGNED_INT,
								 (void*)(sizeof(unsigned int) * p_drawCommands[i].second.m_baseIndex),
								 p_drawCommands[i].second.m_baseVertex);
	}
}

void RendererBackend::processDrawing(const ScreenSpaceDrawCommands &p_screenSpaceDrawCommands, const UniformFrameData &p_frameData)
{
	for(decltype(p_screenSpaceDrawCommands.size()) i = 0, size = p_screenSpaceDrawCommands.size(); i < size; i++)
	{
		// Get uniform data
		const UniformObjectData &uniformObjectData = p_screenSpaceDrawCommands[i].second.m_uniformObjectData;

		// Get various handles
		const auto shaderHandle = p_screenSpaceDrawCommands[i].second.m_shaderHandle;
		const auto &uniformUpdater = p_screenSpaceDrawCommands[i].second.m_uniformUpdater;

		// Bind the shader
		bindShader(shaderHandle);

		// Update shader uniforms
		textureUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		frameUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		modelUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		meshUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);

		// Bind VAO
		m_fullscreenTriangle.bind();

		// Draw the full-screen triangle
		m_fullscreenTriangle.render();
	}
}

void RendererBackend::processDrawing(const ComputeDispatchCommands &p_computeDispatchCommands, const UniformFrameData &p_frameData)
{
	for(decltype(p_computeDispatchCommands.size()) i = 0, size = p_computeDispatchCommands.size(); i < size; i++)
	{
		// Get uniform data
		const UniformObjectData &uniformObjectData = p_computeDispatchCommands[i].m_uniformObjectData;

		// Get various handles
		const auto shaderHandle = p_computeDispatchCommands[i].m_shaderHandle;
		const auto *uniformUpdater = &p_computeDispatchCommands[i].m_uniformUpdater;

		// Bind the shader
		bindShader(shaderHandle);

		// Update shader uniforms
		textureUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		frameUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		modelUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);
		meshUniformUpdate(shaderHandle, uniformUpdater, uniformObjectData, p_frameData);

		// Launch compute work groups
		glDispatchCompute(p_computeDispatchCommands[i].m_numOfGroups[0], p_computeDispatchCommands[i].m_numOfGroups[1], p_computeDispatchCommands[i].m_numOfGroups[2]);

		// Determine the memory barrier type
		GLbitfield memoryBarrierBit = 0;
		switch(p_computeDispatchCommands[i].m_memoryBarrier)
		{
			case MemoryBarrierType::MemoryBarrierType_All:
				memoryBarrierBit = GL_ALL_BARRIER_BITS;
				break;

			case MemoryBarrierType::MemoryBarrierType_ImageAccessBarrier:
				memoryBarrierBit = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
				break;

			case MemoryBarrierType::MemoryBarrierType_AccessAndFetchBarrier:
				memoryBarrierBit = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT;
				break;

			case MemoryBarrierType::MemoryBarrierType_ShaderStorageBarrier:
				memoryBarrierBit = GL_SHADER_STORAGE_BARRIER_BIT;
				break;
		}

		// Define the memory barrier for the data in the compute shader
		glMemoryBarrier(memoryBarrierBit);
	}
}

