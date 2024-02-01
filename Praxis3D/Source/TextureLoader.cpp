
#include <functional>

#include "ErrorHandlerLocator.h"
#include "Filesystem.h"
#include "ModelLoader.h"
#include "SceneLoader.h"
#include "TaskManagerLocator.h"
#include "TextureLoader.h"
#include "Utilities.h"


TextureLoader2D::TextureLoader2D()
{
	m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_texture, m_objectPool.size(), 0);
	m_defaultTextures[DefaultTextureType::DefaultTextureType_Emissive] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_emissive_texture, m_objectPool.size(), 0);
	m_defaultTextures[DefaultTextureType::DefaultTextureType_Height] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_height_texture, m_objectPool.size(), 0);
	m_defaultTextures[DefaultTextureType::DefaultTextureType_Normal] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_normal_texture, m_objectPool.size(), 0);
	m_defaultTextures[DefaultTextureType::DefaultTextureType_RMHA] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_RMHA_texture, m_objectPool.size(), 0);

	for(unsigned int i = 0; i < DefaultTextureType::DefaultTextureType_NumOfTypes; i++)
		m_defaultTextureHandles[i] = nullptr;
}

TextureLoader2D::~TextureLoader2D()
{
	for(unsigned int i = 0; i < DefaultTextureType::DefaultTextureType_NumOfTypes; i++)
	{
		delete m_defaultTextureHandles[i];
		delete m_defaultTextures[i];
	}
}

ErrorCode TextureLoader2D::init()
{
	// If the default texture filename changed upon loading the configuration
	if(m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse]->m_filename != Config::filepathVar().engine_assets_path + Config::textureVar().default_texture)
	{
		delete m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse];
		m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_texture, m_objectPool.size(), 0);
	}
	if(m_defaultTextures[DefaultTextureType::DefaultTextureType_Emissive]->m_filename != Config::filepathVar().engine_assets_path + Config::textureVar().default_emissive_texture)
	{
		delete m_defaultTextures[DefaultTextureType::DefaultTextureType_Emissive];
		m_defaultTextures[DefaultTextureType::DefaultTextureType_Emissive] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_emissive_texture, m_objectPool.size(), 0);
	}
	if(m_defaultTextures[DefaultTextureType::DefaultTextureType_Height]->m_filename != Config::filepathVar().engine_assets_path + Config::textureVar().default_height_texture)
	{
		delete m_defaultTextures[DefaultTextureType::DefaultTextureType_Height];
		m_defaultTextures[DefaultTextureType::DefaultTextureType_Height] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_height_texture, m_objectPool.size(), 0);
	}
	if(m_defaultTextures[DefaultTextureType::DefaultTextureType_Normal]->m_filename != Config::filepathVar().engine_assets_path + Config::textureVar().default_normal_texture)
	{
		delete m_defaultTextures[DefaultTextureType::DefaultTextureType_Normal];
		m_defaultTextures[DefaultTextureType::DefaultTextureType_Normal] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_normal_texture, m_objectPool.size(), 0);
	}
	if(m_defaultTextures[DefaultTextureType::DefaultTextureType_RMHA]->m_filename != Config::filepathVar().engine_assets_path + Config::textureVar().default_RMHA_texture)
	{
		delete m_defaultTextures[DefaultTextureType::DefaultTextureType_RMHA];
		m_defaultTextures[DefaultTextureType::DefaultTextureType_RMHA] = new Texture2D(this, Config::filepathVar().engine_assets_path + Config::textureVar().default_RMHA_texture, m_objectPool.size(), 0);
	}
	
	// Load default textures
	for(unsigned int i = 0; i < DefaultTextureType::DefaultTextureType_NumOfTypes; i++)
		if(const auto error = m_defaultTextures[i]->loadToMemory(); error != ErrorCode::Success)
			ErrHandlerLoc::get().log(error, m_defaultTextures[i]->getFilename(), ErrorSource::Source_TextureLoader);

	// Create texture handles for the default textures
	for(unsigned int i = 0; i < DefaultTextureType::DefaultTextureType_NumOfTypes; i++)
		m_defaultTextureHandles[i] = new Texture2DHandle(m_defaultTextures[i]);

	return ErrorCode::Success;
}

TextureLoader2D::Texture2DHandle TextureLoader2D::load(const std::string &p_filename, MaterialType p_materialType, bool p_startBackgroundLoading)
{
	Texture2D *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// If the filename is empty, or the file itself doesn't exist, return a default texture instead
	if(p_filename.empty() || !Filesystem::exists(Config::filepathVar().texture_path + p_filename))
	{
		// If the filename wasn't empty, log an error
		if(!p_filename.empty())
			ErrHandlerLoc::get().log(ErrorCode::Texture_not_found, ErrorSource::Source_TextureLoader, p_filename);

		switch(p_materialType)
		{
		case MaterialType_Normal:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Normal];
			break;
		case MaterialType_Emissive:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Emissive];
			break;
		case MaterialType_Height:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Height];
			break;
		case MaterialType_Combined:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_RMHA];
			break;
		case MaterialType_Diffuse:
		case MaterialType_Roughness:
		case MaterialType_Metalness:
		case MaterialType_AmbientOcclusion:
		default:
			returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse];
			break;
		}
	}
	else
	{
		// Assign an appropriate default texture
		unsigned int defaultTextureHandle = m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse]->m_handle;
		switch(p_materialType)
		{
		case MaterialType_Normal:
			defaultTextureHandle = m_defaultTextures[DefaultTextureType::DefaultTextureType_Normal]->m_handle;
			break;
		case MaterialType_Emissive:
			defaultTextureHandle = m_defaultTextures[DefaultTextureType::DefaultTextureType_Emissive]->m_handle;
			break;
		case MaterialType_Height:
			defaultTextureHandle = m_defaultTextures[DefaultTextureType::DefaultTextureType_Height]->m_handle;
			break;
		case MaterialType_Combined:
			defaultTextureHandle = m_defaultTextures[DefaultTextureType::DefaultTextureType_RMHA]->m_handle;
			break;
		}

		// Go through the texture pool and check if the texture hasn't been already loaded (to avoid duplicates)
		for(decltype(m_objectPool.size()) size = m_objectPool.size(), i = 0; i < size; i++)
		{
			if(*m_objectPool[i] == p_filename)
				return Texture2DHandle(m_objectPool[i]);
		}

		// Texture wasn't loaded before, so create a new one
		// Assign default handle (as a placeholder to be used before the texture is loaded from HDD)
		returnTexture = new Texture2D(this, p_filename, m_objectPool.size(), defaultTextureHandle);

		if(p_startBackgroundLoading)
		{
			// Start loading the texture from file, in a background thread
			TaskManagerLocator::get().startBackgroundThread(
				std::bind((ErrorCode(Texture2D::*)(void))&Texture2D::loadToMemory, returnTexture));

			//TaskManagerLocator::get().startBackgroundThread(std::bind(&Texture2D::loadToMemory, returnTexture));
		}

		// Add the new texture to the list
		m_objectPool.push_back(returnTexture);
	}

	// Return the new texture
	return Texture2DHandle(returnTexture);
}

TextureLoader2D::Texture2DHandle TextureLoader2D::load(const std::string &p_filename, unsigned int p_textureHandle)
{
	Texture2D *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// If the filename is empty, or the file itself doesn't exist, return a default texture instead
	if(p_filename.empty() || !Filesystem::exists(Config::PathsVariables().texture_path + p_filename))
	{
		// If the filename wasn't empty, log an error
		if(!p_filename.empty())
			ErrHandlerLoc::get().log(ErrorCode::Texture_not_found, ErrorSource::Source_TextureLoader, p_filename);

		returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse];
	}
	else
	{
		// Go through the texture pool and check if the texture hasn't been already loaded (to avoid duplicates)
		for(decltype(m_objectPool.size()) size = m_objectPool.size(), i = 0; i < size; i++)
		{
			if(*m_objectPool[i] == p_filename && m_objectPool[i]->m_handle == p_textureHandle)
				return Texture2DHandle(m_objectPool[i]);
		}

		returnTexture = new Texture2D(this, p_filename, m_objectPool.size(), p_textureHandle);

		// Set the loaded flag to true, because we have already provided the texture handle
		returnTexture->setLoadedToVideoMemory(true);

		// Add the new texture to the list
		m_objectPool.push_back(returnTexture);
	}

	// Return the new texture
	return Texture2DHandle(returnTexture);
}

TextureLoader2D::Texture2DHandle TextureLoader2D::load(const std::string &p_filename)
{	
	Texture2D *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// If the filename is empty, or the file itself doesn't exist, return a default texture instead
	if(p_filename.empty() || !Filesystem::exists(Config::PathsVariables().texture_path + p_filename))
	{
		// If the filename wasn't empty, log an error
		if(!p_filename.empty())
			ErrHandlerLoc::get().log(ErrorCode::Texture_not_found, ErrorSource::Source_TextureLoader, p_filename);

		returnTexture = m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse];
	}
	else
	{
		// Go through the texture pool and check if the texture hasn't been already loaded (to avoid duplicates)
		for(decltype(m_objectPool.size()) size = m_objectPool.size(), i = 0; i < size; i++)
		{
			if(*m_objectPool[i] == p_filename)
				return Texture2DHandle(m_objectPool[i]);
		}

		// Texture wasn't loaded before, so create a new one
		// Assign default handle (as a placeholder to be used before the texture is loaded from HDD)
		returnTexture = new Texture2D(this, p_filename, m_objectPool.size(), m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse]->m_handle);

		// Set the loaded flag to true, because we have already provided the texture handle
		returnTexture->setLoadedToVideoMemory(true);

		// Add the new texture to the list
		m_objectPool.push_back(returnTexture);
	}

	// Return the new texture
	return Texture2DHandle(returnTexture);
}

TextureLoader2D::Texture2DHandle TextureLoader2D::create(const std::string &p_name, const unsigned int p_width, const unsigned int p_height, const TextureFormat p_textureFormat, const TextureDataFormat p_textureDataFormat, const TextureDataType p_textureDataType, const bool p_createMipmap, const void *p_data)
{
	Texture2D *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// Go through the texture pool and check if the texture hasn't been already loaded (to avoid duplicates)
	for(decltype(m_objectPool.size()) size = m_objectPool.size(), i = 0; i < size; i++)
	{
		if(*m_objectPool[i] == p_name)
			return Texture2DHandle(m_objectPool[i]);
	}

	// Texture wasn't loaded before, so create a new one
	// Assign default handle (as a placeholder to be used before the texture is loaded from HDD)
	returnTexture = new Texture2D(this, p_name, m_objectPool.size(), m_defaultTextures[DefaultTextureType::DefaultTextureType_Diffuse]->m_handle);

	returnTexture->m_textureWidth = p_width;
	returnTexture->m_textureHeight = p_height;
	returnTexture->m_textureFormat = p_textureFormat;
	returnTexture->m_textureDataFormat = p_textureDataFormat;
	returnTexture->m_textureDataType = p_textureDataType;
	returnTexture->m_enableMipmap = p_createMipmap;
	returnTexture->m_pixelData = (unsigned char*)p_data;

	returnTexture->setLoadedToMemory(true);

	// Add the new texture to the list
	m_objectPool.push_back(returnTexture);

	// Return the new texture
	return Texture2DHandle(returnTexture);
}

void TextureLoader2D::unload(Texture2D &p_object, SceneLoader &p_sceneLoader)
{
	// Get the texture handle
	unsigned int *textureHandle = new unsigned int(p_object.m_handle);

	// Send a notification to graphics scene to unload the texture; set deleteAfterReceiving flag to true, to transfer the ownership of the texture handle pointer to the graphics scene (so it will be responsible for deleting it)
	p_sceneLoader.getChangeController()->sendData(p_sceneLoader.getSystemScene(Systems::Graphics), DataType::DataType_UnloadTexture2D, (void*)textureHandle, true);
}

TextureLoaderCubemap::TextureLoaderCubemap()
{
	std::string defaultFilenames[CubemapFace_NumOfFaces];
	for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		defaultFilenames[face] = Config::filepathVar().engine_assets_path + Config::textureVar().default_texture;

	m_defaultCubemap = new TextureCubemap(this, Config::textureVar().default_texture, defaultFilenames, m_objectPool.size(), 0);
}

TextureLoaderCubemap::~TextureLoaderCubemap()
{
}

ErrorCode TextureLoaderCubemap::init()
{
	// If the default texture filename changed upon loading the configuration
	// (don't need to compare all filenames, since the default cubemap will have the same texture for all faces)
	if(m_defaultCubemap->m_filenames[CubemapFace_PositiveX] != Config::filepathVar().engine_assets_path + Config::textureVar().default_texture)
	{
		delete m_defaultCubemap;

		std::string defaultFilenames[CubemapFace_NumOfFaces];
		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
			defaultFilenames[face] = Config::filepathVar().engine_assets_path + Config::textureVar().default_texture;

		m_defaultCubemap = new TextureCubemap(this, Config::textureVar().default_texture, defaultFilenames, m_objectPool.size(), 0);
	}

	// Load default texture to memory and video memory
	m_defaultCubemap->loadToMemory();
	//m_defaultCubemap->loadToVideoMemory();

	// Add default texture to the texture pool
	m_objectPool.push_back(m_defaultCubemap);

	return ErrorCode::Success;
}

TextureLoaderCubemap::TextureCubemapHandle TextureLoaderCubemap::load(const std::string &p_filenamePosX, const std::string &p_filenameNegX, const std::string &p_filenamePosY, const std::string &p_filenameNegY, const std::string &p_filenamePosZ, const std::string &p_filenameNegZ, bool p_startBackgroundLoading)
{
	std::string filenames[CubemapFace_NumOfFaces];

	filenames[CubemapFace_PositiveX] = p_filenamePosX;
	filenames[CubemapFace_NegativeX] = p_filenameNegX;
	filenames[CubemapFace_PositiveY] = p_filenamePosY;
	filenames[CubemapFace_NegativeY] = p_filenameNegY;
	filenames[CubemapFace_PositiveZ] = p_filenamePosZ;
	filenames[CubemapFace_NegativeZ] = p_filenameNegZ;

	return load(filenames, p_startBackgroundLoading);
}

TextureLoaderCubemap::TextureCubemapHandle TextureLoaderCubemap::load(const std::string(&p_filenames)[CubemapFace_NumOfFaces], bool p_startBackgroundLoading)
{
	TextureCubemap *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// If any of the filenames are empty, return a default texture instead
	for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		if(p_filenames[face].empty())
			return m_defaultCubemap;

	// Combine the filenames of individual faces into one
	std::string combinedFilename;
	for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		combinedFilename += p_filenames[face] + ", ";

	// Remove the last comma and space 
	combinedFilename.pop_back();
	combinedFilename.pop_back();

	// Go through the texture pool and check if the texture hasn't been already loaded (to avoid duplicates)
	for(decltype(m_objectPool.size()) size = m_objectPool.size(), i = 0; i < size; i++)
	{
		if(*m_objectPool[i] == combinedFilename)
			return TextureCubemapHandle(m_objectPool[i]);
	}

	// Texture wasn't loaded before, so create a new one
	// Assign default handle (as a placeholder to be used before the texture is loaded from HDD)
	returnTexture = new TextureCubemap(this, combinedFilename, p_filenames, m_objectPool.size(), m_defaultCubemap->m_handle);

	if(p_startBackgroundLoading)
	{
		// Start loading the texture from file, in a background thread
		TaskManagerLocator::get().startBackgroundThread(
			std::bind((ErrorCode(TextureCubemap::*)(void))&TextureCubemap::loadToMemory, returnTexture));
	}

	// Add the new texture to the list
	m_objectPool.push_back(returnTexture);

	// Return the new texture
	return TextureCubemapHandle(returnTexture);
}

TextureLoaderCubemap::TextureCubemapHandle TextureLoaderCubemap::load(const std::string &p_filename, unsigned int p_textureHandle)
{
	TextureCubemap *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// If the filename is empty, return a default texture instead
	if(p_filename == "")
	{
		returnTexture = m_defaultCubemap;
	}
	else
	{
		// Go through the texture pool and check if the texture hasn't been already loaded (to avoid duplicates)
		for(decltype(m_objectPool.size()) size = m_objectPool.size(), i = 0; i < size; i++)
		{
			if(*m_objectPool[i] == p_filename && m_objectPool[i]->m_handle == p_textureHandle)
				return TextureCubemapHandle(m_objectPool[i]);
		}

		returnTexture = new TextureCubemap(this, p_filename, m_objectPool.size(), p_textureHandle);

		// Set the loaded flag to true, because we have already provided the texture handle
		returnTexture->setLoadedToVideoMemory(true);

		// Add the new texture to the list
		m_objectPool.push_back(returnTexture);
	}

	// Return the new texture
	return TextureCubemapHandle(returnTexture);
}
