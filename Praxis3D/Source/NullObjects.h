#pragma once

#include <string>

#include "CommonDefinitions.h"
#include "Math.h"

struct SpatialData;
struct SpatialTransformData;
class SpatialDataManager;
class GUIDataManager;
class PhysicsDataManager;

namespace NullObjects
{
	const extern glm::quat				NullQuaterion;
	const extern glm::vec3				NullVec3f;
	const extern glm::vec4				NullVec4f;
	const extern glm::mat3				NullMat3f;
	const extern glm::mat4				NullMat4f;
	const extern bool					NullBool;
	const extern int					NullInt;
	const extern unsigned int			NullUnsignedInt;
	const extern float					NullFloat;
	const extern double					NullDouble;
	const extern std::string			NullString;
	const extern SpatialData			NullSpacialData;
	const extern SpatialTransformData	NullSpacialTransformData;
	const extern SpatialDataManager		NullSpatialDataManager;
	const extern GUIDataManager			NullGUIDataManager;
	const extern PhysicsDataManager		NullPhysicsDataManager;
	const extern Functors				NullFunctors;
}