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

		Quaternion conjugateQuat = rotationQuat.getConjugated();
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
	void Mat4f::translate(const Vec3f &p_vec3)
	{
		Mat4f translateMat;
		translateMat.transform(p_vec3);
		*this = *this * translateMat;
	}
	void Mat4f::rotate(const Vec3f& p_vec3f)
	{
		/*Mat4f rotX, rotY, rotZ;

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

		*this = (*this * rotY * rotX * rotZ);*/

		const float t1 = toRadian(p_vec3f.x);
		const float t2 = toRadian(p_vec3f.y);
		const float t3 = toRadian(p_vec3f.z);

		float c1 = cos(-t1);
        float c2 = cos(-t2);
        float c3 = cos(-t3);
        float s1 = sin(-t1);
        float s2 = sin(-t2);
        float s3 = sin(-t3);

		//		 0		 1		 2		 3
		//	0	m[0]	m[4]	m[8]	m[12]
		//	1	m[1]	m[5]	m[9]	m[13]
		//	2	m[2]	m[6]	m[10]	m[14]
		//	3	m[3]	m[7]	m[11]	m[15]

        //mat<4, 4, T, defaultp> Result;
		Mat4f rotMat;
        rotMat.m[0] = c2 * c3;
        rotMat.m[4] =-c1 * s3 + s1 * s2 * c3;
        rotMat.m[8] = s1 * s3 + c1 * s2 * c3;
        rotMat.m[12] = static_cast<float>(0);
        rotMat.m[1] = c2 * s3;
        rotMat.m[5] = c1 * c3 + s1 * s2 * s3;
        rotMat.m[9] =-s1 * c3 + c1 * s2 * s3;
        rotMat.m[13] = static_cast<float>(0);
        rotMat.m[2] =-s2;
        rotMat.m[6] = s1 * c2;
        rotMat.m[10] = c1 * c2;
        rotMat.m[14] = static_cast<float>(0);
        rotMat.m[3] = static_cast<float>(0);
        rotMat.m[7] = static_cast<float>(0);
        rotMat.m[11] = static_cast<float>(0);
        rotMat.m[15] = static_cast<float>(1);

		*this = *this * rotMat;
	}
	void Mat4f::perspectiveRadian(const float p_FOV, const int p_screenWidth, const int p_screenHeight)
	{
		float aspectRatio = static_cast<float>(p_screenWidth) / static_cast<float>(p_screenHeight);
		float kFovY = 80.0f / 180.0f * (float)PI;
		float kTanFovY = tan(kFovY / 2.0f);

		 m[0] = kTanFovY * aspectRatio;	 m[4] = 0.0f;		m[8] = 0.0f;	m[12] = 0.0f;
		 m[1] = 0.0f;					 m[5] = kTanFovY;	m[9] = 0.0f;	m[13] = 0.0f;
		 m[2] = 0.0f;					 m[6] = 0.0f;		m[10] = 0.0f;	m[14] = 1.0f;
		 m[3] = 0.0f;					 m[7] = 0.0f;		m[11] = -1.0f;	m[15] = 1.0f;
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