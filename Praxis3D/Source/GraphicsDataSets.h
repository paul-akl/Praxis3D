#pragma once

#include "Math.h"
#include "Loaders.h"

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

struct RenderableObjectData
{
	RenderableObjectData(ModelLoader::ModelHandle p_model, ShaderLoader::ShaderProgram *p_shader, GraphicsData &p_baseObjectData) :
		m_model(p_model),
		m_shader(p_shader),
		m_loadedToMemory(false),
		m_baseObjectData(p_baseObjectData) { }
	
	GraphicsData &m_baseObjectData;

	bool m_loadedToMemory;

	ModelLoader::ModelHandle m_model;
	ShaderLoader::ShaderProgram *m_shader;
	std::vector<bool> m_defaultMaterial[MaterialType_NumOfTypes];
	std::vector<TextureLoader2D::Texture2DHandle> m_materials[MaterialType_NumOfTypes];
	std::vector<TextureLoader2D::Texture2DHandle>::size_type m_numMaterials;

	// Unused, as renderer checks texture handle not being 0 instead
	//std::bitset<Model::NumOfModelMaterials> m_materialPresent;
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