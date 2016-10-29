#pragma once

#include "RenderPassBase.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass(RendererFrontend &p_renderer) : RenderPass(p_renderer) { }

	ErrorCode init()
	{
		ErrorCode returnError;

		// Create a property-set used to load geometry shader
		PropertySet geomShaderProperties(Properties::Shaders);
		geomShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().geometry_pass_vert_shader);
		geomShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().geometry_pass_frag_shader);

		// Create shaders
		m_shaderGeometry = Loaders::shader().load(geomShaderProperties);

		// Load shaders to memory
		returnError = m_shaderGeometry->loadToMemory();

		if(returnError == ErrorCode::Success)
		{
			// Queue the shaders to be loaded to GPU
			m_renderer.queueForLoading(*m_shaderGeometry);
		}

		return returnError;
	}

	void update(const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferGeometry);

		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferPosition);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferDiffuse);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferNormal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferMatProperties);

		// Get known shader details
		auto geomShaderHandle = m_shaderGeometry->getShaderHandle();
		auto &geomUniformUpdater = m_shaderGeometry->getUniformUpdater();

		// Iterate over all objects to be rendered with geometry shader
		for(decltype(p_sceneObjects.m_modelObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_modelObjects.size(); objIndex < numObjects; objIndex++)
		{
			m_renderer.queueForDrawing(*p_sceneObjects.m_modelObjects[objIndex],
								geomShaderHandle, 
								geomUniformUpdater, 
								m_renderer.m_viewProjMatrix);
		}

		// Iterate over all objects to be rendered with a custom shader
		for(decltype(p_sceneObjects.m_customShaderObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_customShaderObjects.size(); objIndex < numObjects; objIndex++)
		{
			//	queueForDrawing(*p_sceneObjects.m_customShaderObjects[objIndex], 
			//					p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getShaderHandle(),
			//					p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getUniformUpdater(),
			//					viewProjMatrix);
		}
	}

	std::string getName() { return "Geometry Rendering Pass"; }

private:
	ShaderLoader::ShaderProgram	*m_shaderGeometry;
};