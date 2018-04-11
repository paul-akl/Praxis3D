#pragma once

#include "RenderPassBase.h"

class ReflectionPass : public RenderPass
{
public:
	ReflectionPass(RendererFrontend &p_renderer) : RenderPass(p_renderer) { }

	~ReflectionPass() { }

	ErrorCode init()
	{
		ErrorCode returnError;

		m_name = "Reflection Rendering Pass";

		// Create a property-set used to load reflection shader
		PropertySet reflectionShaderProperties(Properties::Shaders);
		reflectionShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().reflection_pass_vert_shader);
		reflectionShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().reflection_pass_frag_shader);

		// Create shaders
		m_shaderReflectionPass = Loaders::shader().load(reflectionShaderProperties);

		// Load shaders to memory
		m_shaderReflectionPass->loadToMemory();

		if(returnError == ErrorCode::Success)
		{
			// Queue the shaders to be loaded to GPU
			m_renderer.queueForLoading(*m_shaderReflectionPass);
		}

		return returnError;
	}

	void update(const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{

	}

private:
	ShaderLoader::ShaderProgram	*m_shaderReflectionPass;
};