#pragma once

#include <string>

#include "Config.h"
#include "CommonDefinitions.h"
#include "Math.h"
#include "PropertySet.h"

// Stores two template objects to be used for double buffering
template <class T_Object>
struct DoubleBufferedContainer
{
	DoubleBufferedContainer()
	{
		m_swapFlag = false;
	}

	T_Object &getFront() { return m_buffers[(int)m_swapFlag]; }
	T_Object &getBack() { return m_buffers[(int)!m_swapFlag]; }

	void swapBuffer() { m_swapFlag = !m_swapFlag; }

	bool m_swapFlag;

	T_Object m_buffers[2];
};

// Contains GUI settings for an editor window
struct EditorWindowSettings
{
	EditorWindowSettings()
	{
		m_enabled = true;
	}

	bool m_enabled;
};

// Stores an engine change type and all associated data needed for that change
struct EngineChangeData
{
	EngineChangeData(const EngineChangeType p_changeType = EngineChangeType::EngineChangeType_None, const EngineStateType p_engineStateType = EngineStateType::EngineStateType_Default) : m_changeType(p_changeType), m_engineStateType(p_engineStateType) { }
	EngineChangeData(const EngineChangeType p_changeType, const EngineStateType p_engineStateType, const std::string &p_filename) : m_changeType(p_changeType), m_engineStateType(p_engineStateType), m_filename(p_filename) { }
	EngineChangeData(const EngineChangeType p_changeType, const EngineStateType p_engineStateType, const PropertySet &p_sceneProperties) : m_changeType(p_changeType), m_engineStateType(p_engineStateType), m_sceneProperties(p_sceneProperties) { }
	~EngineChangeData() { }

	void setChangeType(const EngineChangeType p_changeType) { m_changeType = p_changeType; }
	void setEngineStateType(const EngineStateType p_engineStateType) { m_engineStateType = p_engineStateType; }
	void setFilename(const std::string &p_filename) { m_filename = p_filename; }

	EngineChangeType m_changeType;
	EngineStateType m_engineStateType;
	std::string m_filename;
	PropertySet m_sceneProperties;
};

// Contains Major, Minor and Patch engine version values
struct EngineVersion
{
	EngineVersion() : m_major(-1), m_minor(-1), m_patch(-1) { }
	EngineVersion(const int p_major, const int p_minor, const int p_patch) :
		m_major(p_major), m_minor(p_minor), m_patch(p_patch) { }

	void reset()
	{
		m_major = -1;
		m_minor = -1;
		m_patch = -1;
	}

	inline int getVersionSingleFormat() const noexcept
	{
		if(valid())
			return (m_major * 1000000) + (m_minor * 1000) + m_patch;

		return 0;
	}

	inline bool valid() const noexcept
	{
		if(m_major < 0)
			return false;
		if(m_minor < 0)
			return false;
		if(m_patch < 0)
			return false;
		return true;
	}

	inline explicit operator bool() const noexcept { return valid(); }

	const inline bool operator==(const EngineVersion &p_version) const noexcept
	{
		return (getVersionSingleFormat() == p_version.getVersionSingleFormat());
	}
	const inline bool operator<(const EngineVersion &p_version) const noexcept
	{
		return (getVersionSingleFormat() < p_version.getVersionSingleFormat());
	}
	const inline bool operator>(const EngineVersion &p_version) const noexcept
	{
		return (getVersionSingleFormat() > p_version.getVersionSingleFormat());
	}

	int m_major;
	int m_minor;
	int m_patch;
};

// Stores an entity ID and a component type, used for sending data to create / delete components
struct EntityAndComponent
{
	//EntityAndComponent() : m_entityID(NULL_ENTITY_ID), m_componentType(ComponentType::ComponentType_None) { }
	EntityAndComponent(const EntityID p_entityID, const ComponentType p_componentType) : m_entityID(p_entityID), m_componentType(p_componentType) { }

	EntityID m_entityID;
	ComponentType m_componentType;
};

// Stores settings for face culling
struct FaceCullingSettings
{
	FaceCullingSettings() : m_faceCullingEnabled(true), m_backFaceCulling(true) { }
	FaceCullingSettings(const bool p_faceCullingEnabled, const bool p_backFaceCulling) : m_faceCullingEnabled(p_faceCullingEnabled), m_backFaceCulling(p_backFaceCulling) { }

	bool m_faceCullingEnabled;
	bool m_backFaceCulling;
};

// Stores a vector of std::functions (Functors) and methods to add, clear and get them
struct FunctorSequence
{
	FunctorSequence() { }

	template<typename Functor>
	inline void addFunctor(Functor p_functor) { m_functors.push_back(p_functor); }

	const Functors &getFunctors() const { return m_functors; }

	void clear() { m_functors.clear(); }

	Functors m_functors;
};

// Stores all GUI data
struct GUIData
{
	GUIData()
	{
		clear();
	}

	// Set all the data to default
	void clear()
	{
		m_functors.clear();
	}

	Functors m_functors;
};

// Packs a single unsigned 64bit int into two unsigned 32bit ints
struct Int64Packer
{
	Int64Packer() : x(0), y(0) { }
	Int64Packer(unsigned __int64 p_int) { set(p_int); }

	inline void set(const unsigned __int64 p_int)
	{
		x = (unsigned __int32)(p_int >> 32);
		y = (unsigned __int32)p_int;
	}

	inline unsigned __int64 get() const { return ((unsigned __int64)x) << 32 | y; }

	unsigned __int32 x, y;
};

// Stores all physics data
struct PhysicsData
{
	PhysicsData()
	{
		clear();
	}

	// Set all the data to default
	void clear()
	{

	}
};

// Stores all spatial data (position, rotation, scale)
struct SpatialData
{	
	SpatialData() 
	{
		clear();
	}
	SpatialData(const glm::vec3 &p_position, const glm::vec3 &p_rotationEuler, const glm::vec3 &p_scale, const glm::quat &p_rotationQuat)
		: m_position(p_position), m_scale(p_scale), m_rotationEuler(p_rotationEuler), m_rotationQuat(p_rotationQuat) { }

	friend const inline SpatialData operator+(const SpatialData &p_left, const SpatialData &p_right)
	{
		return SpatialData(	p_left.m_position + p_right.m_position, 
							p_left.m_rotationEuler + p_right.m_rotationEuler, 
							p_left.m_scale + p_right.m_scale, 
							p_left.m_rotationQuat * p_right.m_rotationQuat);
	}

	const inline SpatialData operator+=(const SpatialData &p_data)
	{
		return SpatialData(	m_position + p_data.m_position,
							m_rotationEuler + p_data.m_rotationEuler,
							m_scale + p_data.m_scale,
							m_rotationQuat * p_data.m_rotationQuat);
	}

	// Set all the data to default
	void clear()
	{
		m_position = glm::vec3(0.0f);
		m_rotationEuler = glm::vec3(0.0f);
		m_scale = glm::vec3(1.0f);
		m_rotationQuat = glm::quat();
	}

	glm::vec3	m_position,
				m_rotationEuler,
				m_scale;
	glm::quat m_rotationQuat;
};

// Stores all spatial data (position, rotation, scale) plus the transform matrix
struct SpatialTransformData
{
	SpatialTransformData() 
	{
		clear();
	}
	SpatialTransformData(const SpatialData &p_spatialData, const glm::mat4 &p_transformMat) : m_spatialData(p_spatialData), m_transformMatNoScale(p_transformMat) { }

	friend const inline SpatialTransformData operator+(const SpatialTransformData &p_left, const SpatialTransformData &p_right)
	{
		return SpatialTransformData(p_left.m_spatialData + p_right.m_spatialData, p_left.m_transformMatNoScale * p_right.m_transformMatNoScale);
	}
	const inline SpatialTransformData operator+=(const SpatialTransformData &p_data)
	{ 
		return SpatialTransformData(m_spatialData + p_data.m_spatialData, m_transformMatNoScale * p_data.m_transformMatNoScale);
	}

	// Set all the data to default
	void clear()
	{
		m_spatialData.clear();
		m_transformMatNoScale = glm::mat4(1.0f);
	}

	SpatialData m_spatialData;
	glm::mat4 m_transformMatNoScale;
};

/*	 _______________________________
	|								|
	|   Renderer data containers:	|
	|_______________________________|
*/

// Stores settings for atmospheric light scattering
struct AtmosphericScatteringData
{
	struct AtmosphericDensityLayer
	{
		AtmosphericDensityLayer() : AtmosphericDensityLayer(0.0f, 0.0f, 0.0f, 0.0f, 0.0f) { }
		AtmosphericDensityLayer(float p_width, float p_expTerm, float p_expScale, float p_linearTerm, float p_constantTerm) :
			m_width(p_width), m_expTerm(p_expTerm), m_expScale(p_expScale), m_linearTerm(p_linearTerm), m_constantTerm(p_constantTerm) { }

		float m_width;
		float m_expTerm;
		float m_expScale;
		float m_linearTerm;
		float m_constantTerm;
	};

	AtmosphericScatteringData()
	{
		m_atmosphereBottomRadius = 6360.0f;
		m_atmosphereTopRadius = 6420.0f;

		m_rayleighScattering = glm::vec3(0.005802f, 0.013558f, 0.033100f);
		m_mieScattering = glm::vec3(0.003996f, 0.003996f, 0.003996f);
		m_mieExtinction = glm::vec3(0.004440f, 0.004440f, 0.004440f);
		m_absorptionExtinction = glm::vec3(0.000650f, 0.001881f, 0.000085f);

		m_rayleighDensity[0] = AtmosphericDensityLayer(0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f);
		m_rayleighDensity[1] = AtmosphericDensityLayer(0.000000f, 1.000000f, -0.125000f, 0.000000f, 0.000000f);
		m_mieDensity[0] = AtmosphericDensityLayer(0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f);
		m_mieDensity[1] = AtmosphericDensityLayer(0.000000f, 1.000000f, -0.833333f, 0.000000f, 0.000000f);
		m_absorptionDensity[0] = AtmosphericDensityLayer(25.000000f, 0.000000f, 0.000000f, 0.066667f, -0.666667f);
		m_absorptionDensity[1] = AtmosphericDensityLayer(0.000000f, 0.000000f, 0.000000f, -0.066667f, 2.666667f);

		m_planetGroundColor = glm::vec3(0.1f, 0.1f, 0.1f);
		m_planetCenterPosition = glm::vec3(0.0f, -6360000.0f, 0.0f);

		m_sunIrradiance = glm::vec3(1.474000f, 1.850400f, 1.911980f);
		m_sunSize = 0.02935f;
		//m_sunSize = 0.00935f; old value
	}

	// Atmosphere
	float m_atmosphereBottomRadius;
	float m_atmosphereTopRadius;
	glm::vec3 m_rayleighScattering;
	glm::vec3 m_mieScattering;
	glm::vec3 m_mieExtinction;
	glm::vec3 m_absorptionExtinction;

	AtmosphericDensityLayer m_rayleighDensity[2];
	AtmosphericDensityLayer m_mieDensity[2];
	AtmosphericDensityLayer m_absorptionDensity[2];

	// Ground
	glm::vec3 m_planetGroundColor;
	glm::vec3 m_planetCenterPosition;

	// Sun
	glm::vec3 m_sunIrradiance;
	float m_sunSize;
};

// Stores data used for tweaking the ambient occlusion effect
struct AmbientOcclusionData
{
	AmbientOcclusionData()
	{
		setDefaultValues();
		m_aoType = AmbientOcclusionType::AmbientOcclusionType_None;
	}

	void setDefaultValues()
	{
		m_aoNumOfDirections = Config::graphicsVar().ao_num_of_directions;
		m_aoNumOfSamples = Config::graphicsVar().ao_num_of_samples;
		m_aoNumOfSteps = Config::graphicsVar().ao_num_of_steps;
		m_aoBlurNumOfSamples = Config::graphicsVar().ao_blur_num_of_samples;
		m_aoBlurSharpness = Config::graphicsVar().ao_blurSharpness;
		m_aoType = AmbientOcclusionData::ambientOcclusionTypeToAmbientOcclusionType(Config::graphicsVar().ao_type);

		// Apply default AO type specific values
		switch(m_aoType)
		{
			case AmbientOcclusionType_SSAO:
				m_aoBias = Config::graphicsVar().ao_ssao_bias;
				m_aoIntensity = Config::graphicsVar().ao_ssao_intensity;
				m_aoRadius = Config::graphicsVar().ao_ssao_radius;
				break;
			case AmbientOcclusionType_HBAO:
				m_aoBias = Config::graphicsVar().ao_hbao_bias;
				m_aoIntensity = Config::graphicsVar().ao_hbao_intensity;
				m_aoRadius = Config::graphicsVar().ao_hbao_radius;
				break;
		}
	}

	bool operator==(const AmbientOcclusionData &p_aoData) 
	{ 
		return	m_aoNumOfDirections == p_aoData.m_aoNumOfDirections &&
				m_aoNumOfSamples	== p_aoData.m_aoNumOfSamples &&
				m_aoNumOfSteps		== p_aoData.m_aoNumOfSteps &&
				m_aoIntensity		== p_aoData.m_aoIntensity &&
				m_aoBias			== p_aoData.m_aoBias &&
				m_aoRadius			== p_aoData.m_aoRadius &&
				m_aoBlurSharpness	== p_aoData.m_aoBlurSharpness &&
				m_aoType			== p_aoData.m_aoType;
	}

	static Properties::PropertyID ambientOcclusionTypeToPropertyID(const AmbientOcclusionType p_aoType)
	{
		switch(p_aoType)
		{
			case AmbientOcclusionType::AmbientOcclusionType_None:
			default:
				return Properties::None;
				break;
			case AmbientOcclusionType::AmbientOcclusionType_SSAO:
				return Properties::SSAO;
				break;
			case AmbientOcclusionType::AmbientOcclusionType_HBAO:
				return Properties::HBAO;
				break;
		}
	}
	static AmbientOcclusionType propertyIDToAmbientOcclusionType(const Properties::PropertyID p_propertyID)
	{
		switch(p_propertyID)
		{
			case Properties::None:
			default:
				return AmbientOcclusionType::AmbientOcclusionType_None;
				break;
			case Properties::SSAO:
				return AmbientOcclusionType::AmbientOcclusionType_SSAO;
				break;
			case Properties::HBAO:
				return AmbientOcclusionType::AmbientOcclusionType_HBAO;
				break;
		}
	}
	static AmbientOcclusionType ambientOcclusionTypeToAmbientOcclusionType(const int p_aoType)
	{
		switch(p_aoType)
		{
			case AmbientOcclusionType::AmbientOcclusionType_None:
			default:
				return AmbientOcclusionType::AmbientOcclusionType_None;
				break;
			case AmbientOcclusionType::AmbientOcclusionType_SSAO:
				return AmbientOcclusionType::AmbientOcclusionType_SSAO;
				break;
			case AmbientOcclusionType::AmbientOcclusionType_HBAO:
				return AmbientOcclusionType::AmbientOcclusionType_HBAO;
				break;
		}
	}

	int m_aoNumOfDirections;
	int m_aoNumOfSamples;
	int m_aoNumOfSteps;
	int m_aoBlurNumOfSamples;
	float m_aoIntensity;
	float m_aoBias;
	float m_aoRadius;
	float m_aoBlurSharpness;
	AmbientOcclusionType m_aoType;
};

// Stores misc scene settings
struct MiscSceneData
{
	MiscSceneData() : 
		m_ambientIntensityDirectional(Config::graphicsVar().ambient_intensity_directional),
		m_ambientIntensityPoint(Config::graphicsVar().ambient_intensity_point),
		m_ambientIntensitySpot(Config::graphicsVar().ambient_intensity_spot),
		m_stochasticSamplingSeamFix(Config::rendererVar().stochastic_sampling_seam_fix) { }

	// Ambient light intensity multiplier
	float m_ambientIntensityDirectional;
	float m_ambientIntensityPoint;
	float m_ambientIntensitySpot;

	// Stochastic sampling mipmap seam fix flag
	bool m_stochasticSamplingSeamFix;
};

// Stores a single CSM cascade settings
struct ShadowCascadeData
{
	ShadowCascadeData() : m_cascadeFarDistance(1.0f), m_maxBias(Config::rendererVar().csm_max_shadow_bias), m_distanceIsDivider(true), m_penumbraScale(1.0f) { }
	ShadowCascadeData(const float p_cascadeDistance, const bool p_distanceIsDivider, const float p_maxBias, const float p_penumbraScale) : m_cascadeFarDistance(p_cascadeDistance), m_distanceIsDivider(p_distanceIsDivider), m_maxBias(p_maxBias), m_penumbraScale(p_penumbraScale) { }

	// Float value holds either cascade distance or a divider
	float m_cascadeFarDistance;

	// Maximum bias value that the slope bias calculation result is clamped at
	float m_maxBias;

	// PCF Poisson sampling "spread" gets multiplied by penumbra scale
	float m_penumbraScale;

	// Bool value indicates whether the float is a literal distance unit or a divider (meant to divide z-far to get the distance unit)
	// true - divider; false - literal distance unit
	bool m_distanceIsDivider;
};

// Stores shadow mapping settings
struct ShadowMappingData
{
	ShadowMappingData()
	{
		m_penumbraScaleRange = glm::vec2(Config::rendererVar().csm_penumbra_size_scale_min, Config::rendererVar().csm_penumbra_size_scale_max);
		m_csmBiasScale = Config::rendererVar().csm_bias_scale;
		m_csmCascadePlaneZMultiplier = 10.0f;
		m_penumbraSize = Config::rendererVar().csm_penumbra_size;
		m_csmResolution = Config::rendererVar().csm_resolution;
		m_numOfPCFSamples = Config::rendererVar().csm_num_of_pcf_samples;
		m_shadowMappingEnabled = false;
		m_zClipping = false;
	}

	void setDefaultValues()
	{
		m_penumbraScaleRange = glm::vec2(Config::rendererVar().csm_penumbra_size_scale_min, Config::rendererVar().csm_penumbra_size_scale_max);
		m_csmBiasScale = Config::rendererVar().csm_bias_scale;
		m_csmCascadePlaneZMultiplier = 10.0f;
		m_penumbraSize = Config::rendererVar().csm_penumbra_size;
		m_csmResolution = Config::rendererVar().csm_resolution;
		m_numOfPCFSamples = Config::rendererVar().csm_num_of_pcf_samples;
		m_shadowMappingEnabled = true;
		m_zClipping = true;

		// Populate the cascade vector with default entries
		m_shadowCascadePlaneDistances.emplace_back(8.0f, false, 0.0075f, 1.0f);
		m_shadowCascadePlaneDistances.emplace_back(16.0f, false, 0.0005f, 1.0f);
		m_shadowCascadePlaneDistances.emplace_back(32.0f, false, 0.0005f, 1.0f);
		m_shadowCascadePlaneDistances.emplace_back(64.0f, false, 0.0005f, 1.0f);
		m_shadowCascadePlaneDistances.emplace_back(128.0f, false, 0.0005f, 1.0f);
		m_shadowCascadePlaneDistances.emplace_back(256.0f, false, 0.0005f, 1.0f);
		m_shadowCascadePlaneDistances.emplace_back(512.0f, false, 0.0005f, 1.0f);
		m_shadowCascadePlaneDistances.emplace_back(1.0f, true, 0.0005f, 1.0f);
	}

	std::vector<ShadowCascadeData> m_shadowCascadePlaneDistances;

	// Cascaded shadow mapping data
	glm::vec2 m_penumbraScaleRange;		// Min and Max range of penumbra size scaling based on fragment distance from the camera
	float m_csmBiasScale;				// Value that the automatically calculated CSM bias in the shader is scaled with
	float m_csmCascadePlaneZMultiplier;	// Multiplier for CSM cascade planes z clip distance
	float m_penumbraSize;				// Shadow edge softness
	unsigned int m_csmResolution;		// Resolution of CSM shadow map depth buffer
	unsigned int m_numOfPCFSamples;		// Number of shadow map samples taken and averaged out during the Percentage-Closer Filtering
	bool m_shadowMappingEnabled;		// Is the shadow mapping pass enabled
	bool m_zClipping;					// Turn on z-clipping when rendering geometry for CSM depth pass (fixes VERY distant objects getting clipped out of the orthogonal shadow plane)
};