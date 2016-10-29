#pragma once

#include <FreeImage.h>
#include <GL/glew.h>
#include <string>
#include <tbb/atomic.h>
#include <vector>

#include "Config.h"
#include "ErrorCodes.h"
#include "ErrorHandlerLocator.h"
#include "LoaderBase.h"

enum TextureColorChannelOffset : unsigned int
{
	ColorOffset_Red		= 0,
	ColorOffset_Green	= 1,
	ColorOffset_Blue	= 2,
	ColorOffset_Alpha	= 3,
	ColorOffset_NumChannels
};

class TextureLoader;
enum ModelMaterialType : unsigned int;

class Texture2D : public LoaderBase<TextureLoader, Texture2D>::UniqueObject
{
	friend class TextureLoader;
	friend class Texture2DHandle;
	friend class LoaderBase<TextureLoader, Texture2D>::UniqueObject;
protected:
	Texture2D(LoaderBase<TextureLoader, Texture2D> *p_loaderBase, std::string p_filename, size_t p_uniqueID, unsigned int p_handle) : UniqueObject(p_loaderBase, p_uniqueID, p_filename), m_handle(p_handle)
	{
		m_textureWidth = 0;
		m_textureHeight = 0;
		m_pixelData = nullptr;

		// Since the texture handle has been provided, set loaded to video memory flag to true
		//setLoadedToVideoMemory(true);
	}

	// Loads pixel data (using the filename) from HDD to RAM, re-factors it
	ErrorCode loadToMemory()
	{
		ErrorCode returnError = ErrorCode::Success;

		// If the same texture is being used in multiple objects, loadToMemory might be called
		// from multiple threads at the same time. Use a spin wait to deal with it
		SpinWait::Lock lock(m_mutex);

		// Texture might have already been loaded when called from a different thread. Check if it was
		if(!loadedToMemory())
		{
			// Read the format of the texture
			FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType((Config::PathsVariables().texture_path + m_filename).c_str(), 0);
			
			// Read the actual texture
			m_bitmap = FreeImage_Load(imageFormat, (Config::PathsVariables().texture_path + m_filename).c_str());
			m_bitmap = FreeImage_ConvertTo32Bits(m_bitmap);

			if(m_bitmap)
			{
				// Get texture width, height and size
				m_textureWidth = FreeImage_GetWidth(m_bitmap);
				m_textureHeight = FreeImage_GetHeight(m_bitmap);
				m_size = m_textureWidth * m_textureHeight;
				
				// Texture data passed to the GPU must be in an unsigned char array format
				m_pixelData = (unsigned char*)FreeImage_GetBits(m_bitmap);

				// Temp variable for swapping color channels
				unsigned char blue = 0;

				// FreeImage loads in BGR format, therefore swap of bytes is needed (Or usage of GL_BGR)
				for(unsigned int i = 0; i < m_size; i++)
				{
					blue = m_pixelData[i * 4 + 0];

					m_pixelData[i * 4 + 0] = m_pixelData[i * 4 + 2];	// Red
					m_pixelData[i * 4 + 2] = blue;						// Blue
				}

				setLoadedToMemory(true);
				setLoadedToVideoMemory(false);
			}
			else
			{
				ErrHandlerLoc::get().log(ErrorCode::Texture_not_found, ErrorSource::Source_TextureLoader, m_filename);
				returnError = ErrorCode::Texture_not_found;
			}
		}

		return returnError;
	}
	
	// Load pixel data from RAM to Video RAM
	ErrorCode loadToVideoMemory()
	{
		ErrorCode returnError = ErrorCode::Success;

		if(m_bitmap)
		{
			auto oldHandle = m_handle;

			// Generate, bind and upload the texture
			glGenTextures(1, &m_handle);
			glBindTexture(GL_TEXTURE_2D, m_handle);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)m_pixelData);

			// Generate  mipmaps if they are enabled
			if(Config::textureVar().generate_mipmaps)
				glGenerateMipmap(GL_TEXTURE_2D);

			// Texture filtering mode, when image is minimized
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Config::textureVar().gl_texture_minification);
			// Texture filtering mode, when image is magnified
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Config::textureVar().gl_texture_magnification);
			// Texture anisotropic filtering
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Config::textureVar().gl_texture_anisotropy);

			if(m_handle == 0)
				m_handle = oldHandle;

			// Release memory
			FreeImage_Unload(m_bitmap);
			m_pixelData = nullptr;
			m_bitmap = nullptr;
		}
		else
		{
			returnError = ErrorCode::Texture_not_found;
		}

		setLoadedToVideoMemory(true);

		return returnError;
	}

	// Deletes pixel data stored in RAM. Does not delete the texture that is loaded on GPU VRAM
	ErrorCode unloadMemory()
	{
		ErrorCode returnError = ErrorCode::Success;
		glDeleteTextures(1, &m_handle);
		return returnError;
	}

	// Deletes texture from GPU VRAM
	ErrorCode unloadVideoMemory()
	{
		ErrorCode returnError = ErrorCode::Success;

		// Release memory if it hasn't been freed already
		if(m_bitmap)
		{
			FreeImage_Unload(m_bitmap);
			m_bitmap = nullptr;
			m_pixelData = nullptr;
		}

		return returnError;
	}

	// Copies the pixel data from source channel from passed texture, to the destination channel
	// Repeats the texture, if the passed texture size does not match
	void setColorChannel(const GLubyte *p_pixelData, unsigned int p_textureWidth, unsigned int p_textureHeight, TextureColorChannelOffset p_destChannel, TextureColorChannelOffset p_sourceChannel)
	{		
		// Loop through the 1D array in a 2D fashion, to make the texture-repeat easier
		// Loop through the width of the texture
		for(unsigned int destWidth = 0, srcWidth = 0; destWidth < m_textureWidth; destWidth++, srcWidth++)
		{
			// If the width index exceeds the passed texture width, clamp it back to 0, so it repeats
			if(srcWidth >= p_textureWidth)
				srcWidth = 0;

			// Loop through the height of the texture
			for(unsigned int destHeight = 0, srcHeight = 0; destHeight < m_textureHeight; destHeight++, srcHeight++)
			{
				// If the height index exceeds the passed texture height, clamp it back to 0, so it repeats
				if(srcHeight >= p_textureHeight)
					srcHeight = 0;
				
				// Copy the pixel data
				// Width-index * texture-width + height-index is the translation from 2D array indices to 1D
				// Multiply the indices by 4, because textures contain RGBA channels, and add the channel offset
				m_pixelData[(destWidth * m_textureWidth + destHeight) * 4 + p_destChannel] =
					p_pixelData[(srcWidth * p_textureWidth + srcHeight) * 4 + p_sourceChannel];
			}
		}
	}

protected:
	unsigned int m_size;
	unsigned int m_handle;
	unsigned int m_textureWidth;
	unsigned int m_textureHeight;
	unsigned char *m_pixelData;
	FIBITMAP* m_bitmap;
};

class TextureLoader : public LoaderBase<TextureLoader, Texture2D>
{
protected:
	Texture2D *m_default2DTexture;
	Texture2D *m_defaultEmissive;
	Texture2D *m_defaultHeight;
	Texture2D *m_defaultNormal;

public:
	enum TextureWrapMode
	{
		Repeat = GL_REPEAT,
		ClampToEdge = GL_CLAMP_TO_EDGE,
		ClampToBorder = GL_CLAMP_TO_BORDER,
		MirroredRepeat = GL_MIRRORED_REPEAT
	};
	class Texture2DHandle
	{
		friend class TextureLoader;
	public:
		~Texture2DHandle() { 
			m_textureData->decRefCounter();
			//std::cout << m_textureData->m_filename << ": dtor Reference Counter: " << m_textureData->m_refCounter << std::endl;
		}

		// Bind texture for rendering if it is loaded
		// Load the texture to GPU memory instead if it's not loaded
		void bind(unsigned int p_activeTextureID = GL_TEXTURE0)
		{
			// Declare the handle to bind at the start here, and bind it at the end of the function,
			// so there is only one path to bind function and the branch can be predicted easier on CPU
			decltype(m_textureData->m_handle) handle = m_textureData->m_handle;

			// If the texture is already loaded (to GPU), just bind it (the handle is already assigned).
			// Otherwise load it to GPU. This way, the texture loading is hidden away,
			// so graphics system doesn't have to deal with it. The thread that calls bind
			// on a texture is guaranteed to be rendering thread, so it is save to load it.
			if(!m_textureData->loadedToVideoMemory())
			{
				if(m_textureData->loadedToMemory())
				{
					// Reassign the handle, since it has been changed while loading to video memory
					handle = m_textureData->m_handle;

					// If loading to memory failed, log an error
					ErrorCode error = m_textureData->loadToVideoMemory();
					if(error != ErrorCode::Success)
						ErrHandlerLoc::get().log(error, ErrorSource::Source_TextureLoader, m_textureData->getFilename());

					// Set loaded to video memory flag to true even if loading failed.
					// This way, the failed loading attempt is not repeated every frame.
					// (And since loaded to memory flag is already true, it is not the cause,
					// and it will not "fix" itself)
					m_textureData->setLoadedToVideoMemory(true);
				}
				else
				{
					// Set handle to zero if the texture is not yet ready to be used
					handle = 0;
				}
			}

			// Set the active texture position and bind the handle
			glActiveTexture(GL_TEXTURE0 + p_activeTextureID);
			glBindTexture(GL_TEXTURE_2D, handle);
		}

		// Upload the texture to GPU memory
		ErrorCode preload()
		{
			ErrorCode returnError = ErrorCode::Success;

			if(!m_textureData->loadedToVideoMemory())
			{
				if(m_textureData->loadedToMemory())
				{
					returnError = m_textureData->loadToVideoMemory();
					m_textureData->setLoadedToVideoMemory(true);
				}
				else
				{
					returnError = m_textureData->loadToMemory();
					if(returnError == ErrorCode::Success)
					{
						m_textureData->setLoadedToMemory(true);
						returnError = m_textureData->loadToVideoMemory();
						m_textureData->setLoadedToVideoMemory(true);
					}
				}
			}

			return returnError;
		}

		// Loads data from HDD to RAM and restructures it to be used to fill buffers later
		ErrorCode loadToMemory()
		{
			ErrorCode returnError = ErrorCode::Success;

			// If it's not loaded to memory already, call load
			if(!m_textureData->loadedToMemory())
				returnError = m_textureData->loadToMemory();

			return returnError;
		}

		// Loads data from RAM to buffer and uploads them to VRAM
		// WARNING: should probably only be called from rendering thread, since this code deals with graphics API
		ErrorCode loadToVideoMemory()
		{
			ErrorCode returnError = ErrorCode::Success;

			// If it's not loaded to video memory already, and has been loaded to memory, call load
			if(!m_textureData->loadedToVideoMemory())
				if(m_textureData->loadedToMemory())
					returnError = m_textureData->loadToVideoMemory();

			return returnError;
		}
		
		void setWrapMode(TextureWrapMode p_wrapMode)
		{
			bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p_wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p_wrapMode);
		}

		void setColorChannel(const Texture2DHandle &p_textureHandle, TextureColorChannelOffset p_destChannel, TextureColorChannelOffset p_sourceChannel = ColorOffset_Red)
		{
			// If both textures are not loaded to video memory already and have pixel data
			if(m_textureData->m_loadedToVideoMemory == false && m_textureData->m_pixelData &&
			   p_textureHandle.m_textureData->m_loadedToVideoMemory == false && p_textureHandle.m_textureData->m_pixelData)
			{
				// Set the color channel of the internal texture
				m_textureData->setColorChannel(p_textureHandle.m_textureData->m_pixelData,
											   p_textureHandle.m_textureData->m_textureWidth,
											   p_textureHandle.m_textureData->m_textureHeight,
											   p_destChannel, p_sourceChannel);

				// Assign a new filename, since the texture data was modified
				m_textureData->m_filename += ", " + p_textureHandle.m_textureData->m_filename;
			}
		}

		// Assignment operator
		Texture2DHandle &operator=(const Texture2DHandle &p_textureHandle)
		{
			m_textureData->decRefCounter();
			m_textureData = p_textureHandle.m_textureData;
			m_textureData->incRefCounter();
			return *this;
		}

		// Getters
		inline unsigned int getHandle() const { return m_textureData->m_handle; }
		inline std::string getFilename() const { return m_textureData->m_filename; }

	private:
		// Increment the reference counter when creating a handle
		Texture2DHandle(Texture2D *p_textureData) : m_textureData(p_textureData) { m_textureData->incRefCounter(); }

		Texture2D *m_textureData;
	};

	TextureLoader();
	~TextureLoader();

	ErrorCode init();

	virtual Texture2DHandle load2D(const std::string &p_filename, Model::ModelMaterialType p_materialType, bool p_startBackgroundLoading = true);
	virtual Texture2DHandle load2D(const std::string &p_filename, unsigned int p_textureHandle);
};