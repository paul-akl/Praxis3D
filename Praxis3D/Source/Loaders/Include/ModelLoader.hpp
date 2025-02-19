#pragma once

#include <assimp\scene.h>
#include <bitset>
#include <GL\glew.h>

#include "Definitions/Include/CommonDefinitions.hpp"
#include "ServiceLocators/Include/ErrorHandlerLocator.hpp"
#include "LoaderBase.hpp"
#include "Math/Include/Math.hpp"

class ModelLoader;

// Model class, stores all the data for a model, and contains all the functionality for loading
// and processing the model data. Used only by the Model Loader, should not be accessible from outside.
// Uses template inheritance, derives from Unique Object in Loader Base.
class Model : public LoaderBase<ModelLoader, Model>::UniqueObject
{
	friend class ModelLoader;
	friend class MeshIterator;
	friend class LoaderBase<ModelLoader, Model>::UniqueObject;
public:
	// Note: caution with modifying. Correlates with enum in Renderer class, for convenience
	// Note: the order is sensitive

	struct Mesh
	{
		Mesh()
		{
			m_materialIndex = 0;
			m_numIndices = 0;
			m_baseVertex = 0;
			m_baseIndex = 0;
		}

		unsigned int m_materialIndex;
		unsigned int m_numIndices;
		unsigned int m_baseVertex;
		unsigned int m_baseIndex;
	};
	struct Material
	{
		Material() : m_defaultMaterial(true) { }
		Material(const std::string &p_filename) : m_filename(p_filename), m_defaultMaterial(true) { }

		inline Material &operator=(const std::string &p_filename)
		{
			m_filename = p_filename; 
			m_defaultMaterial = false;
			return *this; 
		}
		const inline bool isEmpty() const { return m_filename.empty(); }
		const inline bool isDefaultMaterial() const { return m_defaultMaterial; }

		std::string m_filename;
		bool m_defaultMaterial;
	};
	struct MaterialArrays
	{
		MaterialArrays()
		{
			m_numMaterials = 0;
		}

		// Resizes all the vectors and assigns a new number of materials
		void resize(std::vector<Material>::size_type p_size)
		{
			for(unsigned int i = 0; i < MaterialType_NumOfTypes; i++)
				m_materials[i].resize(p_size);
			m_numMaterials = p_size;
		}

		// Frees memory of the vectors by swapping them with empty ones
		void clear()
		{
			for(unsigned int i = 0; i < MaterialType_NumOfTypes; i++)
				std::vector<Material>().swap(m_materials[i]);
			m_numMaterials = 0;
		}

		std::vector<Material> m_materials[MaterialType_NumOfTypes];
		std::vector<Material>::size_type m_numMaterials;
	};

	Model(LoaderBase<ModelLoader, Model> *p_loaderBase, std::string p_filename, size_t p_uniqueID, unsigned int p_handle) : UniqueObject(p_loaderBase, p_uniqueID, p_filename), m_handle(p_handle)
	{
		m_loadedToVideoMemory = false;
		m_isBeingLoaded = false;

		m_currentNumMeshes = 0;
		m_numVertices = 0;
		m_numMeshes = 0;
		m_handle = 0;

		for(int i = 0; i < ModelBuffer_NumAllTypes; i++)
			m_buffers[i] = 0;

		for(int i = 0; i < ModelBuffer_NumAllTypes; i++)
			m_bufferSize[i] = 0;
	}

	// Loads data from HDD to RAM and restructures it to be used to fill buffers later
	ErrorCode loadToMemory();
	// Deletes data stored in RAM. Does not delete buffers that are loaded on GPU VRAM.
	ErrorCode unloadMemory();

	// Loads the model data from file (using internal filename)
	void loadFromFile();
	// Processes data from AI scene
	ErrorCode loadFromScene(const aiScene &p_assimpScene);
	// Fills mesh buffers
	ErrorCode loadMeshes(const aiScene &p_assimpScene);
	// Gets material filenames from model file
	ErrorCode loadMaterials(const aiScene &p_assimpScene);
	// Load textures embedded in the model file. Note: currently unused / no implementation
	ErrorCode loadTextures(aiTexture **p_assimpTextures, size_t p_numTextures);

	inline MaterialArrays &getMaterialArrays() { return m_materials; }

	// Returns an array of pointers to buffer data
	const inline void **getData()
	{
		const void **data = new const void*[ModelBuffer_Index + 1];

		data[ModelBuffer_Position]		= m_positions.data();
		data[ModelBuffer_Normal]		= m_normals.data();
		data[ModelBuffer_TexCoord]		= m_texCoords.data();
		data[ModelBuffer_Tangents]		= m_tangents.data();
		data[ModelBuffer_Bitangents]	= m_bitangents.data();
		data[ModelBuffer_Index]			= m_indices.data();

		return data;
	}

	// m_handle is a VAO handle
	unsigned int m_handle;
	unsigned int m_buffers[ModelBuffer_NumAllTypes];
	int64_t m_bufferSize[ModelBuffer_NumAllTypes];

	std::vector<Mesh> m_meshPool;
	std::vector<std::string> m_meshNames;
	
	std::vector<unsigned int> m_indices;
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texCoords;
	std::vector<glm::vec3> m_tangents;
	std::vector<glm::vec3> m_bitangents;
	
	MaterialArrays m_materials;

	size_t	m_numVertices,
			m_numMeshes;

	// Is set to 0 while model is loading in the background, otherwise a copy of m_numMeshes
	size_t	m_currentNumMeshes;

	tbb::atomic<bool> m_isBeingLoaded;

	const static int m_numElements[ModelBuffer_NumAllTypes];
};

// Model Loader, CRTP inheritance from Loader Base. Designed to be used for loading models,
// stores all the loaded models internally, checks for duplicates when loading. Uses model
// wrapper class to maintain a reference counter for automatic "garbage collector". Returns
// an empty default model, if the loading failed.
class ModelLoader : public LoaderBase<ModelLoader, Model>
{
	friend class Model;
public:
	class ModelHandle;

	// Wrapper class for a model, to provide data needed for rendering, without exposing the
	// private variables. Two ways of iterating over data: using subscription operator, or checking
	// nextMeshPresent with calling operator++ in-between and using getters to retrieve data.
	class MeshIterator
	{
		friend class ModelHandle;
	public:
		// Check if the next mesh (at internal mesh index) is present
		const inline bool nextMeshPresent() const { return m_nextMeshPresent; }

		// Increments the internal mesh index
		const inline void operator++(int)
		{
			if(m_meshIndex < m_meshPoolSize - 1)
				m_meshIndex++;
			else
				m_nextMeshPresent = false;
		}

		// Array subscription operator; unsafe - does not check for index being out of bounds
		const inline Model::Mesh &operator[] (size_t p_index) const { return m_model.m_meshPool[p_index]; }

		// Getters
		inline size_t getNumMeshes() const				{ return m_meshPoolSize; }
		const inline int getMeshMaterialIndex() const	{ return m_model.m_meshPool[m_meshIndex].m_materialIndex;	}
		const inline int getMeshNumIndices() const		{ return m_model.m_meshPool[m_meshIndex].m_numIndices;		}
		const inline int getMeshBaseIndex()	const		{ return m_model.m_meshPool[m_meshIndex].m_baseIndex;		}
		const inline int getMeshBaseVertex() const		{ return m_model.m_meshPool[m_meshIndex].m_baseVertex;		}

	private:
		MeshIterator(Model &p_model) : m_model(p_model), m_nextMeshPresent(false), m_meshIndex(0) 
		{ 
			m_meshPoolSize = p_model.m_numMeshes; 
		}
		~MeshIterator() { }

		bool m_nextMeshPresent;
		Model &m_model;
		size_t	m_meshIndex,
				m_meshPoolSize;
	};
	
	// Wrapper class for a model, designed to be used for various book-keeping (maintaining reference counter),
	// and hiding internal data from outside classes. Private constructor.
	class ModelHandle
	{
		friend struct LoadableObjectsContainer;
		friend class CommandBuffer;
		friend class ModelLoader;
		friend class RendererFrontend;
	public:
		// Increment the reference counter when creating a handle
		ModelHandle(const ModelHandle &p_modelHandle) : m_model(p_modelHandle.m_model) { m_model->incRefCounter(); }
		ModelHandle(ModelHandle &&p_modelHandle) noexcept : m_model(p_modelHandle.m_model) { m_model->incRefCounter(); }
		~ModelHandle() { m_model->decRefCounter(); }
		
		// Loads data from HDD to RAM and restructures it to be used to fill buffers later
		ErrorCode loadToMemory(bool p_setLoadedToMemoryFlag = true)
		{
			ErrorCode returnError = ErrorCode::Success;

			//TODO: wait for the object to load, if it is currently being loaded
			// If it's not loaded to memory already and is not currently being loaded, call load
			if(!m_model->isLoadedToMemory() && !m_model->isBeingLoaded())
			{
				returnError = m_model->loadToMemory();

				// If the setLoadedToMemory flag is true and loading was successful, set the flag
				if(p_setLoadedToMemoryFlag)
					if(returnError == ErrorCode::Success)
						m_model->setLoadedToMemory(true);
			}

			return returnError;
		}
		
		// Copy assignment operator
		ModelHandle &operator=(const ModelHandle &p_modelHandle)
		{
			m_model = p_modelHandle.m_model;
			m_model->incRefCounter();
			return *this;
		}

		// Move assignment operator
		ModelHandle &operator=(ModelHandle &&p_modelHandle) noexcept
		{
			if(this == &p_modelHandle)
				return *this;

			m_model = p_modelHandle.m_model;
			m_model->incRefCounter();
			return *this;
		}

		// Array subscription operator; retrieves meshes, intended for rendering without getting Model Iterator
		const inline Model::Mesh &operator[] (size_t p_index) const { return m_model->m_meshPool[p_index]; }
		
		// Getters
		inline Model::MaterialArrays &getMaterialArrays() const		{ return m_model->getMaterialArrays();		}
		inline const std::vector<Model::Mesh> &getMeshArray() const	{ return m_model->m_meshPool;				}
		inline size_t getNumMeshes() const							{ return m_model->m_numMeshes;				}
		inline MeshIterator getMeshIterator() const					{ return MeshIterator(*m_model);			}
		inline const size_t getMeshSize() const						{ return m_model->m_numMeshes;				}
		inline unsigned int getHandle() const						{ return m_model->m_handle;					}
		inline std::string &getFilename() const						{ return m_model->m_filename;				}
		inline const bool isLoadedToMemory() const					{ return m_model->isLoadedToMemory();		}
		inline const bool isLoadedToVideoMemory() const				{ return m_model->isLoadedToVideoMemory();	}
		inline std::string getMeshName(size_t p_meshIndex) const
		{
			if(p_meshIndex < m_model->m_meshNames.size())
				return m_model->m_meshNames[p_meshIndex];

			return std::string();
		}

	private:
		ModelHandle(Model &p_model) : m_model(&p_model) { m_model->incRefCounter(); }

		void setLoadedToVideoMemory(bool p_loaded) { m_model->m_loadedToVideoMemory = p_loaded; }

		Model *m_model;
	};

	ModelLoader();
	~ModelLoader();

	ErrorCode init();

	virtual ModelHandle load(std::string p_filename, bool p_startBackgroundLoading = true);

private:
	void unload(Model &p_object, SceneLoader &p_sceneLoader);

	ModelHandle *m_defaultModelHandle;
	Model *m_defaultModel;
};