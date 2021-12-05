#pragma once

#include "Math.h"
#include "Loaders.h"

// Contains data of a single material
struct MaterialData
{

	MaterialData(TextureLoader2D::Texture2DHandle &m_texture = Loaders::texture2D().getDefaultTexture()) : 
		m_texture(m_texture), 
		m_textureScale(1.0f, 1.0f), 
		m_parallaxScale(1.0f), 
		m_alphaCutoff(1.0f) { }

	// Handle to a texture
	TextureLoader2D::Texture2DHandle m_texture;
	// Texture coordinates scale (for example, used for tilling)
	Math::Vec2f m_textureScale;
	// Texture parallax effect scale (height multiplier)
	float m_parallaxScale;
	// Transparency threshold after which the fragment is discarded
	float m_alphaCutoff;
};

// Contains data of a single mesh and its materials
struct MeshData
{
	MeshData(Model::Mesh &p_mesh, MaterialData p_materials[MaterialType_NumOfTypes]) : m_mesh(p_mesh), m_materials{ *p_materials } { }

	// Handle to a mesh
	Model::Mesh &m_mesh;

	// An array of materials of each type
	MaterialData m_materials[MaterialType_NumOfTypes];
};

// Contains data of a single model and its meshes
struct ModelData
{
	ModelData(ModelLoader::ModelHandle &p_model) : m_model(p_model) { }

	// Handle to a model
	ModelLoader::ModelHandle m_model;
	// An array of meshes
	std::vector<MeshData>  m_meshes;
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

// All graphics objects contain an instance of this struct, which holds the necessary spacial and other data
struct GraphicsData
{
	GraphicsData() : m_scale(1.0f, 1.0f, 1.0f), m_alphaThreshold(0.0f), m_emissiveThreshold(0.0f), m_heightScale(0.0f), m_textureTilingFactor(1.0f) { }

	Math::Vec3f m_position,
				m_rotation,
				m_scale,
				m_offsetPosition,
				m_offsetRotation;

	Math::Mat4f m_modelMat;

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

//	 ===========================================================================================================
//	|												WARNING:													|
//	|					The layout of data sets below are order-sensitive. Do not edit.							|
//	|																											|
//	|		Their member variables are arranged by the order of buffer offsets used in shaders.					|
//	| Editing their location would break the synchronization, since these structs are put into buffers as is.	|
//	 ===========================================================================================================

struct DirectionalLightDataSet
{
	DirectionalLightDataSet(Math::Vec3f p_color = Math::Vec3f(1.0f), Math::Vec3f p_direction = Math::Vec3f(0.0f, 1.0f, 0.0f),
							float p_intensity = 1.0f) : m_color(p_color), m_direction(p_direction), m_intensity(p_intensity) { }

	void clear()
	{
		m_color = Math::Vec3f(0.0f);
		m_intensity = 0.0f;
	}

	Math::Vec3f m_color;
	Math::Vec3f m_direction;
	float m_intensity;
};

struct PointLightDataSet
{
	PointLightDataSet(Math::Vec3f p_color = Math::Vec3f(1.0f), Math::Vec3f p_position = Math::Vec3f(0.0f), 
					  Math::Vec3f p_attenuation = Math::Vec3f(0.0f, 0.0f, 1.0f), float p_intensity = 1.0f)
					: m_color(p_color), m_position(p_position), m_attenuation(p_attenuation), m_intensity(p_intensity) { }

	Math::Vec3f m_color;
	float m_padding1;	// Unused variable, declared for padding
	Math::Vec3f m_position;
	Math::Vec3f m_attenuation;
	float m_intensity;
	float m_padding2;	// Unused variable, declared for padding
};

struct SpotLightDataSet
{
	SpotLightDataSet(Math::Vec3f p_color = Math::Vec3f(1.0f), Math::Vec3f p_position = Math::Vec3f(0.0f),
					 Math::Vec3f p_direction = Math::Vec3f(0.0f, 1.0f, 0.0f), Math::Vec3f p_attenuation = Math::Vec3f(0.0f, 0.0f, 1.0f),
					 float p_intensity = 1.0f)
		: m_color(p_color), m_position(p_position), m_direction(p_direction), m_attenuation(p_attenuation), m_intensity(p_intensity) { }
	
	Math::Vec3f m_color;
	float m_padding1;	// Unused variable, declared for padding
	Math::Vec3f m_position;
	float m_padding2;	// Unused variable, declared for padding
	Math::Vec3f m_direction;
	Math::Vec3f m_attenuation;
	float m_intensity;
	float m_cutoffAngle;
};

struct HDRDataSet
{
	HDRDataSet()
	{
		m_screenBrightness = 0.5f;
	}

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

	Math::Vec3f m_padding1;
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
		Math::Vec3f p_solarIrradiance, 
		float p_sunAngularRadius, 
		DensityProfile p_rayleighDensity, 
		DensityProfile p_mieDensity, 
		DensityProfile p_absorptionDensity, 
		Math::Vec3f p_rayleighScattering, 
		float p_bottomRadius, 
		Math::Vec3f p_mieScattering, 
		float p_topRadius, 
		Math::Vec3f p_mieExtinction, 
		float p_miePhaseFunctionG, 
		Math::Vec3f p_absorptionExtinction, 
		float p_muSMin, 
		Math::Vec3f p_groundAlbedo) :
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
	Math::Vec3f m_solarIrradiance;
	
	// The sun's angular radius
	float m_sunAngularRadius;

	// The density profile of air molecules
	DensityProfile m_rayleighDensity;

	// The density profile of aerosols
	DensityProfile m_mieDensity;

	// The density profile of air molecules that absorb light (e.g. ozone)
	DensityProfile m_absorptionDensity;

	// The scattering coefficient of air molecules at the altitude where their density is maximum
	Math::Vec3f m_rayleighScattering;
	
	// The distance between the planet center and the bottom of the atmosphere.
	float m_bottomRadius;

	// The scattering coefficient of aerosols at the altitude where their density is maximum
	Math::Vec3f m_mieScattering;

	// The distance between the planet center and the top of the atmosphere.
	float m_topRadius;

	// The extinction coefficient of aerosols at the altitude where their density is maximum
	Math::Vec3f m_mieExtinction;
	
	// The asymetry parameter for the Cornette-Shanks phase function for the aerosols.
	float m_miePhaseFunctionG;

	// The extinction coefficient of molecules that absorb light (e.g. ozone)
	Math::Vec3f m_absorptionExtinction;
	
	// The cosine of the maximum Sun zenith angle for which atmospheric scattering must be precomputed
	float m_muSMin;

	// The average albedo of the ground.
	Math::Vec3f m_groundAlbedo;
};

// Parameters for atmospheric scattering shader
struct AtmScatteringParameters
{
	AtmScatteringParameters() :
		m_whitePoint(1.0f, 1.0f, 1.0f),
		m_earthCenter(0.0f, -6360000.0f / 1000.0f, 0.0f),
		m_sunSize(tan(0.01935f / 2.0f), cos(0.01935f / 2.0f)) { }

	AtmScatteringParameters(AtmosphereParameters p_atmosphereParam) : 
		m_atmosphereParam(p_atmosphereParam),
		m_whitePoint(1.0f, 1.0f, 1.0f),
		m_earthCenter(0.0f, -6360000.0f / 1000.0f, 0.0f),
		m_sunSize(tan(0.01935f / 2.0f), cos(0.01935f / 2.0f)) { }
	
	AtmScatteringParameters(
		AtmosphereParameters p_atmosphereParam,
		Math::Vec3f p_whitePoint,
		Math::Vec3f p_earthCenter,
		Math::Vec2f p_sunSize) : 
		m_atmosphereParam(p_atmosphereParam),
		m_whitePoint(p_whitePoint),
		m_earthCenter(p_earthCenter),
		m_sunSize(p_sunSize) { }

	Math::Vec3f m_whitePoint;
	float m_padding1;
	Math::Vec3f m_earthCenter;
	float m_padding2;
	Math::Vec2f m_sunSize;
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