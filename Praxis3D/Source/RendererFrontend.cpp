
#include "RendererFrontend.h"

ErrorCode RendererFrontend::init()
{
	return ErrorCode::Success;
}

void RendererFrontend::renderFrame(const SceneObjects &p_sceneObjects, const float p_deltaTime)
{
	// Load all the objects in the load-to-gpu queue. This needs to be done before any rendering, as objects in this
	// array might have been also added to objects-to-render arrays, so they need to be loaded first
	for(decltype(p_sceneObjects.m_objectsToLoad.size()) i = 0, size = p_sceneObjects.m_objectsToLoad.size(); i < size; i++)
	{
		p_sceneObjects.m_objectsToLoad[i]->loadToVideoMemory();

		// In case shader failed to load or was not specified, assign a geometry shader
		//if(p_sceneObjects.m_objectsToLoad[i]->m_shader->isDefaultProgram())
			//p_sceneObjects.m_objectsToLoad[i]->m_shader = m_shaderGeometry;
		//else
		//{
			//p_sceneObjects.m_objectsToLoad[i]->m_shader->bind();
			//p_sceneObjects.m_objectsToLoad[i]->m_shader->getUniformUpdater().updateTextureUniforms(*m_rendererState);
		//}

	}

	// Iterate over all objects to be rendered with geometry shader
	for(decltype(p_sceneObjects.m_modelObjects.size()) objIndex = 0, numObjects = p_sceneObjects.m_modelObjects.size(); objIndex < numObjects; objIndex++)
	{
		

		//drawModelObject(p_sceneObjects.m_modelObjects[objIndex], m_shaderGeometry);
	}
}

void RendererFrontend::queueForDrawing(const RenderableObjectData &p_object)
{
	/*/ Declare temp values used for indexing
	size_t shaderIndex = 0;
	size_t modelIndex = 0;

	// Get different object handles
	auto shaderHandle = p_object.m_shader->getShaderHandle();
	auto modelVAOHandle = p_object.m_model.getHandle();

	// Loop through the shader array, to check if the current shader has already been added
	for(shaderIndex = 0; shaderIndex < m_objects.m_numShaders; shaderIndex++)
		if(m_objects.m_shaders[shaderIndex].m_shaderHandle == shaderHandle)
			break;

	// If the shader has not been added already, add it
	if(m_objects.m_shaders[shaderIndex].m_shaderHandle != shaderHandle)
	{
		// Assign correct shader index
		shaderIndex = m_objects.m_shaders.size();

		// Add the current shader to the array
		m_objects.m_shaders.push_back(RendererBackend::RendererShader(p_object.m_shader->getUniformUpdater(), shaderHandle));

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
	if(m_objects.m_models[modelIndex].m_VAO != modelVAOHandle)
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
