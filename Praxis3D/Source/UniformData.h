#pragma once

#include "Math.h"

struct UniformFrameData
{
	UniformFrameData()
	{
		m_numPointLights = 0;
		m_numSpotLights = 0;
	}

	// Framebuffer size (can be different from the window size)
	Math::Vec2i m_screenSize;

	// Camera's position in the scene
	Math::Vec3f m_cameraPosition;

	// Matrices that can only change once per frame
	Math::Mat4f m_projMatrix,
				m_viewMatrix,
				m_viewProjMatrix;

	// Parameters of direction light, since there can be only one of it
	Math::Vec3f m_dirLightColor,
				m_dirLightDirection;
	float		m_dirLightIntensity;

	// Current number of lights in the light buffers
	unsigned int m_numPointLights,
				 m_numSpotLights;
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

	UniformObjectData(const Math::Mat4f &p_modelMat,
					  const Math::Mat4f &p_modelViewProjMatrix,
					  //const unsigned int (&p_materials)[MaterialType_NumOfTypes],
					  float	p_heightScale = 0.0f,
					  float p_alphaThreshold = 0.0f,
					  float p_emissiveThreshold = 0.0f,
					  float p_textureTilingFactor = 1.0f)
	{
		m_heightScale = p_heightScale;
		m_alphaThreshold = p_alphaThreshold;
		m_emissiveThreshold = p_emissiveThreshold;
		m_textureTilingFactor = p_textureTilingFactor;

		m_modelMat = p_modelMat;
		m_modelViewProjMatrix = p_modelViewProjMatrix;

		//memcpy(m_materials, p_materials, sizeof(m_materials));
	}

	Math::Mat4f m_modelMat,
				m_modelViewProjMatrix;

	float	m_heightScale,
			m_alphaThreshold,
			m_emissiveThreshold,
			m_textureTilingFactor;

	//unsigned int m_materials[MaterialType_NumOfTypes];
};

struct UniformData
{
	UniformData(const UniformObjectData &p_objectData, const UniformFrameData &p_frameData)
		: m_objectData(p_objectData), m_frameData(p_frameData) { }

	const UniformObjectData &m_objectData;
	const UniformFrameData &m_frameData;
};