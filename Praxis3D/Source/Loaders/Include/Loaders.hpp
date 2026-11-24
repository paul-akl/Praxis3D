#pragma once

#include "Loaders/Include/ModelLoader.hpp"
#include "Loaders/Include/ShaderLoader.hpp"
#include "Loaders/Include/TextureLoader.hpp"

#include "Systems/Base/Include/Universal.hpp"

class Loaders
{
public:
	inline static ModelLoader &model() { return m_modelLoader; }
	inline static ShaderLoader &shader() { return m_shaderLoader; }
	inline static TextureLoader2D &texture2D() { return m_texture2DLoader; }
	inline static TextureLoaderCubemap &textureCubemap() { return m_textureCubemapLoader; }
	
private:
	static ModelLoader m_modelLoader;
	static ShaderLoader m_shaderLoader;
	static TextureLoader2D m_texture2DLoader;
	static TextureLoaderCubemap m_textureCubemapLoader;
};