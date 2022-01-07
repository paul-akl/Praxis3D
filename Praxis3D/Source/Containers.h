#pragma once

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

	glm::vec3 m_position,
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
	SpatialTransformData(const SpatialData &p_spatialData, const glm::mat4 &p_transformMat) : m_spatialData(p_spatialData), m_transformMat(p_transformMat) { }

	friend const inline SpatialTransformData operator+(const SpatialTransformData &p_left, const SpatialTransformData &p_right)
	{
		return SpatialTransformData(p_left.m_spatialData + p_right.m_spatialData, p_left.m_transformMat * p_right.m_transformMat);
	}
	const inline SpatialTransformData operator+=(const SpatialTransformData &p_data)
	{ 
		return SpatialTransformData(m_spatialData + p_data.m_spatialData, m_transformMat * p_data.m_transformMat);
	}

	// Set all the data to default
	void clear()
	{
		m_spatialData.clear();
		m_transformMat = glm::mat4(1.0f);
	}

	SpatialData m_spatialData;
	glm::mat4 m_transformMat;
};

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