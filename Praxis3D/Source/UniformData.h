#pragma once

#include "GraphicsDataSets.h"
#include "Math.h"

struct UniformFrameData
{
	UniformFrameData()
	{
		m_ambientIntensity = Config::graphicsVar().ambient_light_intensity;

		m_deltaTime = 0.0f;

		m_numPointLights = 0;
		m_numSpotLights = 0;

		m_zFar = Config::graphicsVar().z_far;
		m_zNear = Config::graphicsVar().z_near;
		m_fov = Config::graphicsVar().fov;

		m_bloomTreshold = glm::vec4(0.0f);

		m_texelSize = glm::vec2(1.0f);
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
				m_transposeViewMatrix,
				m_transposeInverseViewMatrix;

	// Parameters of directional light, since there can be only one of it
	DirectionalLightDataSet m_directionalLight;
	float m_ambientIntensity;

	// Delta time of the last frame
	float m_deltaTime;

	// Current number of lights in the light buffers
	unsigned int m_numPointLights,
				 m_numSpotLights;

	// Projection data
	float m_zFar;
	float m_zNear;
	float m_fov;

	// Ambient occlusion data
	AmbientOcclusionData m_aoData;

	// Bloom pass data
	glm::vec4 m_bloomTreshold;

	// Cascaded shadow mapping data
	ShadowMappingData m_shadowMappingData;

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
		m_emissiveIntensity = 0.0f;
		m_textureTilingFactor = 1.0f;
	}

	UniformObjectData(const glm::mat4 &p_modelMat,
					  const glm::mat4 &p_modelViewProjMatrix,
					  float	p_heightScale = 0.0f,
					  float p_alphaThreshold = 0.0f,
					  float p_emissiveIntensity = 0.0f,
					  float p_textureTilingFactor = 1.0f)
	{
		m_heightScale = p_heightScale;
		m_alphaThreshold = p_alphaThreshold;
		m_emissiveIntensity = p_emissiveIntensity;
		m_textureTilingFactor = p_textureTilingFactor;

		m_modelMat = p_modelMat;
		m_modelViewProjMatrix = p_modelViewProjMatrix;
	}

	glm::mat4	m_modelMat,
				m_modelViewProjMatrix;

	float	m_heightScale,
			m_alphaThreshold,
			m_emissiveIntensity,
			m_textureTilingFactor;
};

struct UniformData
{
	UniformData(const UniformObjectData &p_objectData, const UniformFrameData &p_frameData)
		: m_objectData(p_objectData), m_frameData(p_frameData) { }

	const UniformObjectData &m_objectData;
	const UniformFrameData &m_frameData;
};