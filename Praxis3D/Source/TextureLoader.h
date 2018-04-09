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
enum TextureCubemapFaces : unsigned int
{
	CubemapFace_PositiveX = 0,
	CubemapFace_NegativeX = 1,
	CubemapFace_PositiveY = 2,
	CubemapFace_NegativeY = 3,
	CubemapFace_PositiveZ = 4,
	CubemapFace_NegativeZ = 5,
	CubemapFace_NumOfFaces
};
enum TextureWrapMode
{
	Repeat = GL_REPEAT,
	ClampToEdge = GL_CLAMP_TO_EDGE,
	ClampToBorder = GL_CLAMP_TO_BORDER,
	MirroredRepeat = GL_MIRRORED_REPEAT
};

class TextureLoader2D;
class TextureLoaderCubemap;
enum ModelMaterialType : unsigned int;

class Texture2D : public LoaderBase<TextureLoader2D, Texture2D>::UniqueObject
{
	friend class RendererFrontend;
	friend class TextureLoader2D;
	friend class Texture2DHandle;
	friend class LoaderBase<TextureLoader2D, Texture2D>::UniqueObject;
protected:
	Texture2D(LoaderBase<TextureLoader2D, Texture2D> *p_loaderBase, std::string p_filename, size_t p_uniqueID, unsigned int p_handle) : UniqueObject(p_loaderBase, p_uniqueID, p_filename), m_handle(p_handle)
	{
		m_mipmapLevel = 0;
		m_textureWidth = 0;
		m_textureHeight = 0;
		m_pixelData = nullptr;
		m_bitmap = nullptr;
		m_textureFormat = TextureFormat_RGBA;
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
			
			if(m_bitmap)
			{
				// Calculate the number of bytes per pixel
				unsigned int bytesPerPixel = FreeImage_GetLine(m_bitmap) / FreeImage_GetWidth(m_bitmap);
				// Calculate the number of samples per pixel
				unsigned int samplesPerPixel = bytesPerPixel / sizeof(BYTE);

				// Only supporting 24bits or 32bits per pixel
				if(samplesPerPixel == 3)
				{
					m_textureFormat = TextureFormat_RGB;
					m_bitmap = FreeImage_ConvertTo24Bits(m_bitmap);
				}
				else
				{
					m_textureFormat = TextureFormat_RGBA;
					m_bitmap = FreeImage_ConvertTo32Bits(m_bitmap);
				}

				// Get texture width, height and size
				m_textureWidth = FreeImage_GetWidth(m_bitmap);
				m_textureHeight = FreeImage_GetHeight(m_bitmap);
				m_size = m_textureWidth * m_textureHeight;
				
				// Texture data passed to the GPU must be in an unsigned char array format
				m_pixelData = (unsigned char*)FreeImage_GetBits(m_bitmap);

				// Temp variable for swapping color channels
				unsigned char blue = 0;

				// Define number of channels per pixel
				const unsigned int numChan = m_textureFormat == TextureFormat_RGB ? 3 : 4;

				// FreeImage loads in BGR format, therefore swap of bytes is needed (Or usage of GL_BGR)
				for(unsigned int i = 0; i < m_size; i++)
				{
					blue = m_pixelData[i * numChan + 0];						 // Store blue
					m_pixelData[i * numChan + 0] = m_pixelData[i * numChan + 2]; // Set red
					m_pixelData[i * numChan + 2] = blue;						 // Set blue
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
	
	// Deletes pixel data stored in RAM. Does not delete the texture that is loaded on GPU VRAM
	ErrorCode unloadMemory()
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

	// Deletes texture from GPU VRAM
	/*ErrorCode unloadVideoMemory()
	{
		ErrorCode returnError = ErrorCode::Success;
		glDeleteTextures(1, &m_handle);
		return returnError;
	}*/

	// Copies the pixel data from source channel from passed texture, to the destination channel
	// Repeats the texture, if the passed texture size does not match
	void setColorChannel(const GLubyte *p_pixelData, unsigned int p_textureWidth, unsigned int p_textureHeight, TextureColorChannelOffset p_destChannel, TextureColorChannelOffset p_sourceChannel)
	{
		// TODO: add support for RGB format as well
		if(m_textureFormat == TextureFormat_RGBA)
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
	}

	// Returns a void pointer to the pixel data
	const inline void *getData() { return (void*)m_pixelData; }

protected:
	TextureFormat m_textureFormat;
	int m_mipmapLevel;

	FIBITMAP* m_bitmap;
	unsigned char *m_pixelData;
	unsigned int m_size,
				 m_handle,
				 m_textureWidth,
				 m_textureHeight;
};
class TextureLoader2D : public LoaderBase<TextureLoader2D, Texture2D>
{
protected:
	Texture2D *m_default2DTexture;
	Texture2D *m_defaultEmissive;
	Texture2D *m_defaultHeight;
	Texture2D *m_defaultNormal;

public:
	class Texture2DHandle
	{
		friend class CommandBuffer;
		friend class TextureLoader2D;
		friend class RendererFrontend;
	public:
		~Texture2DHandle() { m_textureData->decRefCounter(); }

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
					//ErrorCode error = m_textureData->loadToVideoMemory();
					//if(error != ErrorCode::Success)
					//	ErrHandlerLoc::get().log(error, ErrorSource::Source_TextureLoader, m_textureData->getFilename());

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
				//	returnError = m_textureData->loadToVideoMemory();
					m_textureData->setLoadedToVideoMemory(true);
				}
				else
				{
					returnError = m_textureData->loadToMemory();
					if(returnError == ErrorCode::Success)
					{
						m_textureData->setLoadedToMemory(true);
						//returnError = m_textureData->loadToVideoMemory();
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
			//if(!m_textureData->loadedToVideoMemory())
				//if(m_textureData->loadedToMemory())
				//	returnError = m_textureData->loadToVideoMemory();

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
		inline unsigned int getTextureHeight() const { return m_textureData->m_textureHeight; }
		inline unsigned int getTextureWidth() const { return m_textureData->m_textureWidth; }
		inline unsigned int getHandle() const { return m_textureData->m_handle; }
		inline int getMipmapLevel() const { return m_textureData->m_mipmapLevel; }
		inline std::string getFilename() const { return m_textureData->m_filename; }
		inline TextureFormat getTextureFormat() const { return m_textureData->m_textureFormat; }

	private:
		// Increment the reference counter when creating a handle
		Texture2DHandle(Texture2D *p_textureData) : m_textureData(p_textureData) { m_textureData->incRefCounter(); }

		inline unsigned int &getHandleRef() { return m_textureData->m_handle; }

		// Returns a void pointer to the pixel data
		const inline void *getData() { return m_textureData->getData(); }

		Texture2D *m_textureData;
	};
		
	TextureLoader2D();
	~TextureLoader2D();

	ErrorCode init();

	Texture2DHandle load(const std::string &p_filename, MaterialType p_materialType, bool p_startBackgroundLoading = true);
	Texture2DHandle load(const std::string &p_filename, unsigned int p_textureHandle);
};

class TextureCubemap : public LoaderBase<TextureLoaderCubemap, TextureCubemap>::UniqueObject
{
	friend class TextureLoaderCubemap;
	friend class Texture2DHandle;
	friend class LoaderBase<TextureLoaderCubemap, TextureCubemap>::UniqueObject;
protected:
	TextureCubemap(LoaderBase<TextureLoaderCubemap, TextureCubemap> *p_loaderBase, std::string p_combinedName, const std::string(&p_filenames)[CubemapFace_NumOfFaces], size_t p_uniqueID, unsigned int p_handle) : UniqueObject(p_loaderBase, p_uniqueID, p_combinedName), m_handle(p_handle)
	{
		m_mipmapLevel = 0;
		m_textureWidth = 0;
		m_textureHeight = 0;
		m_textureFormat = TextureFormat_RGBA;

		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		{
			m_filenames[face] = p_filenames[face];
			m_pixelData[face] = nullptr;
			m_bitmap[face] = nullptr;
		}

		// Since the texture handle has been provided, set loaded to video memory flag to true
		//setLoadedToVideoMemory(true);
	}
	TextureCubemap(LoaderBase<TextureLoaderCubemap, TextureCubemap> *p_loaderBase, std::string p_combinedName, size_t p_uniqueID, unsigned int p_handle) : UniqueObject(p_loaderBase, p_uniqueID, p_combinedName), m_handle(p_handle)
	{
		m_mipmapLevel = 0;
		m_textureWidth = 0;
		m_textureHeight = 0;
		m_textureFormat = TextureFormat_RGBA;

		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		{
			m_filenames[face] = Utilities::toString(face);
			m_pixelData[face] = nullptr;
			m_bitmap[face] = nullptr;
		}

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
			for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
			{
				// Read the format of the texture
				FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType((Config::PathsVariables().texture_path + m_filenames[face]).c_str(), 0);

				// Read the actual texture
				m_bitmap[face] = FreeImage_Load(imageFormat, (Config::PathsVariables().texture_path + m_filenames[face]).c_str());
				m_bitmap[face] = FreeImage_ConvertTo32Bits(m_bitmap[face]);

				if(m_bitmap[face])
				{
					// Calculate the number of bytes per pixel
					unsigned int bytesPerPixel = FreeImage_GetLine(m_bitmap[face]) / FreeImage_GetWidth(m_bitmap[face]);
					// Calculate the number of samples per pixel
					unsigned int samplesPerPixel = bytesPerPixel / sizeof(BYTE);

					// Only supporting 24bits or 32bits per pixel
					if (samplesPerPixel == 3)
					{
						m_textureFormat = TextureFormat_RGB;
						m_bitmap[face] = FreeImage_ConvertTo24Bits(m_bitmap[face]);
					}
					else
					{
						m_textureFormat = TextureFormat_RGBA;
						m_bitmap[face] = FreeImage_ConvertTo32Bits(m_bitmap[face]);
					}

					// Get texture width, height and size
					m_textureWidth = FreeImage_GetWidth(m_bitmap[face]);
					m_textureHeight = FreeImage_GetHeight(m_bitmap[face]);
					m_size = m_textureWidth * m_textureHeight;

					// Texture data passed to the GPU must be in an unsigned char array format
					m_pixelData[face] = (unsigned char*)FreeImage_GetBits(m_bitmap[face]);

					// Temp variable for swapping color channels
					unsigned char blue = 0;

					// FreeImage loads in BGR format, therefore swap of bytes is needed (Or usage of GL_BGR)
					for(unsigned int i = 0; i < m_size; i++)
					{
						blue = m_pixelData[face][i * 4 + 0];							// Store blue
						m_pixelData[face][i * 4 + 0] = m_pixelData[face][i * 4 + 2];	// Red
						m_pixelData[face][i * 4 + 2] = blue;							// Blue
					}
				}
				else
				{
					ErrHandlerLoc::get().log(ErrorCode::Texture_not_found, ErrorSource::Source_TextureLoader, m_filename);
					returnError = ErrorCode::Texture_not_found;
				}
			}

			if(returnError == ErrorCode::Success)
			{
				setLoadedToMemory(true);
				setLoadedToVideoMemory(false);
			}
		}

		return returnError;
	}

	/*/ Load pixel data from RAM to Video RAM
	ErrorCode loadToVideoMemory()
	{
		ErrorCode returnError = ErrorCode::Success;

		// Make sure that every texture is valid
		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
			if(!m_bitmap[face])
				return ErrorCode::Texture_not_found;

		auto oldHandle = m_handle;

		// Generate, bind and upload the texture
		glGenTextures(1, &m_handle);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);

		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)m_pixelData[face]);

			// Release memory
			FreeImage_Unload(m_bitmap[face]);
			m_pixelData[face] = nullptr;
			m_bitmap[face] = nullptr;
		}

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// Generate  mipmaps if they are enabled
		if(Config::textureVar().generate_mipmaps)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// Texture filtering mode, when image is minimized
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// Texture filtering mode, when image is magnified
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Texture anisotropic filtering
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, Config::textureVar().gl_texture_anisotropy);

		if(m_handle == 0)
			m_handle = oldHandle;

		setLoadedToVideoMemory(true);

		return returnError;
	}*/

	// Deletes pixel data stored in RAM. Does not delete the texture that is loaded on GPU VRAM
	ErrorCode unloadMemory()
	{
		ErrorCode returnError = ErrorCode::Success;
		glDeleteTextures(1, &m_handle);
		return returnError;
	}

	/*/ Deletes texture from GPU VRAM
	ErrorCode unloadVideoMemory()
	{
		ErrorCode returnError = ErrorCode::Success;

		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		{
			// Release memory if it hasn't been freed already
			if(m_bitmap[face])
			{
				FreeImage_Unload(m_bitmap[face]);
				m_bitmap[face] = nullptr;
				m_pixelData[face] = nullptr;
			}
		}

		return returnError;
	}*/

	// Returns a void pointer to the pixel data array
	const inline void **getData() { return (const void**)m_pixelData; }

protected:
	int m_mipmapLevel;
	unsigned int m_size;
	unsigned int m_handle;
	unsigned int m_textureWidth;
	unsigned int m_textureHeight;
	TextureFormat m_textureFormat;
	unsigned char *m_pixelData[CubemapFace_NumOfFaces];

	FIBITMAP* m_bitmap[CubemapFace_NumOfFaces];

	std::string m_filenames[CubemapFace_NumOfFaces];
};
class TextureLoaderCubemap : public LoaderBase<TextureLoaderCubemap, TextureCubemap>
{
protected:
	TextureCubemap *m_defaultCubemap;

public:
	class TextureCubemapHandle
	{
		friend class CommandBuffer;
		friend class RendererFrontend;
		friend class TextureLoaderCubemap;
	public:
		~TextureCubemapHandle() { m_textureData->decRefCounter(); }

		/*/ Bind texture for rendering if it is loaded
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
			glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
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
		}*/

		// Loads data from HDD to RAM and restructures it to be used to fill buffers later
		ErrorCode loadToMemory()
		{
			ErrorCode returnError = ErrorCode::Success;

			// If it's not loaded to memory already, call load
			if(!m_textureData->loadedToMemory())
			{
				returnError = m_textureData->loadToMemory();
				m_textureData->setLoadedToVideoMemory(false);
			}

			return returnError;
		}

		/*/ Loads data from RAM to buffer and uploads them to VRAM
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
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, p_wrapMode);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, p_wrapMode);
		}*/

		// Assignment operator
		TextureCubemapHandle &operator=(const TextureCubemapHandle &p_textureHandle)
		{
			m_textureData->decRefCounter();
			m_textureData = p_textureHandle.m_textureData;
			m_textureData->incRefCounter();
			return *this;
		}

		// Getters
		inline unsigned int getTextureHeight() const { return m_textureData->m_textureHeight; }
		inline unsigned int getTextureWidth() const { return m_textureData->m_textureWidth; }
		inline unsigned int getHandle() const { return m_textureData->m_handle; }
		inline std::string getCombinedFilename() const { return m_textureData->m_filename; }
		inline std::string getFaceFilename(unsigned int p_face) const
		{
			// Make sure the passed index is within bounds
			if(p_face >= CubemapFace_PositiveX && p_face < CubemapFace_NumOfFaces)
				return m_textureData->m_filenames[p_face];

			return std::string();
		}
		inline int getMipmapLevel() const { return m_textureData->m_mipmapLevel; }
		inline TextureFormat getTextureFormat() const { return m_textureData->m_textureFormat; }


	private:
		// Increment the reference counter when creating a handle
		TextureCubemapHandle(TextureCubemap *p_textureData) : m_textureData(p_textureData) { m_textureData->incRefCounter(); }

		inline unsigned int &getHandleRef() { return m_textureData->m_handle; }

		// Returns a void pointer to the pixel data array
		const inline void **getData() { return m_textureData->getData(); }

		TextureCubemap *m_textureData;
	};

	TextureLoaderCubemap();
	~TextureLoaderCubemap();

	ErrorCode init();

	TextureCubemapHandle load(	const std::string &p_filenamePosX, const std::string &p_filenameNegX, 
								const std::string &p_filenamePosY, const std::string &p_filenameNegY, 
								const std::string &p_filenamePosZ, const std::string &p_filenameNegZ, 
								bool p_startBackgroundLoading = true);

	// Returns a default (empty) cubemap handle that is safe to be used
	TextureCubemapHandle load() { return TextureCubemapHandle(m_defaultCubemap); }

	TextureCubemapHandle load(const std::string (&p_filenames)[CubemapFace_NumOfFaces], bool p_startBackgroundLoading = true);

	TextureCubemapHandle load(const std::string &p_filename, unsigned int p_textureHandle);
};