#pragma once

#include "ErrorCodes.h"
#include "Framebuffer.h"

class GeometryBuffer : public Framebuffer
{
public:
	enum GBufferTextureType : unsigned int
	{
		GBufferPosition,
		GBufferDiffuse,
		GBufferNormal,
		GBufferEmissive,
		GBufferNumTextures,
		GBufferFinal = GBufferNumTextures,
		GBufferBlur,
		GBufferTotalNumTextures
	};

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

	void bindForReading(GBufferTextureType p_buffer, int p_activeTexture);
	void bindForReading(GBufferTextureType p_buffer);
	void bindForWriting(GBufferTextureType p_buffer);
	
protected:

	GLuint  m_GBTextures[GBufferNumTextures],	// Geometry pass textures
			m_blurBuffer,						// Intermediate buffer between vertical and horizontal blur passes
			m_finalBuffer,						// Final buffer that gets copied to the screen
			m_depthBuffer,
			m_internalFormats[GBufferNumTextures];

	GLenum  m_texBuffers[GBufferNumTextures],	// Handles for binding geometry buffers, used for multiple render to textures
			m_emissiveAndFinalBuffers[2],
			m_textureFormats[GBufferNumTextures],
			m_textureTypes[GBufferNumTextures];

	GBufferTextureType m_finalPassBuffer;
};

