
#include "DeferredRenderer.h"
#include "RendererScene.h"
#include "ShaderUniformUpdater.h"
#include "WindowLocator.h"

DeferredRenderer::SingleTriangle DeferredRenderer::m_fullscreenTriangle;

DeferredRenderer::DeferredRenderer()
{
	m_rendererState = new DeferredRendererState(this);
	m_currentObjectData = nullptr;
	m_gbuffer = nullptr;
	m_boundShaderHandle = 0;
	m_spotLightBufferHandle = 0;
	m_pointLightBufferHandle = 0;

	for(int i = 0; i < Model::NumOfModelMaterials; i++)
		m_boundTextureHandles[i] = 0;
}
DeferredRenderer::~DeferredRenderer()
{
	delete m_gbuffer;
}

ErrorCode DeferredRenderer::init()
{
	ErrorCode returnCode = ErrorCode::Success;

	// Get the current screen size
	m_screenSize.x = Config::graphicsVar().current_resolution_x;
	m_screenSize.y = Config::graphicsVar().current_resolution_y;

	// Initialize gbuffer (and also pass the screen size to be used as the buffer size)
	m_gbuffer = new GeometryBuffer((unsigned int) m_screenSize.x, (unsigned int) m_screenSize.y);

	// Check if the gbuffer initialization was successful
	if(ErrHandlerLoc::get().ifSuccessful(m_gbuffer->init(), returnCode))
	{
		// Create a property-set used to load geometry shader
		PropertySet geomShaderProperties(Properties::Shaders);
		geomShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().geometry_pass_vert_shader);
		geomShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().geometry_pass_frag_shader);

		// Create a property-set used to load lighting shader
		PropertySet lightShaderProperties(Properties::Shaders);
		lightShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().light_pass_vert_shader);
		lightShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().light_pass_frag_shader);

		// Create shaders
		m_shaderGeometry = Loaders::shader().load(geomShaderProperties);
		m_shaderLightPass = Loaders::shader().load(lightShaderProperties);

		// Load geometry shaders
		m_shaderGeometry->loadToMemory();
		m_shaderGeometry->loadToVideoMemory();

		// Load lighting shaders
		m_shaderLightPass->loadToMemory();
		m_shaderLightPass->loadToVideoMemory();

		// Load fullscreen triangle (used to render post-processing effects)
		m_fullscreenTriangle.load();

		// Enable / disable face culling
		if(Config::rendererVar().face_culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		// Enable / disable depth test
		if(Config::rendererVar().depth_test)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		glDepthFunc(GL_LESS);

		// Set face culling mode
		glCullFace(Config::rendererVar().face_culling_mode);

		// Set depth test function
		glDepthFunc(Config::rendererVar().depth_test_func);

		// Declare light buffer's block size variable
		GLint pointLightBlockSize = 0;
		GLint spotLightBlockSize = 0;

		m_shaderLightPass->bind();

		// Get point light buffer's uniform block index
		auto pointLightBlockIndex = glGetUniformBlockIndex(m_shaderLightPass->getShaderHandle(), "PointLights");
		// Get point light buffer's uniform block size
		glGetActiveUniformBlockiv(m_shaderLightPass->getShaderHandle(), pointLightBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &pointLightBlockSize);
		// Bind the uniform buffer at point light binding point
		glUniformBlockBinding(m_shaderLightPass->getShaderHandle(), pointLightBlockIndex, PointLightBindingPoint);

		// Get spot light buffer's uniform block index
		auto spotLightBlockIndex = glGetUniformBlockIndex(m_shaderLightPass->getShaderHandle(), Config::shaderVar().spotLightBuffer.c_str());
		// Get spot light buffer's uniform block size
		glGetActiveUniformBlockiv(m_shaderLightPass->getShaderHandle(), spotLightBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &spotLightBlockSize);
		// Bind the uniform buffer at spot light binding point
		glUniformBlockBinding(m_shaderLightPass->getShaderHandle(), spotLightBlockIndex, SpotLightBindingPoint);

		// Calculate the maximum number of lights supported
		int maxNumPointLights = pointLightBlockSize / sizeof(PointLightDataSet);
		int maxNumSpotLights = spotLightBlockSize / sizeof(SpotLightDataSet);

		// Make sure the maximum number of lights does not exceed the supported amount
		if(Config::rendererVar().max_num_point_lights > maxNumPointLights)
			Config::m_rendererVar.max_num_point_lights = maxNumPointLights;
		if(Config::rendererVar().max_num_spot_lights > maxNumSpotLights)
			Config::m_rendererVar.max_num_spot_lights = maxNumSpotLights;

		// Allocate point light buffer on VRAM and bind it to uniform buffer in the shader
		glGenBuffers(1, &m_pointLightBufferHandle);
		glBindBuffer(GL_UNIFORM_BUFFER, m_pointLightBufferHandle);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLightDataSet) * Config::rendererVar().max_num_point_lights, NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, pointLightBlockIndex, m_pointLightBufferHandle);

		// Allocate spot light buffer on VRAM and bind it to uniform buffer in the shader
		glGenBuffers(1, &m_spotLightBufferHandle);
		glBindBuffer(GL_UNIFORM_BUFFER, m_spotLightBufferHandle);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SpotLightDataSet) * Config::rendererVar().max_num_spot_lights, NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, spotLightBlockIndex, m_spotLightBufferHandle);

		// Calculate projection matrix
		updateProjectionMatrix();
	}
	
	return returnCode;
}

void DeferredRenderer::beginRenderCycle(float p_deltaTime)
{

}
void DeferredRenderer::endRenderCycle(float p_deltaTime)
{

}

void DeferredRenderer::renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	// Load all the objects in the load-to-gpu queue. This needs to be done before any rendering, as objects in this
	// array might have been also added to objects-to-render arrays, so they need to be loaded first
	for(decltype(p_sceneObjects.m_objectsToLoad.size()) i = 0, size = p_sceneObjects.m_objectsToLoad.size(); i < size; i++)
	{
		p_sceneObjects.m_objectsToLoad[i]->loadToVideoMemory();

		// In case shader failed to load or was not specified, assign a geometry shader
		if(p_sceneObjects.m_objectsToLoad[i]->m_shader->isDefaultProgram())
			p_sceneObjects.m_objectsToLoad[i]->m_shader = m_shaderGeometry;
		else
		{
			p_sceneObjects.m_objectsToLoad[i]->m_shader->bind();
			p_sceneObjects.m_objectsToLoad[i]->m_shader->getUniformUpdater().updateTextureUniforms(*m_rendererState);
		}

	}

	m_gbuffer->initFrame();

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//m_testVec = Math::Vec4f(1.0, 0.0, 0.0, 0.0);

	m_currentCamera = p_sceneObjects.m_camera;

	update();

	geometryPass(p_sceneObjects, p_deltaTime);

	lightingPass(p_sceneObjects, p_deltaTime);

	postLightingPass(p_sceneObjects, p_deltaTime);

	finalPass();
}

void DeferredRenderer::geometryPass(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	// Bind buffers
	m_gbuffer->initGeometryPass();

	// Enable face culling 
	//if(Config::rendererVar().face_culling)
	//	glEnable(GL_CULL_FACE);

	//glDepthMask(GL_TRUE);			// Make sure to turn on depth testing
	//glEnable(GL_DEPTH_TEST);		// as this is much like a regular forward render pass
	//glDisable(GL_BLEND);

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	m_shaderGeometry->bind();
	m_shaderGeometry->getUniformUpdater().updateFrame(*m_rendererState);
	m_shaderGeometry->getUniformUpdater().updateTextureUniforms(*m_rendererState);
	m_boundShaderHandle = m_shaderGeometry->getShaderHandle();

	// Iterate over all objects to be rendered with geometry shader
	for(decltype(p_sceneObjects.m_modelObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_modelObjects.size(); objIndex < numObjects; objIndex++)
	{
		drawModelObject(p_sceneObjects.m_modelObjects[objIndex], m_shaderGeometry);
	}

	// Iterate over all objects to be rendered with a custom shader
	for(decltype(p_sceneObjects.m_customShaderObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_customShaderObjects.size(); objIndex < numObjects; objIndex++)
	{
		// If shader handle is not already bound, bind it
		//if(p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getShaderHandle() != m_boundShaderHandle)
		{
			p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->bind();
			p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getUniformUpdater().updateFrame(*m_rendererState);

			m_boundShaderHandle = p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getShaderHandle();
		}

		if(p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->isTessellated())
			drawTessellatedObject(p_sceneObjects.m_customShaderObjects[objIndex], p_sceneObjects.m_customShaderObjects[objIndex]->m_shader);
		else
			drawModelObject(p_sceneObjects.m_customShaderObjects[objIndex], p_sceneObjects.m_customShaderObjects[objIndex]->m_shader);
	}

	// Unbind VBO so it is not changed from outside
	//glBindVertexArray(0);
}

void DeferredRenderer::lightingPass(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	m_gbuffer->initLightPass();

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

	// Bind and update the point light buffer
	glBindBuffer(GL_UNIFORM_BUFFER, m_pointLightBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLightDataSet) * p_sceneObjects.m_pointLights.size(), 
					p_sceneObjects.m_pointLights.data());

	// Bind and update the spot light buffer
	glBindBuffer(GL_UNIFORM_BUFFER, m_spotLightBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SpotLightDataSet) * p_sceneObjects.m_spotLights.size(),
					p_sceneObjects.m_spotLights.data());

	// Update the directional light data set (or clear it if there is no directional light in the scene)
	if(p_sceneObjects.m_directionalLight != nullptr)
		m_directionalLight = *p_sceneObjects.m_directionalLight;
	else
		m_directionalLight.clear();

	// Update the current number of lights in the light buffers
	m_numPointLights = p_sceneObjects.m_pointLights.size();
	m_numSpotLights = p_sceneObjects.m_spotLights.size();

	m_shaderLightPass->bind();
	m_shaderLightPass->getUniformUpdater().updateTextureUniforms(*m_rendererState);
	m_shaderLightPass->getUniformUpdater().updateFrame(*m_rendererState);

	m_fullscreenTriangle.bind();
	m_fullscreenTriangle.render();
	//glBindVertexArray(0);
}

void DeferredRenderer::postLightingPass(const SceneObjects & p_sceneObjects, const float p_deltaTime)
{
	//glDepthMask(GL_FALSE);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	//glDisable(GL_BLEND);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glDisable(GL_CULL_FACE);

	//glDepthMask(GL_TRUE);			// Make sure to turn on depth testing
	glEnable(GL_DEPTH_TEST);		// as this is much like a regular forward render pass
	//glDepthFunc(GL_LESS);
	//glDisable(GL_BLEND);

	// Iterate over all objects to be rendered with a custom shader
	for(decltype(p_sceneObjects.m_postLightingObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_postLightingObjects.size(); objIndex < numObjects; objIndex++)
	{
		// If shader handle is not already bound, bind it
		//if(p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getShaderHandle() != m_boundShaderHandle)
		{
			p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->bind();
			p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getUniformUpdater().updateFrame(*m_rendererState);
			p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getUniformUpdater().updateTextureUniforms(*m_rendererState);

			m_boundShaderHandle = p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getShaderHandle();
		}

		drawModelObject(p_sceneObjects.m_postLightingObjects[objIndex], p_sceneObjects.m_postLightingObjects[objIndex]->m_shader);
	}
}

void DeferredRenderer::finalPass()
{
	m_gbuffer->initFinalPass();
	glBlitFramebuffer(0, 0, m_screenSize.x, m_screenSize.y,
					  0, 0, m_screenSize.x, m_screenSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void DeferredRenderer::update()
{
	// If the resolution changed
	if(m_screenSize.x != Config::graphicsVar().current_resolution_x ||
	   m_screenSize.y != Config::graphicsVar().current_resolution_y)
	{
		// Set the new resolution
		m_screenSize.x = Config::graphicsVar().current_resolution_x;
		m_screenSize.y = Config::graphicsVar().current_resolution_y;

		// Reload the geometry buffers with the new size
		m_gbuffer->setBufferSize((unsigned int)m_screenSize.x, (unsigned int)m_screenSize.y);

		// Update projection matrix and set OpenGL view-port with the new size
		updateProjectionMatrix();
		glViewport(0, 0, m_screenSize.x, m_screenSize.y);
	}


	m_viewProjMatrix = m_projMatrix * m_currentCamera->getBaseObjectData().m_modelMat;
}

void DeferredRenderer::drawModelObject(const RenderableObjectData *p_renderableObject, const ShaderLoader::ShaderProgram *p_shader)
{
	// Assign a current base object data
	m_currentObjectData = &(p_renderableObject->m_baseObjectData);
	
	// Calculate matrices
	m_modelViewMatrix = m_currentCamera->getBaseObjectData().m_modelMat * m_currentObjectData->m_modelMat;
	m_modelViewProjMatrix = m_projMatrix * m_modelViewMatrix;

	// Update per-model uniforms
	p_shader->getUniformUpdater().updateModel(*m_rendererState);
		
	// Bind model's VAO
	glBindVertexArray(p_renderableObject->m_model.getHandle());

	// Iterate over all the meshes in the model
	for(decltype(p_renderableObject->m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_renderableObject->m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
	{
		// Update per-mesh uniforms
		p_shader->getUniformUpdater().updateMesh(*m_rendererState);

		// Iterate over all materials and bind them
		for(int matType = 0; matType < Model::NumOfModelMaterials; matType++)
		{
			// Get texture handle
			unsigned int textureHandle = p_renderableObject->m_materials[matType][p_renderableObject->m_model[meshIndex].m_materialIndex].getHandle();

			// If texture handle is not already bound, bind it
			//if(textureHandle != m_boundTextureHandles[matType])
			//{
				glActiveTexture(GL_TEXTURE0 + matType);
				glBindTexture(GL_TEXTURE_2D, textureHandle);
			//	m_boundTextureHandles[matType] = textureHandle;
			//}
		}
		
		// Draw the actual geometry
		glDrawElementsBaseVertex(GL_TRIANGLES, 
								 p_renderableObject->m_model[meshIndex].m_numIndices, 
								 GL_UNSIGNED_INT,
								 (void*)(sizeof(unsigned int) * p_renderableObject->m_model[meshIndex].m_baseIndex), 
								 p_renderableObject->m_model[meshIndex].m_baseVertex);
	}
}

void DeferredRenderer::drawTessellatedObject(const RenderableObjectData *p_renderableObject, const ShaderLoader::ShaderProgram *p_shader)
{
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	// Assign a current base object data
	m_currentObjectData = &(p_renderableObject->m_baseObjectData);

	// Calculate matrices
	m_modelViewMatrix = m_currentCamera->getBaseObjectData().m_modelMat * m_currentObjectData->m_modelMat;
	m_modelViewProjMatrix = m_projMatrix * m_modelViewMatrix;

	// Update per-model uniforms
	p_shader->getUniformUpdater().updateModel(*m_rendererState);
	//p_shader->getUniformUpdater().updateTextureUniforms(*m_rendererState);

	// Bind model's VAO
	glBindVertexArray(p_renderableObject->m_model.getHandle());

	// Iterate over all the meshes in the model
	for(decltype(p_renderableObject->m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_renderableObject->m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
	{
		// Update per-mesh uniforms
		p_shader->getUniformUpdater().updateMesh(*m_rendererState);

		// Iterate over all materials and bind them
		for(int matType = 0; matType < Model::NumOfModelMaterials; matType++)
		{
			// Get texture handle
			unsigned int textureHandle = p_renderableObject->m_materials[matType][p_renderableObject->m_model[meshIndex].m_materialIndex].getHandle();

			// If texture handle is not already bound, bind it
			//if(textureHandle != m_boundTextureHandles[matType])
			{
				glActiveTexture(GL_TEXTURE0 + matType);
				glBindTexture(GL_TEXTURE_2D, textureHandle);
				m_boundTextureHandles[matType] = textureHandle;
			}
		}

		// Draw the actual geometry
		glDrawElementsBaseVertex(GL_PATCHES,
								 p_renderableObject->m_model[meshIndex].m_numIndices,
								 GL_UNSIGNED_INT,
								 (void*)(sizeof(unsigned int) * p_renderableObject->m_model[meshIndex].m_baseIndex),
								 p_renderableObject->m_model[meshIndex].m_baseVertex);
	}
}
