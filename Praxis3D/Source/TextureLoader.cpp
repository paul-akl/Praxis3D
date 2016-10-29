
#include <functional>

#include "ErrorHandlerLocator.h"
#include "ModelLoader.h"
#include "TaskManagerLocator.h"
#include "TextureLoader.h"
#include "Utilities.h"

TextureLoader::TextureLoader()
{
	m_default2DTexture = new Texture2D(this, Config::textureVar().default_texture, m_objectPool.size(), 0);
	m_defaultEmissive = new Texture2D(this, Config::textureVar().default_emissive_texture, m_objectPool.size(), 0);
	m_defaultHeight = new Texture2D(this, Config::textureVar().default_height_texture, m_objectPool.size(), 0);
	m_defaultNormal = new Texture2D(this, Config::textureVar().default_normal_texture, m_objectPool.size(), 0);
}

TextureLoader::~TextureLoader()
{

}

ErrorCode TextureLoader::init()
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
	m_default2DTexture->loadToVideoMemory();

	m_defaultEmissive->loadToMemory();
	m_defaultEmissive->loadToVideoMemory();

	m_defaultHeight->loadToMemory();
	m_defaultHeight->loadToVideoMemory();

	m_defaultNormal->loadToMemory();
	m_defaultNormal->loadToVideoMemory();

	// Add default textures to the texture pool
	m_objectPool.push_back(m_default2DTexture);
	m_objectPool.push_back(m_defaultEmissive);
	m_objectPool.push_back(m_defaultHeight);
	m_objectPool.push_back(m_defaultNormal);

	return ErrorCode::Success;
}

TextureLoader::Texture2DHandle TextureLoader::load2D(const std::string &p_filename, Model::ModelMaterialType p_materialType, bool p_startBackgroundLoading)
{
	Texture2D *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// If the filename is empty, return a default texture instead
	if(p_filename == "")
	{
		switch(p_materialType)
		{
		case Model::ModelMat_normal:
			returnTexture = m_defaultNormal;
			break;
		case Model::ModelMat_emissive:
			returnTexture = m_defaultEmissive;
			break;
		case Model::ModelMat_height:
			returnTexture = m_defaultHeight;
			break;
		case Model::ModelMat_diffuse:
		case Model::ModelMat_specular:
		case Model::ModelMat_gloss:
		case Model::NumOfModelMaterials:
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
		case Model::ModelMat_normal:
			defaultTextureHandle = m_defaultNormal->m_handle;
			break;
		case Model::ModelMat_emissive:
			defaultTextureHandle = m_defaultEmissive->m_handle;
			break;
		case Model::ModelMat_height:
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
TextureLoader::Texture2DHandle TextureLoader::load2D(const std::string &p_filename, unsigned int p_textureHandle)
{
	Texture2D *returnTexture;

	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	// If the filename is empty, return a default texture instead
	if(p_filename == "")
	{
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
