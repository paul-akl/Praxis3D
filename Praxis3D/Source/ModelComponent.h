#pragma once

#include "BaseGraphicsComponent.h"
#include "GraphicsDataSets.h"
#include "InheritanceObjects.h"

class ModelComponent : public SystemObject, public LoadableGraphicsObject
{
	friend class RendererScene;
public:
	struct MeshMaterialsProperties
	{
		MeshMaterialsProperties()
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
					//m_meshMaterials.emplace_back(emptyStrings);
				}

				// Resize mesh materials scale and initialize each element to 1.0f
				const auto oldSize = m_meshMaterialsScale.size();
				if(p_size > oldSize)
				{
					std::vector<glm::vec2> emptyVec2(MaterialType::MaterialType_NumOfTypes, glm::vec2(1.0f, 1.0f));
					m_meshMaterialsScale.resize(p_size, emptyVec2);

					//for(size_t i = oldSize; i < p_size; i++)
					//	for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
					//		m_meshMaterialsScale[i][iMatType] = glm::vec2(1.0f, 1.0f);
				}

				// Resize mesh material alpha threshold and initialize each element to 0.0f
				if(p_size > m_alphaThreshold.size())
					m_alphaThreshold.resize(p_size, 0.0f);

				// Resize mesh material height scale and initialize each element to 1.0f
				if(p_size > m_heightScale.size())
					m_heightScale.resize(p_size, 1.0f);

				// Resize the "mesh is present" array and initialize each element to false
				if(p_size > m_present.size())
					m_present.resize(p_size, false);
			}
		}

		std::size_t m_numOfMeshes;
		std::vector<std::vector<std::string>> m_meshMaterials;
		std::vector< std::vector<glm::vec2>> m_meshMaterialsScale;
		std::vector<float> m_alphaThreshold;
		std::vector<float> m_heightScale;
		std::vector<bool> m_present;
	};
	struct ModelsProperties
	{
		std::vector<std::string> m_modelNames;
	};

	struct ModelComponentConstructionInfo : public SystemObject::SystemObjectConstructionInfo
	{
		ModelComponentConstructionInfo()
		{

		}

		ModelsProperties m_modelsProperties;
		MeshMaterialsProperties m_materialsFromProperties;
	};

	ModelComponent(SystemScene *p_systemScene, std::string p_name, const EntityID p_entityID, std::size_t p_id = 0) : SystemObject(p_systemScene, p_name, Properties::PropertyID::Models, p_entityID)
	{
		m_materialsFromProperties = nullptr;
		m_modelsProperties = nullptr;
	}
	~ModelComponent() { }

	ErrorCode init() { return ErrorCode::Success; }

	void loadToMemory()
	{
		if(m_modelsProperties != nullptr)
		{
			for(decltype(m_modelsProperties->m_modelNames.size()) modelIndex = 0, modelSize = m_modelsProperties->m_modelNames.size(); modelIndex < modelSize; modelIndex++)
			{
				// Add a new model data entry, and get a reference to it
				m_modelData.emplace_back(Loaders::model().load(m_modelsProperties->m_modelNames[modelIndex], false));
				auto &newModelData = m_modelData.back();

				// Load the model to memory, to be able to access all of its meshes 
				auto modelLoadError = newModelData.m_model.loadToMemory();

				if(modelLoadError == ErrorCode::Success)
				{
					// Get the material arrays that were loaded from the model file
					auto &materialArrayFromModel = newModelData.m_model.getMaterialArrays();

					for(decltype(newModelData.m_model.getMeshSize()) meshIndex = 0, meshSize = newModelData.m_model.getMeshSize(); meshIndex < meshSize; meshIndex++)
					{
						if(m_materialsFromProperties != nullptr && m_materialsFromProperties->m_meshMaterials.size() > meshIndex)
						{
							if(m_materialsFromProperties->m_present[meshIndex] == true)
							{
								// Define material data and material properties
								MaterialData materials[MaterialType::MaterialType_NumOfTypes];

								// Go over each material type
								for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
								{
									auto filename = materialArrayFromModel.m_materials[iMatType][meshIndex].m_filename;

									if(!m_materialsFromProperties->m_meshMaterials[meshIndex][iMatType].empty())
									{
										filename = m_materialsFromProperties->m_meshMaterials[meshIndex][iMatType];
									}

									materials[iMatType].m_texture = Loaders::texture2D().load(filename, static_cast<MaterialType>(iMatType), false);
									auto materialLoadError = materials[iMatType].m_texture.loadToMemory();

									materials[iMatType].m_textureScale = m_materialsFromProperties->m_meshMaterialsScale[meshIndex][iMatType];
								}

								// Add the data for this mesh. Include materials loaded from the model itself, if they were present, otherwise, include default textures instead

								newModelData.m_meshes.push_back(MeshData(newModelData.m_model.getMeshArray()[meshIndex], materials, m_materialsFromProperties->m_heightScale[meshIndex], m_materialsFromProperties->m_alphaThreshold[meshIndex]));

								ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ModelComponent, m_name + " - Model \'" + m_name + "\' loaded to memory");
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

							newModelData.m_meshes.push_back(MeshData(newModelData.m_model.getMeshArray()[meshIndex], materials, Config::graphicsVar().height_scale, Config::graphicsVar().alpha_threshold));

							ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ModelComponent, m_name + " - Model \'" + m_name + "\' loaded to memory");
						}
					}
				}
			}
		}

		// Set the component as loaded, because the load function was called
		setLoadedToMemory(true);

		if(m_materialsFromProperties != nullptr)
		{
			delete m_materialsFromProperties;
			m_materialsFromProperties = nullptr;
		}
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

	void changeOccurred(ObservedSubject *p_subject, BitMask p_changeType) { }

	ErrorCode importObject(const PropertySet &p_properties)
	{
		ErrorCode importError = ErrorCode::Failure;

		// Check if models node is present and the component hasn't been loaded already
		if(p_properties && !isLoadedToMemory())
		{
			auto &modelsProperty = p_properties.getPropertySetByID(Properties::Models);

			if(modelsProperty)
			{
				importError = ErrorCode::Success;

				if(m_modelsProperties != nullptr)
					delete m_modelsProperties;

				m_modelsProperties = new ModelsProperties();

				// Loop over each model entry in the node
				for(decltype(modelsProperty.getNumPropertySets()) iModel = 0, numModels = modelsProperty.getNumPropertySets(); iModel < numModels; iModel++)
				{
					// Get model filename
					auto modelName = modelsProperty.getPropertySet(iModel).getPropertyByID(Properties::Filename).getString();

					// Add a new model data entry, and get a reference to it
					//m_modelData.emplace_back(Loaders::model().load(modelName, false));
					m_modelsProperties->m_modelNames.push_back(modelName);
					//auto &newModelData = m_modelData.back();

					// Load the model to memory, to be able to access all of its meshes
					//auto modelLoadError = newModelData.m_model.loadToMemory();

					//if(modelLoadError == ErrorCode::Success)
					//{
						// Set the component as not being empty anymore, since a model has been loaded successfully
						//setEmpty(false);

						// Get the meshes array
						//const std::vector<Model::Mesh> &meshesInModelArray = newModelData.m_model.getMeshArray();

						// Get the meshes array
					auto &meshesProperty = modelsProperty.getPropertySet(iModel).getPropertySetByID(Properties::Meshes);

					// Check if the meshes array node is present;
					// If it is present, only add the meshes included in the meshes node
					// If it is not present, add all the meshes included in the model
					if(meshesProperty)
					{
						if(meshesProperty.getNumPropertySets() > 0)
						{
							m_materialsFromProperties = new MeshMaterialsProperties();

							// Loop over each mesh entry in the model node
							for(decltype(meshesProperty.getNumPropertySets()) iMesh = 0, numMeshes = meshesProperty.getNumPropertySets(); iMesh < numMeshes; iMesh++)
							{
								// Try to get the mesh index property node and check if it is present
								auto &meshIndexProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::Index);
								if(meshIndexProperty)
								{
									// Get the mesh index, check if it is valid and within the range of mesh array that was loaded from the model
									const int meshDataIndex = meshIndexProperty.getInt();

									// Make sure the meshMaterials vector can fit the given mesh index
									if(meshDataIndex >= m_materialsFromProperties->m_meshMaterials.size())
									{
										m_materialsFromProperties->resize(meshDataIndex + 1);
										m_materialsFromProperties->m_present[meshDataIndex] = true;
									}

									// Get material alpha threshold value, if it is present
									auto alphaThresholdProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::AlphaThreshold);
									if(alphaThresholdProperty)
										m_materialsFromProperties->m_alphaThreshold[meshDataIndex] = alphaThresholdProperty.getFloat();

									// Get material height scale value, if it is present
									auto heightScaleProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::HeightScale);
									if(heightScaleProperty)
										m_materialsFromProperties->m_heightScale[meshDataIndex] = heightScaleProperty.getFloat();

									// Get material properties
									auto materialsProperty = meshesProperty.getPropertySet(iMesh).getPropertySetByID(Properties::Materials);

									// Define material data and material properties
									MaterialData materials[MaterialType::MaterialType_NumOfTypes];
									PropertySet materialProperties[MaterialType::MaterialType_NumOfTypes] =
									{
										materialsProperty.getPropertySetByID(Properties::Diffuse),
										materialsProperty.getPropertySetByID(Properties::Normal),
										materialsProperty.getPropertySetByID(Properties::Emissive),
										materialsProperty.getPropertySetByID(Properties::RMHAO)
									};

									// Go over each material type
									for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
									{
										// Check if an entry for the current material type was present within the properties
										if(materialProperties[iMatType])
										{
											// Get texture filename property, check if it is valid
											auto filenameProperty = materialProperties[iMatType].getPropertyByID(Properties::Filename);
											if(filenameProperty.isVariableTypeString())
											{
												// Get texture filename string, check if it is valid
												m_materialsFromProperties->m_meshMaterials[meshDataIndex][iMatType] = filenameProperty.getString();
											}

											// Get texture scale property, check if it is valid
											auto scaleProperty = materialProperties[iMatType].getPropertyByID(Properties::TextureScale);
											if(scaleProperty)
												m_materialsFromProperties->m_meshMaterialsScale[meshDataIndex][iMatType] = scaleProperty.getVec2f();
										}
									}
								}
							}
						}

						/*/ Loop over each mesh entry in the model node
						for(decltype(meshesProperty.getNumPropertySets()) iMesh = 0, numMeshes = meshesProperty.getNumPropertySets(); iMesh < numMeshes; iMesh++)
						{
							// Try to get the mesh index property node and check if it is present
							auto &meshIndexProperty = meshesProperty.getPropertySet(iMesh).getPropertyByID(Properties::Index);
							if(meshIndexProperty)
							{
								// Get the mesh index, check if it is valid and within the range of mesh array that was loaded from the model
								const int meshDataIndex = meshIndexProperty.getInt();
								if(meshDataIndex >= 0 && meshDataIndex < meshesInModelArray.size())
								{
									// Get material properties
									auto materialsProperty = meshesProperty.getPropertySet(iMesh).getPropertySetByID(Properties::Materials);

									// Define material data and material properties
									MaterialData materials[MaterialType::MaterialType_NumOfTypes];
									PropertySet materialProperties[MaterialType::MaterialType_NumOfTypes] =
									{
										materialsProperty.getPropertySetByID(Properties::Diffuse),
										materialsProperty.getPropertySetByID(Properties::Normal),
										materialsProperty.getPropertySetByID(Properties::Emissive),
										materialsProperty.getPropertySetByID(Properties::RMHAO)
									};

									// Go over each material type
									for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
									{
										// Check if an entry for the current material type was present within the properties
										if(materialProperties[iMatType])
										{
											// Load the material data
											materials[iMatType] = loadMaterialData(materialProperties[iMatType], newModelData.m_model.getMaterialArrays(), static_cast<MaterialType>(iMatType), meshDataIndex);
										}
									}

									newModelData.m_meshes.push_back(MeshData(meshesInModelArray[iMesh], materials));

									ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ModelComponent, m_name + " - Model \'" + modelName + "\' imported");
								}
							}
						}*/
					}
					/*else
					{
						// Get the material arrays that were loaded from the model file
						auto &materialArrayFromModel = newModelData.m_model.getMaterialArrays();

						// Iterate over every mesh in the model
						for(decltype(meshesInModelArray.size()) iMesh = 0, numMeshes = meshesInModelArray.size(); iMesh < numMeshes; iMesh++)
						{
							// Define material data and material properties
							MaterialData materials[MaterialType::MaterialType_NumOfTypes];

							// Go over each mesh in the model
							//if(iMesh > materialArrayFromModel.m_numMaterials)
							{
								// Go over each material type
								for(unsigned int iMatType = 0; iMatType < MaterialType::MaterialType_NumOfTypes; iMatType++)
								{
									// Get the texture filename and load it to memory
									auto textureFromModel = Loaders::texture2D().load(materialArrayFromModel.m_materials[iMatType][iMesh].m_filename, static_cast<MaterialType>(iMatType), false);
									auto materialLoadError = textureFromModel.loadToMemory();

									// Check if the texture was loaded successfully
									if(materialLoadError == ErrorCode::Success)
									{
										materials[MaterialType::MaterialType_Diffuse].m_texture = textureFromModel;
									}
									else
									{
										ErrHandlerLoc::get().log(materialLoadError, ErrorSource::Source_Renderer);
									}
								}

								// Add the data for this mesh. Include materials loaded from the model itself, if they were present, otherwise, include default textures instead
								newModelData.m_meshes.push_back(MeshData(meshesInModelArray[iMesh], materials));

								ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ModelComponent, m_name + " - Model \'" + modelName + "\' imported");
							}
						}
					}*/
				}
			}

			if(p_properties.getNumPropertySets() == 0)
				ErrHandlerLoc().get().log(ErrorType::Info, ErrorSource::Source_ModelComponent, m_name + " - missing model data");
		}
		
		return importError;
	}

	PropertySet exportObject()
	{ 
		return PropertySet(); 
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

	std::vector<ModelData> m_modelData;
	MeshMaterialsProperties *m_materialsFromProperties;
	ModelsProperties *m_modelsProperties;
};