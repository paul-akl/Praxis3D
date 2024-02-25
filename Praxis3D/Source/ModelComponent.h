#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"

class ModelComponent : public SystemObject, public LoadableGraphicsObject
{
	friend class RendererScene;
public:
	struct SingleMeshData
	{
		SingleMeshData()
		{
			m_meshMaterials.resize(MaterialType::MaterialType_NumOfTypes);
			m_meshMaterialScales.resize(MaterialType::MaterialType_NumOfTypes, glm::vec2(1.0f));
			m_meshMaterialFraming.resize(MaterialType::MaterialType_NumOfTypes, glm::vec2(0.0f));
			m_meshMaterialColors.resize(MaterialType::MaterialType_NumOfTypes, glm::vec4(1.0f));

			m_alphaThreshold = Config::graphicsVar().alpha_threshold;
			m_emissiveIntensity = Config::graphicsVar().emissive_multiplier;
			m_heightScale = Config::graphicsVar().height_scale;
			m_stochasticSamplingScale = 1.0f;
			m_active = true;
			m_present = false;
			m_stochasticSampling = false;
			m_textureWrapMode = TextureWrapType::TextureWrapType_Repeat;
		}

		SingleMeshData(
			const std::vector<std::string> &p_meshMaterials,
			const std::vector<glm::vec2> &p_meshMaterialsScales,
			const std::vector<glm::vec2> &p_meshMaterialFraming,
			const std::vector<glm::vec4> &p_meshMaterialsColor,
			const std::string &p_meshName,
			const float p_alphaThreshold,
			const float p_emissiveIntensity,
			const float p_heightScale,
			const float p_stochasticSamplingScale,
			const bool p_active,
			const bool p_present,
			const bool p_stochasticSampling,
			const TextureWrapType p_textureWrapMode) :
			m_meshMaterials(p_meshMaterials),
			m_meshMaterialScales(p_meshMaterialsScales),
			m_meshMaterialFraming(p_meshMaterialFraming),
			m_meshMaterialColors(p_meshMaterialsColor),
			m_meshName(p_meshName),
			m_alphaThreshold(p_alphaThreshold),
			m_emissiveIntensity(p_emissiveIntensity),
			m_heightScale(p_heightScale),
			m_stochasticSamplingScale(p_stochasticSamplingScale),
			m_active(p_active),
			m_present(p_present),
			m_stochasticSampling(p_stochasticSampling),
			m_textureWrapMode(p_textureWrapMode) { }

		std::vector<std::string> m_meshMaterials;
		std::vector<glm::vec2> m_meshMaterialScales;
		std::vector<glm::vec2> m_meshMaterialFraming;
		std::vector<glm::vec4> m_meshMaterialColors;
		std::string m_meshName;
		float m_alphaThreshold;
		float m_emissiveIntensity;
		float m_heightScale;
		float m_stochasticSamplingScale;
		bool m_active;
		bool m_present;
		bool m_stochasticSampling;
		TextureWrapType m_textureWrapMode;
	};
	struct MeshProperties
	{
		MeshProperties() { }
		MeshProperties(
			const FaceCullingSettings p_drawFaceCulling, 
			const FaceCullingSettings p_shadowFaceCulling) : 
			m_drawFaceCulling(p_drawFaceCulling), 
			m_shadowFaceCulling(p_shadowFaceCulling) { }

		void resize(const std::size_t p_size)
		{
			if(p_size > m_meshData.size())
				m_meshData.resize(p_size);
		}

		void clear()
		{
			m_modelName.clear();
			m_meshData.clear();
		}

		std::string m_modelName;
		std::vector<SingleMeshData> m_meshData;

		FaceCullingSettings m_drawFaceCulling;
		FaceCullingSettings m_shadowFaceCulling;
	};
	struct ModelsProperties
	{
		std::vector<MeshProperties> m_models;
	};

	struct ModelComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		ModelComponentConstructionInfo()
		{

		}

		ModelsProperties m_modelsProperties;
	};

	ModelComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::ModelComponent, p_entityID)
	{
		m_modelsProperties = nullptr;
		m_modelsNeedLoading = false;
		m_texturesNeedLoading = false;
		m_loadPending = false;
	}
	~ModelComponent() { }

	ErrorCode init() { return ErrorCode::Success; }

	void loadToMemory()
	{
		importModels();
		if(m_modelsNeedLoading)
			loadModelsToMemory();

		importTextures();
		if(m_texturesNeedLoading)
			loadTexturesToMemory();
	}

	void importModels()
	{
		m_modelsNeedLoading = false;

		if(m_modelsProperties != nullptr)
		{
			m_modelData.clear();

			// Go over each model
			for(decltype(m_modelsProperties->m_models.size()) modelIndex = 0, modelSize = m_modelsProperties->m_models.size(); modelIndex < modelSize; modelIndex++)
			{
				// Add a new model data entry, and get a reference to it
				m_modelData.emplace_back(Loaders::model().load(m_modelsProperties->m_models[modelIndex].m_modelName, false));

				auto &newModel = m_modelData.back();

				// Add face culling settings
				newModel.m_drawFaceCulling = m_modelsProperties->m_models[modelIndex].m_drawFaceCulling;
				newModel.m_shadowFaceCulling = m_modelsProperties->m_models[modelIndex].m_shadowFaceCulling;

				if(!newModel.m_model.isLoadedToMemory())
					m_modelsNeedLoading = true;
			}
		}
	}

	void importTextures()
	{
		m_texturesNeedLoading = false;

		if(m_modelsProperties != nullptr)
		{
			// Go over each model
			for(decltype(m_modelsProperties->m_models.size()) modelIndex = 0, modelSize = m_modelsProperties->m_models.size(); modelIndex < modelSize; modelIndex++)
			{
				auto &newModelData = m_modelData[modelIndex];

				// Get the material arrays that were loaded from the model file
				auto &materialArrayFromModel = newModelData.m_model.getMaterialArrays();

				// Go over each mesh
				for(decltype(newModelData.m_model.getMeshSize()) meshIndex = 0, meshSize = newModelData.m_model.getMeshSize(); meshIndex < meshSize; meshIndex++)
				{
					// Define material data and material properties
					std::vector<TextureLoader2D::Texture2DHandle> materials;// (MaterialType::MaterialType_NumOfTypes, Loaders::texture2D().getDefaultTexture());
					materials.reserve(MaterialType::MaterialType_NumOfTypes);
					MaterialData materialData;

					if(m_modelsProperties->m_models[modelIndex].m_meshData.size() > meshIndex)// && m_modelsProperties->m_models[modelIndex].m_present[meshIndex] == true)
					{
						// Go over each material type
						for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
						{
							std::string filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

							if(!m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_meshMaterials[iMatType].empty())
							{
								filename = m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_meshMaterials[iMatType];
							}

							materials.push_back(Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false));

							if(!materials[iMatType].isLoadedToMemory())
								m_texturesNeedLoading = true;

							materialData.m_parameters[iMatType].m_scale = m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_meshMaterialScales[iMatType];

							if(materialData.m_parameters[iMatType].m_scale == glm::vec2(0.0f, 0.0f))
								materialData.m_parameters[iMatType].m_scale = glm::vec2(Config::graphicsVar().texture_tiling_factor, Config::graphicsVar().texture_tiling_factor);

							materialData.m_parameters[iMatType].m_framing = m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_meshMaterialFraming[iMatType];

							materialData.m_parameters[iMatType].m_color =  m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_meshMaterialColors[iMatType];
						}

						// Add the data for this mesh. Include materials loaded from the model itself, if they were present, otherwise, include default textures instead
						newModelData.m_meshes.push_back(MeshData(
							newModelData.m_model.getMeshArray()[meshIndex],
							materials,
							materialData,
							m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_heightScale,
							m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_alphaThreshold,
							m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_emissiveIntensity,
							m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_stochasticSampling,
							m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_stochasticSamplingScale,
							m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_textureWrapMode,
							m_modelsProperties->m_models[modelIndex].m_meshData[meshIndex].m_active));
					}
					else
					{
						// Go over each material type
						for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
						{
							std::string filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

							materials.push_back(Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false));

							if(!materials[iMatType].isLoadedToMemory())
								m_texturesNeedLoading = true;

							materialData.m_parameters[iMatType].m_scale = glm::vec2(Config::graphicsVar().texture_tiling_factor, Config::graphicsVar().texture_tiling_factor);
						}

						newModelData.m_meshes.push_back(MeshData(
							newModelData.m_model.getMeshArray()[meshIndex], 
							materials,
							materialData,
							Config::graphicsVar().height_scale, 
							Config::graphicsVar().alpha_threshold, 
							Config::graphicsVar().emissive_multiplier, 
							false, Config::graphicsVar().stochastic_sampling_scale, 
							TextureWrapType::TextureWrapType_Repeat, 
							true));
					}
				}
			}
		}

		if(!m_texturesNeedLoading)
		{
			// Set the component as loaded, because the load function was called
			setLoadedToMemory(true);

			if(m_modelsProperties != nullptr)
			{
				delete m_modelsProperties;
				m_modelsProperties = nullptr;
			}
		}
	}

	void loadModelsToMemory()
	{
		// Go over each model
		for(decltype(m_modelData.size()) modelIndex = 0, modelSize = m_modelData.size(); modelIndex < modelSize; modelIndex++)
		{
			// Load the model to memory, to be able to access all of its meshes 
			auto modelLoadError = m_modelData[modelIndex].m_model.loadToMemory();

			if(modelLoadError == ErrorCode::Success)
				ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_success, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
			else
				ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_failure, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
		}

		resetModelsNeedsLoading();

		importTextures();
		if(m_texturesNeedLoading)
			loadTexturesToMemory();
	}

	void loadTexturesToMemory()
	{
		// Go over each model
		for(decltype(m_modelData.size()) modelIndex = 0, modelSize = m_modelData.size(); modelIndex < modelSize; modelIndex++)
		{
			// Get the material arrays that were loaded from the model file
			auto &materialArrayFromModel = m_modelData[modelIndex].m_model.getMaterialArrays();

			// Go over each mesh
			for(decltype(m_modelData[modelIndex].m_meshes.size()) meshIndex = 0, meshSize = m_modelData[modelIndex].m_meshes.size(); meshIndex < meshSize; meshIndex++)
			{
				// Go over each material type
				for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
					m_modelData[modelIndex].m_meshes[meshIndex].m_materials[iMatType].loadToMemory();
			}
		}

		// Set the component as loaded, because the load function was called
		setLoadedToMemory(true);
		resetModelsNeedsLoading();
		resetTexturesNeedsLoading();

		if(m_modelsProperties != nullptr)
		{
			delete m_modelsProperties;
			m_modelsProperties = nullptr;
		}
	}

	// Unload all assets from RAM (does not unload from GPU VRAM)
	inline void unloadFromMemory()
	{
		unloadModelsFromMemory();
		unloadTexturesFromMemory();
	}

	// Unload all models from RAM (does not unload from GPU VRAM)
	void unloadModelsFromMemory()
	{
		// Go over each model
		for(decltype(m_modelData.size()) modelIndex = 0, modelSize = m_modelData.size(); modelIndex < modelSize; modelIndex++)
		{
			//m_modelData[modelIndex].m_model.u
			//// Load the model to memory, to be able to access all of its meshes 
			//auto modelLoadError = m_modelData[modelIndex].m_model.loadToMemory();

			//if(modelLoadError == ErrorCode::Success)
			//	ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_success, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
			//else
			//	ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_failure, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
		}
	}

	// Unload all textures from RAM (does not unload from GPU VRAM)
	void unloadTexturesFromMemory()
	{
		// Go over each model
		for(decltype(m_modelData.size()) modelIndex = 0, modelSize = m_modelData.size(); modelIndex < modelSize; modelIndex++)
		{
			// Get the material arrays that were loaded from the model file
			auto &materialArrayFromModel = m_modelData[modelIndex].m_model.getMaterialArrays();

			// Go over each mesh
			for(decltype(m_modelData[modelIndex].m_meshes.size()) meshIndex = 0, meshSize = m_modelData[modelIndex].m_meshes.size(); meshIndex < meshSize; meshIndex++)
			{
				// Go over each material type
				for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
					if(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[iMatType].isLoadedToMemory() && m_modelData[modelIndex].m_meshes[meshIndex].m_materials[iMatType].isLoadedToVideoMemory())
						if(auto error = m_modelData[modelIndex].m_meshes[meshIndex].m_materials[iMatType].unloadFromMemory(); error != ErrorCode::Success)
							ErrHandlerLoc::get().log(error, m_name, ErrorSource::Source_ModelComponent);
			}
		}
	}

	BitMask getSystemType() { return Systems::Graphics; }

	void update(const float p_deltaTime) { }

	BitMask getDesiredSystemChanges() { return Systems::Changes::None; }

	BitMask getPotentialSystemChanges() { return Systems::Changes::None; }

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType)
	{
		if(CheckBitmask(p_changeType, Systems::Changes::Generic::Active))
			setActive(p_subject->getBool(this, Systems::Changes::Generic::Active));
	}

	void receiveData(const DataType p_dataType, void *p_data, const bool p_deleteAfterReceiving = false)
	{
		switch(p_dataType)
		{
			case DataType_ModelsProperties:
				{
					if(m_modelsProperties != nullptr)
						delete m_modelsProperties;

					m_modelsProperties = new ModelsProperties(*static_cast<ModelsProperties*>(p_data));

					m_loadPending = true;
				}
				break;

			default:
				assert(p_deleteAfterReceiving == true && "Memory leak - unhandled orphaned void data pointer in receiveData");
				break;
		}
	}

	std::vector<LoadableObjectsContainer> getLoadableObjects()
	{
		std::vector<LoadableObjectsContainer> loadableObjects;

		// Go over each model
		for(decltype(m_modelData.size()) modelSize = m_modelData.size(), modelIndex = 0; modelIndex < modelSize; modelIndex++)
		{
			// Add model handle to the loadable object list
			loadableObjects.emplace_back(m_modelData[modelIndex].m_model);

			// Go over each mesh -> go over each material -> add texture handle to the loadable object list
			for(decltype(m_modelData[modelIndex].m_meshes.size()) meshSize = m_modelData[modelIndex].m_meshes.size(), meshIndex = 0; meshIndex < meshSize; meshIndex++)
				for(unsigned int materialIndex = 0; materialIndex < MaterialType::MaterialType_NumOfTypes; materialIndex++)
					loadableObjects.emplace_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex]);
		}

		return loadableObjects;
	}

	void performCheckIsLoadedToVideoMemory() 
	{
		setLoadedToVideoMemory(false);

		for(decltype(m_modelData.size()) modelSize = m_modelData.size(), modelIndex = 0; modelIndex < modelSize; modelIndex++)
		{
			if(!m_modelData[modelIndex].m_model.isLoadedToVideoMemory())
				return;

			for(decltype(m_modelData[modelIndex].m_meshes.size()) meshSize = m_modelData[modelIndex].m_meshes.size(), meshIndex = 0; meshIndex < meshSize; meshIndex++)
			{
				for(unsigned int materialIndex = 0; materialIndex < MaterialType::MaterialType_NumOfTypes; materialIndex++)
				{
					if(!m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex].isLoadedToVideoMemory())
						return;
				}
			}
		}

		setLoadedToVideoMemory(true);
	}

	const inline std::vector<ModelData> &getModelData() const { return m_modelData; }
	const inline bool getLoadPending() const { return m_loadPending; }
	const inline bool getModelsNeedsLoading() const { return m_modelsNeedLoading; }
	const inline bool getTexturesNeedsLoading() const { return m_texturesNeedLoading; }

	inline void getModelsProperties(ModelsProperties &p_modelsProperties) const
	{
		// Clear the models array if it's not empty
		if(!p_modelsProperties.m_models.empty())
			p_modelsProperties.m_models.clear();

		// Go over each model
		for(decltype(m_modelData.size()) modelSize = m_modelData.size(), modelIndex = 0; modelIndex < modelSize; modelIndex++)
		{
			// Add a new model entry
			p_modelsProperties.m_models.push_back(MeshProperties());
			auto &newModelEntry = p_modelsProperties.m_models.back();

			// Add model filename to the models properties
			newModelEntry.m_modelName = m_modelData[modelIndex].m_model.getFilename();

			// Add face culling settings
			newModelEntry.m_drawFaceCulling = m_modelData[modelIndex].m_drawFaceCulling;
			newModelEntry.m_shadowFaceCulling = m_modelData[modelIndex].m_shadowFaceCulling;

			// Loop over each mesh
			for(decltype(m_modelData[modelIndex].m_meshes.size()) meshSize = m_modelData[modelIndex].m_meshes.size(), meshIndex = 0; meshIndex < meshSize; meshIndex++)
			{
				// Declaration of material and scale vectors, that are filled in during the material loop
				std::vector<std::string> meshMaterials;
				std::vector<glm::vec2> meshMaterialScales;
				std::vector<glm::vec2> meshMaterialFramings;
				std::vector<glm::vec4> meshMaterialColors;
				bool materialPresent = false;

				// Loop over each material
				for(unsigned int materialIndex = 0; materialIndex < MaterialType::MaterialType_NumOfTypes; materialIndex++)
				{
					// Mark material as present if any of the textures are not default
					if(!Loaders::texture2D().isTextureDefault(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex]))
						materialPresent = true;

					// Add texture filename
					meshMaterials.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex].getFilename());

					// Add texture scale
					meshMaterialScales.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materialData.m_parameters[materialIndex].m_scale);

					// Add texture framing
					meshMaterialFramings.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materialData.m_parameters[materialIndex].m_framing);

					// Add texture color
					meshMaterialColors.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materialData.m_parameters[materialIndex].m_color);
				}

				newModelEntry.m_meshData.push_back(SingleMeshData(
					meshMaterials,
					meshMaterialScales,
					meshMaterialFramings,
					meshMaterialColors,
					m_modelData[modelIndex].m_model.getMeshName(meshIndex),
					m_modelData[modelIndex].m_meshes[meshIndex].m_alphaThreshold,
					m_modelData[modelIndex].m_meshes[meshIndex].m_emissiveIntensity,
					m_modelData[modelIndex].m_meshes[meshIndex].m_heightScale,
					m_modelData[modelIndex].m_meshes[meshIndex].m_textureRepetitionScale,
					m_modelData[modelIndex].m_meshes[meshIndex].m_active,
					materialPresent,
					m_modelData[modelIndex].m_meshes[meshIndex].m_stochasticSampling,
					m_modelData[modelIndex].m_meshes[meshIndex].m_textureWrapMode
				));
			}
		}
 	}

	const inline void resetLoadPending() { m_loadPending = false; }
	const inline void resetModelsNeedsLoading() { m_modelsNeedLoading = false; }
	const inline void resetTexturesNeedsLoading() { m_texturesNeedLoading = false; }

private:
	inline void adjustMeshArraySizes()
	{
		// Go over each model
		for(decltype(m_modelData.size()) modelIndex = 0, modelSize = m_modelData.size(); modelIndex < modelSize; modelIndex++)
		{
			auto numOfMeshes = m_modelData[modelIndex].m_model.getNumMeshes();

			if(m_modelData[modelIndex].m_meshes.size() != numOfMeshes)
			{
				if(m_modelData[modelIndex].m_meshes.size() > numOfMeshes)
				{
					while(m_modelData[modelIndex].m_meshes.size() != numOfMeshes)
						m_modelData[modelIndex].m_meshes.pop_back();
				}
				else
				{
					//while(m_modelData[modelIndex].m_meshes.size() != numOfMeshes)
					//{
					//	auto meshIndex = m_modelData[modelIndex].m_meshes.size();

					//	if(m_modelsProperties->m_models[modelIndex].m_meshMaterials.size() > meshIndex)
					//	{
					//		if(m_modelsProperties->m_models[modelIndex].m_present[meshIndex] == true)
					//		{
					//			// Define material data and material properties
					//			MaterialData materials[MaterialType::MaterialType_NumOfTypes];

					//			// Go over each material type
					//			for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
					//			{
					//				auto filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

					//				if(!m_modelsProperties->m_models[modelIndex].m_meshMaterials[meshIndex][iMatType].empty())
					//				{
					//					filename = m_modelsProperties->m_models[modelIndex].m_meshMaterials[meshIndex][iMatType];
					//				}

					//				materials[iMatType].m_texture = Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false);

					//				if(!materials[iMatType].m_texture.isLoadedToMemory())
					//					m_texturesNeedLoading = true;

					//				materials[iMatType].m_textureScale = m_modelsProperties->m_models[modelIndex].m_meshMaterialsScale[meshIndex][iMatType];
					//			}

					//			// Add the data for this mesh. Include materials loaded from the model itself, if they were present, otherwise, include default textures instead
					//			newModelData.m_meshes.push_back(MeshData(
					//				newModelData.m_model.getMeshArray()[meshIndex],
					//				materials,
					//				m_modelsProperties->m_models[modelIndex].m_heightScale[meshIndex],
					//				m_modelsProperties->m_models[modelIndex].m_alphaThreshold[meshIndex],
					//				m_modelsProperties->m_models[modelIndex].m_emissiveIntensity[meshIndex]));

					//		}
					//	}
					//	else
					//	{
					//		// Define material data and material properties
					//		MaterialData materials[MaterialType::MaterialType_NumOfTypes];

					//		// Go over each material type
					//		for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
					//		{
					//			auto filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

					//			materials[iMatType].m_texture = Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false);

					//			if(!materials[iMatType].m_texture.isLoadedToMemory())
					//				m_texturesNeedLoading = true;

					//			materials[iMatType].m_textureScale = glm::vec2(Config::graphicsVar().texture_tiling_factor, Config::graphicsVar().texture_tiling_factor);
					//		}

					//		newModelData.m_meshes.push_back(MeshData(newModelData.m_model.getMeshArray()[meshIndex], materials, Config::graphicsVar().height_scale, Config::graphicsVar().alpha_threshold, Config::graphicsVar().emissive_multiplier));

					//	}
					//}
				}
			}
		}
	}

	std::vector<ModelData> m_modelData;
	ModelsProperties *m_modelsProperties;

	bool m_modelsNeedLoading;
	bool m_texturesNeedLoading;

	bool m_loadPending;
};