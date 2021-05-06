#pragma once

#include "Math.h"

// Stores all spatial data (position, rotation, scale)
struct SpatialData
{	
	// Set all data to default
	void clear()
	{
		m_position = Math::Vec3f();
		m_scale = Math::Vec3f();
		m_rotationEuler = Math::Vec3f();
		m_rotationQuat = Math::Quaternion();
	}

	Math::Vec3f m_position,
				m_scale,
				m_rotationEuler;
	Math::Quaternion m_rotationQuat;
};

// Stores all spatial data (position, rotation, scale) plus transform matrix
struct SpatialTransformData
{
	// Set all data to default
	void clear()
	{
		m_spatialData.clear();
		m_transformMat.identity();
	}

	SpatialData m_spatialData;
	Math::Mat4f m_transformMat;
};