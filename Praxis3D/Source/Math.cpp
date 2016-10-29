#include "Math.h"

namespace Math
{
	Vec2f::Vec2f(const Vec3f p_vec)
	{
		x = p_vec.x;	y = p_vec.y;
	}
	Vec2f::Vec2f(const Vec4f p_vec)
	{
		x = p_vec.x;	y = p_vec.y;
	}
	Vec3f::Vec3f(const Vec4f p_vec)
	{
		x = p_vec.x;	y = p_vec.y;	z = p_vec.z;
	}

	void Vec3f::rotate(float p_angle, const Vec3f& p_axis)
	{
		const float sinHalfAngle = sinf(toRadian(p_angle / 2));

		Quaternion rotationQuat(p_axis.x * sinHalfAngle,
								p_axis.y * sinHalfAngle,
								p_axis.z * sinHalfAngle,
								cosf(toRadian(p_angle / 2)));

		Quaternion conjugateQuat = rotationQuat.conjugate();
		//ConjugateQ.normalize();
		Quaternion ret = rotationQuat * (*this) * conjugateQuat;

		x = ret.x;
		y = ret.y;
		z = ret.z;
	}
	void Vec3f::rotate(const Vec3f &p_angleVec)
	{
		if(p_angleVec.z != 0.0f)
			rotate(p_angleVec.z, Math::Vec3f(0.0f, 0.0f, 1.0f));
		if(p_angleVec.x != 0.0f)
			rotate(p_angleVec.x, Math::Vec3f(1.0f, 0.0f, 0.0f));
		if(p_angleVec.y != 0.0f)
			rotate(p_angleVec.y, Math::Vec3f(0.0f, 1.0f, 0.0f));
	}
	void Mat4f::rotate(const Vec3f& p_vec3f)
	{
		Mat4f rotX, rotY, rotZ;

		const float x = toRadian(p_vec3f.x);
		const float y = toRadian(p_vec3f.y);
		const float z = toRadian(p_vec3f.z);

		rotX.m[0] = 1.0f;	 rotX.m[4] = 0.0f;		rotX.m[8] = 0.0f;
		rotX.m[1] = 0.0f;	 rotX.m[5] = cosf(x);	rotX.m[9] = -sinf(x);
		rotX.m[2] = 0.0f;	 rotX.m[6] = sinf(x);	rotX.m[10] = cosf(x);

		rotY.m[0] = cosf(y); rotY.m[4] = 0.0f;		rotY.m[8] = -sinf(y);
		rotY.m[1] = 0.0f;	 rotY.m[5] = 1.0f;		rotY.m[9] = 0.0f;
		rotY.m[2] = sinf(y); rotY.m[6] = 0.0f;		rotY.m[10] = cosf(y);

		rotZ.m[0] = cosf(z); rotZ.m[4] = -sinf(z);	rotZ.m[8] = 0.0f;
		rotZ.m[1] = sinf(z); rotZ.m[5] = cosf(z);	rotZ.m[9] = 0.0f;
		rotZ.m[2] = 0.0f;	 rotZ.m[6] = 0.0f;		rotZ.m[10] = 1.0f;

		*this = (*this * rotY * rotX * rotZ);
	}
	void Mat4f::perspective(const float p_FOV, const int p_screenWidth, const int p_screenHeight, const float p_zNear, const float p_zFar)
	{
		float	radFOV = toRadian(p_FOV),
				height = cosf(0.5f * radFOV) / sinf(0.5f * radFOV),
				width = height * p_screenHeight / p_screenWidth,
				zRange = p_zFar - p_zNear;
				
		m[0] = width;					 m[4] = 0.0f;			m[8] = 0.0f;								m[12] = 0.0f;
		m[1] = 0.0f;					 m[5] = height;			m[9] = 0.0f;								m[13] = 0.0f;
		m[2] = 0.0f;					 m[6] = 0.0f;			m[10] = -(p_zNear + p_zFar) / zRange;		m[14] = -(2.0f * p_zFar * p_zNear) / zRange;
		m[3] = 0.0f;					 m[7] = 0.0f;			m[11] = -1.0f;								m[15] = 0.0f;
	}
	void Mat4f::perspective(const float p_FOV, const float p_aspectRatio, const float p_zNear, const float p_zFar)
	{
		float	range = tanf(toRadian(p_FOV / 2.0f)) * p_zNear,
				left = -range * p_aspectRatio,
				right = range * p_aspectRatio,
				bottom = -range,
				top = range,
				zRange = p_zFar - p_zNear;

		m[0] = (2.0f * p_zNear) / (right - left);	 m[4] = 0.0f;									m[8] = 0.0f;								m[12] = 0.0f;
		m[1] = 0.0f;								 m[5] = (2.0f * p_zNear) / (top - bottom);		m[9] = 0.0f;								m[13] = 0.0f;
		m[2] = 0.0f;								 m[6] = 0.0f;									m[10] = -(p_zFar + p_zNear) / zRange;		m[14] = -(2.0f * p_zFar * p_zNear) / zRange;
		m[3] = 0.0f;								 m[7] = 0.0f;									m[11] = -1.0f;								m[15] = 0.0f;
	}
}