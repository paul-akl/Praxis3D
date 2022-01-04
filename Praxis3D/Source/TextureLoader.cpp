
#include <functional>

#include "ErrorHandlerLocator.h"
#include "Filesystem.h"
#include "ModelLoader.h"
#include "TaskManagerLocator.h"
#include "TextureLoader.h"
#include "Utilities.h"


TextureLoader2D::TextureLoader2D()
{
	m_default2DTexture = new Texture2D(this, Config::textureVar().default_texture, m_objectPool.size(), 0);
	m_defaultEmissive = new Texture2D(this, Config::textureVar().default_emissive_texture, m_objectPool.size(), 0);
	m_defaultHeight = new Texture2D(this, Config::textureVar().default_height_texture, m_objectPool.size(), 0);
	m_defaultNormal = new Texture2D(this, Config::textureVar().default_normal_texture, m_objectPool.size(), 0);
}

TextureLoader2D::~TextureLoader2D()
{
}

ErrorCode TextureLoader2D::init()
{
	// If the default texture filename changed upon loading the configuration
	if(m_default2DTexture->m_filename != Config::textureVar().default_texture)
	{
		delete m_default2DTexture;
		m_default2DTexture = new Texture2D(this, Config::textureVar().default_texture, m_objectPool.size(), 0);
	}
	if(m_defaultEmissive->m_filename != Config::textureVar().default_emissive_texture)
	{
		delete m_defaultEmissive;
		m_defaultEmissive = new Texture2D(this, Config::textureVar().default_emissive_texture, m_objectPool.size(), 0);
	}
	if(m_defaultHeight->m_filename != Config::textureVar().default_height_texture)
	{
		delete m_defaultHeight;
		m_defaultHeight = new Texture2D(this, Config::textureVar().default_height_texture, m_objectPool.size(), 0);
	}
	if(m_defaultNormal->m_filename != Config::textureVar().default_normal_texture)
	{
		delete m_defaultNormal;
		m_defaultNormal = new Texture2D(this, Config::textureVar().default_normal_texture, m_objectPool.size(), 0);
	}
	
	// Load default textures to memory and video memory
	m_default2DTexture->loadToMemory();
	//m_default2DTexture->loadToVideoMemory();

	m_defaultEmissive->loadToMemory();
	//m_defaultEmissive->loadToVideoMemory();

	m_defaultHeight->loadToMemory();
	//m_defaultHeight->loadToVideoMemory();

	m_defaultNormal->loadToMemory();
	//m_defaultNormal->loadToVideoMemory();

	// Add default textures to the texture pool
	m_objectPool.push_back(m_default2DTexture);
	m_objectPool.push_back(m_defaultEmissive);
	m_objectPool.push_back(m_defaultHeight);
	m_objectPool.push_back(m_defaultNormal);

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
			returnTexture = m_defaultNormal;
			break;
		case MaterialType_Emissive:
			returnTexture = m_defaultEmissive;
			break;
		case MaterialType_Height:
			returnTexture = m_defaultHeight;
			break;
		case MaterialType_Diffuse:
		case MaterialType_Combined:
		case MaterialType_Roughness:
		case MaterialType_Metalness:
		case MaterialType_AmbientOcclusion:
		default:
			returnTexture = m_default2DTexture;
			break;
		}
	}
	else
	{
		// Assign an appropriate default texture
		unsigned int defaultTextureHandle = m_default2DTexture->m_handle;
		switch(p_materialType)
		{
		case MaterialType_Normal:
			defaultTextureHandle = m_defaultNormal->m_handle;
			break;
		case MaterialType_Emissive:
			defaultTextureHandle = m_defaultEmissive->m_handle;
			break;
		case MaterialType_Height:
			defaultTextureHandle = m_defaultHeight->m_handle;
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

		returnTexture = m_default2DTexture;
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

		returnTexture = m_default2DTexture;
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
		returnTexture = new Texture2D(this, p_filename, m_objectPool.size(), m_default2DTexture->m_handle);

		// Set the loaded flag to true, because we have already provided the texture handle
		returnTexture->setLoadedToVideoMemory(true);

		// Add the new texture to the list
		m_objectPool.push_back(returnTexture);
	}

	// Return the new texture
	return Texture2DHandle(returnTexture);
}

TextureLoaderCubemap::TextureLoaderCubemap()
{
	std::string defaultFilenames[CubemapFace_NumOfFaces];
	for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
		defaultFilenames[face] = Config::textureVar().default_texture;

	m_defaultCubemap = new TextureCubemap(this, Config::textureVar().default_texture, defaultFilenames, m_objectPool.size(), 0);
}

TextureLoaderCubemap::~TextureLoaderCubemap()
{
}

ErrorCode TextureLoaderCubemap::init()
{
	// If the default texture filename changed upon loading the configuration
	// (don't need to compare all filenames, since the default cubemap will have the same texture for all faces)
	if(m_defaultCubemap->m_filenames[CubemapFace_PositiveX] != Config::textureVar().default_texture)
	{
		delete m_defaultCubemap;

		std::string defaultFilenames[CubemapFace_NumOfFaces];
		for(unsigned int face = CubemapFace_PositiveX; face < CubemapFace_NumOfFaces; face++)
			defaultFilenames[face] = Config::textureVar().default_texture;

		m_defaultCubemap = new TextureCubemap(this, Config::textureVar().default_texture, defaultFilenames, m_objectPool.size(), 0);
	}

	// Load default texture to memory and video memory
	m_defaultCubemap->loadToMemory();
	//m_defaultCubemap->loadToVideoMemory();

	// Add default texture to the texture pool
	m_objectPool.push_back(m_defaultCubemap);

	return ErrorCode::Success;
}

TextureLoaderCubemap::TextureCubemapHandle TextureLoaderCubemap::load(const std::string & p_filenamePosX, const std::string & p_filenameNegX, const std::string & p_filenamePosY, const std::string & p_filenameNegY, const std::string & p_filenamePosZ, const std::string & p_filenameNegZ, bool p_startBackgroundLoading)
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

TextureLoaderCubemap::TextureCubemapHandle TextureLoaderCubemap::load(const std::string & p_filename, unsigned int p_textureHandle)
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
