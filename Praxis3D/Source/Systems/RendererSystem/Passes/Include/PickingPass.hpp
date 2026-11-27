#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Systems/RendererSystem/Passes/Include/RenderPassBase.hpp"

// Renders the entity ID of every pickable mesh into an offscreen integer buffer and reads
// back the single texel under the requested pixel, which determines the entity that is
// under the mouse cursor. Runs only in the frames where a picking request is pending.
class PickingPass : public RenderPass
{
public:
	PickingPass(RendererFrontend &p_renderer) :
		RenderPass(p_renderer, RenderPassType::RenderPassType_Picking),
		m_shaderPicking(nullptr)
	{
	}

	~PickingPass() { }

	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		m_name = "Picking Pass";

		// Create a property-set used to load the shader
		PropertySet shaderProperties(Properties::Shaders);
		shaderProperties.addProperty(Properties::Name, std::string("picking"));
		shaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().picking_pass_vert_shader);
		shaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().picking_pass_frag_shader);

		// Create the shader
		m_shaderPicking = Loaders::shader().load(shaderProperties);

		// Load the shader to memory
		ErrorCode shaderLoadResult = m_shaderPicking->loadToMemory();
		if(shaderLoadResult == ErrorCode::Success)
		{
			// Queue the shader to be loaded to GPU
			m_renderer.queueForLoading(*m_shaderPicking);
		}
		else
			returnError = shaderLoadResult;

		// Create the picking framebuffer
		if(ErrorCode framebufferError = m_renderer.m_backend.createFramebuffer(FramebufferType::FramebufferType_PickingBuffer, m_renderer.getFrameData()); framebufferError != ErrorCode::Success)
			returnError = framebufferError;

		// Check for errors and log either a successful or a failed initialization
		if(returnError == ErrorCode::Success)
		{
			ErrHandlerLoc::get().log(ErrorCode::Initialize_success, ErrorSource::Source_Renderer);
			setInitialized(true);
		}
		else
			ErrHandlerLoc::get().log(ErrorCode::Initialize_failure, ErrorSource::Source_Renderer);

		return returnError;
	}

	void update(RenderPassData &p_renderPassData, const SceneObjects &p_sceneObjects, const float p_deltaTime)
	{
		// Nothing to do if no picking was requested this frame
		if(!p_renderPassData.m_pickRequested)
			return;

		PickingFramebuffer *pickingFramebuffer = m_renderer.m_backend.getPickingFramebuffer();
		if(pickingFramebuffer == nullptr)
		{
			// There is nothing to render into, so drop the request instead of leaving it pending
			p_renderPassData.setPickingResult(NULL_ENTITY_ID);
			return;
		}

		// Nothing has been rendered into the ID buffer, so it cannot be used to determine what is under the cursor
		if(!p_sceneObjects.m_processDrawing)
		{
			p_renderPassData.setPickingResult(NULL_ENTITY_ID);
			return;
		}

		// Keep the ID buffer at the current render resolution
		pickingFramebuffer->setBufferSize(m_renderer.m_frameData.m_screenSize.x, m_renderer.m_frameData.m_screenSize.y);

		// Prepare the ID buffer for rendering
		pickingFramebuffer->initFrame();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);

		{
			const unsigned int shaderHandle = m_shaderPicking->getShaderHandle();
			ShaderUniformUpdater &uniformUpdater = m_shaderPicking->getUniformUpdater();

			// Iterate over all objects to be rendered with the default shader
			for(auto entity : p_sceneObjects.m_models)
			{
				ModelComponent &model = p_sceneObjects.m_models.get<ModelComponent>(entity);
				if(model.isObjectActive())
				{
					auto &modelData = model.getModelData();
					const glm::mat4 &modelMatrix = p_sceneObjects.m_models.get<SpatialComponent>(entity).getSpatialDataChangeManager().getWorldTransformWithScale();
					const glm::mat4 modelViewProjMatrix = m_renderer.m_viewProjMatrix * modelMatrix;

					// Go over each model
					for(decltype(modelData.size()) modelIndex = 0, modelSize = modelData.size(); modelIndex < modelSize; modelIndex++)
					{
						// Go over each mesh
						for(decltype(modelData[modelIndex].m_model.getNumMeshes()) meshIndex = 0, meshSize = modelData[modelIndex].m_model.getNumMeshes(); meshIndex < meshSize; meshIndex++)
						{
							// Only draw active meshes
							if(modelData[modelIndex].m_meshes[meshIndex].m_active)
							{
								m_renderer.queueForDrawing(
									modelData[modelIndex].m_model[meshIndex],
									modelData[modelIndex].m_meshes[meshIndex],
									modelData[modelIndex].m_model.getHandle(),
									shaderHandle,
									uniformUpdater,
									DrawCommandTextureBinding::DrawCommandTextureBinding_None,
									modelData[modelIndex].m_drawFaceCulling,
									modelMatrix,
									modelViewProjMatrix,
									entity);
							}
						}
					}
				}
			}

			// Iterate over all objects to be rendered with a custom shader
			for(auto entity : p_sceneObjects.m_modelsWithShaders)
			{
				ModelComponent &model = p_sceneObjects.m_modelsWithShaders.get<ModelComponent>(entity);
				if(model.isObjectActive())
				{
					SpatialComponent &spatialData = p_sceneObjects.m_modelsWithShaders.get<SpatialComponent>(entity);
					auto &modelData = model.getModelData();

					for(decltype(modelData.size()) i = 0, size = modelData.size(); i < size; i++)
					{
						m_renderer.queueForDrawing(modelData[i],
							shaderHandle,
							uniformUpdater,
							DrawCommandTextureBinding::DrawCommandTextureBinding_None,
							spatialData.getSpatialDataChangeManager().getWorldTransformWithScale(),
							m_renderer.m_viewProjMatrix,
							entity);
					}
				}
			}

			// Pass all the draw commands to be executed
			m_renderer.passDrawCommandsToBackend();
		}

		// Read back the ID under the requested pixel and convert "no entity" into NULL_ENTITY_ID
		EntityID pickedEntityID = NULL_ENTITY_ID;
		const glm::ivec2 pixel = p_renderPassData.m_pickPixel;
		if(pixel.x >= 0 && pixel.y >= 0 &&
		   pixel.x < (int)pickingFramebuffer->getBufferWidth() &&
		   pixel.y < (int)pickingFramebuffer->getBufferHeight())
		{
			const unsigned int entityID = pickingFramebuffer->readID((unsigned int)pixel.x, (unsigned int)pixel.y);
			if(entityID != PickingFramebuffer::NoEntityID)
				pickedEntityID = entityID;
		}

		p_renderPassData.setPickingResult(pickedEntityID);

		// Restore the state that the following passes expect.
		// Rebound geometry buffer specifically, so the scene is not rendered to the picking buffer
		m_renderer.m_backend.getGeometryBuffer()->bindFramebufferForWriting(GeometryBuffer::GBufferFramebufferType::FramebufferGeometry);
		glViewport(0, 0, m_renderer.m_frameData.m_screenSize.x, m_renderer.m_frameData.m_screenSize.y);
		glDepthFunc(Config::rendererVar().depth_test_func);
	}

private:
	ShaderLoader::ShaderProgram *m_shaderPicking;
};
