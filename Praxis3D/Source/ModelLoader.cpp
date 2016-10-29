#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\ProgressHandler.hpp>
#include <functional>
#include <iostream>

#include "Config.h"
#include "ErrorHandlerLocator.h"
#include "ModelLoader.h"
#include "TaskManagerLocator.h"

#include "Loaders.h"

ErrorCode Model::loadToMemory()
{
	// If the model is not currently already being loaded in another thread
	if(!m_isBeingLoaded)
	{
		// Lock calls from other treads
		SpinWait::Lock lock(m_mutex);

		// Make sure another thread doesn't start another instance of loading
		m_isBeingLoaded = true;

		// To not cause crashes from outside code, since meshes will be modified when loading
		//m_currentNumMeshes = 0;

		// Assign flags for assimp loader
		unsigned int assimpFlags = 0;
		if(Config::modelVar().calcTangentSpace)
			assimpFlags |= aiProcess_CalcTangentSpace;
		if(Config::modelVar().joinIdenticalVertices)
			assimpFlags |= aiProcess_JoinIdenticalVertices;
		if(Config::modelVar().makeLeftHanded)
			assimpFlags |= aiProcess_MakeLeftHanded;
		if(Config::modelVar().triangulate)
			assimpFlags |= aiProcess_Triangulate;
		if(Config::modelVar().removeComponent)
			assimpFlags |= aiProcess_RemoveComponent;
		if(Config::modelVar().genNormals)
			assimpFlags |= aiProcess_GenNormals;
		if(Config::modelVar().genSmoothNormals)
			assimpFlags |= aiProcess_GenSmoothNormals;
		if(Config::modelVar().genUVCoords)
			assimpFlags |= aiProcess_GenUVCoords;
		if(Config::modelVar().optimizeMeshes)
			assimpFlags |= aiProcess_OptimizeMeshes;
		if(Config::modelVar().optimizeGraph)
			assimpFlags |= aiProcess_OptimizeGraph;

		Assimp::Importer assimpImporter;

		// Load data from file to assimp scene structure
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
ErrorCode Model::loadToVideoMemory()
{
	ErrorCode returnError = ErrorCode::Success;
	
	// Create and bind the Vertex Array Object
	glGenVertexArrays(1, &m_handle);
	glBindVertexArray(m_handle);

	// Create the m_buffers
	glGenBuffers(sizeof(m_buffers) / sizeof(m_buffers[0]), m_buffers);

	// Upload indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[IndexBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_numVertices, &m_indices[0], GL_STATIC_DRAW);

	// Upload positions
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[PositionBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_positions[0]) * m_numVertices, &m_positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(PositionBuffer);
	glVertexAttribPointer(PositionBuffer, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Upload normals
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[NormalBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_normals[0]) * m_numVertices, &m_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NormalBuffer);
	glVertexAttribPointer(NormalBuffer, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Upload texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[TexCoordBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_texCoords[0]) * m_numVertices, &m_texCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TexCoordBuffer);
	glVertexAttribPointer(TexCoordBuffer, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Upload tangents
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[TangentsBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_tangents[0]) * m_numVertices, &m_tangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TangentsBuffer);
	glVertexAttribPointer(TangentsBuffer, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Upload bitangents
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[BitangentsBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bitangents[0]) * m_numVertices, &m_bitangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(BitangentsBuffer);
	glVertexAttribPointer(BitangentsBuffer, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Make sure the VAO is not changed from the outside
	glBindVertexArray(0);

	setLoadedToVideoMemory(true);

	return returnError;
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

	for(int matType = 0; matType < Model::NumOfModelMaterials; matType++)
		m_materials.m_materials[matType].clear();

	return returnError;
}
ErrorCode Model::unloadVideoMemory()
{
	ErrorCode returnError = ErrorCode::Success;

	glDeleteBuffers(NumBufferTypes, m_buffers);

	return returnError;
}

void Model::loadFromFile()
{
	ErrorCode error = loadToMemory();

	// If data restructuring was successful, set loaded to memory flag
	if(error == ErrorCode::Success)
		setLoadedToMemory(true);
	// If data restructuring failed, log an error
	else
		ErrHandlerLoc::get().log(error, ErrorSource::Source_ModelLoader);
}
ErrorCode Model::loadFromScene(const aiScene &p_assimpScene)
{
	ErrorCode returnError = ErrorCode::Success;

	// Reserve space in the vectors for every mesh
	m_meshPool.resize(p_assimpScene.mNumMeshes);
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

	// Deal with each mesh
	returnError = loadMeshes(p_assimpScene.mMeshes, p_assimpScene.mNumMeshes);
	
	// Load material file names
	if(returnError == ErrorCode::Success)
		returnError = loadMaterials(p_assimpScene.mMaterials, p_assimpScene.mNumMaterials);
	
	return returnError;
}
ErrorCode Model::loadMeshes(aiMesh **p_assimpMeshes, size_t p_numMeshes)
{
	ErrorCode returnError = ErrorCode::Success;

	for(unsigned int meshIndex = 0, verticeIndex = 0, indicesIndex = 0; meshIndex < p_numMeshes; meshIndex++)
	{
		// Make sure that the texture coordinates array exist (by checking if the first member of the array does)
		bool textureCoordsExist = p_assimpMeshes[meshIndex]->mTextureCoords[0] ? true : false;

		// Check if arrays exist (to not cause an error if they are absent)
		bool normalsExist = p_assimpMeshes[meshIndex]->mNormals != nullptr;
		bool tangentsExist = p_assimpMeshes[meshIndex]->mTangents != nullptr;
		bool bitangentsExist = p_assimpMeshes[meshIndex]->mBitangents != nullptr;

		// Put the mesh data from assimp to memory
		for(unsigned int i = 0; i < p_assimpMeshes[meshIndex]->mNumVertices; i++, verticeIndex++)
		{
			m_positions[verticeIndex].x = p_assimpMeshes[meshIndex]->mVertices[i].x;
			m_positions[verticeIndex].y = p_assimpMeshes[meshIndex]->mVertices[i].y;
			m_positions[verticeIndex].z = p_assimpMeshes[meshIndex]->mVertices[i].z;

			if(normalsExist)
			{
				m_normals[verticeIndex].x = p_assimpMeshes[meshIndex]->mNormals[i].x;
				m_normals[verticeIndex].y = p_assimpMeshes[meshIndex]->mNormals[i].y;
				m_normals[verticeIndex].z = p_assimpMeshes[meshIndex]->mNormals[i].z;
			}

			if(tangentsExist)
			{
				m_tangents[verticeIndex].x = p_assimpMeshes[meshIndex]->mTangents[i].x;
				m_tangents[verticeIndex].y = p_assimpMeshes[meshIndex]->mTangents[i].y;
				m_tangents[verticeIndex].z = p_assimpMeshes[meshIndex]->mTangents[i].z;
			}

			if(bitangentsExist)
			{
				m_bitangents[verticeIndex].x = p_assimpMeshes[meshIndex]->mBitangents[i].x;
				m_bitangents[verticeIndex].y = p_assimpMeshes[meshIndex]->mBitangents[i].y;
				m_bitangents[verticeIndex].z = p_assimpMeshes[meshIndex]->mBitangents[i].z;
			}

			if(textureCoordsExist)
			{
				m_texCoords[verticeIndex].x = p_assimpMeshes[meshIndex]->mTextureCoords[0][i].x;
				m_texCoords[verticeIndex].y = p_assimpMeshes[meshIndex]->mTextureCoords[0][i].y;
			}
		}

		// Put the m_indices data from assimp to memory
		for(unsigned int i = 0, size = p_assimpMeshes[meshIndex]->mNumFaces; i < size; i++)
		{
			if(p_assimpMeshes[meshIndex]->mFaces[i].mNumIndices == 3)
			{
				m_indices[indicesIndex] = p_assimpMeshes[meshIndex]->mFaces[i].mIndices[0];
				m_indices[indicesIndex + 1] = p_assimpMeshes[meshIndex]->mFaces[i].mIndices[1];
				m_indices[indicesIndex + 2] = 
					p_assimpMeshes[meshIndex]->mFaces[i].mIndices[2];

				indicesIndex += 3;
			}
		}
	}

	return returnError;
}
ErrorCode Model::loadMaterials(aiMaterial **p_assimpMaterials, size_t p_numMaterials)
{
	ErrorCode returnError = ErrorCode::Success;
	aiString materialPath;

	// Make space in materials arrays
	m_materials.resize(p_numMaterials);

	for(unsigned int index = 0, i = 0; i < m_materials.m_numMaterials; i++)
	{
		if(p_assimpMaterials[i]->GetTexture(aiTextureType_DIFFUSE, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[ModelMat_diffuse][i].m_filename = materialPath.data;

		if(p_assimpMaterials[i]->GetTexture(aiTextureType_NORMALS, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[ModelMat_normal][i].m_filename = materialPath.data;

		if(p_assimpMaterials[i]->GetTexture(aiTextureType_EMISSIVE, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[ModelMat_emissive][i].m_filename = materialPath.data;

		if(p_assimpMaterials[i]->GetTexture(aiTextureType_SPECULAR, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[ModelMat_specular][i].m_filename = materialPath.data;

		if(p_assimpMaterials[i]->GetTexture(aiTextureType_SHININESS, index, &materialPath) == aiReturn_SUCCESS)
			m_materials.m_materials[ModelMat_gloss][i].m_filename = materialPath.data;

		if((p_assimpMaterials[i]->GetTexture(aiTextureType_HEIGHT, index, &materialPath) == aiReturn_SUCCESS) ||
		   (p_assimpMaterials[i]->GetTexture(aiTextureType_DISPLACEMENT, index, &materialPath) == aiReturn_SUCCESS))
			m_materials.m_materials[ModelMat_height][i].m_filename = materialPath.data;
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
}

ModelLoader::~ModelLoader()
{
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