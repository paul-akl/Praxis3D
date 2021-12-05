#pragma once

#include "Math.h"

// Stores all spatial data (position, rotation, scale)
struct SpatialData
{	
	SpatialData() { }
	SpatialData(const Math::Vec3f &p_position, const Math::Vec3f &p_rotationEuler, const Math::Vec3f &p_scale, const Math::Quaternion &p_rotationQuat)
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

	// Set all data to default
	void clear()
	{
		m_position = Math::Vec3f();
		m_rotationEuler = Math::Vec3f();
		m_scale = Math::Vec3f();
		m_rotationQuat = Math::Quaternion();
	}

	Math::Vec3f m_position,
				m_rotationEuler,
				m_scale;
	Math::Quaternion m_rotationQuat;
};

// Stores all spatial data (position, rotation, scale) plus the transform matrix
struct SpatialTransformData
{
	SpatialTransformData() { }
	SpatialTransformData(const SpatialData &p_spatialData, const Math::Mat4f &p_transformMat) : m_spatialData(p_spatialData), m_transformMat(p_transformMat) { }

	friend const inline SpatialTransformData operator+(const SpatialTransformData &p_left, const SpatialTransformData &p_right)
	{
		return SpatialTransformData(p_left.m_spatialData + p_right.m_spatialData, p_left.m_transformMat * p_right.m_transformMat);
	}
	const inline SpatialTransformData operator+=(const SpatialTransformData &p_data)
	{ 
		return SpatialTransformData(m_spatialData + p_data.m_spatialData, m_transformMat * p_data.m_transformMat);
	}

	// Set all data to default
	void clear()
	{
		m_spatialData.clear();
		m_transformMat.identity();
	}

	SpatialData m_spatialData;
	Math::Mat4f m_transformMat;
};