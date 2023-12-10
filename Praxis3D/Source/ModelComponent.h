#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"

class ModelComponent : public SystemObject, public LoadableGraphicsObject
{
	friend class RendererScene;
public:
	struct MeshProperties
	{
		MeshProperties()
		{
			m_numOfMeshes = 0;
		}

		void resize(const std::size_t p_size)
		{
			if(p_size > m_numOfMeshes)
			{
				m_numOfMeshes = p_size;

				// Resize mesh materials
				if(p_size > m_meshMaterials.size())
				{
					std::vector<std::string> emptyStrings(MaterialType::MaterialType_NumOfTypes);
					m_meshMaterials.resize(p_size, emptyStrings);
				}

				// Resize mesh materials scale and initialize each element to 1.0f
				const auto oldSize = m_meshMaterialsScale.size();
				if(p_size > oldSize)
				{
					std::vector<glm::vec2> emptyVec2(MaterialType::MaterialType_NumOfTypes, glm::vec2(1.0f, 1.0f));
					m_meshMaterialsScale.resize(p_size, emptyVec2);
				}

				// Resize mesh material alpha threshold and initialize each element to 0.0f
				if(p_size > m_alphaThreshold.size())
					m_alphaThreshold.resize(p_size, Config::graphicsVar().alpha_threshold);

				// Resize mesh material emissive intensity initialize each element to 0.0f
				if(p_size > m_emissiveIntensity.size())
					m_emissiveIntensity.resize(p_size, Config::graphicsVar().emissive_multiplier);

				// Resize mesh material height scale and initialize each element to 1.0f
				if(p_size > m_heightScale.size())
					m_heightScale.resize(p_size, Config::graphicsVar().height_scale);

				// Resize the "mesh is active" array and initialize each element to true
				if(p_size > m_active.size())
					m_active.resize(p_size, true);

				// Resize the "mesh is present" array and initialize each element to false
				if(p_size > m_present.size())
					m_present.resize(p_size, false);
			}
		}

		void clear()
		{
			m_numOfMeshes = 0;
			m_modelName.clear();
			m_meshMaterials.clear();
			m_meshMaterialsScale.clear();
			m_alphaThreshold.clear();
			m_emissiveIntensity.clear();
			m_heightScale.clear();
			m_active.clear();
			m_present.clear();
		}

		std::string m_modelName;

		std::size_t m_numOfMeshes;
		std::vector<std::vector<std::string>> m_meshMaterials;
		std::vector<std::vector<glm::vec2>> m_meshMaterialsScale;
		std::vector<std::string> m_meshNames;
		std::vector<float> m_alphaThreshold;
		std::vector<float> m_emissiveIntensity;
		std::vector<float> m_heightScale;
		std::vector<bool> m_active;
		std::vector<bool> m_present;
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

		return;

		if(m_modelsProperties != nullptr)
		{
			m_modelData.clear();

			// Go over each model
			for(decltype(m_modelsProperties->m_models.size()) modelIndex = 0, modelSize = m_modelsProperties->m_models.size(); modelIndex < modelSize; modelIndex++)
			{
				// Add a new model data entry, and get a reference to it
				m_modelData.emplace_back(Loaders::model().load(m_modelsProperties->m_models[modelIndex].m_modelName, false));
				auto &newModelData = m_modelData.back();

				// Load the model to memory, to be able to access all of its meshes 
				auto modelLoadError = newModelData.m_model.loadToMemory();

				if(modelLoadError == ErrorCode::Success)
				{
					// Get the material arrays that were loaded from the model file
					auto &materialArrayFromModel = newModelData.m_model.getMaterialArrays();

					// Go over each mesh
					for(decltype(newModelData.m_model.getMeshSize()) meshIndex = 0, meshSize = newModelData.m_model.getMeshSize(); meshIndex < meshSize; meshIndex++)
					{
						if(m_modelsProperties->m_models[modelIndex].m_meshMaterials.size() > meshIndex)
						{
							if(m_modelsProperties->m_models[modelIndex].m_present[meshIndex] == true)
							{
								// Define material data and material properties
								MaterialData materials[MaterialType::MaterialType_NumOfTypes];

								// Go over each material type
								for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
								{
									auto filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

									if(!m_modelsProperties->m_models[modelIndex].m_meshMaterials[meshIndex][iMatType].empty())
									{
										filename = m_modelsProperties->m_models[modelIndex].m_meshMaterials[meshIndex][iMatType];
									}

									materials[iMatType].m_texture = Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false);
									auto materialLoadError = materials[iMatType].m_texture.loadToMemory();

									materials[iMatType].m_textureScale = m_modelsProperties->m_models[modelIndex].m_meshMaterialsScale[meshIndex][iMatType];
								}

								// Add the data for this mesh. Include materials loaded from the model itself, if they were present, otherwise, include default textures instead
								newModelData.m_meshes.push_back(MeshData(
									newModelData.m_model.getMeshArray()[meshIndex], 
									materials, 
									m_modelsProperties->m_models[modelIndex].m_heightScale[meshIndex], 
									m_modelsProperties->m_models[modelIndex].m_alphaThreshold[meshIndex], 
									m_modelsProperties->m_models[modelIndex].m_emissiveIntensity[meshIndex]));

								ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_success, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
							}
						}
						else
						{								
							// Define material data and material properties
							MaterialData materials[MaterialType::MaterialType_NumOfTypes];

							// Go over each material type
							for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
							{
								auto filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

								materials[iMatType].m_texture = Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false);
								auto materialLoadError = materials[iMatType].m_texture.loadToMemory();

								materials[iMatType].m_textureScale = glm::vec2(Config::graphicsVar().texture_tiling_factor, Config::graphicsVar().texture_tiling_factor);
							}

							newModelData.m_meshes.push_back(MeshData(newModelData.m_model.getMeshArray()[meshIndex], materials, Config::graphicsVar().height_scale, Config::graphicsVar().alpha_threshold, Config::graphicsVar().emissive_multiplier));

							ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_success, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
						}
					}
				}
				else
				{
					ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_failure, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
				}
			}
		}

		// Set the component as loaded, because the load function was called
		setLoadedToMemory(true);

		if(m_modelsProperties != nullptr)
		{
			delete m_modelsProperties;
			m_modelsProperties = nullptr;
		}
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

				if(!m_modelData.back().m_model.isLoadedToMemory())
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
					if(m_modelsProperties->m_models[modelIndex].m_meshMaterials.size() > meshIndex)// && m_modelsProperties->m_models[modelIndex].m_present[meshIndex] == true)
					{
						MaterialData materials[MaterialType::MaterialType_NumOfTypes];

						// Go over each material type
						for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
						{
							auto filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

							if(!m_modelsProperties->m_models[modelIndex].m_meshMaterials[meshIndex][iMatType].empty())
							{
								filename = m_modelsProperties->m_models[modelIndex].m_meshMaterials[meshIndex][iMatType];
							}

							materials[iMatType].m_texture = Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false);

							if(!materials[iMatType].m_texture.isLoadedToMemory())
								m_texturesNeedLoading = true;

							materials[iMatType].m_textureScale = m_modelsProperties->m_models[modelIndex].m_meshMaterialsScale[meshIndex][iMatType];

							if(materials[iMatType].m_textureScale == glm::vec2(0.0f, 0.0f))
								materials[iMatType].m_textureScale = glm::vec2(Config::graphicsVar().texture_tiling_factor, Config::graphicsVar().texture_tiling_factor);
						}

						bool active = true;
						if(m_modelsProperties->m_models[modelIndex].m_active.size() > meshIndex)
							active = m_modelsProperties->m_models[modelIndex].m_active[meshIndex];

						float heightScale = Config::graphicsVar().height_scale;
						if(m_modelsProperties->m_models[modelIndex].m_heightScale.size() > meshIndex)
							heightScale = m_modelsProperties->m_models[modelIndex].m_heightScale[meshIndex];

						float alphaThreshold = Config::graphicsVar().alpha_threshold;
						if(m_modelsProperties->m_models[modelIndex].m_alphaThreshold.size() > meshIndex)
							alphaThreshold = m_modelsProperties->m_models[modelIndex].m_alphaThreshold[meshIndex];

						float emissiveIntensity = Config::graphicsVar().emissive_multiplier;
						if(m_modelsProperties->m_models[modelIndex].m_emissiveIntensity.size() > meshIndex)
							emissiveIntensity = m_modelsProperties->m_models[modelIndex].m_emissiveIntensity[meshIndex];

						// Add the data for this mesh. Include materials loaded from the model itself, if they were present, otherwise, include default textures instead
						newModelData.m_meshes.push_back(MeshData(
							newModelData.m_model.getMeshArray()[meshIndex],
							materials,
							heightScale,
							alphaThreshold,
							emissiveIntensity,
							active));
					}
					else
					{
						// Define material data and material properties
						MaterialData materials[MaterialType::MaterialType_NumOfTypes];

						// Go over each material type
						for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
						{
							auto filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

							materials[iMatType].m_texture = Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false);

							if(!materials[iMatType].m_texture.isLoadedToMemory())
								m_texturesNeedLoading = true;

							materials[iMatType].m_textureScale = glm::vec2(Config::graphicsVar().texture_tiling_factor, Config::graphicsVar().texture_tiling_factor);
						}

						newModelData.m_meshes.push_back(MeshData(newModelData.m_model.getMeshArray()[meshIndex], materials, Config::graphicsVar().height_scale, Config::graphicsVar().alpha_threshold, Config::graphicsVar().emissive_multiplier, true));

						//ErrHandlerLoc::get().log(ErrorCode::Load_to_memory_success, ErrorSource::Source_ModelComponent, m_modelsProperties->m_models[modelIndex].m_modelName);
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
					m_modelData[modelIndex].m_meshes[meshIndex].m_materials[iMatType].m_texture.loadToMemory();
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
					loadableObjects.emplace_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex].m_texture);
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
					if(!m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex].m_texture.isLoadedToVideoMemory())
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

			// Loop over each mesh
			for(decltype(m_modelData[modelIndex].m_meshes.size()) meshSize = m_modelData[modelIndex].m_meshes.size(), meshIndex = 0; meshIndex < meshSize; meshIndex++)
			{
				// Declaration of material and scale vectors, that are filled in during the material loop
				std::vector<std::string> meshMaterials;
				std::vector<glm::vec2> meshMaterialScales;
				bool materialPresent = false;

				// Loop over each material
				for(unsigned int materialIndex = 0; materialIndex < MaterialType::MaterialType_NumOfTypes; materialIndex++)
				{
					// Mark material as present if any of the textures are not default
					if(!Loaders::texture2D().isTextureDefault(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex].m_texture))
						materialPresent = true;

					// Add texture filename
					meshMaterials.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex].m_texture.getFilename());

					// Add texture scale
					meshMaterialScales.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_materials[materialIndex].m_textureScale);
				}

				newModelEntry.m_meshMaterials.push_back(meshMaterials);
				newModelEntry.m_meshMaterialsScale.push_back(meshMaterialScales);
				newModelEntry.m_meshNames.push_back(m_modelData[modelIndex].m_model.getMeshName(meshIndex));
				newModelEntry.m_alphaThreshold.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_alphaThreshold);
				newModelEntry.m_emissiveIntensity.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_emissiveIntensity);
				newModelEntry.m_heightScale.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_heightScale);
				newModelEntry.m_active.push_back(m_modelData[modelIndex].m_meshes[meshIndex].m_active);
				newModelEntry.m_present.push_back(materialPresent);
				newModelEntry.m_numOfMeshes++;
			}
		}
	}

	const inline void resetLoadPending() { m_loadPending = false; }
	const inline void resetModelsNeedsLoading() { m_modelsNeedLoading = false; }
	const inline void resetTexturesNeedsLoading() { m_texturesNeedLoading = false; }

private:
	inline MaterialData loadMaterialData(PropertySet &p_materialProperty, Model::MaterialArrays &p_materialArraysFromModel, MaterialType p_materialType, std::size_t p_meshIndex)
	{
		// Declare the material data that is to be returned and a flag showing whether the material data was loaded successfully
		MaterialData newMaterialData;
		bool materialWasLoaded = false;

		// Try to load the material from the filename retrieved from properties
		if(p_materialProperty)
		{
			// Get texture filename property, check if it is valid
			auto filenameProperty = p_materialProperty.getPropertyByID(Properties::Filename);
			if(filenameProperty.isVariableTypeString())
			{
				// Get texture filename string, check if it is valid
				auto filename = filenameProperty.getString();
				if(!filename.empty())
				{
					// Get the texture and load it to memory
					auto materialHandle = Loaders::texture2D().load(filename, p_materialType, false);
					auto materialLoadError = materialHandle.loadToMemory();

					// Check if the texture was loaded successfully
					if(materialLoadError == ErrorCode::Success)
					{
						newMaterialData.m_texture = materialHandle;
						materialWasLoaded = true;
					}
					else
					{
						ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
					}
				}
			}
		}

		// Try to load the material from the filename retrieved from the model
		if(!materialWasLoaded)
		{
			// Check if there are enough materials, and if the material isn't default
			if(p_materialArraysFromModel.m_numMaterials > p_meshIndex 
				&& !p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].isEmpty() 
				&& !p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].isDefaultMaterial())
			{
				// Get the texture and load it to memory
				auto materialHandle = Loaders::texture2D().load(p_materialArraysFromModel.m_materials[p_materialType][p_meshIndex].m_filename, p_materialType, false);
				auto materialLoadError = materialHandle.loadToMemory();
				
				// Check if the texture was loaded successfully
				if(materialLoadError == ErrorCode::Success)
				{
					newMaterialData.m_texture = materialHandle;
					materialWasLoaded = true;
				}
				else
				{
					ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
				}
			}
		}
		
		// All attempts to load the material were unsuccessful, so load a default material
		if(!materialWasLoaded)
		{
			newMaterialData.m_texture = Loaders::texture2D().getDefaultTexture(p_materialType);
		}

		// Return the newly loaded material data
		return newMaterialData;
	}
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