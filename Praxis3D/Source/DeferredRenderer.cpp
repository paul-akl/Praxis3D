
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

	m_shaderGeometry = 0;
	m_shaderLightPass = 0;
	m_shaderFinalPass = 0;

	for(int i = 0; i < MaterialType_NumOfTypes; i++)
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
	//m_gbuffer = new GeometryBuffer((unsigned int) m_screenSize.x, (unsigned int) m_screenSize.y);

	// Initialize gbuffer and check if it was successful
	//if(ErrHandlerLoc::get().ifSuccessful(m_gbuffer->init(), returnCode))
	{
		// Create a property-set used to load geometry shader
		PropertySet geomShaderProperties(Properties::Shaders);
		geomShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().geometry_pass_vert_shader);
		geomShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().geometry_pass_frag_shader);

		// Create a property-set used to load lighting shader
		PropertySet lightShaderProperties(Properties::Shaders);
		lightShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().light_pass_vert_shader);
		lightShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().light_pass_frag_shader);

		// Create a property-set used to load reflection shader
		PropertySet reflectionShaderProperties(Properties::Shaders);
		reflectionShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().reflection_pass_vert_shader);
		reflectionShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().reflection_pass_frag_shader);

		// Create shaders
		m_shaderGeometry = Loaders::shader().load(geomShaderProperties);
		m_shaderLightPass = Loaders::shader().load(lightShaderProperties);
		m_shaderReflectionPass = Loaders::shader().load(reflectionShaderProperties);

		// Load geometry shader
		m_shaderGeometry->loadToMemory();
		
		//m_shaderGeometry->loadToVideoMemory();

		// Load lighting shader
		m_shaderLightPass->loadToMemory();
		//m_shaderLightPass->loadToVideoMemory();

		// Load reflection shader
		m_shaderReflectionPass->loadToMemory();
		//m_shaderReflectionPass->loadToVideoMemory();

#ifndef SETTING_USE_BLIT_FRAMEBUFFER

		// Create a property-set used to load final shader
		PropertySet finalShaderProperties(Properties::Shaders);
		finalShaderProperties.addProperty(Properties::VertexShader, Config::rendererVar().final_pass_vert_shader);
		finalShaderProperties.addProperty(Properties::FragmentShader, Config::rendererVar().final_pass_frag_shader);

		// Create shader
		m_shaderFinalPass = Loaders::shader().load(finalShaderProperties);

		// Load final shader
		m_shaderFinalPass->loadToMemory();
		//m_shaderFinalPass->loadToVideoMemory();

#endif // SETTING_USE_BLIT_FRAMEBUFFER

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

		//m_shaderLightPass->bind();

		// Get point light buffer's uniform block index
		auto pointLightBlockIndex = glGetUniformBlockIndex(m_shaderLightPass->getShaderHandle(), Config::shaderVar().pointLightBuffer.c_str());
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
	//// Load all the objects in the load-to-gpu queue. This needs to be done before any rendering, as objects in this
	//// array might have been also added to objects-to-render arrays, so they need to be loaded first
	//for(decltype(p_sceneObjects.m_objectsToLoad.size()) i = 0, size = p_sceneObjects.m_objectsToLoad.size(); i < size; i++)
	//{
	//	p_sceneObjects.m_objectsToLoad[i]->loadToVideoMemory();

	//	// In case shader failed to load or was not specified, assign a geometry shader
	//	if(p_sceneObjects.m_objectsToLoad[i]->m_shader->isDefaultProgram())
	//		p_sceneObjects.m_objectsToLoad[i]->m_shader = m_shaderGeometry;
	//	else
	//	{
	//		p_sceneObjects.m_objectsToLoad[i]->m_shader->bind();
	//		p_sceneObjects.m_objectsToLoad[i]->m_shader->getUniformUpdater().updateTextureUniforms(*m_rendererState);
	//	}

	//}
	//	
	//m_cubemap = p_sceneObjects.m_staticSkybox;

	//m_gbuffer->initFrame();
	//
	//m_currentCamera = p_sceneObjects.m_camera;

	//update();

	//geometryPass(p_sceneObjects, p_deltaTime);

	//lightingPass(p_sceneObjects, p_deltaTime);

	//reflectionPass(p_sceneObjects, p_deltaTime);

	//postLightingPass(p_sceneObjects, p_deltaTime);
	//
	//finalPass();
}

void DeferredRenderer::geometryPass(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	//m_objects.clear();

	//// Bind buffers
	//m_gbuffer->initGeometryPass();
	//
	//glEnable(GL_DEPTH_TEST);		// Enable depth testing, as this is much like a regular forward render pass
	//glClear(GL_DEPTH_BUFFER_BIT);	// Make sure to clear the depth buffer for the new frame
	//
	//m_objects.clear();

	//for(decltype(p_sceneObjects.m_modelObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_modelObjects.size(); objIndex < numObjects; objIndex++)
	//	drawModelObjectTest(*p_sceneObjects.m_modelObjects[objIndex], m_shaderGeometry);

	//drawObjectsTest(m_objects);

	//return;

	//m_shaderGeometry->bind();
	//m_shaderGeometry->getUniformUpdater().updateFrame(*m_rendererState);
	//m_shaderGeometry->getUniformUpdater().updateTextureUniforms(*m_rendererState);
	//m_boundShaderHandle = m_shaderGeometry->getShaderHandle();

	//// Iterate over all objects to be rendered with geometry shader
	//for(decltype(p_sceneObjects.m_modelObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_modelObjects.size(); objIndex < numObjects; objIndex++)
	//{
	//	drawModelObject(p_sceneObjects.m_modelObjects[objIndex], m_shaderGeometry);
	//}

	//// Iterate over all objects to be rendered with a custom shader
	//for(decltype(p_sceneObjects.m_customShaderObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_customShaderObjects.size(); objIndex < numObjects; objIndex++)
	//{
	//	// If shader handle is not already bound, bind it
	//	//if(p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getShaderHandle() != m_boundShaderHandle)
	//	{
	//		p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->bind();
	//		p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getUniformUpdater().updateFrame(*m_rendererState);

	//		m_boundShaderHandle = p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->getShaderHandle();
	//	}

	//	if(p_sceneObjects.m_customShaderObjects[objIndex]->m_shader->isTessellated())
	//		drawTessellatedObject(p_sceneObjects.m_customShaderObjects[objIndex], p_sceneObjects.m_customShaderObjects[objIndex]->m_shader);
	//	else
	//		drawModelObject(p_sceneObjects.m_customShaderObjects[objIndex], p_sceneObjects.m_customShaderObjects[objIndex]->m_shader);
	//}

	// Unbind VBO so it is not changed from outside
	//glBindVertexArray(0);
}

void DeferredRenderer::lightingPass(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	//m_gbuffer->initLightPass();

	//glDisable(GL_DEPTH_TEST);

	//// Bind and update the point light buffer
	//glBindBuffer(GL_UNIFORM_BUFFER, m_pointLightBufferHandle);
	//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLightDataSet) * p_sceneObjects.m_pointLights.size(), 
	//				p_sceneObjects.m_pointLights.data());

	//// Bind and update the spot light buffer
	//glBindBuffer(GL_UNIFORM_BUFFER, m_spotLightBufferHandle);
	//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SpotLightDataSet) * p_sceneObjects.m_spotLights.size(),
	//				p_sceneObjects.m_spotLights.data());

	//// Update the directional light data set (or clear it if there is no directional light in the scene)
	//if(p_sceneObjects.m_directionalLight != nullptr)
	//	m_directionalLight = *p_sceneObjects.m_directionalLight;
	//else
	//	m_directionalLight.clear();

	//// Update the current number of lights in the light buffers
	//m_numPointLights = p_sceneObjects.m_pointLights.size();
	//m_numSpotLights = p_sceneObjects.m_spotLights.size();

	//m_shaderLightPass->bind();
	//m_shaderLightPass->getUniformUpdater().updateTextureUniforms(*m_rendererState);
	//m_shaderLightPass->getUniformUpdater().updateFrame(*m_rendererState);

	//m_fullscreenTriangle.bind();
	//m_fullscreenTriangle.render();
}

void DeferredRenderer::postLightingPass(const SceneObjects & p_sceneObjects, const float p_deltaTime)
{
	//glEnable(GL_DEPTH_TEST);	// Enable depth testing, as this is much like a regular forward render pass

	//// Iterate over all objects to be rendered with a custom shader
	//for(decltype(p_sceneObjects.m_postLightingObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_postLightingObjects.size(); objIndex < numObjects; objIndex++)
	//{
	//	// If shader handle is not already bound, bind it
	//	//if(p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getShaderHandle() != m_boundShaderHandle)
	//	{
	//		p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->bind();
	//		p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getUniformUpdater().updateFrame(*m_rendererState);
	//		p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getUniformUpdater().updateTextureUniforms(*m_rendererState);

	//		m_boundShaderHandle = p_sceneObjects.m_postLightingObjects[objIndex]->m_shader->getShaderHandle();
	//	}

	//	drawModelObject(p_sceneObjects.m_postLightingObjects[objIndex], p_sceneObjects.m_postLightingObjects[objIndex]->m_shader);
	//}
}

void DeferredRenderer::reflectionPass(const SceneObjects & p_sceneObjects, const float p_deltaTime)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glActiveTexture(GL_TEXTURE0 + StaticEnvMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap->getCubemapHandle());

	//m_shaderReflectionPass->bind();
	//m_shaderReflectionPass->getUniformUpdater().updateTextureUniforms(*m_rendererState);
	//m_shaderReflectionPass->getUniformUpdater().updateFrame(*m_rendererState);

	m_fullscreenTriangle.bind();
	m_fullscreenTriangle.render();

	glDisable(GL_BLEND);
}

void DeferredRenderer::finalPass()
{
	m_gbuffer->initFinalPass();

#ifdef SETTING_USE_BLIT_FRAMEBUFFER

	glBlitFramebuffer(0, 0, m_screenSize.x, m_screenSize.y,
					  0, 0, m_screenSize.x, m_screenSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#else

	glDisable(GL_DEPTH_TEST);
	
	//m_shaderFinalPass->bind();
	//m_shaderFinalPass->getUniformUpdater().updateTextureUniforms(*m_rendererState);
	//m_shaderFinalPass->getUniformUpdater().updateFrame(*m_rendererState);

	m_fullscreenTriangle.bind();
	m_fullscreenTriangle.render();

#endif // SETTING_USE_BLIT_FRAMEBUFFER


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
	//// Assign a current base object data
	//m_currentObjectData = &(p_renderableObject->m_baseObjectData);
	//
	//// Calculate matrices
	//m_modelViewMatrix = m_currentCamera->getBaseObjectData().m_modelMat * m_currentObjectData->m_modelMat;
	//m_modelViewProjMatrix = m_projMatrix * m_modelViewMatrix;

	//// Update per-model uniforms
	//p_shader->getUniformUpdater().updateModel(*m_rendererState);
	//	
	//// Bind model's VAO
	//glBindVertexArray(p_renderableObject->m_model.getHandle());

	//// Iterate over all the meshes in the model
	//for(decltype(p_renderableObject->m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_renderableObject->m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
	//{
	//	// Update per-mesh uniforms
	//	p_shader->getUniformUpdater().updateMesh(*m_rendererState);

	//	// Iterate over all materials and bind them
	//	for(int matType = 0; matType < Model::NumOfModelMaterials; matType++)
	//	{
	//		// Get texture handle
	//		unsigned int textureHandle = p_renderableObject->m_materials[matType][p_renderableObject->m_model[meshIndex].m_materialIndex].getHandle();

	//		// If texture handle is not already bound, bind it
	//		//if(textureHandle != m_boundTextureHandles[matType])
	//		//{
	//			glActiveTexture(GL_TEXTURE0 + matType);
	//			glBindTexture(GL_TEXTURE_2D, textureHandle);
	//		//	m_boundTextureHandles[matType] = textureHandle;
	//		//}
	//	}
	//	
	//	// Draw the actual geometry
	//	glDrawElementsBaseVertex(GL_TRIANGLES, 
	//							 p_renderableObject->m_model[meshIndex].m_numIndices, 
	//							 GL_UNSIGNED_INT,
	//							 (void*)(sizeof(unsigned int) * p_renderableObject->m_model[meshIndex].m_baseIndex), 
	//							 p_renderableObject->m_model[meshIndex].m_baseVertex);
	//}
}

void DeferredRenderer::drawTessellatedObject(const RenderableObjectData *p_renderableObject, const ShaderLoader::ShaderProgram *p_shader)
{
	//glPatchParameteri(GL_PATCH_VERTICES, 3);

	//// Assign a current base object data
	//m_currentObjectData = &(p_renderableObject->m_baseObjectData);

	//// Calculate matrices
	//m_modelViewMatrix = m_currentCamera->getBaseObjectData().m_modelMat * m_currentObjectData->m_modelMat;
	//m_modelViewProjMatrix = m_projMatrix * m_modelViewMatrix;

	//// Update per-model uniforms
	//p_shader->getUniformUpdater().updateModel(*m_rendererState);
	////p_shader->getUniformUpdater().updateTextureUniforms(*m_rendererState);

	//// Bind model's VAO
	//glBindVertexArray(p_renderableObject->m_model.getHandle());

	//// Iterate over all the meshes in the model
	//for(decltype(p_renderableObject->m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_renderableObject->m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
	//{
	//	// Update per-mesh uniforms
	//	p_shader->getUniformUpdater().updateMesh(*m_rendererState);

	//	// Iterate over all materials and bind them
	//	for(int matType = 0; matType < Model::NumOfModelMaterials; matType++)
	//	{
	//		// Get texture handle
	//		unsigned int textureHandle = p_renderableObject->m_materials[matType][p_renderableObject->m_model[meshIndex].m_materialIndex].getHandle();

	//		// If texture handle is not already bound, bind it
	//		//if(textureHandle != m_boundTextureHandles[matType])
	//		{
	//			glActiveTexture(GL_TEXTURE0 + matType);
	//			glBindTexture(GL_TEXTURE_2D, textureHandle);
	//			m_boundTextureHandles[matType] = textureHandle;
	//		}
	//	}

	//	// Draw the actual geometry
	//	glDrawElementsBaseVertex(GL_PATCHES,
	//							 p_renderableObject->m_model[meshIndex].m_numIndices,
	//							 GL_UNSIGNED_INT,
	//							 (void*)(sizeof(unsigned int) * p_renderableObject->m_model[meshIndex].m_baseIndex),
	//							 p_renderableObject->m_model[meshIndex].m_baseVertex);
	//}
}

void DeferredRenderer::drawModelObjectTest(const RenderableObjectData &p_object, const ShaderLoader::ShaderProgram *p_shader)
{

	////m_objects.emplace_back(0, RendererBackend::RenderDataset());

	//const unsigned int shaderHandle = p_shader->getShaderHandle();
	//const unsigned int modelHandle = p_object.m_model.getHandle();
	//const ShaderUniformUpdater &uniformUpdater = p_shader->getUniformUpdater();

	//for(decltype(p_object.m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_object.m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
	//{
	//	m_objects.emplace_back(0, RendererBackend::DrawCommand(
	//		uniformUpdater,
	//		
	//		shaderHandle, 
	//		modelHandle,
	//		p_object.m_model[meshIndex].m_numIndices,
	//		p_object.m_model[meshIndex].m_baseVertex,
	//		p_object.m_model[meshIndex].m_baseIndex
	//		));
	//}

	/*
	// Declare temp values used for indexing
	size_t shaderIndex = 0;
	size_t modelIndex = 0;

	// Get different object handles
	//auto shaderHandle = p_object.m_shader->getShaderHandle();
	auto shaderHandle = p_shader->getShaderHandle();
	auto modelVAOHandle = p_object.m_model.getHandle();

	// Loop through the shader array, to check if the current shader has already been added
	for(shaderIndex = 0; shaderIndex < m_objects.m_numShaders; shaderIndex++)
		if(m_objects.m_shaders[shaderIndex].m_shaderHandle == shaderHandle)
			break;

	// If the shader has not been added already, add it
	if(m_objects.m_shaders.empty() || m_objects.m_shaders[shaderIndex].m_shaderHandle != shaderHandle)
	{
		// Assign correct shader index
		shaderIndex = m_objects.m_shaders.size();

		// Add the current shader to the array
		//m_objects.m_shaders.push_back(RendererBackend::RendererShader(p_object.m_shader->getUniformUpdater(), shaderHandle));
		m_objects.m_shaders.push_back(RendererBackend::RendererShader(p_shader->getUniformUpdater(), shaderHandle));

		// Increment shader count
		m_objects.m_numShaders++;
	}

	// If the shader is not the first one in the shader array,
	// assign model index depending on the number of models from previous shader
	modelIndex = shaderIndex > 0 ? m_objects.m_shaders[shaderIndex - 1].m_numModels - 1 : 0;

	// Loop through the model array, to check if the current model has already been added
	for(; modelIndex < m_objects.m_shaders[shaderIndex].m_numModels; modelIndex++)
		if(m_objects.m_models[modelIndex].m_VAO == modelVAOHandle)
			break;

	// If the model has not been added already, add it
	if(m_objects.m_models.empty() || m_objects.m_models[modelIndex].m_VAO != modelVAOHandle)
	{
		// Assign correct model index
		modelIndex = m_objects.m_models.size();

		// Add the current model to the array
		m_objects.m_models.push_back(RendererBackend::RendererModel());

		// Increment model count
		m_objects.m_shaders[shaderIndex].m_numModels++;
	}

	for(decltype(p_object.m_model.getNumMeshes()) meshIndex = 0, numMeshes = p_object.m_model.getNumMeshes(); meshIndex < numMeshes; meshIndex++)
	{
		m_objects.m_meshes.push_back(RendererBackend::RendererMesh(p_object.m_model[meshIndex]));
	}

	m_objects.m_models[modelIndex].m_numMeshes += p_object.m_model.getNumMeshes() - 1;*/
}

void DeferredRenderer::drawObjectsTest(const RendererBackend::DrawCommands &p_objects)
{
	for(decltype(p_objects.size()) i = 0, size = p_objects.size(); i < size; i++)
	{
		// Bind shader
		glUseProgram(p_objects[i].second.m_shaderHandle);

		//m_currentObjectData = &(p_objects[i].first);

		// Calculate matrices
		//m_modelViewMatrix = m_currentCamera->getBaseObjectData().m_modelMat * m_currentObjectData->m_modelMat;
		//m_modelViewProjMatrix = m_projMatrix * m_modelViewMatrix;

		//p_objects[i].second.m_uniformUpdater.updateTextureUniforms(*m_rendererState);
		//p_objects[i].second.m_uniformUpdater.updateFrame(*m_rendererState);
		//p_objects[i].second.m_uniformUpdater.updateModel(*m_rendererState);

		// Bind model's VAO
		glBindVertexArray(p_objects[i].second.m_modelHandle);

		// Draw the actual geometry
		glDrawElementsBaseVertex(GL_TRIANGLES,
								 p_objects[i].second.m_numIndices,
								 GL_UNSIGNED_INT,
								 (void*)(sizeof(unsigned int) * p_objects[i].second.m_baseIndex),
								 p_objects[i].second.m_baseVertex);
	}

	/*for(decltype(p_objects.m_numShaders) shaderIndex = 0; shaderIndex < p_objects.m_numShaders; shaderIndex++)
	{
		// Bind shader
		glUseProgram(p_objects.m_shaders[shaderIndex].m_shaderHandle);
		p_objects.m_shaders[shaderIndex].m_uniformUpdater.updateTextureUniforms(*m_rendererState);
		p_objects.m_shaders[shaderIndex].m_uniformUpdater.updateFrame(*m_rendererState);
		p_objects.m_shaders[shaderIndex].m_uniformUpdater.updateModel(*m_rendererState);

		for(decltype(p_objects.m_shaders[shaderIndex].m_numModels) modelIndex = 0; modelIndex < p_objects.m_shaders[shaderIndex].m_numModels; modelIndex++)
		{
			// Bind model's VAO
			glBindVertexArray(p_objects.m_models[modelIndex].m_VAO);

			for(decltype(p_objects.m_models[modelIndex].m_numMeshes) meshIndex = 0; meshIndex < p_objects.m_models[modelIndex].m_numMeshes; meshIndex++)
			{
				// Draw the actual geometry
				glDrawElementsBaseVertex(GL_TRIANGLES,
										 p_objects.m_meshes[meshIndex].m_mesh.m_numIndices,
										 GL_UNSIGNED_INT,
										 (void*)(sizeof(unsigned int) * p_objects.m_meshes[meshIndex].m_mesh.m_baseIndex),
										 p_objects.m_meshes[meshIndex].m_mesh.m_baseVertex);
			}
		}
	}*/
}
