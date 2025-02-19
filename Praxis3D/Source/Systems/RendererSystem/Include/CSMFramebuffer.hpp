#pragma once

#include <vector>

#include "Loaders/Include/Config.hpp"
#include "ErrorHandler/Include/ErrorCodes.hpp"
#include "Framebuffer.hpp"

class CSMFramebuffer : public Framebuffer
{
public:
	CSMFramebuffer(const UniformFrameData &p_frameData) : Framebuffer(p_frameData.m_shadowMappingData.m_csmResolution, p_frameData.m_shadowMappingData.m_csmResolution)
	{
		m_depthBuffers = 0;
		m_numOfCascades = 0;
	}
	~CSMFramebuffer()
	{
		if(m_depthBuffers != 0)
			glDeleteTextures(1, &m_depthBuffers);
	}

	// Generates buffers, set's up FBO
	ErrorCode init(const UniformFrameData &p_frameData)
	{
		ErrorCode returnCode = ErrorCode::Success;

		if(!initialized())
		{
			m_numOfCascades = int(p_frameData.m_shadowMappingData.m_shadowCascadePlaneDistances.size());

			// Create the FBO
			glGenFramebuffers(1, &m_FBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

			glGenTextures(1, &m_depthBuffers);
			createDepthBuffers();

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

			glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthBuffers, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			
			// Check for errors and return an error in case of one
			m_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(m_status != GL_FRAMEBUFFER_COMPLETE)
				returnCode = ErrorCode::Geometrybuffer_failed;

			GLenum glError = glGetError();
			if(glError != GL_NO_ERROR)
			{
				std::cout << "csm framebuffer gl error" << std::endl;
			}
		}
		return returnCode;
	}

	// Set the size of all buffers
	virtual void setBufferSize(unsigned int p_bufferWidth, unsigned int p_bufferHeight)
	{
		m_bufferWidth = p_bufferWidth;
		m_bufferHeight = p_bufferHeight;
		createDepthBuffers();
	}	
	
	void setBufferSizeAndNumOfCascades(unsigned int p_bufferWidth, unsigned int p_bufferHeight, unsigned int p_numOfCascades)
	{
		m_bufferWidth = p_bufferWidth;
		m_bufferHeight = p_bufferHeight;
		m_numOfCascades = int(p_numOfCascades);
		createDepthBuffers();
	}

	// Set the size of an individual buffer
	virtual void setBufferSize(GLuint p_buffer, unsigned int p_bufferWidth, unsigned int p_bufferHeight)
	{
		switch(p_buffer)
		{
			case CSMBufferTextureType::CSMBufferTextureType_CSMDepthMap:
				m_bufferWidth = p_bufferWidth;
				m_bufferHeight = p_bufferHeight;
				createDepthBuffers();
				break;
		}
	}

	void setNumOfCascades(unsigned int p_numOfCascades)
	{
		if(m_numOfCascades != p_numOfCascades)
		{
			m_numOfCascades = int(p_numOfCascades);
			createDepthBuffers();
		}
	}

	virtual void initFrame()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
		glViewport(0, 0, m_bufferWidth, m_bufferHeight);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);		// Enable depth testing, as this is much like a regular forward render pass
		glDepthFunc(GL_LEQUAL);
		glClear(GL_DEPTH_BUFFER_BIT);	// Make sure to clear the depth buffer for the new frame
	}

	// Buffer binding functions
	inline void bindBufferForReading(CSMBufferTextureType p_buffer, int p_activeTexture = 0)
	{
		glActiveTexture(GL_TEXTURE0 + p_activeTexture);
		switch(p_buffer)
		{
			case CSMBufferTextureType::CSMBufferTextureType_CSMDepthMap:
				glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthBuffers);
				break;
		}
	}

private:
	inline void createDepthBuffers()
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthBuffers);
		glTexImage3D(
			GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, m_bufferWidth, m_bufferHeight, m_numOfCascades,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	GLuint m_depthBuffers;

	int m_numOfCascades;
};