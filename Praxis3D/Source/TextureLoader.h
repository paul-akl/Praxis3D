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
public:
	inline unsigned int getHandle() const { return m_handle; }

	inline unsigned int getTextureHeight() const { return m_textureHeight; }
	inline unsigned int getTextureWidth() const { return m_textureWidth; }

	inline TextureFormat getTextureFormat()			const { return m_textureFormat; }
	inline TextureDataFormat getTextureDataFormat() const { return m_textureDataFormat; }
	inline TextureDataType getTextureDataType()		const { return m_textureDataType; }
	
	inline bool getCompressionEnabled()				const { return m_enableCompression; }
	inline bool getDownsamplingEnabled()			const { return m_enableDownsampling; }
	inline bool getMipmapEnabled()					const { return m_enableMipmap; }
	inline int getMipmapLevel()						const { return m_mipmapLevel; }
	inline const void *getPixelData()				const { return m_pixelData; }

	inline void setPixelData(void *p_pixelData)
	{
		if(m_pixelData != nullptr)
			delete m_pixelData;

		m_pixelData = p_pixelData;
	}

	// Returns true of the texture was loaded from file
	const inline bool isLoadedFromFile() const { return m_loadedFromFile; }

	// Convert the GL filter type (int) to TextureFilterType enum
	static inline TextureFilterType convertToTextureFilterType(const int p_textureFilterType)
	{
		switch(p_textureFilterType)
		{
			case TextureFilterType::TextureFilterType_Linear:
			default:
				return TextureFilterType::TextureFilterType_Linear;
				break;
			case TextureFilterType::TextureFilterType_LinearMipmapLinear:
				return TextureFilterType::TextureFilterType_LinearMipmapLinear;
				break;
			case TextureFilterType::TextureFilterType_LinearMipmapNearest:
				return TextureFilterType::TextureFilterType_LinearMipmapNearest;
				break;
			case TextureFilterType::TextureFilterType_Nearest:
				return TextureFilterType::TextureFilterType_Nearest;
				break;
			case TextureFilterType::TextureFilterType_NearestMipmapLinear:
				return TextureFilterType::TextureFilterType_NearestMipmapLinear;
				break;
			case TextureFilterType::TextureFilterType_NearestMipmapNearest:
				return TextureFilterType::TextureFilterType_NearestMipmapNearest;
				break;
		}
	}

protected:
	Texture2D(LoaderBase<TextureLoader2D, Texture2D> *p_loaderBase, const std::string p_filename, const size_t p_uniqueID, const unsigned int p_handle, const MaterialType p_materialType) : UniqueObject(p_loaderBase, p_uniqueID, p_filename), m_handle(p_handle), m_materialType(p_materialType)
	{
		m_size = 0;
		m_loadedFromFile = false;
		m_mipmapLevel = 0;
		m_textureWidth = 0;
		m_textureHeight = 0;
		m_pixelData = nullptr;
		m_bitmap = nullptr;
		m_textureFormat = TextureFormat::TextureFormat_RGBA;
		m_textureDataFormat = TextureDataFormat::TextureDataFormat_RGBA8;
		m_textureDataType = TextureDataType::TextureDataType_UnsignedByte;

		switch(m_materialType)
		{
			case MaterialType_Normal:
				m_enableCompression = Config::textureVar().texture_normal_compression;
				m_enableDownsampling = Config::textureVar().texture_downsample;
				m_enableMipmap = Config::textureVar().generate_mipmaps;
				break;
			case MaterialType_Noise:
			case MaterialType_Data:
				m_enableCompression = false;
				m_enableDownsampling = false;
				m_enableMipmap = false;
				break;
			case MaterialType_Diffuse:
			case MaterialType_Emissive:
			case MaterialType_Combined:
			default:
				m_enableCompression = Config::textureVar().texture_compression;
				m_enableDownsampling = Config::textureVar().texture_downsample;
				m_enableMipmap = Config::textureVar().generate_mipmaps;
				break;
		}

		setEnableMipmapping(Config::textureVar().generate_mipmaps);
	}

	// Loads pixel data (using the filename) from HDD to RAM, re-factors it
	ErrorCode loadToMemory()
	{
		ErrorCode returnError = ErrorCode::Success;

		// If the same texture is being used in multiple objects, loadToMemory might be called
		// from multiple threads at the same time. Use a spin wait to deal with it
		SpinWait::Lock lock(m_mutex);

		// Texture might have already been loaded when called from a different thread. Check if it was
		if(!isLoadedToMemory())
		{
			// Read the format of the texture
			FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType((Config::PathsVariables().texture_path + m_filename).c_str(), 0);
			
			// Read the actual texture
			m_bitmap = FreeImage_Load(imageFormat, (Config::PathsVariables().texture_path + m_filename).c_str());
			
			if(m_bitmap)
			{
				// Calculate the number of bytes per pixel
				const unsigned int bytesPerPixel = FreeImage_GetLine(m_bitmap) / FreeImage_GetWidth(m_bitmap);
				// Calculate the number of samples per pixel
				const unsigned int samplesPerPixel = bytesPerPixel / sizeof(BYTE);

				// Only supporting 24bits or 32bits per pixel
				if(samplesPerPixel == 3)
				{
					// 3 color channels
					m_textureFormat = TextureFormat::TextureFormat_RGB;

					// Save the original bitmap
					auto *oldBitmap = m_bitmap;

					// Convert to 24 bits for RGB
					m_bitmap = FreeImage_ConvertTo24Bits(oldBitmap);

					//Unload the original bitmap
					FreeImage_Unload(oldBitmap);

					// Should the texture compression be used
					if(m_enableCompression && Config::textureVar().texture_compression)
					{
						// Assign texture compression method based on the material type
						switch(m_materialType)
						{
							case MaterialType_Normal:
								m_textureDataFormat = s_textureCompressionFormatNormal;
								break;
							case MaterialType_Noise:
							case MaterialType_Data:
								m_textureDataFormat = TextureDataFormat::TextureDataFormat_RGB8;
								break;
							case MaterialType_Diffuse:
							case MaterialType_Emissive:
							case MaterialType_Combined:
							default:
								m_textureDataFormat = s_textureCompressionFormatRGB;
								break;
						}
					}
					else
						m_textureDataFormat = TextureDataFormat::TextureDataFormat_RGB8;	// No texture compression
				}
				else
				{
					// 4 color channels
					m_textureFormat = TextureFormat::TextureFormat_RGBA;

					// Save the original bitmap
					auto *oldBitmap = m_bitmap;

					// Convert to 24 bits for RGB
					m_bitmap = FreeImage_ConvertTo32Bits(oldBitmap);

					//Unload the original bitmap
					FreeImage_Unload(oldBitmap);

					// Should the texture compression be used
					if(m_enableCompression && Config::textureVar().texture_compression)
					{
						// Assign texture compression method based on the material type
						switch(m_materialType)
						{
							case MaterialType_Normal:
								m_textureDataFormat = s_textureCompressionFormatNormal;
								break;
							case MaterialType_Noise:
							case MaterialType_Data:
								m_textureDataFormat = TextureDataFormat::TextureDataFormat_RGBA8;
								break;
							case MaterialType_Diffuse:
							case MaterialType_Emissive:
							case MaterialType_Combined:
							default:
								m_textureDataFormat = s_textureCompressionFormatRGBA;
								break;
						}
					}
					else
						m_textureDataFormat = TextureDataFormat::TextureDataFormat_RGBA8;	// No texture compression
				}

				// Get texture width, height
				m_textureWidth = FreeImage_GetWidth(m_bitmap);
				m_textureHeight = FreeImage_GetHeight(m_bitmap);

				// Calculate texture size
				m_size = m_textureWidth * m_textureHeight;

				if(m_enableDownsampling)
				{
					unsigned int downsampleScale = 1;

					if(Config::textureVar().texture_downsample_scale > 1)
					{
						// Set the defined scale
						downsampleScale = (unsigned int)Config::textureVar().texture_downsample_scale;

						// Downsample the texture
						//auto *newBitmap = FreeImage_Rescale(m_bitmap, m_textureWidth / downsampleScale, m_textureHeight / downsampleScale);

						//// Calculate new texture width, height and size
						//m_textureWidth = FreeImage_GetWidth(newBitmap);
						//m_textureHeight = FreeImage_GetHeight(newBitmap);

						//// Unload the original texture
						//FreeImage_Unload(m_bitmap);

						//// Set the downsampled texture as the current one
						//m_bitmap = newBitmap;
					}
					else
					{
						if(Config::textureVar().texture_downsample_max_resolution > 0)
						{
							const unsigned int maxResolution = (unsigned int)Config::textureVar().texture_downsample_max_resolution;

							// Check if texture resolution is higher than the maximum allowed
							if(m_textureWidth > maxResolution)
							{
								downsampleScale = m_textureWidth / maxResolution;
							}
							else
							{
								if(m_textureHeight > maxResolution)
								{
									downsampleScale = m_textureHeight / maxResolution;
								}
							}
						}
					}

					// If downsample is scale is higher than 1, perform the downsample on the texture
					if(downsampleScale > 1)
					{
						// Downsample the texture
						auto *newBitmap = FreeImage_Rescale(m_bitmap, m_textureWidth / downsampleScale, m_textureHeight / downsampleScale);

						// Unload the original texture
						FreeImage_Unload(m_bitmap);

						// Set the downsampled texture as the current one
						m_bitmap = newBitmap;

						// Recalculate texture dimensions and size
						m_textureWidth = FreeImage_GetWidth(m_bitmap);
						m_textureHeight = FreeImage_GetHeight(m_bitmap);
						m_size = m_textureWidth * m_textureHeight;
					}
				}

				// Texture data passed to the GPU must be in an unsigned char array format
				auto pixelData = FreeImage_GetBits(m_bitmap);

				// Define number of channels per pixel
				const unsigned int numChan = m_textureFormat == TextureFormat_RGB ? 3 : 4;

				// Temp variable for swapping color channels
				unsigned char blue = 0;

				// FreeImage loads in BGR format, therefore swap of bytes is needed (Or usage of GL_BGR)
				for(unsigned int i = 0; i < m_size; i++)
				{
					blue = pixelData[i * numChan + 0];						 // Store blue
					pixelData[i * numChan + 0] = pixelData[i * numChan + 2]; // Set red
					pixelData[i * numChan + 2] = blue;						 // Set blue
				}

				m_pixelData = pixelData;

				//if(m_materialType == MaterialType_Normal && samplesPerPixel == 3)
				//{
				//	m_textureFormat = TextureFormat::TextureFormat_RG;
				//	auto *oldPixelData = pixelData;
				//	unsigned char *newData = new unsigned char[m_size * 2];

				//	for(unsigned int i = 0; i < m_size; i++)
				//	{
				//		pixelData[i * 2 + 0] = oldPixelData[i * samplesPerPixel + 0];
				//		pixelData[i * 2 + 1] = oldPixelData[i * samplesPerPixel + 1];
				//	}

				//	m_pixelData = newData;
				//}

				setLoadedToMemory(true);
				m_loadedFromFile = true;
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
		if(m_bitmap != nullptr)
		{
			FreeImage_Unload(m_bitmap);
			m_bitmap = nullptr;
			m_pixelData = nullptr;
		}

		return returnError;
	}

	// Set whether the mipmapping is enabled. Also sets the appropriate magnification and minification filtering
	void setEnableMipmapping(const bool p_enableCompression)
	{
		m_enableMipmap = p_enableCompression;

		if(m_enableMipmap)
		{
			m_magnificationFilter = convertToTextureFilterType(Config::textureVar().gl_texture_magnification_mipmap);
			m_minificationFilter = convertToTextureFilterType(Config::textureVar().gl_texture_minification_mipmap);
		}
		else
		{
			m_magnificationFilter = convertToTextureFilterType(Config::textureVar().gl_texture_magnification);
			m_minificationFilter = convertToTextureFilterType(Config::textureVar().gl_texture_minification);
		}
	}

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
					((GLubyte *)m_pixelData)[(destWidth * m_textureWidth + destHeight) * 4 + p_destChannel] =
						p_pixelData[(srcWidth * p_textureWidth + srcHeight) * 4 + p_sourceChannel];
				}
			}
		}
	}

	// Returns true if the texture handle has been assigned (i.e. not 0), meaning the texture was loaded to GPU
	const inline bool handleAssigned() const { return (m_handle != 0); }

	// Returns a void pointer to the pixel data
	const inline void *getData() { return (void*)m_pixelData; }

protected:
	inline static TextureDataFormat s_textureCompressionFormatRGB = TextureDataFormat::TextureDataFormat_RGB8;
	inline static TextureDataFormat s_textureCompressionFormatRGBA = TextureDataFormat::TextureDataFormat_RGBA8;
	inline static TextureDataFormat s_textureCompressionFormatNormal = TextureDataFormat::TextureDataFormat_RGB8;

	MaterialType m_materialType;
	TextureFormat m_textureFormat;
	TextureDataFormat m_textureDataFormat;
	TextureDataType m_textureDataType;
	TextureFilterType m_magnificationFilter;
	TextureFilterType m_minificationFilter;
	bool m_loadedFromFile;
	bool m_enableCompression;
	bool m_enableDownsampling;
	bool m_enableMipmap;
	int m_mipmapLevel;

	FIBITMAP* m_bitmap;
	void *m_pixelData;
	unsigned int m_size,
				 m_handle,
				 m_textureWidth,
				 m_textureHeight;
};
class TextureLoader2D : public LoaderBase<TextureLoader2D, Texture2D>
{
	friend class RendererFrontend;
public:
	class Texture2DHandle
	{
		friend struct LoadableObjectsContainer;
		friend class CommandBuffer;
		friend class TextureLoader2D;
		friend class RendererFrontend;
	public:
		// Increment the reference counter when creating a handle
		Texture2DHandle(const Texture2DHandle &p_textureHandle) : m_textureData(p_textureHandle.m_textureData) { m_textureData->incRefCounter(); }
		Texture2DHandle(Texture2DHandle &&p_textureHandle) noexcept : m_textureData(p_textureHandle.m_textureData) { m_textureData->incRefCounter(); }
		~Texture2DHandle() { m_textureData->decRefCounter(); }

		// Loads data from HDD to RAM and restructures it to be used to fill buffers later
		ErrorCode loadToMemory()
		{
			ErrorCode returnError = ErrorCode::Success;

			// If it's not loaded to memory already, call load
			if(!m_textureData->isLoadedToMemory())
				returnError = m_textureData->loadToMemory();

			return returnError;
		}

		// Deletes pixel data stored in RAM. Does not delete the texture that is loaded on GPU VRAM
		ErrorCode unloadFromMemory()
		{
			return m_textureData->unloadMemory();
		}

		void setColorChannel(const Texture2DHandle &p_textureHandle, TextureColorChannelOffset p_destChannel, TextureColorChannelOffset p_sourceChannel = ColorOffset_Red)
		{
			// If both textures have pixel data loaded
			if(m_textureData->m_pixelData && p_textureHandle.m_textureData->m_pixelData)
			{
				// Set the color channel of the internal texture
				m_textureData->setColorChannel((GLubyte *)p_textureHandle.m_textureData->m_pixelData,
											   p_textureHandle.m_textureData->m_textureWidth,
											   p_textureHandle.m_textureData->m_textureHeight,
											   p_destChannel, p_sourceChannel);

				// Assign a new filename, since the texture data was modified
				m_textureData->m_filename += ", " + p_textureHandle.m_textureData->m_filename;
			}
		}

		// Copy assignment operator
		Texture2DHandle &operator=(const Texture2DHandle &p_textureHandle)
		{
			m_textureData = p_textureHandle.m_textureData;
			m_textureData->incRefCounter();
			return *this;
		}		
		
		// Move assignment operator
		Texture2DHandle &operator=(Texture2DHandle &&p_textureHandle) noexcept
		{
			if(this == &p_textureHandle)
				return *this;

			m_textureData = p_textureHandle.m_textureData;
			m_textureData->incRefCounter();

			return *this;
		}

		// Has the texture been loaded to memory (system RAM)
		const inline bool isLoadedToMemory() const { return m_textureData->isLoadedToMemory(); }

		// Has the texture been already loaded to video memory (GPU VRAM)
		const inline bool isLoadedToVideoMemory() const { return m_textureData->isLoadedToVideoMemory(); }

		// Has the texture been queued to be loaded to video memory (GPU VRAM)
		inline const bool isQueuedLoadToVideoMemory() const { return m_textureData->isQueuedLoadToVideoMemory(); }

		// Getters
		inline unsigned int getTextureHeight() const { return m_textureData->m_textureHeight; }
		inline unsigned int getTextureWidth() const { return m_textureData->m_textureWidth; }
		inline unsigned int getHandle() const { return m_textureData->m_handle; }
		inline int getMipmapLevel() const { return m_textureData->m_mipmapLevel; }
		inline std::string &getFilename() const { return m_textureData->m_filename; }
		inline TextureFormat getTextureFormat() const { return m_textureData->m_textureFormat; }
		inline TextureDataFormat getTextureDataFormat() const { return m_textureData->m_textureDataFormat; }
		inline TextureDataType getTextureDataType() const { return m_textureData->m_textureDataType; }
		inline TextureFilterType getMagnificationFilterType() const { return m_textureData->m_magnificationFilter; }
		inline TextureFilterType getMinificationFilterType() const { return m_textureData->m_minificationFilter; }
		inline bool getEnableMipmap() const { return m_textureData->m_enableMipmap; }
		inline const void *getPixelData() const { return m_textureData->m_pixelData; }
		const inline bool isLoadedFromFile() const { return m_textureData->m_loadedFromFile; }

		// Setters
		inline void setEnableCompression(const bool p_enableCompression) { m_textureData->m_enableCompression = p_enableCompression; }
		inline void setEnableDownsampling(const bool p_enableDownsampling) { m_textureData->m_enableDownsampling = p_enableDownsampling; }
		inline void setEnableMipmapping(const bool p_enableMipmapping) { m_textureData->setEnableMipmapping(p_enableMipmapping); }
		inline void setPixelData(void *p_pixelData) { m_textureData->setPixelData(p_pixelData); }
		inline void setHandle(unsigned int p_handle) { m_textureData->m_handle = p_handle; }
		inline void setMagnificationFilterType(const TextureFilterType p_filterType) { m_textureData->m_magnificationFilter = p_filterType; }
		inline void setMinificationFilterType(const TextureFilterType p_filterType) { m_textureData->m_minificationFilter = p_filterType; }

	private:
		// Increment the reference counter when creating a handle
		Texture2DHandle(Texture2D *p_textureData) : m_textureData(p_textureData) { m_textureData->incRefCounter(); }

		// Setters
		inline void setLoadedToMemory(bool p_loaded) { m_textureData->setLoadedToMemory(p_loaded); }
		inline void setLoadedToVideoMemory(bool p_loaded) { m_textureData->setLoadedToVideoMemory(p_loaded); }

		inline unsigned int &getHandleRef() { return m_textureData->m_handle; }

		// Returns a void pointer to the pixel data
		const inline void *getData() { return m_textureData->getData(); }

		//unsigned int m_handle;
		Texture2D *m_textureData;
	};
		
	TextureLoader2D();
	~TextureLoader2D();

	ErrorCode init();

	Texture2DHandle load(const std::string &p_filename, MaterialType p_materialType, bool p_startBackgroundLoading = true);
	Texture2DHandle create(const std::string &p_name, const unsigned int p_width, const unsigned int p_height, const TextureFormat p_textureFormat, const TextureDataFormat p_textureDataFormat, const TextureDataType p_textureDataType, const bool p_createMipmap = false, const void *p_data = NULL);

	Texture2DHandle getDefaultTexture(MaterialType p_materialType = MaterialType::MaterialType_Diffuse)
	{
		Texture2D *returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse];

		switch(p_materialType)
		{
		case MaterialType_Diffuse:
			break;
		case MaterialType_Normal:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Normal];
			break;
		case MaterialType_Emissive:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Emissive];
			break;
		case MaterialType_Combined:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_RMHA];
			break;
		case MaterialType_Roughness:
			break;
		case MaterialType_Data:
			break;
		case MaterialType_Metalness:
			break;
		case MaterialType_Height:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Height];
			break;
		case MaterialType_AmbientOcclusion:
			break;
		case MaterialType_NumOfTypes_Extended:
			break;
		default:
			break;
		}

		return Texture2DHandle(returnTexture);
	}

	inline bool isTextureDefault(const Texture2DHandle &p_textureHandle) const 
	{
		for(unsigned int i = 0; i < DefaultTextureType::DefaultTextureType_NumOfTypes; i++)
			if(p_textureHandle.m_textureData == m_defaultTextures[i])
				return true;

		return false;
	}

protected:
	enum DefaultTextureType : unsigned int
	{
		DefaultTextureType_Diffuse = 0,
		DefaultTextureType_Emissive,
		DefaultTextureType_Height,
		DefaultTextureType_Normal,
		DefaultTextureType_RMHA,
		DefaultTextureType_NumOfTypes
	};

	// Convert an int to TextureDataFormat enum. If no enum is matched, TextureDataFormat_RGBA8 is returned
	TextureDataFormat intToTextureDataFormat(const int p_format)
	{
		switch(p_format)
		{
			case TextureDataFormat::TextureDataFormat_R8:
				return TextureDataFormat::TextureDataFormat_R8;
			case TextureDataFormat::TextureDataFormat_R16:
				return TextureDataFormat::TextureDataFormat_R16;
			case TextureDataFormat::TextureDataFormat_R16F:
				return TextureDataFormat::TextureDataFormat_R16F;
			case TextureDataFormat::TextureDataFormat_R32F:
				return TextureDataFormat::TextureDataFormat_R32F;
			case TextureDataFormat::TextureDataFormat_RG8:
				return TextureDataFormat::TextureDataFormat_RG8;
			case TextureDataFormat::TextureDataFormat_RG16:
				return TextureDataFormat::TextureDataFormat_RG16;
			case TextureDataFormat::TextureDataFormat_RG16F:
				return TextureDataFormat::TextureDataFormat_RG16F;
			case TextureDataFormat::TextureDataFormat_RG32F:
				return TextureDataFormat::TextureDataFormat_RG32F;
			case TextureDataFormat::TextureDataFormat_RGB8:
				return TextureDataFormat::TextureDataFormat_RGB8;
			case TextureDataFormat::TextureDataFormat_RGB16:
				return TextureDataFormat::TextureDataFormat_RGB16;
			case TextureDataFormat::TextureDataFormat_RGB16F:
				return TextureDataFormat::TextureDataFormat_RGB16F;
			case TextureDataFormat::TextureDataFormat_RGB32F:
				return TextureDataFormat::TextureDataFormat_RGB32F;
			case TextureDataFormat::TextureDataFormat_RGBA8:
				return TextureDataFormat::TextureDataFormat_RGBA8;
			case TextureDataFormat::TextureDataFormat_RGBA16:
				return TextureDataFormat::TextureDataFormat_RGBA16;
			case TextureDataFormat::TextureDataFormat_RGBA16SN:
				return TextureDataFormat::TextureDataFormat_RGBA16SN;
			case TextureDataFormat::TextureDataFormat_RGBA16F:
				return TextureDataFormat::TextureDataFormat_RGBA16F;
			case TextureDataFormat::TextureDataFormat_RGBA32F:
				return TextureDataFormat::TextureDataFormat_RGBA32F;
			case TextureDataFormat::TextureDataFormat_R16I:
				return TextureDataFormat::TextureDataFormat_R16I;
			case TextureDataFormat::TextureDataFormat_R32I:
				return TextureDataFormat::TextureDataFormat_R32I;
			case TextureDataFormat::TextureDataFormat_R16UI:
				return TextureDataFormat::TextureDataFormat_R16UI;
			case TextureDataFormat::TextureDataFormat_R32UI:
				return TextureDataFormat::TextureDataFormat_R32UI;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGB:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_RGB;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGBA:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_RGBA;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGB:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGB;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGBA:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_DXT1_RGBA;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT3_RGBA:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_DXT3_RGBA;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_DXT5_RGBA:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_DXT5_RGBA;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC1_R:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC1_R;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC2_RG:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC2_RG;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_BPTC_RGBA:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_BPTC_RGBA;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_R:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_R;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_RG:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_EAC_RG;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGB:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGB;
			case TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGBA:
				return TextureDataFormat::TextureDataFormat_COMPRESSED_ETC2_RGBA;	
		}

		return TextureDataFormat::TextureDataFormat_RGBA8;
	}

	void unload(Texture2D &p_object, SceneLoader &p_sceneLoader);

	// Returns a vector with all default 2D textures
	// Meant to be called during initialization to load the 
	// default textures to GPU before starting to load a scene
	std::vector<TextureLoader2D::Texture2DHandle> getDefaultTextures()
	{
		std::vector<TextureLoader2D::Texture2DHandle> returnVector;

		// Make sure to only return the textures that haven't been loaded
		if(!m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Diffuse]->m_textureData->handleAssigned())
			returnVector.push_back(*m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Diffuse]);

		if(!m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Emissive]->m_textureData->handleAssigned())
			returnVector.push_back(*m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Emissive]);

		if(!m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Height]->m_textureData->handleAssigned())
			returnVector.push_back(*m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Height]);

		if(!m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Normal]->m_textureData->handleAssigned())
			returnVector.push_back(*m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_Normal]);

		return returnVector;
	}

	// Default textures used in place of missing ones when loading textures
	Texture2D *m_defaultTextures[DefaultTextureType::DefaultTextureType_NumOfTypes];

	// Handles for the default textures are held so that the reference counter for the default texture do not reach 0
	Texture2DHandle *m_defaultTextureHandles[DefaultTextureType::DefaultTextureType_NumOfTypes];
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
		m_size = 0;
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
		m_size = 0;
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
		if(!isLoadedToMemory())
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
			if(!m_textureData->isLoadedToMemory())
			{
				// Load texture to memory (RAM)
				returnError = m_textureData->loadToMemory();

				// Only set the loaded to video memory flag to false if the texture has been loaded successfully,
				// otherwise, flag it as loaded to GPU VRAM already, so to not attempt to load to VRAM
				/*if(returnError == ErrorCode::Success)
					m_textureData->setLoadedToVideoMemory(false);
				else
					m_textureData->setLoadedToVideoMemory(true);*/
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

protected:
	void unload(TextureCubemap &p_object)
	{
	}
};