#pragma once

#include "ErrorCodes.h"
#include "Framebuffer.h"

class GeometryBuffer : public Framebuffer
{
	friend class BloomPass;
public:
	enum GBufferFramebufferType : unsigned int
	{
		FramebufferDefault,
		FramebufferGeometry
	};

	enum GBufferTextureType : unsigned int
	{
		GBufferPosition,
		GBufferDiffuse,
		GBufferNormal,
		GBufferEmissive,
		GBufferMatProperties,
		GBufferFinal,
		GBufferNumTextures = GBufferFinal,
		GBufferIntermediate,
		GBufferTotalNumTextures,
		GBufferInputTexture = GBufferTotalNumTextures,
		GbufferOutputTexture
	};

	typedef unsigned int GBufferTexture;

	GeometryBuffer(unsigned int p_bufferWidth, unsigned int p_bufferHeight);
	~GeometryBuffer();
	
	//virtual void bindReadBuffer(GBufferTextureType p_type) { glReadBuffer(GL_COLOR_ATTACHMENT0 + p_type); }

	// Generates buffers, set's up FBO
	virtual ErrorCode init();
	virtual void setBufferSize(unsigned int p_bufferWidth, unsigned int p_bufferHeight);
	virtual void setBufferSize(GLuint p_buffer, unsigned int p_bufferWidth, unsigned int p_bufferHeight);

	virtual void initFrame();			// This should be called every frame, to clear the final buffer
	virtual void initGeometryPass();	// Bind geometry buffers for drawing
	virtual void initLightPass();		// Bind buffers from geometry pass so they can be accessed when rendering lights
	virtual void initFinalPass();		// Bind the final buffer to 'read from' and the default screen buffer to 'write to'
	
	// Buffer binding functions
	inline void bindBufferForReading(GBufferTextureType p_buffer, int p_activeTexture)
	{
		glActiveTexture(GL_TEXTURE0 + p_activeTexture);
		switch(p_buffer)
		{
		case GeometryBuffer::GBufferPosition:
		case GeometryBuffer::GBufferDiffuse:
		case GeometryBuffer::GBufferNormal:
		case GeometryBuffer::GBufferEmissive:
			glBindTexture(GL_TEXTURE_2D, m_GBTextures[p_buffer]);
			break;
		case GeometryBuffer::GBufferMatProperties:
			glBindTexture(GL_TEXTURE_2D, m_GBTextures[p_buffer]);
			break;
		case GeometryBuffer::GBufferFinal:
			glBindTexture(GL_TEXTURE_2D, m_finalBuffer);
			break;
		case GeometryBuffer::GBufferIntermediate:
			glBindTexture(GL_TEXTURE_2D, m_intermediateBuffer);
			break;
		}
	}
	inline void bindBufferForReading(GBufferTextureType p_buffer)
	{
		glActiveTexture(GL_TEXTURE0);
		switch(p_buffer)
		{
		case GeometryBuffer::GBufferPosition:
		case GeometryBuffer::GBufferDiffuse:
		case GeometryBuffer::GBufferNormal:
		case GeometryBuffer::GBufferEmissive:
			glBindTexture(GL_TEXTURE_2D, m_GBTextures[p_buffer]);
			break;
		case GeometryBuffer::GBufferMatProperties:
			glBindTexture(GL_TEXTURE_2D, m_GBTextures[p_buffer]);
			break;
		case GeometryBuffer::GBufferFinal:
			glBindTexture(GL_TEXTURE_2D, m_finalBuffer);
			break;
		case GeometryBuffer::GBufferIntermediate:
			glBindTexture(GL_TEXTURE_2D, m_intermediateBuffer);
			break;
		}
	}
	inline void bindBufferForWriting(GBufferTextureType p_buffer)
	{
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + p_buffer);
	}
	inline void bindBuffersForWriting(const std::vector<GLenum> &p_buffers)
	{
		glDrawBuffers((GLsizei)p_buffers.size(), p_buffers.data());
	}

	inline void bindBufferToImageUnitForReading(const GBufferTextureType p_buffer, const int p_imageUnitIndex = 0, const int p_mipLevel = 0)
	{
		bindBufferToImageUnit(p_buffer, p_imageUnitIndex, p_mipLevel, GL_READ_ONLY);
	}
	inline void bindBufferToImageUnitForWriting(const GBufferTextureType p_buffer, const int p_imageUnitIndex = 0, const int p_mipLevel = 0)
	{
		bindBufferToImageUnit(p_buffer, p_imageUnitIndex, p_mipLevel, GL_WRITE_ONLY);
	}
	inline void bindBufferToImageUnitForReadWrite(const GBufferTextureType p_buffer, const int p_imageUnitIndex = 0, const int p_mipLevel = 0)
	{
		bindBufferToImageUnit(p_buffer, p_imageUnitIndex, p_mipLevel, GL_READ_WRITE);
	}

	// Framebuffer binding functions
	inline void bindFramebufferForReading(GBufferFramebufferType p_framebufferType)
	{
		switch(p_framebufferType)
		{
		case GeometryBuffer::FramebufferDefault:
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			break;
		case GeometryBuffer::FramebufferGeometry:
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
			break;
		}
	}
	inline void bindFramebufferForWriting(GBufferFramebufferType p_framebufferType)
	{
		switch(p_framebufferType)
		{
		case GeometryBuffer::FramebufferDefault:
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			break;
		case GeometryBuffer::FramebufferGeometry:
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
			break;
		}
	}

	// Generates mipmaps for the texture inside the gbuffer
	inline void generateMipmap(GBufferTextureType p_buffer)
	{
		glActiveTexture(GL_TEXTURE0);

		// Bind the supplied texture
		glBindTexture(GL_TEXTURE_2D, m_finalBuffer);

		// Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	// Getters
	inline GLenum getBufferLocation(GBufferTextureType p_buffer)
	{
		return GL_COLOR_ATTACHMENT0 + p_buffer;
	}

protected:
	inline void bindBufferToImageUnit(const GBufferTextureType p_buffer, const int p_imageUnitIndex, const int p_mipLevel, const GLenum p_access)
	{
		switch(p_buffer)
		{
		case GeometryBuffer::GBufferPosition:
			glBindImageTexture(p_imageUnitIndex, m_GBTextures[p_buffer], p_mipLevel, GL_FALSE, 0, p_access, Config::FramebfrVariables().gl_position_buffer_internal_format);
		case GeometryBuffer::GBufferDiffuse:
			glBindImageTexture(p_imageUnitIndex, m_GBTextures[p_buffer], p_mipLevel, GL_FALSE, 0, p_access, Config::FramebfrVariables().gl_diffuse_buffer_internal_format);
		case GeometryBuffer::GBufferNormal:
			glBindImageTexture(p_imageUnitIndex, m_GBTextures[p_buffer], p_mipLevel, GL_FALSE, 0, p_access, Config::FramebfrVariables().gl_normal_buffer_internal_format);
		case GeometryBuffer::GBufferEmissive:
			glBindImageTexture(p_imageUnitIndex, m_GBTextures[p_buffer], p_mipLevel, GL_FALSE, 0, p_access, Config::FramebfrVariables().gl_emissive_buffer_internal_format);
			break;
		case GeometryBuffer::GBufferMatProperties:
			glBindImageTexture(p_imageUnitIndex, m_GBTextures[p_buffer], p_mipLevel, GL_FALSE, 0, p_access, Config::FramebfrVariables().gl_mat_properties_buffer_internal_format);
			break;
		case GeometryBuffer::GBufferFinal:
			glBindTextureUnit(0, m_finalBuffer);
			glBindImageTexture(p_imageUnitIndex, m_finalBuffer, p_mipLevel, GL_FALSE, 0, p_access, Config::FramebfrVariables().gl_final_buffer_internal_format);
			break;
		case GeometryBuffer::GBufferIntermediate:
			glBindImageTexture(p_imageUnitIndex, m_intermediateBuffer, p_mipLevel, GL_FALSE, 0, p_access, Config::FramebfrVariables().gl_blur_buffer_internal_format);
			break;
		}
	}

	GLuint  m_GBTextures[GBufferNumTextures],	// Geometry pass textures
			m_intermediateBuffer,				// Intermediate buffer between vertical and horizontal blur passes
			m_finalBuffer,						// Final buffer that gets copied to the screen
			m_depthBuffer,
			m_internalFormats[GBufferNumTextures];

	GLenum  m_texBuffers[GBufferNumTextures],	// Handles for binding geometry buffers, used for multiple render to textures
			m_emissiveAndFinalBuffers[2],
			m_textureFormats[GBufferNumTextures],
			m_textureTypes[GBufferNumTextures];

	GBufferTextureType m_finalPassBuffer;
};

