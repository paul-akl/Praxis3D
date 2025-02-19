#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\ProgressHandler.hpp>
#include <functional>
#include <iostream>

#include "Loaders/Include/Config.hpp"
#include "ServiceLocators/Include/ErrorHandlerLocator.hpp"
#include "Loaders/Include/ModelLoader.hpp"
#include "Loaders/Include/SceneLoader.hpp"
#include "ServiceLocators/Include/TaskManagerLocator.hpp"

#include "Loaders/Include/Loaders.hpp"

// This initialization depends on the order of BufferType enum entries
const int Model::m_numElements[ModelBuffer_NumAllTypes] = { 3, 3, 2, 3, 3, 0 };

ErrorCode Model::loadToMemory()
{
	// If the model is not currently already being loaded in another thread
	if(!m_isBeingLoaded)
	{
		// Make sure another thread doesn't start another instance of loading
		m_isBeingLoaded = true;

		// Lock calls from other treads
		SpinWait::Lock lock(m_mutex);

		// To not cause crashes from outside code, since meshes will be modified when loading
		//m_currentNumMeshes = 0;

		// Assign flags for assimp loader
		unsigned int assimpFlags = 0;

		if(Config::modelVar().calcTangentSpace)
			assimpFlags |= aiPostProcessSteps::aiProcess_CalcTangentSpace;
		if(Config::modelVar().joinIdenticalVertices)
			assimpFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;
		if(Config::modelVar().makeLeftHanded)
			assimpFlags |= aiPostProcessSteps::aiProcess_MakeLeftHanded;
		if(Config::modelVar().triangulate)
			assimpFlags |= aiPostProcessSteps::aiProcess_Triangulate;
		if(Config::modelVar().removeComponent)
			assimpFlags |= aiPostProcessSteps::aiProcess_RemoveComponent;
		if(Config::modelVar().genBoundingBoxes)
			assimpFlags |= aiPostProcessSteps::aiProcess_GenBoundingBoxes;
		if(Config::modelVar().genNormals)
			assimpFlags |= aiPostProcessSteps::aiProcess_GenNormals;
		if(Config::modelVar().genSmoothNormals)
			assimpFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
		if(Config::modelVar().genUVCoords)
			assimpFlags |= aiPostProcessSteps::aiProcess_GenUVCoords;
		if(Config::modelVar().optimizeCacheLocality)
			assimpFlags |= aiPostProcessSteps::aiProcess_ImproveCacheLocality;
		if(Config::modelVar().optimizeMeshes)
			assimpFlags |= aiPostProcessSteps::aiProcess_OptimizeMeshes;
		if(Config::modelVar().optimizeGraph)
			assimpFlags |= aiPostProcessSteps::aiProcess_OptimizeGraph;

		Assimp::Importer assimpImporter;

		// Load data from file to Assimp scene structure
		const aiScene* assimpScene = assimpImporter.ReadFile(Config::filepathVar().model_path + m_filename, assimpFlags);

		// If loading wasn't successful, log an error
		if(!assimpScene)
		{
			ErrHandlerLoc::get().log(ErrorCode::AssimpScene_failed, ErrorSource::Source_ModelLoader, m_filename);
			m_loadingToMemoryError = ErrorCode::AssimpScene_failed;
		}
		// If loading was successful, start restructuring the data to be loaded to video memory later
		else
		{
			m_loadingToMemoryError = loadFromScene(*assimpScene);

			// If data restructuring failed, log an error
			if(m_loadingToMemoryError != ErrorCode::Success)
			{
				ErrHandlerLoc::get().log(m_loadingToMemoryError, ErrorSource::Source_ModelLoader, m_filename);
			}
		}

		m_isBeingLoaded = false;
	}
	else
	{
		// Wait for loading in another thread to finish before continuing
		SpinWait::Lock lock(m_mutex);
	}

	return m_loadingToMemoryError;
}
ErrorCode Model::unloadMemory()
{
	ErrorCode returnError = ErrorCode::Success;

	m_indices.clear();
	m_positions.clear();
	m_normals.clear();
	m_texCoords.clear();
	m_tangents.clear();
	m_bitangents.clear();
	m_meshPool.clear();

	for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
		m_materials.m_materials[matType].clear();

	return returnError;
}

void Model::loadFromFile()
{
	ErrorCode error = loadToMemory();

	// If data restructuring was successful, set loaded to memory flag
	if(error == ErrorCode::Success)
		setLoadedToMemory(true);
	else // If data restructuring failed, log an error
		ErrHandlerLoc::get().log(error, ErrorSource::Source_ModelLoader);
}
ErrorCode Model::loadFromScene(const aiScene &p_assimpScene)
{
	ErrorCode returnError = ErrorCode::Success;

	// Reserve space in the vectors for every mesh
	m_meshPool.resize(p_assimpScene.mNumMeshes);
	m_meshNames.resize(p_assimpScene.mNumMeshes);
	m_numMeshes = p_assimpScene.mNumMeshes;

	unsigned int numIndicesTotal = 0;

	// Count the number of vertices and m_indices
	for(unsigned int i = 0; i < p_assimpScene.mNumMeshes; i++)
	{
		m_meshPool[i].m_materialIndex = p_assimpScene.mMeshes[i]->mMaterialIndex;
		m_meshPool[i].m_numIndices = p_assimpScene.mMeshes[i]->mNumFaces * 3;

		m_meshPool[i].m_baseVertex = (unsigned int)m_numVertices;
		m_meshPool[i].m_baseIndex = numIndicesTotal;

		m_numVertices += p_assimpScene.mMeshes[i]->mNumVertices;
		numIndicesTotal += m_meshPool[i].m_numIndices;
	}

	// Reserve space in the vectors for the vertex attributes and m_indices
	m_positions.resize(m_numVertices);
	m_normals.resize(m_numVertices);
	m_texCoords.resize(m_numVertices);
	m_tangents.resize(m_numVertices);
	m_bitangents.resize(m_numVertices);
	m_indices.resize(numIndicesTotal);

	// Set buffer sizes
	m_bufferSize[ModelBuffer_Position]		= sizeof(m_positions[0])	* m_numVertices;
	m_bufferSize[ModelBuffer_Normal]		= sizeof(m_normals[0])		* m_numVertices;
	m_bufferSize[ModelBuffer_TexCoord]		= sizeof(m_texCoords[0])	* m_numVertices;
	m_bufferSize[ModelBuffer_Tangents]		= sizeof(m_tangents[0])		* m_numVertices;
	m_bufferSize[ModelBuffer_Bitangents]	= sizeof(m_bitangents[0])	* m_numVertices;
	m_bufferSize[ModelBuffer_Index]			= sizeof(m_indices[0])		* m_numVertices;

	// Deal with each mesh
	returnError = loadMeshes(p_assimpScene);
	
	// Load material file names
	if(returnError == ErrorCode::Success)
		returnError = loadMaterials(p_assimpScene);

	return returnError;
}
ErrorCode Model::loadMeshes(const aiScene &p_assimpScene)
{
	ErrorCode returnError = ErrorCode::Success;

	for(size_t meshIndex = 0, verticeIndex = 0, indicesIndex = 0; meshIndex < p_assimpScene.mNumMeshes; meshIndex++)
	{
		// Set the mesh name
		m_meshNames[meshIndex] = p_assimpScene.mMeshes[meshIndex]->mName.C_Str();

		// Make sure that the texture coordinates array exist (by checking if the first member of the array does)
		bool textureCoordsExist = p_assimpScene.mMeshes[meshIndex]->mTextureCoords[0] ? true : false;

		// Check if arrays exist (to not cause an error if they are absent)
		//bool normalsExist = p_assimpMeshes[meshIndex]->mNormals != nullptr;
		const bool tangentsExist = p_assimpScene.mMeshes[meshIndex]->mTangents != nullptr;
		const bool bitangentsExist = p_assimpScene.mMeshes[meshIndex]->mBitangents != nullptr;
		for(decltype(m_positions.size()) i = 0, size = m_positions.size(); i < size; i++)
		{

		}
		// Put the mesh data from assimp to memory
		for(decltype(p_assimpScene.mMeshes[meshIndex]->mNumVertices) i = 0, tangentIndex = 2, size = p_assimpScene.mMeshes[meshIndex]->mNumVertices; i < size; i++, verticeIndex++)
		{
			m_positions[verticeIndex].x = p_assimpScene.mMeshes[meshIndex]->mVertices[i].x;
			m_positions[verticeIndex].y = p_assimpScene.mMeshes[meshIndex]->mVertices[i].y;
			m_positions[verticeIndex].z = p_assimpScene.mMeshes[meshIndex]->mVertices[i].z;

			m_normals[verticeIndex].x = p_assimpScene.mMeshes[meshIndex]->mNormals[i].x;
			m_normals[verticeIndex].y = p_assimpScene.mMeshes[meshIndex]->mNormals[i].y;
			m_normals[verticeIndex].z = p_assimpScene.mMeshes[meshIndex]->mNormals[i].z;

			if(textureCoordsExist)
			{
				m_texCoords[verticeIndex].x = p_assimpScene.mMeshes[meshIndex]->mTextureCoords[0][i].x;
				m_texCoords[verticeIndex].y = p_assimpScene.mMeshes[meshIndex]->mTextureCoords[0][i].y;
			}
			
			if(!tangentsExist || !bitangentsExist)
			{
				if(verticeIndex == tangentIndex)
				{
					// Get vertex positions of the polygon
					const glm::vec3 &v0 = m_positions[verticeIndex - 2];
					const glm::vec3 &v1 = m_positions[verticeIndex - 1];
					const glm::vec3 &v2 = m_positions[verticeIndex - 0];

					// Get texture coordinates of the polygon
					const glm::vec2 &uv0 = m_texCoords[verticeIndex - 2];
					const glm::vec2 &uv1 = m_texCoords[verticeIndex - 1];
					const glm::vec2 &uv2 = m_texCoords[verticeIndex - 0];

					// Get normals of the polygon
					const glm::vec3 &n0 = m_normals[verticeIndex - 2];
					const glm::vec3 &n1 = m_normals[verticeIndex - 1];
					const glm::vec3 &n2 = m_normals[verticeIndex - 0];

					// Calculate position difference
					glm::vec3 deltaPos1 = v1 - v0;
					glm::vec3 deltaPos2 = v2 - v0;

					// Calculate texture coordinate difference
					glm::vec2 deltaUV1 = uv1 - uv0;
					glm::vec2 deltaUV2 = uv2 - uv0;

					// Calculate tangent and bitangent
					float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
					glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
					glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

					// Orthogonalize using Gram–Schmidt process, to make tangents and bitangents smooth based on normal
					m_tangents[verticeIndex - 2] = glm::normalize(tangent - n0 * glm::dot(n0, tangent));
					m_tangents[verticeIndex - 1] = glm::normalize(tangent - n1 * glm::dot(n1, tangent));
					m_tangents[verticeIndex - 0] = glm::normalize(tangent - n2 * glm::dot(n2, tangent));

					m_bitangents[verticeIndex - 2] = glm::normalize(bitangent - n0 * glm::dot(n0, bitangent));
					m_bitangents[verticeIndex - 1] = glm::normalize(bitangent - n1 * glm::dot(n1, bitangent));
					m_bitangents[verticeIndex - 0] = glm::normalize(bitangent - n2 * glm::dot(n2, bitangent));

					tangentIndex += 3;
				}
			}
			else
			{
				m_tangents[verticeIndex].x = p_assimpScene.mMeshes[meshIndex]->mTangents[i].x;
				m_tangents[verticeIndex].y = p_assimpScene.mMeshes[meshIndex]->mTangents[i].y;
				m_tangents[verticeIndex].z = p_assimpScene.mMeshes[meshIndex]->mTangents[i].z;

				m_bitangents[verticeIndex].x = p_assimpScene.mMeshes[meshIndex]->mBitangents[i].x;
				m_bitangents[verticeIndex].y = p_assimpScene.mMeshes[meshIndex]->mBitangents[i].y;
				m_bitangents[verticeIndex].z = p_assimpScene.mMeshes[meshIndex]->mBitangents[i].z;
			}
		}

		// Put the m_indices data from assimp to memory
		for(unsigned int i = 0, size = p_assimpScene.mMeshes[meshIndex]->mNumFaces; i < size; i++)
		{
			if(p_assimpScene.mMeshes[meshIndex]->mFaces[i].mNumIndices == 3)
			{
				m_indices[indicesIndex] = p_assimpScene.mMeshes[meshIndex]->mFaces[i].mIndices[0];
				m_indices[indicesIndex + 1] = p_assimpScene.mMeshes[meshIndex]->mFaces[i].mIndices[1];
				m_indices[indicesIndex + 2] = p_assimpScene.mMeshes[meshIndex]->mFaces[i].mIndices[2];

				indicesIndex += 3;
			}
		}
	}

	return returnError;
}
ErrorCode Model::loadMaterials(const aiScene &p_assimpScene)
{
	ErrorCode returnError = ErrorCode::Success;
	aiString materialPath;

	// Assign number of materials. If there are more meshes than the materials, 
	// set the number of materials the same as the number of meshes, to avoid
	// going out of bounds of materials vector, when rendering (as each mesh must have a texture)
	size_t numMaterials = p_assimpScene.mNumMaterials;
	if(p_assimpScene.mNumMeshes > numMaterials)
		numMaterials = p_assimpScene.mNumMeshes;

	// Make space in materials arrays
	m_materials.resize(numMaterials);

	// Iterate over all Assimp textures; the m_materials vector might be larger than the Assimp mMaterials array, but never smaller
	for(decltype(p_assimpScene.mNumMaterials) size = p_assimpScene.mNumMaterials, index = 0, i = 0; i < size; i++)
	{
		if(p_assimpScene.mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[MaterialType_Diffuse][i].m_filename = materialPath.data;

		if(p_assimpScene.mMaterials[i]->GetTexture(aiTextureType_NORMALS, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[MaterialType_Normal][i].m_filename = materialPath.data;

		if(p_assimpScene.mMaterials[i]->GetTexture(aiTextureType_EMISSIVE, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[MaterialType_Emissive][i].m_filename = materialPath.data;

		// Unused with the new shading model (PBS). Might be used in the future to combine textures into one (RMHAO)
		/*
		if(p_assimpMaterials[i]->GetTexture(aiTextureType_SPECULAR, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[ModelMat_specular][i].m_filename = materialPath.data;

		if(p_assimpMaterials[i]->GetTexture(aiTextureType_SHININESS, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[ModelMat_gloss][i].m_filename = materialPath.data;

		if((p_assimpMaterials[i]->GetTexture(aiTextureType_HEIGHT, index, &materialPath) == aiReturn_SUCCESS) ||
		   (p_assimpMaterials[i]->GetTexture(aiTextureType_DISPLACEMENT, index, &materialPath) == aiReturn_SUCCESS))
			m_materials.m_materials[ModelMat_height][i].m_filename = materialPath.data;
			*/
	}

	return returnError;
}
ErrorCode Model::loadTextures(aiTexture **p_assimpTextures, size_t p_numTextures)
{
	return ErrorCode::Success;
}

ModelLoader::ModelLoader()
{
	m_defaultModel = new Model(this, "null_model", 0, 0);
	m_defaultModelHandle = new ModelHandle(*m_defaultModel);
}

ModelLoader::~ModelLoader()
{
	delete m_defaultModelHandle;
}

ErrorCode ModelLoader::init()
{
	m_defaultModel->setLoadedToMemory(true);
	m_defaultModel->setLoadedToVideoMemory(true);
	m_objectPool.push_back(m_defaultModel);

	return ErrorCode::Success;
}

ModelLoader::ModelHandle ModelLoader::load(std::string p_filename, bool p_startBackgroundLoading)
{
	// Make sure calls from other threads are locked, while current call is in progress
	// This is needed to as the object that is being requested might be currently loading /
	// being added to the pool. Mutex prevents duplicates being loaded, and same data being changed.
	SpinWait::Lock lock(m_mutex);

	if(p_filename.empty())
		return ModelHandle(*m_defaultModel);

	// Go through the model pool and check if the model hasn't been already loaded (to avoid duplicates)
	for(decltype(m_objectPool.size()) size = m_objectPool.size(), i = 0; i < size; i++)
	{
		if(*(m_objectPool[i]) == p_filename)
			return ModelHandle(*m_objectPool[i]);
	}

	// Model wasn't loaded before, so create a new one and assign the default placeholder handle 
	// (since it's not loaded yet, and might fail to load)
	Model *model = new Model(this, p_filename, m_objectPool.size(), m_defaultModel->m_handle);

	if(p_startBackgroundLoading)
	{
		// Start loading the model from file, in a background thread
		TaskManagerLocator::get().startBackgroundThread(std::bind(&Model::loadFromFile, model));
		//TaskManagerLocator::get().startBackgroundThread([&] { model->loadFromFile(); });
	}

	// Add the new model to the list
	m_objectPool.push_back(model);

	// Return the new texture
	return ModelHandle(*model);
}

void ModelLoader::unload(Model &p_object, SceneLoader &p_sceneLoader)
{
	// Get the model handle
	unsigned int *modelHandle = new unsigned int(p_object.m_handle);

	// Send a notification to graphics scene to unload the model; set deleteAfterReceiving flag to true, to transfer the ownership of the model handle pointer to the graphics scene (so it will be responsible for deleting it)
	p_sceneLoader.getChangeController()->sendData(p_sceneLoader.getSystemScene(Systems::Graphics), DataType::DataType_UnloadModel, (void *)modelHandle, true);
}
