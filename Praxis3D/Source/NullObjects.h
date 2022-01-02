#pragma once

#include <string>

#include "Math.h"

/*namespace Math
{
	struct Quaternion;
	struct Vec3f;
	struct Vec4f;
	class Mat4f;
}*/

struct SpatialData;
struct SpatialTransformData;
class SpatialDataManager;

namespace NullObjects
{
	const extern glm::quat		NullQuaterion;
	const extern glm::vec3			NullVec3f;
	const extern glm::vec4			NullVec4f;
	const extern glm::mat4			NullMat4f;
	const extern bool					NullBool;
	const extern int					NullInt;
	const extern float					NullFloat;
	const extern double					NullDouble;
	const extern std::string			NullString;
	const extern SpatialData			NullSpacialData;
	const extern SpatialTransformData	NullSpacialTransformData;
	const extern SpatialDataManager		NullSpatialDataManager;
}