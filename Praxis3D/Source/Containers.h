#pragma once

#include <string>

#include "CommonDefinitions.h"
#include "Math.h"

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

// Contains GUI settings for an editor window
struct EditorWindowSettings
{
	EditorWindowSettings()
	{
		m_enabled = true;
	}

	bool m_enabled;
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

// Stores an engine change type and all associated data needed for that change
struct EngineChangeData
{
	EngineChangeData() : m_changeType(EngineChangeType::EngineChangeType_None), m_engineStateType(EngineStateType::EngineStateType_Default) { }
	EngineChangeData(EngineChangeType p_changeType, EngineStateType p_engineStateType = EngineStateType::EngineStateType_Default, std::string p_filename = "") : m_changeType(p_changeType), m_engineStateType(p_engineStateType), m_filename(p_filename) { }
	~EngineChangeData() { }

	void setChangeType(const EngineChangeType p_changeType) { m_changeType = p_changeType; }
	void setEngineStateType(const EngineStateType p_engineStateType) { m_engineStateType = p_engineStateType; }
	void setFilename(const std::string &p_filename) { m_filename = p_filename; }

	EngineChangeType m_changeType;
	EngineStateType m_engineStateType;
	std::string m_filename;
};