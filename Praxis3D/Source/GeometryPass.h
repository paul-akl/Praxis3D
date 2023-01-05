#pragma once

#include "RenderPassBase.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass(RendererFrontend &p_renderer) : RenderPass(p_renderer) { }

	~GeometryPass() { }

	ErrorCode init()
	{
		ErrorCode returnError;

		m_name = "Geometry Rendering Pass";

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

			// Log a successful initialization
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_GeometryPass);
		}
		else
		{
			// Log a failed initialization
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_GeometryPass);
		}

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);

		// Set input and output color maps for this frame
		p_renderPassData.setColorInputMap(GeometryBuffer::GBufferIntermediate);
		p_renderPassData.setColorOutputMap(GeometryBuffer::GBufferFinal);

		// Bind the geometry framebuffer to be used
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::FramebufferGeometry);

		// Bind all the geometry buffer textures to be drawn to
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferPosition);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferDiffuse);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferNormal);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferEmissive);
		m_renderer.m_backend.getGeometryBuffer()->bindBufferForWriting(GeometryBuffer::GBufferMatProperties);

		// Initialize the geometry framebuffer for the geometry pass
		m_renderer.m_backend.getGeometryBuffer()->initGeometryPass();

		// Get known shader details
		auto geomShaderHandle = m_shaderGeometry->getShaderHandle();
		auto &geomUniformUpdater = m_shaderGeometry->getUniformUpdater();

		// Iterate over all objects to be rendered with geometry shader
		for(auto entity : p_sceneObjects.m_models)
		{
			ModelComponent &model = p_sceneObjects.m_models.get<ModelComponent>(entity);
			if(model.isObjectActive() && model.isLoadedToVideoMemory())
			{
				SpatialComponent &spatialData = p_sceneObjects.m_models.get<SpatialComponent>(entity);
				auto &modelData = model.getModelData();

				for(decltype(modelData.size()) i = 0, size = modelData.size(); i < size; i++)
				{
					m_renderer.queueForDrawing(modelData[i],
						geomShaderHandle,
						geomUniformUpdater,
						spatialData.getSpatialDataChangeManager().getWorldTransform(),
						m_renderer.m_viewProjMatrix);
				}
			}
		}

		// Iterate over all objects to be rendered with a custom shader
		for(auto entity : p_sceneObjects.m_modelsWithShaders)
		{
			ModelComponent &model = p_sceneObjects.m_modelsWithShaders.get<ModelComponent>(entity);
			if(model.isObjectActive() && model.isLoadedToVideoMemory())
			{
				SpatialComponent &spatialData = p_sceneObjects.m_modelsWithShaders.get<SpatialComponent>(entity);
				ShaderComponent &shader = p_sceneObjects.m_modelsWithShaders.get<ShaderComponent>(entity);
				if(shader.isLoadedToVideoMemory())
				{
					auto &modelData = model.getModelData();

					for(decltype(modelData.size()) i = 0, size = modelData.size(); i < size; i++)
					{
						m_renderer.queueForDrawing(modelData[i],
							shader.getShaderData()->m_shader.getShaderHandle(),
							shader.getShaderData()->m_shader.getUniformUpdater(),
							spatialData.getSpatialDataChangeManager().getWorldTransform(),
							m_renderer.m_viewProjMatrix);
					}
				}
			}
		}


		/*/ Iterate over all objects to be rendered with geometry shader
		for(decltype(p_sceneObjects.m_modelData.size()) objIndex = 0, numObjects = p_sceneObjects.m_modelData.size(); objIndex < numObjects; objIndex++)
		{
			m_renderer.queueForDrawing(p_sceneObjects.m_modelData[objIndex].m_modelData,
								geomShaderHandle, 
								geomUniformUpdater,
								p_sceneObjects.m_modelData[objIndex].m_modelMatrix,
								m_renderer.m_viewProjMatrix);
		}*/

		/*/ Iterate over all objects to be rendered with a custom shader
		for(decltype(p_sceneObjects.m_modelDataWithShaders.size()) objIndex = 0, numObjects = p_sceneObjects.m_modelDataWithShaders.size(); objIndex < numObjects; objIndex++)
		{
			m_renderer.queueForDrawing(p_sceneObjects.m_modelDataWithShaders[objIndex].m_modelData,
				p_sceneObjects.m_modelDataWithShaders[objIndex].m_shader->getShaderHandle(),
				p_sceneObjects.m_modelDataWithShaders[objIndex].m_shader->getUniformUpdater(),
				p_sceneObjects.m_modelDataWithShaders[objIndex].m_modelMatrix,
				m_renderer.m_viewProjMatrix);
		}*/

		// Pass all the draw commands to be executed
		m_renderer.passDrawCommandsToBackend();
	}
	
private:
	ShaderLoader::ShaderProgram	*m_shaderGeometry;
};