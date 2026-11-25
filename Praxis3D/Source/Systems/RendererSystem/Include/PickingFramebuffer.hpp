#pragma once

#include "Loaders/Include/Config.hpp"
#include "ErrorHandler/Include/ErrorCodes.hpp"
#include "Framebuffer.hpp"

// Offscreen framebuffer used for GPU object picking. Every pickable mesh renders its
// entity ID into a single-channel unsigned integer color attachment, so a single texel
// can be read back to determine which entity is under the mouse cursor.
class PickingFramebuffer : public Framebuffer
{
public:
	// Value written to (and cleared to) the ID buffer when no object is present
	static constexpr unsigned int NoEntityID = 0;

	PickingFramebuffer(unsigned int p_bufferWidth, unsigned int p_bufferHeight) : Framebuffer(p_bufferWidth, p_bufferHeight)
	{
		m_colorBuffer = 0;
		m_depthBuffer = 0;
	}

	~PickingFramebuffer()
	{
		if(m_colorBuffer != 0)
			glDeleteTextures(1, &m_colorBuffer);

		if(m_depthBuffer != 0)
			glDeleteRenderbuffers(1, &m_depthBuffer);
	}

	ErrorCode init(const UniformFrameData &p_frameData) override
	{
		ErrorCode returnCode = ErrorCode::Success;

		if(!initialized())
		{
			// Restore the active texture unit and its binding afterwards, so this does not
			// disturb the textures bound by the other passes
			GLint previousActiveTexture = 0;
			GLint previousTextureBinding = 0;
			glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
			glActiveTexture(GL_TEXTURE0);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTextureBinding);

			// Create the framebuffer and bind it for both reading and writing, so the draw and
			// read buffers can be set on it
			glGenFramebuffers(1, &m_FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

			// Create the entity ID color buffer (generating a texture does not bind it)
			glGenTextures(1, &m_colorBuffer);
			glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
			createColorBuffer();

			// Create the depth buffer (generating a renderbuffer does not bind it)
			glGenRenderbuffers(1, &m_depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
			createDepthBuffer();

			// Attach both buffers to the framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);

			// Only the color attachment is used for writing and reading
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);

			// Check for errors and return an error in case of one
			m_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(m_status != GL_FRAMEBUFFER_COMPLETE)
				returnCode = ErrorCode::Geometrybuffer_failed;

			// Restore the previously bound texture and texture unit
			glBindTexture(GL_TEXTURE_2D, (GLuint)previousTextureBinding);
			glActiveTexture((GLenum)previousActiveTexture);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		return returnCode;
	}

	void setBufferSize(unsigned int p_bufferWidth, unsigned int p_bufferHeight) override
	{
		if(m_bufferWidth != p_bufferWidth || m_bufferHeight != p_bufferHeight)
		{
			m_bufferWidth = p_bufferWidth;
			m_bufferHeight = p_bufferHeight;

			// Restore the active texture unit and its binding afterwards, so resizing the
			// picking buffer does not disturb the textures bound by the other passes
			GLint previousActiveTexture = 0;
			GLint previousTextureBinding = 0;
			glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
			glActiveTexture(GL_TEXTURE0);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTextureBinding);

			// Re-create the buffers with the new size
			glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
			createColorBuffer();

			glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
			createDepthBuffer();

			// Restore the previously bound texture and texture unit
			glBindTexture(GL_TEXTURE_2D, (GLuint)previousTextureBinding);
			glActiveTexture((GLenum)previousActiveTexture);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
	}

	void setBufferSize(GLuint p_buffer, unsigned int p_bufferWidth, unsigned int p_bufferHeight) override
	{
		setBufferSize(p_bufferWidth, p_bufferHeight);
	}

	// Binds the framebuffer, sets the viewport and clears the ID and depth buffers
	void initFrame() override
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
		glViewport(0, 0, (GLsizei)m_bufferWidth, (GLsizei)m_bufferHeight);

		// Clear the ID buffer to the "no entity" value
		const GLuint noEntityID = NoEntityID;
		glClearBufferuiv(GL_COLOR, 0, &noEntityID);

		// Clear the depth buffer so the closest object writes the ID
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	// Reads back the entity ID stored at the given pixel (origin in the bottom-left corner of the buffer)
	unsigned int readID(const unsigned int p_x, const unsigned int p_y)
	{
		unsigned int entityID = NoEntityID;

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels((GLint)p_x, (GLint)p_y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &entityID);

		return entityID;
	}

	inline GLuint getColorBufferHandle() const { return m_colorBuffer; }

private:
	// Creates (or re-creates) the storage and filtering of the ID texture. The caller must have the texture bound
	void createColorBuffer()
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, (GLsizei)m_bufferWidth, (GLsizei)m_bufferHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

		// No filtering or mipmapping, since the buffer is read back a single texel at a time
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void createDepthBuffer()
	{
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, (GLsizei)m_bufferWidth, (GLsizei)m_bufferHeight);
	}

	GLuint m_colorBuffer;
	GLuint m_depthBuffer;
};
