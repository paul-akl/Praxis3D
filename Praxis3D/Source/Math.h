#pragma once

//#define _USE_MATH_DEFINES
//#define GLM_FORCE_CTOR_INIT

#include <algorithm>
#include <bullet3/LinearMath/btMatrix3x3.h>
#include <bullet3/LinearMath/btTransform.h>
#include <bullet3/LinearMath/btQuaternion.h>
#include <bullet3/LinearMath/btVector3.h>
#include <cmath>
#include <fmod/fmod_common.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>


namespace Math
{
	constexpr float E_CONST			= 2.71828182845904523536f;
	constexpr float LOG2E			= 1.44269504088896340736f;
	constexpr float LOG10E			= 0.434294481903251827651f;
	constexpr float LN2				= 0.693147180559945309417f;
	constexpr float LN10			= 2.30258509299404568402f;
	constexpr float PI				= 3.14159265358979323846f;
	constexpr float PI_2			= 1.57079632679489661923f;
	constexpr float PI_4			= 0.785398163397448309616f;
	constexpr float SQRTPI			= 1.12837916709551257390f;
	constexpr float SQRT2			= 1.41421356237309504880f;
	constexpr float SQRT1_2			= 0.707106781186547524401f;
	constexpr float RAD				= (PI / 180.0f);
	constexpr float DEG				= (180.0f / PI);
	constexpr float RAD2DEG			= (360.0f / (PI * 2.0f));

	constexpr double E_CONST_DOUBLE = 2.71828182845904523536;
	constexpr double LOG2E_DOUBLE	= 1.44269504088896340736;
	constexpr double LOG10E_DOUBLE	= 0.434294481903251827651;
	constexpr double LN2_DOUBLE		= 0.693147180559945309417;
	constexpr double LN10_DOUBLE	= 2.30258509299404568402;
	constexpr double PI_DOUBLE		= 3.14159265358979323846;
	constexpr double PI_2_DOUBLE	= 1.57079632679489661923;
	constexpr double PI_4_DOUBLE	= 0.785398163397448309616;
	constexpr double SQRTPI_DOUBLE	= 1.12837916709551257390;
	constexpr double SQRT2_DOUBLE	= 1.41421356237309504880;
	constexpr double SQRT1_2_DOUBLE = 0.707106781186547524401;
	constexpr double RAD_DOUBLE		= (PI_DOUBLE / 180.0);
	constexpr double DEG_DOUBLE		= (180.0 / PI_DOUBLE);
	constexpr double RAD2DEG_DOUBLE = (360.0 / (PI_DOUBLE * 2.0));

	const glm::mat4 createTransformMat(const glm::vec3 &p_position, const glm::vec3 &p_rotation, const glm::vec3 &p_scale) noexcept;
	const inline glm::mat4 createTransformMat(const glm::vec3 &p_position, const glm::quat &p_rotation) noexcept
	{
		glm::mat4 returnMatrix(1.0f);

		returnMatrix = glm::translate(returnMatrix, p_position);

		returnMatrix *= glm::toMat4(p_rotation);

		return returnMatrix;
	}
	const inline glm::mat4 createTransformMat(const glm::vec3 &p_position, const glm::quat &p_rotation, const glm::vec3 &p_scale) noexcept
	{
		glm::mat4 returnMatrix(1.0f);

		returnMatrix = glm::translate(returnMatrix, p_position);

		returnMatrix *= glm::toMat4(p_rotation);

		returnMatrix = glm::scale(returnMatrix, p_scale);
		//returnMatrix *= glm::orientate4(glm::vec3(p_rotation.y, p_rotation.z, p_rotation.x));
		//returnMatrix = glm::rotate(returnMatrix, rotAngle, Rotation);

		return returnMatrix;
	}

	const inline glm::mat4 perspectiveRadian(const float p_FOV, const int p_screenWidth, const int p_screenHeight) noexcept
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
	const inline glm::quat eulerDegreesToQuaterion(const glm::vec3 &p_degrees) noexcept
	{
		return glm::quat{glm::radians(p_degrees)};
		//glm::quat qPitch = glm::angleAxis(glm::radians(p_degrees.x), glm::vec3(1, 0, 0));
		//glm::quat qYaw = glm::angleAxis(glm::radians(p_degrees.y), glm::vec3(0, 1, 0));
		//glm::quat qRoll = glm::angleAxis(glm::radians(p_degrees.z), glm::vec3(0, 0, 1));

		//return glm::normalize(qPitch * qYaw * qRoll);
	}

	const inline FMOD_VECTOR toFmodVector(const glm::vec3 &p_vec) noexcept { return FMOD_VECTOR{ p_vec.x, p_vec.y, p_vec.z }; }

	const inline FMOD_VECTOR toFmodVector(const glm::vec4 &p_vec) noexcept { return FMOD_VECTOR{ p_vec.x, p_vec.y, p_vec.z }; }

	const inline FMOD_VECTOR toFmodVector(const btVector3 &p_vec) noexcept { return FMOD_VECTOR{ p_vec[0], p_vec[1], p_vec[2] }; }

	const inline btVector3 toBtVector3(const glm::vec3 &p_vec) noexcept { return btVector3(p_vec.x, p_vec.y, p_vec.z); }

	const inline btVector3 toBtVector3(const glm::vec4 &p_vec) noexcept { return btVector3(p_vec.x, p_vec.y, p_vec.z); }

	const inline glm::vec3 toGlmVec3(const btVector3 &p_vec) noexcept { return glm::vec3(p_vec.getX(), p_vec.getY(), p_vec.getZ()); }

	const inline glm::vec4 toGlmVec4(const glm::quat &p_quat) noexcept { return glm::vec4(p_quat.w, p_quat.x, p_quat.y, p_quat.z); }

	const inline glm::quat toGlmQuat(const btQuaternion &p_quat) noexcept { return glm::quat(p_quat.getW(), p_quat.getX(), p_quat.getY(), p_quat.getZ()); }

	const inline glm::quat toGlmQuat(const glm::vec4 &p_vec) noexcept { return glm::quat(p_vec.x, p_vec.y, p_vec.z, p_vec.w); }

	const inline btQuaternion toBtQuaternion(const glm::quat &p_quat) noexcept { return btQuaternion(p_quat.x, p_quat.y, p_quat.z, p_quat.w); }

	const inline btMatrix3x3 toBtMatrix3x3(const glm::mat3 &p_mat) noexcept { return btMatrix3x3(p_mat[0][0], p_mat[1][0], p_mat[2][0], p_mat[0][1], p_mat[1][1], p_mat[2][1], p_mat[0][2], p_mat[1][2], p_mat[2][2]); }

	// btTransform does not contain a full 4x4 matrix, so this transform is lossy.
	// Affine transformations are OK but perspective transformations are not.
	const inline btTransform toBtTransform(const glm::mat4 &p_mat) noexcept
	{
		glm::mat3 m3(p_mat);
		return btTransform(toBtMatrix3x3(m3), toBtVector3(glm::vec3(p_mat[3][0], p_mat[3][1], p_mat[3][2])));
	}

	const inline glm::mat4 toGlmMat4(const btTransform &p_trans) noexcept
	{
		glm::mat4 m(1.0f);
		const btMatrix3x3 &basis = p_trans.getBasis();
		// rotation
		for(int r = 0; r < 3; r++)
		{
			for(int c = 0; c < 3; c++)
			{
				m[c][r] = basis[r][c];
			}
		}
		// traslation
		btVector3 origin = p_trans.getOrigin();
		m[3][0] = origin.getX();
		m[3][1] = origin.getY();
		m[3][2] = origin.getZ();
		// unit scale
		m[0][3] = 0.0f;
		m[1][3] = 0.0f;
		m[2][3] = 0.0f;
		m[3][3] = 1.0f;
		return m;
	}
	
	const inline glm::mat4 toGlmMat4(btScalar *p_mat) noexcept
	{
		return glm::mat4(
			p_mat[0], p_mat[1], p_mat[2], p_mat[3],
			p_mat[4], p_mat[5], p_mat[6], p_mat[7],
			p_mat[8], p_mat[9], p_mat[10], p_mat[11],
			p_mat[12], p_mat[13], p_mat[14], p_mat[15]);
	}

	// Perform an approximate gamma correction on the RGB color
	const inline glm::vec3 gammaCorrectionApprox(const glm::vec3 &p_rgb, const float p_gamma) noexcept { return glm::pow(p_rgb, glm::vec3(p_gamma)); }

	// Perform an approximate gamma correction on the RGBA color
	const inline glm::vec4 gammaCorrectionApprox(const glm::vec4 &p_rgba, const float p_gamma) noexcept { return glm::pow(p_rgba, glm::vec4(p_gamma)); }

	// Perform an accurate gamma correction on the RGB color
	const inline glm::vec3 gammaCorrectionAccurate(const glm::vec3 &p_rgb, const float p_gamma = 2.4f) noexcept
	{ 
		const glm::vec3 lo = p_rgb * 12.92f;
		const glm::vec3 hi = pow(abs(p_rgb), glm::vec3(1.0f / p_gamma)) * 1.055f - 0.055f;
		return mix(hi, lo, glm::vec3(glm::lessThanEqual(p_rgb, glm::vec3(0.0031308f))));
	}	
	
	// Perform an accurate gamma correction on the RGBA color
	const inline glm::vec4 gammaCorrectionAccurate(const glm::vec4 &p_rgba, const float p_gamma = 2.4f) noexcept
	{
		const glm::vec4 lo = p_rgba * 12.92f;
		const glm::vec4 hi = pow(abs(p_rgba), glm::vec4(1.0f / p_gamma)) * 1.055f - 0.055f;
		return mix(hi, lo, glm::vec4(glm::lessThanEqual(p_rgba, glm::vec4(0.0031308f))));
	}	
	
	// Perform an accurate gamma correction on the RGBA color, while not affecting the alpha channel
	const inline glm::vec4 gammaCorrectionAccurateNoAlpha(const glm::vec4 &p_rgba, const float p_gamma = 2.4f) noexcept
	{
		return glm::vec4(gammaCorrectionAccurate(glm::vec3(p_rgba)), p_rgba.a);
	}
}