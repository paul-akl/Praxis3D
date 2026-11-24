#pragma once

#include <GL/glew.h>

#include "Systems/RendererSystem/Include/UniformData.hpp"

class Framebuffer
{
public:
	Framebuffer(unsigned int p_bufferWidth, unsigned int p_bufferHeight)
	{
		m_bufferWidth = p_bufferWidth;
		m_bufferHeight = p_bufferHeight;

		m_FBO = 0;
		m_status = 0;
	}
	~Framebuffer()
	{ 
		if(initialized() && m_FBO != 0)
			glDeleteFramebuffers(1, &m_FBO); 
	}

	virtual void bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	}
	virtual void unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	virtual void bindForReading()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
	}
	virtual void bindForWriting()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	}

	GLuint getStatus() { return m_status; }

	virtual ErrorCode init(const UniformFrameData &p_frameData) = 0;
	
	inline unsigned int getBufferWidth() const { return m_bufferWidth; }
	inline unsigned int getBufferHeight() const { return m_bufferHeight; }
	
	// Set the size of all buffers
	virtual void setBufferSize(unsigned int p_bufferWidth, unsigned int p_bufferHeight) = 0;

	// Set the size of an individual buffer
	virtual void setBufferSize(GLuint p_buffer, unsigned int p_bufferWidth, unsigned int p_bufferHeight) = 0;

	virtual void initFrame() = 0;

protected:

	bool initialized() { return (m_status == GL_FRAMEBUFFER_COMPLETE); }

	GLuint	m_FBO,
			m_status;

	unsigned int m_bufferWidth,
				 m_bufferHeight;
};