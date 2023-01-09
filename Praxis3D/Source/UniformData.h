#pragma once

#include "Math.h"

struct UniformFrameData
{
	UniformFrameData()
	{
		m_dirLightIntensity = 0.0f;
		m_numPointLights = 0;
		m_numSpotLights = 0;
		m_deltaTime = 0.0f;
		m_mipLevel = 1;
	}

	// Framebuffer size (can be different from the window size)
	glm::ivec2 m_screenSize;

	// Camera's position in the scene
	glm::vec3 m_cameraPosition;

	// Camera's target vector in the scene
	glm::vec3 m_cameraTarget;

	// Matrices that can only change once per frame
	glm::mat4 m_projMatrix,
				m_viewMatrix,
				m_viewProjMatrix,
				m_atmScatProjMatrix,
				m_transposeViewMatrix;

	// Parameters of direction light, since there can be only one of it
	glm::vec3 m_dirLightColor,
				m_dirLightDirection;
	float		m_dirLightIntensity;

	// Delta time of the last frame
	float		m_deltaTime;

	// Current number of lights in the light buffers
	unsigned int m_numPointLights,
				 m_numSpotLights;

	// Bloom pass data
	glm::vec4 m_bloomTreshold;

	// Misc
	glm::vec2 m_texelSize;
	int m_mipLevel;
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

	UniformObjectData(const glm::mat4 &p_modelMat,
					  const glm::mat4 &p_modelViewProjMatrix,
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

	glm::mat4	m_modelMat,
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