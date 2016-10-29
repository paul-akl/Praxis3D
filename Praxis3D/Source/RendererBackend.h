#pragma once

#include <stdint.h>

#include "Config.h"
#include "Loaders.h"

struct UniformRendererData
{
	Math::Vec2i m_screenSize;

	Math::Mat4f m_projMatrix,
				m_viewProjMatrix;
};

struct UniformObjectData
{
	UniformObjectData()
	{
		m_heightScale = 0.0f;
		m_alphaThreshold = 0.0f;
		m_emissiveThreshold = 0.0f;
		m_textureTilingFactor = 1.0f;
	}

	Math::Mat4f m_modelMat,
				m_modelViewProjMatrix;

	float	m_heightScale, 
			m_alphaThreshold,
			m_emissiveThreshold,
			m_textureTilingFactor;
};

struct UniformData
{
	UniformData(const UniformObjectData &p_objectData, const UniformRendererData &p_rendererData)
		: m_objectData(p_objectData), m_rendererData(p_rendererData) { }

	const UniformObjectData &m_objectData;
	const UniformRendererData &m_rendererData;
};

class RendererBackend
{
public:	
	enum MaterialTypes : unsigned int
	{
		DiffuseMaterial,
		NormalMaterial,
		EmissiveMaterial,
		CombinedMaterial,
		NumMaterialTypes
	};

	/*struct RendererMesh
	{
		RendererMesh(const Model::Mesh &p_mesh) : m_mesh(p_mesh) { }

		const Model::Mesh &m_mesh;
	};
	struct RendererModel
	{
		RendererModel()
		{
			m_numMeshes = 0;
		}
		RendererModel(const GLuint p_VAO) : m_VAO(p_VAO)
		{
			m_numMeshes = 0;
		}

		GLuint m_VAO;
		size_t m_numMeshes;
	};
	struct RendererShader
	{
		RendererShader(const ShaderUniformUpdater &p_shaderUniform, const GLuint p_shaderHandle) : m_uniformUpdater(p_shaderUniform), m_shaderHandle(p_shaderHandle)
		{
			m_numModels = 0;
		}

		const GLuint m_shaderHandle;
		const ShaderUniformUpdater &m_uniformUpdater;
		size_t m_numModels;
	};
	struct RendererMaterial
	{
		RendererMaterial()
		{
			m_numMeshes = 0;
		}

		GLuint m_materials[NumMaterialTypes];
		size_t m_numMeshes;
	};

	struct Dataset_Mesh
	{
		Dataset_Mesh() : m_mesh(nullptr) { }

		const Model::Mesh *m_mesh;
	};
	struct Dataset_Model
	{
		Dataset_Model() : m_VAO(0) { }
		Dataset_Model(unsigned int p_VAO) : m_VAO(p_VAO) { }

		unsigned int m_VAO;

		std::vector<Dataset_Mesh> m_meshes;
	};
	struct Dataset_Shader
	{
		Dataset_Shader() : m_shaderHandle(0), m_numModels(0), m_modelVectorSize(0), m_uniformUpdater(nullptr) { }
		Dataset_Shader(unsigned int p_shaderHandle, const ShaderUniformUpdater *p_uniformUpdater)
			: m_shaderHandle(p_shaderHandle), m_numModels(0), m_modelVectorSize(0), m_uniformUpdater(p_uniformUpdater) { }

		void clear()
		{
			for(std::vector<Dataset_Model>::size_type i = 0; i < m_modelVectorSize; i++)
				m_models[i].m_meshes.clear();

			m_numModels = 0;
		}

		unsigned int m_shaderHandle;
		const ShaderUniformUpdater *m_uniformUpdater;

		std::vector<Dataset_Model> m_models;
		std::vector<Dataset_Model>::size_type m_numModels;
		std::vector<Dataset_Shader>::size_type m_modelVectorSize;
	};

	struct Datasets
	{
		Datasets() : m_numShaders(0), m_shaderVectorSize(0) { }

		std::vector<Dataset_Shader> m_shaders;
		std::vector<Dataset_Shader>::size_type m_numShaders;
		std::vector<Dataset_Shader>::size_type m_shaderVectorSize;
	};
	
	struct RenderableObjects
	{
		RenderableObjects()
		{
			m_numShaders = 0;
		}
		void clear()
		{
			m_shaders.clear();
			m_models.clear();
			m_materials.clear();
			m_meshes.clear();
			m_numShaders = 0;
		}

		std::vector<RendererShader>		m_shaders;
		std::vector<RendererModel>		m_models;
		std::vector<RendererMaterial>	m_materials;
		std::vector<RendererMesh>		m_meshes;

		size_t m_numShaders;
	};*/

	struct DrawCommand
	{
		DrawCommand(const ShaderUniformUpdater &p_uniformUpdater, unsigned int p_shaderHandle, unsigned int p_modelHandle,
					  unsigned int p_numIndices, unsigned int p_baseVertex, unsigned int p_baseIndex) : m_uniformUpdater(p_uniformUpdater)
		{
			m_shaderHandle = p_shaderHandle;
			m_modelHandle = p_modelHandle;
			m_numIndices = p_numIndices;
			m_baseVertex = p_baseVertex;
			m_baseIndex = p_baseIndex;
		}

		const ShaderUniformUpdater &m_uniformUpdater;
		const UniformObjectData m_uniformObjectData;

		unsigned int m_shaderHandle;
		unsigned int m_modelHandle;

		unsigned int m_numIndices;
		unsigned int m_baseVertex;
		unsigned int m_baseIndex;
	};

	// First element is a sort key, second element is an object to draw
	typedef std::vector<std::pair<int64_t, DrawCommand>> DrawCommands;

	RendererBackend() { }
	~RendererBackend() { }

	ErrorCode init();

	//void renderObjects(DrawCommands &p_objects);
	
protected:

	//const CameraObject *m_currentCamera;
	//const GraphicsData *m_currentObjectData;

	UniformRendererData m_uniformRendererData;
};