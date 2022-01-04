#pragma once

//#define _USE_MATH_DEFINES
#define GLM_FORCE_CTOR_INIT

#include <algorithm>
#include <cmath>
#include <glm/gtx/euler_angles.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>

#define E_CONST		2.71828182845904523536
#define LOG2E		1.44269504088896340736
#define LOG10E		0.434294481903251827651
#define LN2			0.693147180559945309417
#define LN10		2.30258509299404568402
#define PI			3.14159265358979323846
#define PI_2		1.57079632679489661923
#define PI_4		0.785398163397448309616
#define SQRTPI		1.12837916709551257390
#define SQRT2		1.41421356237309504880
#define SQRT1_2		0.707106781186547524401
#define RAD			(PI / 180.0)
#define DEG			(180.0 / PI)
#define TWOPI		(PI * 2)
#define RAD2DEG		(360/(PI*2))

namespace Math
{
	const glm::mat4 createTransformMat(const glm::vec3 &p_position, const glm::vec3 &p_rotation, const glm::vec3 &p_scale);
	const inline glm::mat4 createTransformMat(const glm::vec3 &p_position, const glm::quat &p_rotation, const glm::vec3 &p_scale)
	{
		glm::mat4 returnMatrix(1.0f);

		returnMatrix = glm::translate(returnMatrix, p_position);

		returnMatrix *= glm::toMat4(p_rotation);

		returnMatrix = glm::scale(returnMatrix, p_scale);
		//returnMatrix *= glm::orientate4(glm::vec3(p_rotation.y, p_rotation.z, p_rotation.x));
		//returnMatrix = glm::rotate(returnMatrix, rotAngle, Rotation);

		return returnMatrix;
	}

	const inline glm::mat4 perspectiveRadian(const float p_FOV, const int p_screenWidth, const int p_screenHeight)
	{
		glm::mat4 m(1.0f);

		float aspectRatio = static_cast<float>(p_screenWidth) / static_cast<float>(p_screenHeight);
		float kFovY = p_FOV / 180.0f * (float)PI;
		float kTanFovY = tan(kFovY / 2.0f);

		m[0][0] = kTanFovY * aspectRatio;	m[1][0] = 0.0f;		m[2][0] = 0.0f;		m[3][0] = 0.0f;
		m[0][1] = 0.0f;						m[1][1] = kTanFovY;	m[2][1] = 0.0f;		m[3][2] = 0.0f;
		m[0][2] = 0.0f;						m[1][2] = 0.0f;		m[2][2] = 0.0f;		m[3][3] = 1.0f;
		m[0][3] = 0.0f;						m[1][3] = 0.0f;		m[2][3] = -1.0f;	m[3][3] = 1.0f;

		return m;
	}
	const inline glm::quat eulerDegreesToQuaterion(const glm::vec3 &p_degrees)
	{
		glm::quat qPitch = glm::angleAxis(glm::radians(p_degrees.x), glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(glm::radians(p_degrees.y), glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(p_degrees.z), glm::vec3(0, 0, 1));

		return glm::normalize(qPitch * qYaw * qRoll);
	}

	//float toRadian(const float p_degrees) { return glm::radians(p_degrees); }
	//glm::quat test;
}