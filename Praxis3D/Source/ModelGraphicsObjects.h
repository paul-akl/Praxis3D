#pragma once

#include <functional>

#include "BaseGraphicsObjects.h"
#include "Loaders.h"
#include "Renderer.h"

class RendererScene;

class ModelObject : public BaseGraphicsObject
{
	//friend class RendererScene;
public:
	ModelObject(SystemScene *p_systemScene, const std::string &p_name, ModelLoader::ModelHandle &p_model, Properties::PropertyID p_objectType = Properties::ModelObject)
		: BaseGraphicsObject(p_systemScene, p_name, p_objectType), m_objectData(p_model, Loaders::shader().load(), m_baseObjectData)
	{
		m_customShader = false;
		m_baseObjectData.m_heightScale = Config::graphicsVar().height_scale;
		m_baseObjectData.m_alphaThreshold = Config::graphicsVar().alpha_threshold;
		m_baseObjectData.m_emissiveThreshold = Config::graphicsVar().emissive_threshold;
	}
	ModelObject(SystemScene *p_systemScene, const std::string &p_name, ModelLoader::ModelHandle &p_model, 
				ShaderLoader::ShaderProgram *p_shader, Properties::PropertyID p_objectType = Properties::ModelObject)
		: BaseGraphicsObject(p_systemScene, p_name, p_objectType), m_objectData(p_model, p_shader, m_baseObjectData)
	{
		m_customShader = true;
		m_baseObjectData.m_heightScale = Config::graphicsVar().height_scale;
		m_baseObjectData.m_alphaThreshold = Config::graphicsVar().alpha_threshold;
		m_baseObjectData.m_emissiveThreshold = Config::graphicsVar().emissive_threshold;
	}

	virtual ~ModelObject() { }

	virtual ErrorCode init()
	{
		return ErrorCode::Success;
	}

	// Updates model matrix if any spacial data has been changed
	void update(const float p_deltaTime)
	{
		if(m_needsUpdate)
		{
			// Update model matrix
			m_baseObjectData.m_modelMat.identity();
			m_baseObjectData.m_modelMat.translate(m_baseObjectData.m_position);
			m_baseObjectData.m_modelMat.rotate(m_baseObjectData.m_rotation);
			m_baseObjectData.m_modelMat.scale(m_baseObjectData.m_scale);

			// Mark as updated
			m_needsUpdate = false;
		}
	}

	// Loads model and materials from files to memory
	void loadToMemory()
	{
		// Load model to RAM
		m_objectData.m_model.loadToMemory();

		// Get material arrays from the model (texture filenames only)
		decltype(m_objectData.m_model.getMaterialArrays()) materials = m_objectData.m_model.getMaterialArrays();
		
		// Reserve space in the material arrays before populating them
		for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
		{
			m_objectData.m_materials[matType].reserve(materials.m_numMaterials);
			m_objectData.m_defaultMaterial[matType].reserve(materials.m_numMaterials);
		}
		
		// Save number of materials (from model file) for later
		m_objectData.m_numMaterials = materials.m_numMaterials;

		// Resize the material array to the number of materials loaded from model
		// Material count from model is the correct one (since there can't be more textures than meshes)
		m_materialNames.resize(materials.m_numMaterials);

		// Put material names loaded from model into internal material name array
		// Override from model only if the existing entry is empty. If neither texture's
		// filenames are present, use a default texture filename instead (as a fall-back mechanism)
		for(decltype(materials.m_numMaterials) i = 0, size = materials.m_numMaterials; i < size; i++)
			for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
				if(m_materialNames.m_materials[matType][i].isEmpty())
					if(!materials.m_materials[matType][i].isEmpty())
					{
						m_materialNames.m_materials[matType][i].m_filename = materials.m_materials[matType][i].m_filename;
						m_materialNames.m_materials[matType][i].m_defaultMaterial = false;
					}
					else
					{
						m_materialNames.m_materials[matType][i].m_filename = m_defaultMaterials[matType];
						m_materialNames.m_materials[matType][i].m_defaultMaterial = true;
					}
		
		// Iterate over each material, and try to load it (from filename). If the filename is empty,
		// texture loader is simply going to give out a default texture.
		for(decltype(m_materialNames.m_numMaterials) i = 0; i < m_materialNames.m_numMaterials; i++)
			for(unsigned int matType = 0; matType < MaterialType_NumOfTypes; matType++)
			{
				m_objectData.m_materials[matType].push_back(Loaders::texture2D().load(m_materialNames.m_materials[matType][i].m_filename, static_cast<MaterialType>(matType), false));
				m_objectData.m_defaultMaterial[matType].push_back(m_materialNames.m_materials[matType][i].m_defaultMaterial);
			}
		
		ErrorCode error = ErrorCode::Success;

		// Load each texture to memory (if it has been loaded before, it will simple continue without loading)
		for(decltype(m_materialNames.m_numMaterials) i = 0; i < m_materialNames.m_numMaterials; i++)
			// Load each material type to memory. If it returns an error, log it
			for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
				if(!ErrHandlerLoc::get().ifSuccessful(m_objectData.m_materials[matType][i].loadToMemory(), error))
					ErrHandlerLoc::get().log(error, ErrorSource::Source_GraphicsObject);
		
		// Put the height value into the alpha channel of the normal map
		/*for(decltype(m_materialNames.m_numMaterials) i = 0; i < m_materialNames.m_numMaterials; i++)
			m_objectData.m_materials[Config::rendererVar().heightmap_combine_texture][i].setColorChannel(
				m_objectData.m_materials[Model::ModelMat_height][i],
				static_cast<TextureColorChannelOffset>(Config::rendererVar().heightmap_combine_channel));*/


		// Clear the material names, since after loading we don't need them anymore
		m_materialNames.clear();

		// Set an atomic flag, to indicate that the loaded has been completed
		setLoadedToMemory(true);
	}

	// Loads model and materials from memory to video memory; should only be called by renderer thread
	ErrorCode loadToVideoMemory()
	{
		// Error code used for checking the success of loading; it is not returned
		ErrorCode error = ErrorCode::Success;

		// Load the model to video memory; log an error if it occurs
		//if(!ErrHandlerLoc::get().ifSuccessful(m_objectData.m_model.loadToVideoMemory(), error))
		//	ErrHandlerLoc::get().log(error);

		// Iterate over all materials and load them to video memory; log an error if loading failed
		for(decltype(m_objectData.m_numMaterials) i = 0; i < m_objectData.m_numMaterials; i++)
			for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
				if(!ErrHandlerLoc::get().ifSuccessful(m_objectData.m_materials[matType][i].loadToVideoMemory(), error))
					ErrHandlerLoc::get().log(error);

		return ErrorCode::Success;
	}

	// Exports all the data of the object as a PropertySet
	PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::ModelObject);
		propertySet.addProperty(Properties::Name, m_name);
		propertySet.addProperty(Properties::Position, m_baseObjectData.m_position);
		propertySet.addProperty(Properties::Rotation, m_baseObjectData.m_rotation);
		propertySet.addProperty(Properties::OffsetPosition, m_baseObjectData.m_offsetPosition);
		propertySet.addProperty(Properties::OffsetRotation, m_baseObjectData.m_offsetRotation);
		propertySet.addProperty(Properties::Scale, m_baseObjectData.m_scale);
		propertySet.addProperty(Properties::AlphaThreshold, m_baseObjectData.m_alphaThreshold);
		propertySet.addProperty(Properties::HeightScale, m_baseObjectData.m_heightScale);
		propertySet.addProperty(Properties::Lighting, m_affectedByLighting);
		propertySet.addProperty(Properties::TextureTilingFactor, m_baseObjectData.m_textureTilingFactor);

		// Add model
		auto &models = propertySet.addPropertySet(Properties::Models);
		models.addProperty(Properties::Filename, m_objectData.m_model.getFilename());

		// Add material root property set
		auto &materials = propertySet.addPropertySet(Properties::Materials);
		PropertySet *materialTypes[MaterialType_NumOfTypes];
		
		// Declare individual material property sets
		materialTypes[MaterialType_Diffuse] = new PropertySet(Properties::Diffuse);
		materialTypes[MaterialType_Normal] = new PropertySet(Properties::Normal);
		materialTypes[MaterialType_Emissive] = new PropertySet(Properties::Emissive);
		materialTypes[MaterialType_Combined] = new PropertySet(Properties::RMHAO);

		// Iterate over each material and add it only if it's not using the default texture
		for(decltype(m_objectData.m_numMaterials) i = 0; i < m_objectData.m_numMaterials; i++)
			for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
				if(!m_objectData.m_defaultMaterial[matType][i])
				{
					auto &materialEntry = materialTypes[matType]->addPropertySet(Properties::ArrayEntry);
					materialEntry.addProperty(Properties::Filename, m_objectData.m_materials[matType][i].getFilename());
					materialEntry.addProperty(Properties::Index, (int)i);
				}

		// Add individual material property sets and delete temporary values
		for(int matType = 0; matType < MaterialType_NumOfTypes; matType++)
		{
			materials.addPropertySet(*materialTypes[matType]);
			delete materialTypes[matType];
		}

		// If the object contains custom shaders, add them to the property set
		if(m_customShader)
		{
			// Add shader root property set
			auto &shaders = propertySet.addPropertySet(Properties::Shaders);

			// Iterate over all shader types and add the ones that are present as properties
			for(unsigned int i = 0; i < ShaderType_NumOfTypes; i++)
				if(m_objectData.m_shader->shaderPresent(i))
				{
					switch(i)
					{
					case ShaderType_Fragment:
						shaders.addProperty(Properties::FragmentShader, m_objectData.m_shader->getShaderFilename(i));
						break;
					case ShaderType_Geometry:
						shaders.addProperty(Properties::GeometryShader, m_objectData.m_shader->getShaderFilename(i));
						break;
					case ShaderType_Vertex:
						shaders.addProperty(Properties::VertexShader, m_objectData.m_shader->getShaderFilename(i));
						break;
					case ShaderType_TessControl:
						shaders.addProperty(Properties::TessControlShader, m_objectData.m_shader->getShaderFilename(i));
						break;
					case ShaderType_TessEvaluation:
						shaders.addProperty(Properties::TessEvaluationShader, m_objectData.m_shader->getShaderFilename(i));
						break;
					}
				}
		}

		return propertySet;
	}

	// Adds a material filename to a specific material type and index
	inline void addMaterial(MaterialType p_matType, const std::string &p_fileName, const unsigned int p_materialIndex)
	{
		if(!p_fileName.empty())
		{
			if(p_materialIndex + 1 > m_materialNames.m_numMaterials)
				m_materialNames.resize(p_materialIndex + 1);

			m_materialNames.m_materials[p_matType][p_materialIndex].m_filename = p_fileName;
			m_materialNames.m_materials[p_matType][p_materialIndex].m_defaultMaterial = false;
		}
	}
	
	// Getters
	const inline bool hasCustomShader() const { return m_customShader; }
	const inline bool isAffectedByLighting() const { return m_affectedByLighting; }
	inline RenderableObjectData &getRenderableObjectData() { return m_objectData; }

	// Setters
	inline void setLighting(const bool p_flag)					{ m_affectedByLighting = p_flag;						}
	inline void setAlphaThreshold(const float p_threshold)		{ m_baseObjectData.m_alphaThreshold = p_threshold;		}
	inline void setEmissiveThreshold(const float p_threshold)	{ m_baseObjectData.m_emissiveThreshold = p_threshold;	}
	inline void setHeightScale(const float p_scale)				{ m_baseObjectData.m_heightScale = p_scale;				}
	inline void setTextureTilingFactor(const float p_factor)	{ m_baseObjectData.m_textureTilingFactor = p_factor;	}
	inline void setDefaultMaterial(const unsigned int p_materialType, const std::string &p_filename)
	{
		// Check if material type index is within bounds
		if(p_materialType < MaterialType_NumOfTypes)
			m_defaultMaterials[p_materialType] = p_filename;
	}

protected:
	// Clears data that's not useful after loading (like material names, etc)
	inline void clear()
	{
		m_materialNames.clear();
	}

	Model::MaterialArrays m_materialNames;

	std::string m_defaultMaterials[MaterialType_NumOfTypes];
	
	// If the object hasn't got a custom shader, the regular geometry shader in the renderer is used
	bool m_customShader;

	RenderableObjectData m_objectData;
};

