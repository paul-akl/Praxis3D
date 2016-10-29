#pragma once

#include "ModelLoader.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"

#include "Universal.h"

class Loaders
{
public:
	inline static ModelLoader &model() { return m_modelLoader; }
	inline static ShaderLoader &shader() { return m_shaderLoader; }
	inline static TextureLoader &texture() { return m_textureLoader; }
private:
	static ModelLoader m_modelLoader;
	static ShaderLoader m_shaderLoader;
	static TextureLoader m_textureLoader;
};