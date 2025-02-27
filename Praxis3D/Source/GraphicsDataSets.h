#pragma once

#include <algorithm>
#include <iterator>
#include <cmath>
#include <variant>

#include "Math.h"
#include "Loaders.h"

struct MaterialParameters
{
	MaterialParameters() : m_color(1.0f), m_scale(1.0f), m_framing(0.0f) { }

	inline bool operator==(const MaterialParameters &p_matOaram)
	{
		return	m_color == p_matOaram.m_color &&
				m_scale == p_matOaram.m_scale &&
				m_framing == p_matOaram.m_framing;
	}

	glm::vec4 m_color;
	glm::vec2 m_scale;
	glm::vec2 m_framing;
};

struct MaterialData
{
	MaterialData() { }

	inline bool operator==(const MaterialData &p_matData)
	{
		for(unsigned int i = 0; i < MaterialType::MaterialType_NumOfTypes; i++)
			if(m_parameters[i] != p_matData.m_parameters[i])
				return false;

		return true;
	}

	MaterialParameters m_parameters[MaterialType::MaterialType_NumOfTypes];
};

// Contains data of a single mesh and its materials
struct MeshData
{
	MeshData(
		const Model::Mesh &p_mesh,
		const float p_heightScale,
		const float p_alphaThreshold,
		const float p_emissiveIntensity,
		const bool p_textureRepetition,
		const float p_rextureRepetitionScale,
		const TextureWrapType p_textureWrapMode,
		const bool p_active = true) :
		m_mesh(&p_mesh),
		m_materials(MaterialType::MaterialType_NumOfTypes, Loaders::texture2D().getDefaultTexture()),
		m_heightScale(p_heightScale),
		m_alphaThreshold(p_alphaThreshold),
		m_emissiveIntensity(p_emissiveIntensity),
		m_textureWrapMode(p_textureWrapMode),
		m_stochasticSampling(p_textureRepetition),
		m_textureRepetitionScale(p_rextureRepetitionScale),
		m_active(p_active)
	{
	}

	MeshData(
		const Model::Mesh &p_mesh,
		const std::vector<TextureLoader2D::Texture2DHandle> &p_materials,
		const MaterialData &p_materialData,
		const float p_heightScale, 
		const float p_alphaThreshold, 
		const float p_emissiveIntensity, 
		const bool p_textureRepetition, 
		const float p_textureRepetitionScale, 
		const TextureWrapType p_textureWrapMode, 
		const bool p_active = true) :
		m_mesh(&p_mesh),
		m_materials(p_materials),
		m_materialData(p_materialData),
		m_heightScale(p_heightScale),
		m_alphaThreshold(p_alphaThreshold),
		m_emissiveIntensity(p_emissiveIntensity),
		m_textureWrapMode(p_textureWrapMode),
		m_stochasticSampling(p_textureRepetition),
		m_textureRepetitionScale(p_textureRepetitionScale),
		m_active(p_active)
	{
		// Make sure the material arrays are of the right size
		if(m_materials.size() != MaterialType::MaterialType_NumOfTypes)
			m_materials.resize(MaterialType::MaterialType_NumOfTypes, Loaders::texture2D().getDefaultTexture());
	}

	// Texture parallax effect scale (height multiplier)
	float m_heightScale;
	// Transparency threshold after which the fragment is discarded
	float m_alphaThreshold;
	// Multiplier for emissive texture color
	float m_emissiveIntensity;
	// Scale for texture repetition algorithm
	float m_textureRepetitionScale;
	// Flag specifying whether the texture repetition is turned on
	bool m_stochasticSampling;
	// Flag denoting whether to draw the mesh
	bool m_active;
	// Texture wrap mode
	TextureWrapType m_textureWrapMode;

	// Handle to a mesh
	const Model::Mesh *m_mesh;

	// An array of materials of each type
	std::vector<TextureLoader2D::Texture2DHandle> m_materials;
	MaterialData m_materialData;
};

// Contains data of a single model and its meshes
struct ModelData
{
	ModelData(ModelLoader::ModelHandle p_model) : m_model(p_model) { }
	ModelData(
		ModelLoader::ModelHandle p_model,
		const FaceCullingSettings p_drawFaceCulling,
		const FaceCullingSettings p_shadowFaceCulling) :
		m_drawFaceCulling(p_drawFaceCulling),
		m_shadowFaceCulling(p_shadowFaceCulling),
		m_model(p_model) { }

	// Handle to a model
	ModelLoader::ModelHandle m_model;

	// An array of meshes
	std::vector<MeshData> m_meshes;

	// Face culling settings
	FaceCullingSettings m_drawFaceCulling;
	FaceCullingSettings m_shadowFaceCulling;
};

// Contains model data and spatial data; needed by the renderer to draw a model using default shaders
struct ModelAndSpatialData
{
	ModelAndSpatialData(ModelData &p_modelData, const glm::mat4 &p_modelMatrix) : m_modelData(p_modelData), m_modelMatrix(p_modelMatrix) { }

	ModelData &m_modelData;
	const glm::mat4 &m_modelMatrix;
};

// Contains model data, shader data and spatial data; needed by the renderer to draw a model using custom shaders
struct ModelShaderSpatialData
{
	ModelShaderSpatialData(ModelData &p_modelData, ShaderLoader::ShaderProgram &p_shader, const glm::mat4 &p_modelMatrix) : m_modelData(p_modelData), m_shader(&p_shader), m_modelMatrix(p_modelMatrix) { }

	ModelData &m_modelData;
	ShaderLoader::ShaderProgram *m_shader;
	const glm::mat4 &m_modelMatrix;
};

// Contains data of multiple models, comprising a model component
struct ModelComponentData
{
	std::vector<ModelData> m_modelData;
};

// Contains data of a single shader program
struct ShaderData
{
	ShaderData(ShaderLoader::ShaderProgram &p_shader) : m_shader(p_shader) { }

	// Handle to a shader program
	ShaderLoader::ShaderProgram &m_shader;
};

// Contains data of a camera, needed for view transformation
struct CameraData
{
	SpatialTransformData m_spatialData;
};

// All graphics objects contain an instance of this struct, which holds the necessary spacial and other data
struct GraphicsData
{
	GraphicsData() : m_scale(1.0f, 1.0f, 1.0f), m_textureWrapMode(GL_REPEAT), m_alphaThreshold(0.0f), m_emissiveThreshold(0.0f), m_heightScale(0.0f), m_textureTilingFactor(1.0f) { }

	glm::vec3 m_position,
				m_rotation,
				m_scale,
				m_offsetPosition,
				m_offsetRotation;

	glm::mat4 m_modelMat;

	int m_textureWrapMode;

	float	m_alphaThreshold,
			m_emissiveThreshold,
			m_heightScale,
			m_textureTilingFactor;
};

// Contains all required data to render an object (ModelObject)
struct RenderableObjectData
{
	RenderableObjectData(ModelLoader::ModelHandle p_model, ShaderLoader::ShaderProgram *p_shader, GraphicsData &p_baseObjectData) :
		m_loadedToMemory(false),
		m_renderable(false),
		m_baseObjectData(p_baseObjectData),
		m_model(p_model),
		m_shader(p_shader),
		m_numMaterials(0) { }
	
	// Is the object loaded to RAM
	bool m_loadedToMemory;

	// Should the object be rendered
	bool m_renderable;
	
	// Additional data
	GraphicsData &m_baseObjectData;

	// Model handle
	ModelLoader::ModelHandle m_model;

	// Shader handle
	ShaderLoader::ShaderProgram *m_shader;

	// Is the material the default one (for example, if no materials were provided)
	std::vector<bool> m_defaultMaterial[MaterialType_NumOfTypes];

	// Each material type has a vector of materials
	// (An array of material types of vectors of materials)
	std::vector<TextureLoader2D::Texture2DHandle> m_materials[MaterialType_NumOfTypes];

	// Number of materials in each vector of the material type array (m_materials)
	std::vector<TextureLoader2D::Texture2DHandle>::size_type m_numMaterials;
};

// A simple container storing a union which contains one of the loadable objects; uses an enum to distinguish which object is currently being stored
struct LoadableObjectsContainer
{
	enum LoadableObjectType : unsigned int
	{
		LoadableObjectType_Shader = 0,
		LoadableObjectType_Model,
		LoadableObjectType_Texture
	};

	LoadableObjectsContainer(const LoadableObjectsContainer &p_arg) noexcept
	{
		switch(p_arg.m_loadableObject.index())
		{
			case LoadableObjectsContainer::LoadableObjectType_Shader:
				m_loadableObject = std::get<ShaderLoader::ShaderProgram *>(p_arg.m_loadableObject);
				break;
			case LoadableObjectsContainer::LoadableObjectType_Model:
				m_loadableObject = std::get<ModelLoader::ModelHandle>(p_arg.m_loadableObject);
				break;
			case LoadableObjectsContainer::LoadableObjectType_Texture:
				m_loadableObject = std::get<TextureLoader2D::Texture2DHandle>(p_arg.m_loadableObject);
				break;
		}
	}
	LoadableObjectsContainer(LoadableObjectsContainer &&p_arg) noexcept
	{
		m_loadableObject = std::move(p_arg.m_loadableObject);
	}
	LoadableObjectsContainer(ModelLoader::ModelHandle p_model)				: m_loadableObject(p_model) {}//, m_objectType(LoadableObjectType::LoadableObjectType_Model) { }
	LoadableObjectsContainer(ShaderLoader::ShaderProgram *p_shader)			: m_loadableObject(p_shader) {}//, m_objectType(LoadableObjectType::LoadableObjectType_Shader) { }
	LoadableObjectsContainer(TextureLoader2D::Texture2DHandle p_texture)	: m_loadableObject(p_texture) {}//, m_objectType(LoadableObjectType::LoadableObjectType_Texture) { }
	~LoadableObjectsContainer() { }

	inline const bool isLoadedToVideoMemory() const
	{
		return std::visit([](auto &&p_arg) { return isLoadedToVideoMemory(p_arg); }, m_loadableObject);
	}

	inline const void setLoadedToVideoMemory(const bool p_loaded)
	{
		std::visit([=](auto &&p_arg) { return setLoadedToVideoMemory(p_arg, p_loaded); }, m_loadableObject);
	}

	static const bool isLoadedToVideoMemory(const ShaderLoader::ShaderProgram *p_shader) { return p_shader->isLoadedToVideoMemory(); }
	static const bool isLoadedToVideoMemory(const ModelLoader::ModelHandle &p_model) { return p_model.isLoadedToVideoMemory(); }
	static const bool isLoadedToVideoMemory(const TextureLoader2D::Texture2DHandle &p_texture) { return p_texture.isLoadedToVideoMemory(); }

	static const void setLoadedToVideoMemory(ShaderLoader::ShaderProgram *p_shader, const bool p_loaded) { p_shader->setLoadedToVideoMemory(p_loaded); }
	static const void setLoadedToVideoMemory(ModelLoader::ModelHandle &p_model, const bool p_loaded) { p_model.setLoadedToVideoMemory(p_loaded); }
	static const void setLoadedToVideoMemory(TextureLoader2D::Texture2DHandle &p_texture, const bool p_loaded) { p_texture.setLoadedToVideoMemory(p_loaded); }

	inline const LoadableObjectType getType() const { return static_cast<LoadableObjectType>(m_loadableObject.index()); }

	// Get shader program; UNSAFE: MUST make sure the underlying variant is of ShaderProgram* type
	inline ShaderLoader::ShaderProgram *getShaderProgram() { return std::get<ShaderLoader::ShaderProgram *>(m_loadableObject); }

	// Get model handle; UNSAFE: MUST make sure the underlying variant is of ModelHandle type
	inline ModelLoader::ModelHandle &getModelHandle() { return std::get<ModelLoader::ModelHandle>(m_loadableObject); }

	// Get texture handle; UNSAFE: MUST make sure the underlying variant is of Texture2DHandle type
	inline TextureLoader2D::Texture2DHandle &getTextureHandle() { return std::get<TextureLoader2D::Texture2DHandle>(m_loadableObject); }

	// Copy assignment
	LoadableObjectsContainer &operator=(const LoadableObjectsContainer &p_arg) noexcept
	{
		// Guard for self assignment
		if(this == &p_arg)
			return *this;

		//m_objectType = p_arg.m_objectType;
		m_loadableObject = p_arg.m_loadableObject;
		switch(p_arg.m_loadableObject.index())
		{
			case LoadableObjectsContainer::LoadableObjectType_Shader:
				m_loadableObject = std::get<ShaderLoader::ShaderProgram *>(p_arg.m_loadableObject);
				break;
			case LoadableObjectsContainer::LoadableObjectType_Model:
				m_loadableObject = std::get<ModelLoader::ModelHandle>(p_arg.m_loadableObject);
				break;
			case LoadableObjectsContainer::LoadableObjectType_Texture:
				m_loadableObject = std::get<TextureLoader2D::Texture2DHandle>(p_arg.m_loadableObject);
				break;
		}

		return *this;
	}

	LoadableObjectsContainer &operator=(LoadableObjectsContainer &&p_arg) noexcept
	{
		// Guard for self assignment
		if(this == &p_arg)
			return *this;

		m_loadableObject = std::move(p_arg.m_loadableObject);
		return *this;
	}

	std::variant<ShaderLoader::ShaderProgram *, ModelLoader::ModelHandle, TextureLoader2D::Texture2DHandle> m_loadableObject;
};

struct UnloadableObjectsContainer
{
	UnloadableObjectsContainer() : m_objectType(UnloadObjectType::UnloadObjectType_VAO), m_handle(0) { }
	UnloadableObjectsContainer(const UnloadObjectType p_objectType, const unsigned int p_handle) : m_objectType(p_objectType), m_handle(p_handle) { }

	UnloadObjectType m_objectType;
	unsigned int m_handle;
};

struct RenderableMeshData
{
	RenderableMeshData(glm::mat4 &p_modelMatrix, MeshData &p_meshData)
		: m_modelMatrix(p_modelMatrix), m_meshData(p_meshData), m_shader(*Loaders::shader().load()) { }

	RenderableMeshData(glm::mat4 &p_modelMatrix, MeshData &p_meshData, ShaderLoader::ShaderProgram &p_shader)// = *Loaders::shader().load())
		: m_modelMatrix(p_modelMatrix), m_meshData(p_meshData), m_shader(p_shader) { }

	glm::mat4 &m_modelMatrix;
	MeshData &m_meshData;
	ShaderLoader::ShaderProgram &m_shader;
};

//	 ===========================================================================================================
//	|												WARNING:													|
//	|					The layout of data sets below are order-sensitive. Do not edit.							|
//	|																											|
//	|		Their member variables are arranged by the order of buffer offsets used in shaders.					|
//	| Editing their location would break the synchronization, since these structs are put into buffers as is.	|
//	 ===========================================================================================================

struct DirectionalLightDataSet
{
	DirectionalLightDataSet(glm::vec3 p_color = glm::vec3(0.0f), 
		glm::vec3 p_direction = glm::vec3(0.0f, 1.0f, 0.0f),
		float p_intensity = 0.0f) :
		m_color(p_color), 
		m_direction(p_direction), 
		m_intensity(p_intensity) { }

	DirectionalLightDataSet &operator=(const DirectionalLightDataSet &p_other)
	{
		m_color = p_other.m_color;
		m_direction = p_other.m_direction;
		m_intensity = p_other.m_intensity;

		return *this;
	}

	void clear()
	{
		m_color = glm::vec3(0.0f);
		m_intensity = 0.0f;
	}

	glm::vec3 m_color;
	glm::vec3 m_direction;
	float m_intensity;
};

struct PointLightDataSet
{
	PointLightDataSet(glm::vec3 p_color = glm::vec3(1.0f), 
		glm::vec3 p_position = glm::vec3(0.0f), 
		glm::vec3 p_attenuation = glm::vec3(0.0f, 0.0f, 1.0f), 
		float p_intensity = 1.0f) : 
		m_color(p_color), 
		m_position(p_position), 
		m_attenuation(p_attenuation), 
		m_intensity(p_intensity),
		m_padding1(0.0f),
		m_padding2(0.0f) { }

	glm::vec3 m_color;
	float m_padding1;	// Unused variable, declared for padding
	glm::vec3 m_position;
	glm::vec3 m_attenuation;
	float m_intensity;
	float m_padding2;	// Unused variable, declared for padding
};

struct SpotLightDataSet
{
	SpotLightDataSet(glm::vec3 p_color = glm::vec3(1.0f), 
		glm::vec3 p_position = glm::vec3(0.0f),
		glm::vec3 p_direction = glm::vec3(0.0f, 1.0f, 0.0f), 
		glm::vec3 p_attenuation = glm::vec3(0.0f, 0.0f, 1.0f),
		float p_intensity = 1.0f,
		float p_cutoffAngle = 1.0f) : 
		m_color(p_color), 
		m_position(p_position), 
		m_direction(p_direction),
		m_attenuation(p_attenuation), 
		m_intensity(p_intensity),
		m_cutoffAngle(p_cutoffAngle),
		m_padding1(0.0f),
		m_padding2(0.0f) { }
	
	glm::vec3 m_color;
	float m_padding1;	// Unused variable, declared for padding
	glm::vec3 m_position;
	float m_padding2;	// Unused variable, declared for padding
	glm::vec3 m_direction;
	glm::vec3 m_attenuation;
	float m_intensity;
	float m_cutoffAngle;
};

struct AODataSet
{
	AODataSet()
	{
		m_RadiusToScreen = 0.0;
		m_radius = 0.0;
		m_NegInvR2 = 0.0f;
		m_NDotVBias = 0.0f;

		m_InvFullResolution = glm::vec2(0.0f, 0.0f);
		m_AOMultiplier = 0.0f;
		m_PowExponent = 0.0f;

		m_bias = 0.0f;
		m_numOfDirections = 0;
		m_numOfSamples = 0;
		m_numOfSteps = 0;
	}

	float m_RadiusToScreen;	// radius
	float m_radius;
	float m_NegInvR2;		// radius * radius
	float m_NDotVBias;

	glm::vec2 m_InvFullResolution;
	float m_AOMultiplier;
	float m_PowExponent;

	float m_bias;
	int m_numOfDirections;
	int m_numOfSamples;
	int m_numOfSteps;
};

struct CascadedShadowMapDataSet
{
	CascadedShadowMapDataSet() : m_cascadePlaneDistance(0.0f), m_maxBias(0.0f), m_poissonSampleScale(0.0f), m_penumbraScale(1.0f) { }
	CascadedShadowMapDataSet(const glm::mat4 &p_lightSpaceMatrix, const float p_cascadePlaneDistance, const float p_maxBias, const float p_poissonSampleScale, const float p_penumbraScale) :
		m_lightSpaceMatrix(p_lightSpaceMatrix), m_cascadePlaneDistance(p_cascadePlaneDistance), m_maxBias(p_maxBias), m_poissonSampleScale(p_poissonSampleScale), m_penumbraScale(p_penumbraScale) { }

	glm::mat4 m_lightSpaceMatrix;
	float m_cascadePlaneDistance;
	float m_maxBias;
	float m_poissonSampleScale;
	float m_penumbraScale;
};

struct HDRDataSet
{
	HDRDataSet() : m_screenBrightness(0.5f) { }

	float m_screenBrightness;
};

// An atmosphere density layer
struct DensityProfLayer
{
	DensityProfLayer() : DensityProfLayer(0.0f, 0.0f, 0.0f, 0.0f, 0.0f) { }
	DensityProfLayer(float p_width, float p_expTerm, float p_expScale, float p_linearTerm, float p_constantTerm) :
		m_width(p_width), m_expTerm(p_expTerm), m_expScale(p_expScale), m_linearTerm(p_linearTerm), m_constantTerm(p_constantTerm) { }

	float m_width;
	float m_expTerm;
	float m_expScale;
	float m_linearTerm;
	float m_constantTerm;

	glm::vec3 m_padding1;
};

// An atmosphere density profile made of several layers on top of each other
struct DensityProfile
{
	DensityProfile() { }

	DensityProfile(DensityProfLayer p_layers[2])
	{
		m_layers[0] = p_layers[0];
		m_layers[1] = p_layers[1];
	}
	DensityProfile(DensityProfLayer p_layer1, DensityProfLayer p_layer2)
	{
		m_layers[0] = p_layer1;
		m_layers[1] = p_layer2;
	}

	DensityProfLayer m_layers[2];
};

// A planet's atmosphere parameters
struct AtmosphereParameters
{
	AtmosphereParameters() :
		m_solarIrradiance(0.0f),
		m_sunAngularRadius(0.0f),
		m_bottomRadius(0.0f),
		m_topRadius(0.0f),
		m_rayleighScattering(0.0f),
		m_mieScattering(0.0f),
		m_mieExtinction(0.0f),
		m_miePhaseFunctionG(0.0f),
		m_absorptionExtinction(0.0f),
		m_groundAlbedo(0.0f),
		m_muSMin(0.0f)
	{

	}

	AtmosphereParameters(
		glm::vec3 p_solarIrradiance, 
		float p_sunAngularRadius, 
		DensityProfile p_rayleighDensity, 
		DensityProfile p_mieDensity, 
		DensityProfile p_absorptionDensity, 
		glm::vec3 p_rayleighScattering, 
		float p_bottomRadius, 
		glm::vec3 p_mieScattering, 
		float p_topRadius, 
		glm::vec3 p_mieExtinction, 
		float p_miePhaseFunctionG, 
		glm::vec3 p_absorptionExtinction, 
		float p_muSMin, 
		glm::vec3 p_groundAlbedo) :
		m_solarIrradiance(p_solarIrradiance),
		m_sunAngularRadius(p_sunAngularRadius),
		m_rayleighDensity(p_rayleighDensity),
		m_mieDensity(p_mieDensity),
		m_absorptionDensity(p_absorptionDensity),
		m_rayleighScattering(p_rayleighScattering),
		m_bottomRadius(p_bottomRadius),
		m_mieScattering(p_mieScattering),
		m_topRadius(p_topRadius),
		m_mieExtinction(p_mieExtinction),
		m_miePhaseFunctionG(p_miePhaseFunctionG),
		m_absorptionExtinction(p_absorptionExtinction),
		m_muSMin(p_muSMin),
		m_groundAlbedo(p_groundAlbedo)
	{

	}
	
	// The solar irradiance at the top of the atmosphere.
	glm::vec3 m_solarIrradiance;
	
	// The sun's angular radius
	float m_sunAngularRadius;

	// The density profile of air molecules
	DensityProfile m_rayleighDensity;

	// The density profile of aerosols
	DensityProfile m_mieDensity;

	// The density profile of air molecules that absorb light (e.g. ozone)
	DensityProfile m_absorptionDensity;

	// The scattering coefficient of air molecules at the altitude where their density is maximum
	glm::vec3 m_rayleighScattering;
	
	// The distance between the planet center and the bottom of the atmosphere.
	float m_bottomRadius;

	// The scattering coefficient of aerosols at the altitude where their density is maximum
	glm::vec3 m_mieScattering;

	// The distance between the planet center and the top of the atmosphere.
	float m_topRadius;

	// The extinction coefficient of aerosols at the altitude where their density is maximum
	glm::vec3 m_mieExtinction;
	
	// The asymetry parameter for the Cornette-Shanks phase function for the aerosols.
	float m_miePhaseFunctionG;

	// The extinction coefficient of molecules that absorb light (e.g. ozone)
	glm::vec3 m_absorptionExtinction;
	
	// The cosine of the maximum Sun zenith angle for which atmospheric scattering must be precomputed
	float m_muSMin;

	// The average albedo of the ground.
	glm::vec3 m_groundAlbedo;
};

// Parameters for atmospheric scattering shader
struct AtmScatteringParameters
{
	AtmScatteringParameters() :
		m_whitePoint(1.0f, 1.0f, 1.0f),
		m_earthCenter(0.0f, -6360000.0f / 1000.0f, 0.0f),
		m_sunSize(tan(0.01935f / 2.0f), cos(0.01935f / 2.0f)),
		m_padding1(0.0f),
		m_padding2(0.0f),
		m_padding3(0.0f),
		m_padding4(0.0f) { }

	AtmScatteringParameters(AtmosphereParameters p_atmosphereParam) : 
		m_atmosphereParam(p_atmosphereParam),
		m_whitePoint(1.0f, 1.0f, 1.0f),
		m_earthCenter(0.0f, -6360000.0f / 1000.0f, 0.0f),
		m_sunSize(tan(0.01935f / 2.0f), cos(0.01935f / 2.0f)),
		m_padding1(0.0f),
		m_padding2(0.0f),
		m_padding3(0.0f),
		m_padding4(0.0f) { }
	
	AtmScatteringParameters(
		AtmosphereParameters p_atmosphereParam,
		glm::vec3 p_whitePoint,
		glm::vec3 p_earthCenter,
		glm::vec2 p_sunSize) : 
		m_atmosphereParam(p_atmosphereParam),
		m_whitePoint(p_whitePoint),
		m_earthCenter(p_earthCenter),
		m_sunSize(p_sunSize),
		m_padding1(0.0f),
		m_padding2(0.0f),
		m_padding3(0.0f),
		m_padding4(0.0f) { }

	glm::vec3 m_whitePoint;
	float m_padding1;
	glm::vec3 m_earthCenter;
	float m_padding2;
	glm::vec2 m_sunSize;
	float m_padding3;
	float m_padding4;

	AtmosphereParameters m_atmosphereParam;
};

// Parameters for lens flare effect
struct LensFlareParameters
{
	LensFlareParameters()
	{
		m_ghostCount = 0;
		m_ghostSpacing = 0.0f;
		m_ghostThreshold = 0.0f;
		m_haloAspectRatio = 0.0f;
		m_haloRadius = 0.0f;
		m_haloThickness = 0.0f;
		m_haloThreshold = 0.0f;
		m_chromaticAberration = 0.0f;
		m_lensFlaireDownsample = 0.0f;
	}

	LensFlareParameters(
		int p_ghostCount,
		float p_ghostSpacing,
		float p_ghostThreshold,
		float p_haloAspectRatio,
		float p_haloRadius,
		float p_haloThickness,
		float p_haloThreshold,
		float p_chromaticAberration,
		float p_lensFlaireDownsample,
		float p_starburstOffset): 
		m_ghostCount(p_ghostCount),
		m_ghostSpacing(p_ghostSpacing),
		m_ghostThreshold(p_ghostThreshold),
		m_haloAspectRatio(p_haloAspectRatio),
		m_haloRadius(p_haloRadius),
		m_haloThickness(p_haloThickness),
		m_haloThreshold(p_haloThreshold),
		m_chromaticAberration(p_chromaticAberration),
		m_lensFlaireDownsample(p_lensFlaireDownsample){ }

	int m_ghostCount;
	float m_ghostSpacing;
	float m_ghostThreshold;

	float m_haloAspectRatio;
	float m_haloRadius;
	float m_haloThickness;
	float m_haloThreshold;
	
	float m_chromaticAberration;
	float m_lensFlaireDownsample;
};