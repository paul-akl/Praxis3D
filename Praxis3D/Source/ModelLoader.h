#pragma once

#include <assimp\scene.h>
#include <bitset>
#include <GL\glew.h>

#include "ErrorHandlerLocator.h"
#include "LoaderBase.h"
#include "Math.h"

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
	enum ModelMaterialType : unsigned int
	{
		ModelMat_diffuse = 0,
		ModelMat_normal,
		ModelMat_emissive,
		ModelMat_specular,
		ModelMat_gloss,
		ModelMat_height,
		NumOfModelMaterials
	};
	enum BufferType : unsigned int
	{
		PositionBuffer = 0,
		NormalBuffer,
		TexCoordBuffer,
		TangentsBuffer,
		BitangentsBuffer,
		IndexBuffer,
		NumBufferTypes
	};

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
			for(unsigned int i = 0; i < NumOfModelMaterials; i++)
				m_materials[i].resize(p_size);
			m_numMaterials = p_size;
		}

		// Frees memory of the vectors by swapping them with empty ones
		void clear()
		{
			for(unsigned int i = 0; i < NumOfModelMaterials; i++)
				std::vector<Material>().swap(m_materials[i]);
			m_numMaterials = 0;
		}

		std::vector<Material> m_materials[NumOfModelMaterials];
		std::vector<Material>::size_type m_numMaterials;
	};


	Model(LoaderBase<ModelLoader, Model> *p_loaderBase, std::string p_filename, size_t p_uniqueID, unsigned int p_handle) : UniqueObject(p_loaderBase, p_uniqueID, p_filename), m_handle(p_handle)
	{
		m_isBeingLoaded = false;

		m_numVertices = 0;
		m_numMeshes = 0;
		//m_numMaterials = 0;
		m_handle = 0;
		m_currentNumMeshes = 0;
		
		for(int i = 0; i < NumBufferTypes; i++)
			m_buffers[i] = 0;
	}

	// Loads data from HDD to RAM and restructures it to be used to fill buffers later
	ErrorCode loadToMemory();
	// Loads data from RAM to buffer and uploads them to VRAM
	ErrorCode loadToVideoMemory();
	// Deletes data stored in RAM. Does not delete buffers that are loaded on GPU VRAM.
	ErrorCode unloadMemory();
	// Deletes buffers from GPU VRAM.
	ErrorCode unloadVideoMemory();

	// Loads the model data from file (using internal filename)
	void loadFromFile();
	// Processes data from AI scene
	ErrorCode loadFromScene(const aiScene &p_assimpScene);
	// Fills mesh buffers
	ErrorCode loadMeshes(aiMesh **p_assimpMeshes, size_t p_arraySize);
	// Gets material filenames from model file
	ErrorCode loadMaterials(aiMaterial **p_assimpMaterials, size_t p_numMaterials);
	// Load textures embedded in the model file. Note: currently unused / no implementation
	ErrorCode loadTextures(aiTexture **p_assimpTextures, size_t p_numTextures);

	inline MaterialArrays &getMaterialArrays() { return m_materials; }

	// m_handle is a VAO handle
	unsigned int m_handle;
	unsigned int m_buffers[NumBufferTypes];

	std::vector<Mesh> m_meshPool;
	
	std::vector<unsigned int> m_indices;
	std::vector<Math::Vec3f> m_positions;
	std::vector<Math::Vec3f> m_normals;
	std::vector<Math::Vec2f> m_texCoords;
	std::vector<Math::Vec3f> m_tangents;
	std::vector<Math::Vec3f> m_bitangents;

	MaterialArrays m_materials;

	size_t	m_numVertices,
			m_numMeshes;

	// Is set to 0 while model is loading in the background, otherwise a copy of m_numMeshes
	size_t	m_currentNumMeshes;

	tbb::atomic<bool> m_isBeingLoaded;
};

// Model Loader, CRTP inheritance from Loader Base. Designed to be used for loading models,
// stores all the loaded models internally, checks for duplicates when loading. Uses model
// wrapper class to maintain a reference counter for automatic "garbage collector". Returns
// an empty default model, if the loading failed.
class ModelLoader : public LoaderBase<ModelLoader, Model>
{
private:
	Model *m_defaultModel;

public:
	class ModelHandle;

	// Wrapper class for a model, to provide data needed for rendering, without exposing the
	// private variables. Two ways of iterating over data: using subscription operator, or checking
	// nextMeshPresent with calling operator++ in between and using getters to retrieve data.
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
		friend class ModelLoader;
	public:
		~ModelHandle() { m_model->decRefCounter(); }

		// Bind VAO for rendering if it is loaded
		// Load the model buffers to GPU memory instead if it's not loaded
		void bind()
		{
			// Declare the handle to bind at the start here, and bind it at the end of the function,
			// so there is only one path to bind function and the branch can be predicted easier on CPU
			decltype(m_model->m_handle) handle = m_model->m_handle;
			
			// If the model buffers are loaded, just bind the VAO (by leaving the handle as it is).
			// Otherwise load buffers to GPU. This way, the model loading is hidden away,
			// so graphics system doesn't have to deal with it. The thread that calls bind
			// on a model is guaranteed to be rendering thread, so it is safe to load it.
			if(!m_model->loadedToVideoMemory())
			{
				if(m_model->loadedToMemory())
				{
					ErrorCode error = m_model->loadToVideoMemory();

					// If loading to video memory was successful, set mesh number, so that object could get rendered.
					// Otherwise log an error
					if(error == ErrorCode::Success)
					{
						// Reassign the handle, since it must have changed when loading to video memory
						handle = m_model->m_handle;
						m_model->m_currentNumMeshes = m_model->m_numMeshes;
					}
					else
						ErrHandlerLoc::get().log(error, ErrorSource::Source_ModelLoader, m_model->getFilename());

					// Set loaded to video memory flag to true even if loading failed.
					// This way, the failed loading attempt is not repeated every frame.
					// (And since loaded to memory flag is already true, it is not the cause,
					// and it will not "fix" itself)
					// EDIT: it is now set inside loadToVideoMemory() function
					//m_model->setLoadedToVideoMemory(true);
					m_model->m_currentNumMeshes = m_model->m_numMeshes;
				}
				else
				{
					// Make VAO into zero in case it's not ready to be rendered yet
					handle = 0;
				}

			}

			// Bind VAO
			glBindVertexArray(handle);
		}

		// Perform a complete load, if not loaded already (from HDD to memory to video memory)
		// WARNING: should probably only be called from rendering thread, since this code deals with graphics API
		ErrorCode preload()
		{
			ErrorCode returnError = ErrorCode::Success;

			// If it's not loaded to video memory already
			if(!m_model->loadedToVideoMemory())
			{
				// If it's loaded to memory
				if(m_model->loadedToMemory())
				{
					// Load to video memory
					returnError = m_model->loadToVideoMemory();
					m_model->setLoadedToVideoMemory(true);

					// If loading to video memory was successful, set mesh number, so that object could get rendered
					if(returnError == ErrorCode::Success)
						m_model->m_currentNumMeshes = m_model->m_numMeshes;
				}
				// If it's not loaded to memory
				else
				{
					// Load to memory
					returnError = m_model->loadToMemory();

					// If loading to memory was successful
					if(returnError == ErrorCode::Success)
					{
						m_model->setLoadedToMemory(true);
						returnError = m_model->loadToVideoMemory();
						m_model->setLoadedToVideoMemory(true);

						// If loading to video memory was successful, set mesh number, so that object could get rendered
						if(returnError == ErrorCode::Success)
							m_model->m_currentNumMeshes = m_model->m_numMeshes;
					}
				}
			}

			return returnError;
		}

		// Loads data from HDD to RAM and restructures it to be used to fill buffers later
		ErrorCode loadToMemory(bool p_setLoadedToMemoryFlag = true)
		{
			ErrorCode returnError = ErrorCode::Success;

			// If it's not loaded to memory already, call load
			if(!m_model->loadedToMemory())
			{
				returnError = m_model->loadToMemory();

				// If the setLoadedToMemory flag is true and loading was successful, set the flag
				if(p_setLoadedToMemoryFlag)
					if(returnError == ErrorCode::Success)
						m_model->setLoadedToMemory(true);
			}

			return returnError;
		}

		// Loads data from RAM to buffer and uploads them to VRAM
		// WARNING: should probably only be called from rendering thread, since this code deals with graphics API
		ErrorCode loadToVideoMemory()
		{
			ErrorCode returnError = ErrorCode::Success;

			// If it's not loaded to video memory already, and has been loaded to memory, call load
			if(!m_model->loadedToVideoMemory())
				if(m_model->loadedToMemory())
					returnError = m_model->loadToVideoMemory();

			return returnError;
		}

		// Assignment operator
		ModelHandle &operator=(const ModelHandle &p_modelHandle)
		{
			m_model->decRefCounter();
			m_model = p_modelHandle.m_model;
			m_model->incRefCounter();
			return *this;
		}

		// Array subscription operator; retrieves meshes, intended for rendering without getting Model Iterator
		const inline Model::Mesh &operator[] (size_t p_index) const { return m_model->m_meshPool[p_index]; }
		
		// Getters
		inline Model::MaterialArrays &getMaterialArrays() const		{ return m_model->getMaterialArrays();	}
		inline const std::vector<Model::Mesh> getMeshArray() const	{ return m_model->m_meshPool;			}
		inline size_t getNumMeshes() const							{ return m_model->m_numMeshes;			}
		inline MeshIterator getMeshIterator() const					{ return MeshIterator(*m_model);		}
		inline const size_t getMeshSize() const						{ return m_model->m_numMeshes;			}
		inline unsigned int getHandle() const						{ return m_model->m_handle;				}
		inline std::string getFilename() const						{ return m_model->m_filename;			}
		
	private:
		ModelHandle(Model &p_model) : m_model(&p_model) { m_model->incRefCounter(); }

		Model *m_model;
	};

	ModelLoader();
	~ModelLoader();

	ErrorCode init();

	virtual ModelHandle load(std::string p_filename, bool p_startBackgroundLoading = true);
};