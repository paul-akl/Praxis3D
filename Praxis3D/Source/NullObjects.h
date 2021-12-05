#pragma once

#include <string>

namespace Math
{
	struct Quaternion;
	struct Vec3f;
	struct Vec4f;
	class Mat4f;
}

struct SpatialData;
struct SpatialTransformData;
class SpatialDataManager;

namespace NullObjects
{
	const extern Math::Quaternion		NullQuaterion;
	const extern Math::Vec3f			NullVec3f;
	const extern Math::Vec4f			NullVec4f;
	const extern Math::Mat4f			NullMat4f;
	const extern bool					NullBool;
	const extern int					NullInt;
	const extern float					NullFloat;
	const extern double					NullDouble;
	const extern std::string			NullString;
	const extern SpatialData			NullSpacialData;
	const extern SpatialTransformData	NullSpacialTransformData;
	const extern SpatialDataManager		NullSpatialDataManager;
}