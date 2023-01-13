#include "Config.h"
#include "GeometryBuffer.h"

GeometryBuffer::GeometryBuffer(unsigned int p_bufferWidth, unsigned int p_bufferHeight) : Framebuffer(p_bufferWidth, p_bufferHeight)
{

	m_intermediateBuffer = 0;
	m_depthBuffer = 0;
	m_finalBuffer = 0;
	m_finalPassBuffer = GBufferFinal;

	m_emissiveAndFinalBuffers[0] = GL_COLOR_ATTACHMENT0 + GBufferEmissive;
	m_emissiveAndFinalBuffers[1] = GL_COLOR_ATTACHMENT0 + GBufferFinal;
	
	for(GLuint i = GBufferPosition; i < GBufferNumTextures; i++)	// For each buffer
	{
		m_texBuffers[i] = GL_COLOR_ATTACHMENT0 + i;				// set up it's position in the shader

		m_GBTextures[i] = 0;
		m_internalFormats[i] = 0;
		m_textureFormats[i] = 0;
		m_textureTypes[i] = 0;
	}
}

GeometryBuffer::~GeometryBuffer()
{
	// Clean the existing buffers
	if(m_GBTextures[0] != 0)
		glDeleteTextures(sizeof(m_GBTextures) / sizeof(*m_GBTextures), m_GBTextures);

	if(m_depthBuffer != 0)
		glDeleteTextures(1, &m_depthBuffer);

	if(m_intermediateBuffer != 0)
		glDeleteTextures(1, &m_intermediateBuffer);

	if(m_finalBuffer != 0)
		glDeleteTextures(1, &m_finalBuffer);
}

ErrorCode GeometryBuffer::init()
{
	ErrorCode returnCode = ErrorCode::Success;

	if(!initialized())
	{
		// Create the FBO
		glGenFramebuffers(1, &m_FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

		// Create geometry pass buffers
		glGenTextures(GBufferNumTextures, m_GBTextures);
		glGenTextures(1, &m_depthBuffer);
		glGenTextures(1, &m_intermediateBuffer);
		glGenTextures(1, &m_finalBuffer);

		// Set up texture formats
		m_internalFormats[GBufferPosition] = Config::FramebfrVariables().gl_position_buffer_internal_format;
		m_textureFormats[GBufferPosition] = Config::FramebfrVariables().gl_position_buffer_texture_format;
		m_textureTypes[GBufferPosition] = Config::FramebfrVariables().gl_position_buffer_texture_type;

		m_internalFormats[GBufferDiffuse] = Config::FramebfrVariables().gl_diffuse_buffer_internal_format;
		m_textureFormats[GBufferDiffuse] = Config::FramebfrVariables().gl_diffuse_buffer_texture_format;
		m_textureTypes[GBufferDiffuse] = Config::FramebfrVariables().gl_diffuse_buffer_texture_type;

		m_internalFormats[GBufferNormal] = Config::FramebfrVariables().gl_normal_buffer_internal_format;
		m_textureFormats[GBufferNormal] = Config::FramebfrVariables().gl_normal_buffer_texture_format;
		m_textureTypes[GBufferNormal] = Config::FramebfrVariables().gl_normal_buffer_texture_type;

		m_internalFormats[GBufferMatProperties] = Config::FramebfrVariables().gl_mat_properties_buffer_internal_format;
		m_textureFormats[GBufferMatProperties] = Config::FramebfrVariables().gl_mat_properties_buffer_texture_format;
		m_textureTypes[GBufferMatProperties] = Config::FramebfrVariables().gl_mat_properties_buffer_texture_type;

		m_internalFormats[GBufferEmissive] = Config::FramebfrVariables().gl_emissive_buffer_internal_format;
		m_textureFormats[GBufferEmissive] = Config::FramebfrVariables().gl_emissive_buffer_texture_format;
		m_textureTypes[GBufferEmissive] = Config::FramebfrVariables().gl_emissive_buffer_texture_type;

		// Create textures
		for(GLuint i = 0; i < GBufferNumTextures; i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_GBTextures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormats[i], m_bufferWidth, m_bufferHeight, 0, m_textureFormats[i], m_textureTypes[i], NULL);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Config::FramebfrVariables().gl_buffers_min_filter);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Config::FramebfrVariables().gl_buffers_mag_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Config::FramebfrVariables().gl_buffers_wrap_s_method);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Config::FramebfrVariables().gl_buffers_wrap_t_method);

			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBTextures[i], 0);
		}

		// Create depth buffer
		glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_bufferWidth, m_bufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_depth_buffer_internal_format, m_bufferWidth, m_bufferHeight, 0, 
					 Config::FramebfrVariables().gl_depth_buffer_texture_format, Config::FramebfrVariables().gl_depth_buffer_texture_type, NULL);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);

		// Create the intermediate buffer, that acts as an intermediate buffer between vertical and horizontal blur passes
		glBindTexture(GL_TEXTURE_2D, m_intermediateBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_blur_buffer_internal_format, m_bufferWidth, m_bufferHeight, 0, 
					 Config::FramebfrVariables().gl_blur_buffer_texture_format, Config::FramebfrVariables().gl_blur_buffer_texture_type, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Config::FramebfrVariables().gl_buffers_min_filter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Config::FramebfrVariables().gl_buffers_mag_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Config::FramebfrVariables().gl_buffers_wrap_s_method);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Config::FramebfrVariables().gl_buffers_wrap_t_method);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBufferIntermediate, GL_TEXTURE_2D, m_intermediateBuffer, 0);

		// Create the final buffer, that gets rendered to the screen
		glBindTexture(GL_TEXTURE_2D, m_finalBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_final_buffer_internal_format, m_bufferWidth, m_bufferHeight, 0, 
					 Config::FramebfrVariables().gl_final_buffer_texture_format, Config::FramebfrVariables().gl_final_buffer_texture_type, NULL);
		// If HDR bloom is enabled, buffer should use a different min_filter (to support mipmapping)
		if(Config::graphicsVar().bloom_enabled)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Config::FramebfrVariables().gl_final_buffer_min_filter_HDR);
		else
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Config::FramebfrVariables().gl_final_buffer_min_filter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Config::FramebfrVariables().gl_final_buffer_mag_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Config::FramebfrVariables().gl_buffers_wrap_s_method);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Config::FramebfrVariables().gl_buffers_wrap_t_method);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBufferFinal, GL_TEXTURE_2D, m_finalBuffer, 0);
		// If HDR bloom is enabled, create mip maps for the final buffer
		if(Config::graphicsVar().bloom_enabled)
			generateMipmap(GeometryBuffer::GBufferFinal);

		// Check for errors and return an error in case of one
		m_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(m_status != GL_FRAMEBUFFER_COMPLETE)
			returnCode = ErrorCode::Geometrybuffer_failed;

		// Restore the default FBO, so it doesn't get changed from the outside of the class
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		//glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	}
	return returnCode;
}
void GeometryBuffer::setBufferSize(unsigned int p_bufferWidth, unsigned int p_bufferHeight)
{
	if(initialized())
	{
		m_bufferWidth = p_bufferWidth;
		m_bufferHeight = p_bufferHeight;

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

		// Create textures
		for (GLuint i = 0; i < GBufferNumTextures; i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_GBTextures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormats[i], m_bufferWidth, m_bufferHeight, 0, m_textureFormats[i], m_textureTypes[i], NULL);
		}

		// Create the intermediate buffer, that acts as an intermediate buffer between vertical and horizontal blur passes
		glBindTexture(GL_TEXTURE_2D, m_intermediateBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_blur_buffer_internal_format, m_bufferWidth, m_bufferHeight, 0,
			Config::FramebfrVariables().gl_blur_buffer_texture_format, Config::FramebfrVariables().gl_blur_buffer_texture_type, NULL);

		// Create the final buffer, that gets renderred to the screen
		glBindTexture(GL_TEXTURE_2D, m_finalBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_final_buffer_internal_format, m_bufferWidth, m_bufferHeight, 0,
			Config::FramebfrVariables().gl_final_buffer_texture_format, Config::FramebfrVariables().gl_final_buffer_texture_type, NULL);
		
		// Create depth buffer
		glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_bufferWidth, m_bufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_depth_buffer_internal_format, m_bufferWidth, m_bufferHeight, 0,
			Config::FramebfrVariables().gl_depth_buffer_texture_format, Config::FramebfrVariables().gl_depth_buffer_texture_type, NULL);

	}
}
void GeometryBuffer::setBufferSize(GLuint p_buffer, unsigned int p_bufferWidth, unsigned int p_bufferHeight)
{
	if(initialized())
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

		switch(p_buffer)
		{
		case GBufferPosition:
		case GBufferDiffuse:
		case GBufferNormal:
		case GBufferEmissive:
			// Resize geometry textures
			glBindTexture(GL_TEXTURE_2D, m_GBTextures[p_buffer]);
			glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormats[p_buffer], p_bufferWidth, p_bufferHeight, 0, m_textureFormats[p_buffer], m_textureTypes[p_buffer], NULL);

			break;
		case GBufferFinal:
			// Resize depth buffer
			glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
			glTexImage2D(GL_TEXTURE_2D, 10, Config::FramebfrVariables().gl_depth_buffer_internal_format, p_bufferWidth, p_bufferHeight, 0, 
						 Config::FramebfrVariables().gl_depth_buffer_texture_format, Config::FramebfrVariables().gl_depth_buffer_texture_type, NULL);
			
			// Resize the final buffer
			glBindTexture(GL_TEXTURE_2D, m_finalBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_final_buffer_internal_format, p_bufferWidth, p_bufferHeight, 0, 
						 Config::FramebfrVariables().gl_final_buffer_texture_format, Config::FramebfrVariables().gl_final_buffer_texture_type, NULL);

			m_bufferWidth = p_bufferWidth;
			m_bufferHeight = p_bufferHeight;

			break;
		case GBufferIntermediate:
			// Resize the blur buffer
			glBindTexture(GL_TEXTURE_2D, m_intermediateBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, Config::FramebfrVariables().gl_blur_buffer_internal_format, p_bufferWidth, p_bufferHeight, 0, 
						 Config::FramebfrVariables().gl_blur_buffer_texture_format, Config::FramebfrVariables().gl_blur_buffer_texture_type, NULL);
			
			break;
		default:
			break;
		}
	}
}

void GeometryBuffer::initFrame()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBufferIntermediate);	// Bind intermediate buffer
	glClear(GL_COLOR_BUFFER_BIT);								// and clear it
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBufferFinal);			// Bind final buffer
	glClear(GL_COLOR_BUFFER_BIT);								// and clear it
}
void GeometryBuffer::initGeometryPass()
{
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glDrawBuffers(GBufferNumTextures, m_texBuffers);		// Bind geometry pass buffers to write to
	glClear(GL_COLOR_BUFFER_BIT);
}
void GeometryBuffer::initLightPass()
{
	glActiveTexture(GL_TEXTURE0 + GBufferPosition);
	glBindTexture(GL_TEXTURE_2D, m_GBTextures[GBufferPosition]);

	glActiveTexture(GL_TEXTURE0 + GBufferDiffuse);
	glBindTexture(GL_TEXTURE_2D, m_GBTextures[GBufferDiffuse]);

	glActiveTexture(GL_TEXTURE0 + GBufferNormal);
	glBindTexture(GL_TEXTURE_2D, m_GBTextures[GBufferNormal]);

	glActiveTexture(GL_TEXTURE0 + GBufferMatProperties);
	glBindTexture(GL_TEXTURE_2D, m_GBTextures[GBufferMatProperties]);

	glDrawBuffers(2, m_emissiveAndFinalBuffers);
}
void GeometryBuffer::initFinalPass()
{
#ifdef SETTING_USE_BLIT_FRAMEBUFFER

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);			// Set default framebuffer to paste to
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);		// Set GBuffer's framebuffer to copy from
	glReadBuffer(GL_COLOR_ATTACHMENT0 + GBufferFinal);	// Bind final buffer, to copy from

#else

	// Bind final framebuffer
	glActiveTexture(GL_TEXTURE0 + GBufferFinal);
	glBindTexture(GL_TEXTURE_2D, m_finalBuffer);
	
	// Set the default framebuffer to be drawn to
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

#endif // SETTING_USE_BLIT_FRAMEBUFFER
}