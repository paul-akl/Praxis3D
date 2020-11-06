#include "RendererBackend.h"

RendererBackend::SingleTriangle RendererBackend::m_fullscreenTriangle;

RendererBackend::RendererBackend()
{
	m_gbuffer = nullptr;
}

RendererBackend::~RendererBackend()
{
	delete m_gbuffer;
}

ErrorCode RendererBackend::init(const UniformFrameData &p_frameData)
{
	ErrorCode returnCode = ErrorCode::Success;

	// Initialize gbuffer (and also pass the screen size to be used as the buffer size)
	m_gbuffer = new GeometryBuffer((unsigned int)p_frameData.m_screenSize.x, (unsigned int)p_frameData.m_screenSize.y);

	// Check if the gbuffer initialization was successful
	if(ErrHandlerLoc::get().ifSuccessful(m_gbuffer->init(), returnCode))
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

		//glDepthFunc(GL_LESS);

		// Set face culling mode
		glCullFace(Config::rendererVar().face_culling_mode);

		// Set depth test function
		glDepthFunc(Config::rendererVar().depth_test_func);
	}
	return returnCode;
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

void RendererBackend::processDrawing(const DrawCommands &p_drawCommands, const UniformFrameData &p_frameData)
{
	for(decltype(p_drawCommands.size()) i = 0, size = p_drawCommands.size(); i < size; i++)
	{
		// Get uniform data
		const UniformObjectData &uniformObjectData = p_drawCommands[i].second.m_uniformObjectData;
		//UniformData uniformData(p_drawCommands[i].second.m_uniformObjectData, p_frameData);

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

		glActiveTexture(GL_TEXTURE0 + MaterialType_Normal);
		glBindTexture(GL_TEXTURE_2D, p_drawCommands[i].second.m_matNormal);

		glActiveTexture(GL_TEXTURE0 + MaterialType_Combined);
		glBindTexture(GL_TEXTURE_2D, p_drawCommands[i].second.m_matCombined);

		glActiveTexture(GL_TEXTURE0 + MaterialType_Emissive);
		glBindTexture(GL_TEXTURE_2D, p_drawCommands[i].second.m_matEmissive);

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

