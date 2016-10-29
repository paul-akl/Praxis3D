#pragma once

//#define _USE_MATH_DEFINES

#include <cmath>
//#include <iostream>

#define E			2.71828182845904523536
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

namespace Math
{
	struct Vec3f;
	struct Vec4f;

	struct Vec2i
	{
		int x, y;

		Vec2i()
		{
			x = 0;	y = 0;
		}
		Vec2i(const int p_value)
		{
			x = p_value;	y = p_value;
		}
		Vec2i(const int p_valueX, const int p_valueY)
		{
			x = p_valueX;	y = p_valueY;
		}
	};
	struct Vec2f
	{
		float x, y;

		Vec2f()
		{
			x = 0.0f;	y = 0.0f;
		}
		Vec2f(const float p_float)
		{
			x = p_float;	y = p_float;
		}
		Vec2f(const float p_floatX, const float p_floatY)
		{
			x = p_floatX;	y = p_floatY;
		}
		Vec2f(const Vec3f p_vec);
		Vec2f(const Vec4f p_vec);
	};
	struct Vec3f
	{
		float x, y, z;

		Vec3f()
		{
			x = 0.0f;	y = 0.0f;	z = 0.0f;
		}
		Vec3f(const float p_float)
		{
			x = p_float;	y = p_float;	z = p_float;
		}
		Vec3f(const float p_floatX, const float p_floatY, const float p_floatZ)
		{
			x = p_floatX;	y = p_floatY;	 z = p_floatZ;
		}
		Vec3f(const Vec2f p_vec, const float z_arg)
		{
			x = p_vec.x;	y = p_vec.y;	z = z_arg;
		}
		Vec3f(const Vec4f p_vec);

		const inline Vec3f &operator=(const Vec3f& p_vec)
		{
			x = p_vec.x;
			y = p_vec.y;
			z = p_vec.z;

			return *this;
		}
		const inline Vec3f &operator+=(const Vec3f& p_vec)
		{
			x += p_vec.x;
			y += p_vec.y;
			z += p_vec.z;

			return *this;
		}
		const inline Vec3f &operator-=(const Vec3f& p_vec)
		{
			x -= p_vec.x;
			y -= p_vec.y;
			z -= p_vec.z;

			return *this;
		}
		const inline Vec3f &operator*=(const Vec3f& p_vec)
		{
			x *= p_vec.x;
			y *= p_vec.y;
			z *= p_vec.z;

			return *this;
		}
		const inline Vec3f &operator*=(const float p_float)
		{
			x *= p_float;
			y *= p_float;
			z *= p_float;

			return *this;
		}
		const inline Vec3f &operator/=(const Vec3f& p_vec)
		{
			x /= p_vec.x;
			y /= p_vec.y;
			z /= p_vec.z;

			return *this;
		}
		const inline Vec3f &operator/=(const float p_float)
		{
			x /= p_float;
			y /= p_float;
			z /= p_float;

			return *this;
		}
		inline bool operator==(Vec3f& p_vec) { return (x == p_vec.x && y == p_vec.y && z == p_vec.z); }
		inline bool operator!=(Vec3f& p_vec) { return (x != p_vec.x || y != p_vec.y || z != p_vec.z); }

		const inline float getMax() const { return (x > y) ? (x > z) ? x : z : (y > z) ? y : z; }
		const inline float squareLength() const { return (x * x + y * y + z * z); }
		const inline float length() const { return sqrtf(squareLength()); }
		const inline float dot(const Vec3f& p_vec) const { return (x * p_vec.x + y * p_vec.y + z * p_vec.z); }
		const inline Vec3f &cross(const Vec3f& p_vec)
		{
			*this = Vec3f(y * p_vec.z - z * p_vec.y, z * p_vec.x - x * p_vec.z, x * p_vec.y - y * p_vec.x);
			return *this;
		}
		const inline Vec3f &target(float verticalAngle_arg, float horizontalAngle_arg)
		{
			*this = Vec3f(cosf(verticalAngle_arg) * sinf(horizontalAngle_arg),
						  sinf(verticalAngle_arg),
						  cosf(verticalAngle_arg) * cosf(horizontalAngle_arg));
			return *this;
		}
		const inline Vec3f &horizontal(float horizontalAngle_arg)
		{
			*this = Vec3f(sinf(horizontalAngle_arg - (float) PI / 2.0f),
						  0.0f,
						  cosf(horizontalAngle_arg - (float) PI / 2.0f));
			return *this;
		}

		inline void normalize()
		{
			const float length = this->length();
			x /= length;	y /= length;	z /= length;
		}
		void rotate(float p_angle, const Vec3f &p_axis);
		void rotate(const Vec3f &p_angleVec);
	};
	struct Vec4f
	{
		float x, y, z, w;

		Vec4f()
		{
			x = 0.0f;	y = 0.0f;	z = 0.0f;	w = 0.0f;
		}
		Vec4f(const float p_float)
		{
			x = p_float;	y = p_float;	z = p_float;	w = p_float;
		}
		Vec4f(const float p_floatX, const float p_floatY, const float p_floatZ, const float p_floatW)
		{
			x = p_floatX;	y = p_floatY;	z = p_floatZ;	w = p_floatW;
		}
		Vec4f(const Vec3f vector_arg, const float p_floatW)
		{
			x = vector_arg.x;	y = vector_arg.y;	z = vector_arg.z;	w = p_floatW;
		}
		Vec4f(const Vec2f vector_arg, const float p_floatZ, const float p_floatW)
		{
			x = vector_arg.x;	y = vector_arg.y;	z = p_floatZ;	w = p_floatW;
		}
	};

	const inline Vec3f operator+(const Vec3f& p_left, const Vec3f& p_right) { return Vec3f(p_left.x + p_right.x, p_left.y + p_right.y, p_left.z + p_right.z); }
	const inline Vec3f operator-(const Vec3f& p_left, const Vec3f& p_right) { return Vec3f(p_left.x - p_right.x, p_left.y - p_right.y, p_left.z - p_right.z); }
	const inline Vec3f operator*(const float p_float, const Vec3f& p_vec) { return Vec3f(p_vec.x * p_float, p_vec.y * p_float, p_vec.z * p_float); }
	const inline Vec3f operator*(const Vec3f& p_vec, const float p_float) { return Vec3f(p_vec.x * p_float, p_vec.y * p_float, p_vec.z * p_float); }
	const inline float operator*(const Vec3f& p_left, const Vec3f& p_right) { return (p_left.x * p_right.x, p_left.y * p_right.y, p_left.z * p_right.z); }
	const inline Vec3f operator/(const float p_float, const Vec3f& p_vec) { return (p_vec * (1 / p_float)); }
	const inline Vec3f operator/(const Vec3f& p_vec, const float p_float) { return p_vec * (1 / p_float); }
	const inline Vec3f operator/(const Vec3f& p_left, const Vec3f& p_right) { return Vec3f(p_left.x / p_right.x, p_left.y / p_right.y, p_left.z / p_right.z); }
	const inline Vec3f operator-(const Vec3f& p_vec) { return Vec3f(-p_vec.x, -p_vec.y, -p_vec.z); }
	const inline float dot(const Vec3f& p_left, const Vec3f& p_right) { return (p_left.x * p_right.x + p_left.y * p_right.y + p_left.z * p_right.z); }
	const inline Vec3f operator^(const Vec3f& p_left, const Vec3f& p_right)
	{
		return Vec3f(p_left.y * p_right.z - p_left.z * p_right.y,
					 p_left.z * p_right.x - p_left.x * p_right.z,
					 p_left.x * p_right.y - p_left.y * p_right.x);
	}
	const inline Vec3f normalize(const Vec3f& p_vec)
	{
		const float length = p_vec.length();
		return Vec3f(p_vec.x / length, p_vec.y / length, p_vec.z / length);
	}
	const inline Vec3f cross(const Vec3f& p_left, const Vec3f& p_right)
	{
		return Vec3f(p_left.y * p_right.z - p_left.z * p_right.y,
					 p_left.z * p_right.x - p_left.x * p_right.z,
					 p_left.x * p_right.y - p_left.y * p_right.x);
	}

	struct Quaternion
	{
		float x, y, z, w;

		Quaternion()
		{
			x = 0.0f;	y = 0.0f;	z = 0.0f;	w = 0.0f;
		}
		Quaternion(const float p_floatX, const float p_floatY, const float p_floatZ, const float p_floatW)
		{
			x = p_floatX;	y = p_floatY;	z = p_floatZ;	w = p_floatW;
		}
		Quaternion(const Vec4f p_vec)
		{
			x = p_vec.x;	y = p_vec.y;	z = p_vec.z;	w = p_vec.w;
		}

		inline void normalize()
		{
			float length = sqrtf(x * x + y * y + z * z + w * w);

			x /= length;	y /= length;	z /= length;	w /= length;
		}
		inline void conj()
		{
			x = -x;	y = -y;	z = -z;
		}
		inline Quaternion conjugate() { return Quaternion(-x, -y, -z, w); }

		const inline Quaternion operator*(const Quaternion& p_quat)
		{
			return Quaternion((x * p_quat.w) + (w * p_quat.x) + (y * p_quat.z) - (z * p_quat.y),
							  (y * p_quat.w) + (w * p_quat.y) + (z * p_quat.x) - (x * p_quat.z),
							  (z * p_quat.w) + (w * p_quat.z) + (x * p_quat.y) - (y * p_quat.x),
							  (w * p_quat.w) - (x * p_quat.x) - (y * p_quat.y) - (z * p_quat.z));
		}
		const inline Quaternion operator*(const Vec3f& p_vec)
		{
			return Quaternion((w * p_vec.x) + (y * p_vec.z) - (z * p_vec.y),
							  (w * p_vec.y) + (z * p_vec.x) - (x * p_vec.z),
							  (w * p_vec.z) + (x * p_vec.y) - (y * p_vec.x),
							  -(x * p_vec.x) - (y * p_vec.y) - (z * p_vec.z));
		}
	};

	const inline Quaternion operator*(const Quaternion& p_left, const Quaternion& p_right)
	{
		return Quaternion((p_left.x * p_right.w) + (p_left.w * p_right.x) + (p_left.y * p_right.z) - (p_left.z * p_right.y),
						  (p_left.y * p_right.w) + (p_left.w * p_right.y) + (p_left.z * p_right.x) - (p_left.x * p_right.z),
						  (p_left.z * p_right.w) + (p_left.w * p_right.z) + (p_left.x * p_right.y) - (p_left.y * p_right.x),
						  (p_left.w * p_right.w) - (p_left.x * p_right.x) - (p_left.y * p_right.y) - (p_left.z * p_right.z));
	}
	const inline Quaternion operator*(const Quaternion& p_quat, const Vec3f& p_vec)
	{
		return Quaternion((p_quat.w * p_vec.x) + (p_quat.y * p_vec.z) - (p_quat.z * p_vec.y),
						  (p_quat.w * p_vec.y) + (p_quat.z * p_vec.x) - (p_quat.x * p_vec.z),
						  (p_quat.w * p_vec.z) + (p_quat.x * p_vec.y) - (p_quat.y * p_vec.x),
						  -(p_quat.x * p_vec.x) - (p_quat.y * p_vec.y) - (p_quat.z * p_vec.z));
	}

	class Mat4f
	{
	public:
		float m[16];

		Mat4f()
		{
			m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f;	m[12] = 0.0f;
			m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f;	m[13] = 0.0f;
			m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f;	m[14] = 0.0f;
			m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f;	m[15] = 1.0f;
		}
		Mat4f(float p_float)
		{
			m[0] = p_float; m[4] = p_float; m[8] = p_float; m[12] = p_float;
			m[1] = p_float; m[5] = p_float; m[9] = p_float; m[13] = p_float;
			m[2] = p_float; m[6] = p_float; m[10] = p_float; m[14] = p_float;
			m[3] = p_float; m[7] = p_float; m[11] = p_float; m[15] = p_float;
		}
		Mat4f(float p_float[16])
		{
			m[0] = p_float[0]; m[4] = p_float[4]; m[8] = p_float[8];	 m[12] = p_float[12];
			m[1] = p_float[1]; m[5] = p_float[5]; m[9] = p_float[9];	 m[13] = p_float[13];
			m[2] = p_float[2]; m[6] = p_float[6]; m[10] = p_float[10]; m[14] = p_float[14];
			m[3] = p_float[3]; m[7] = p_float[7]; m[11] = p_float[11]; m[15] = p_float[15];
		}
		Mat4f(float p_m01, float p_m02, float p_m03, float p_m04,
			  float p_m05, float p_m06, float p_m07, float p_m08,
			  float p_m09, float p_m10, float p_m11, float p_m12,
			  float p_m13, float p_m14, float p_m15, float p_m16)
		{
			m[0] = p_m01; m[4] = p_m05; m[8] = p_m09; m[12] = p_m13;
			m[1] = p_m02; m[5] = p_m06; m[9] = p_m10; m[13] = p_m14;
			m[2] = p_m03; m[6] = p_m07; m[10] = p_m11; m[14] = p_m15;
			m[3] = p_m04; m[7] = p_m08; m[11] = p_m12; m[15] = p_m16;
		}
		Mat4f(Vec3f& p_rotX, Vec3f& p_rotY, Vec3f& p_rotZ, Vec3f& p_position)
		{
			m[0] = p_rotX.x; m[4] = p_rotY.y; m[8] = p_rotZ.z; m[12] = p_position.x;
			m[1] = p_rotX.x; m[5] = p_rotY.y; m[9] = p_rotZ.z; m[13] = p_position.y;
			m[2] = p_rotX.x; m[6] = p_rotY.y; m[10] = p_rotZ.z; m[14] = p_position.z;
		}

		inline Vec3f getRotationX() const { return Vec3f(m[0], m[4], m[8]); }
		inline Vec3f getRotationY() const { return Vec3f(m[1], m[5], m[9]); }
		inline Vec3f getRotationZ() const { return Vec3f(m[2], m[6], m[10]); }
		inline Vec3f getPosition()  const { return Vec3f(m[3], m[7], m[11]); }

		inline void setRotationX(Vec3f &p_rotX) { m[0] = p_rotX.x; m[4] = p_rotX.y; m[8] = p_rotX.z; }
		inline void setRotationY(Vec3f &p_rotY) { m[1] = p_rotY.x; m[5] = p_rotY.y; m[9] = p_rotY.z; }
		inline void setRotationZ(Vec3f &p_rotZ) { m[2] = p_rotZ.x; m[6] = p_rotZ.y; m[10] = p_rotZ.z; }
		inline void setPosition(Vec3f &p_pos) { m[3] = p_pos.x;  m[7] = p_pos.y;  m[11] = p_pos.z; }

		inline void setRotationX(const Vec4f &p_rotX) { m[0] = p_rotX.x; m[4] = p_rotX.y; m[8] = p_rotX.z; m[12] = p_rotX.w; }
		inline void setRotationY(const Vec4f &p_rotY) { m[1] = p_rotY.x; m[5] = p_rotY.y; m[9] = p_rotY.z; m[13] = p_rotY.w; }
		inline void setRotationZ(const Vec4f &p_rotZ) { m[2] = p_rotZ.x; m[6] = p_rotZ.y; m[10] = p_rotZ.z; m[14] = p_rotZ.w; }
		inline void setPosition(const Vec4f &p_pos) { m[3] = p_pos.x;  m[7] = p_pos.y;  m[11] = p_pos.z;  m[15] = p_pos.w; }

		inline void Mat4f::identity()
		{
			m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f;	m[12] = 0.0f;
			m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f;	m[13] = 0.0f;
			m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f;	m[14] = 0.0f;
			m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f;	m[15] = 1.0f;
		}
		inline void Mat4f::transform(const Vec3f& p_vec3)
		{
			m[12] = p_vec3.x;
			m[13] = p_vec3.y;
			m[14] = p_vec3.z;
		}
		inline void Mat4f::translate(const Vec3f& p_vec3)
		{
			Mat4f translateMat;
			translateMat.transform(p_vec3);
			*this *= translateMat;
		}
		inline void Mat4f::scale(const float p_scale)
		{
			m[0] *= p_scale; m[4] *= p_scale; m[8] *= p_scale;
			m[1] *= p_scale; m[5] *= p_scale; m[9] *= p_scale;
			m[2] *= p_scale; m[6] *= p_scale; m[10] *= p_scale;
			m[3] *= p_scale; m[7] *= p_scale;
		}
		inline void Mat4f::scale(const Vec3f& p_scale)
		{
			m[0] *= p_scale.x; m[4] *= p_scale.y; m[8] *= p_scale.z;
			m[1] *= p_scale.x; m[5] *= p_scale.y; m[9] *= p_scale.z;
			m[2] *= p_scale.x; m[6] *= p_scale.y; m[10] *= p_scale.z;
			m[3] *= p_scale.x; m[7] *= p_scale.y;
		}

		void rotate(const Vec3f& p_vec3f);
		void perspective(const float p_FOV, const int p_screenWidth, const int p_screenHeight, const float p_zNear, const float p_zFar);
		void perspective(const float p_FOV, const float p_aspectRatio, const float p_zNear, const float p_zFar);
		inline void initCamera(const Vec3f& p_position, const Vec3f& p_target, const Vec3f& p_up)
		{
			Vec3f	N = normalize(p_target - p_position),
					U = normalize(p_up),
					V = normalize(cross(N, U));

			U = cross(V, N);

			m[0] = V.x;		m[4] = V.y;		m[8] = V.z;		m[12] = -dot(V, p_position);
			m[1] = U.x;		m[5] = U.y;		m[9] = U.z;		m[13] = -dot(U, p_position);
			m[2] = -N.x;	m[6] = -N.y;	m[10] = -N.z;	m[14] = dot(N, p_position);
		}
		inline void ortho(const float p_left, const float p_right, const float p_down, const float p_up, const float p_zNear, const float p_zFar)
		{
			m[0] = 2 / (p_right - p_left);						m[4] = 0.0f;								m[8] = 0.0f;										m[12] = 0.0f;
			m[1] = 0.0f;										m[5] = 2 / (p_up - p_down);					m[9] = 0.0f;										m[13] = 0.0f;
			m[2] = 0.0f;										m[6] = 0.0f;								m[10] = -2 / (p_zFar - p_zNear);					m[14] = 0.0f;
			m[3] = -(p_right + p_left) / (p_right - p_left);	m[7] = -(p_up + p_down) / (p_up - p_down);	m[11] = -(p_zFar + p_zNear) / (p_zFar - p_zNear);	m[15] = 1.0f;
		}

		inline Mat4f operator*=(const Mat4f& p_mat4f)
		{
			m[0] = m[0] * p_mat4f.m[0] + m[4] * p_mat4f.m[1] + m[8] * p_mat4f.m[2] + m[12] * p_mat4f.m[3];
			m[1] = m[1] * p_mat4f.m[0] + m[5] * p_mat4f.m[1] + m[9] * p_mat4f.m[2] + m[13] * p_mat4f.m[3];
			m[2] = m[2] * p_mat4f.m[0] + m[6] * p_mat4f.m[1] + m[10] * p_mat4f.m[2] + m[14] * p_mat4f.m[3];
			m[3] = m[3] * p_mat4f.m[0] + m[7] * p_mat4f.m[1] + m[11] * p_mat4f.m[2] + m[15] * p_mat4f.m[3];
			m[4] = m[0] * p_mat4f.m[4] + m[4] * p_mat4f.m[5] + m[8] * p_mat4f.m[6] + m[12] * p_mat4f.m[7];
			m[5] = m[1] * p_mat4f.m[4] + m[5] * p_mat4f.m[5] + m[9] * p_mat4f.m[6] + m[13] * p_mat4f.m[7];
			m[6] = m[2] * p_mat4f.m[4] + m[6] * p_mat4f.m[5] + m[10] * p_mat4f.m[6] + m[14] * p_mat4f.m[7];
			m[7] = m[3] * p_mat4f.m[4] + m[7] * p_mat4f.m[5] + m[11] * p_mat4f.m[6] + m[15] * p_mat4f.m[7];
			m[8] = m[0] * p_mat4f.m[8] + m[4] * p_mat4f.m[9] + m[8] * p_mat4f.m[10] + m[12] * p_mat4f.m[11];
			m[9] = m[1] * p_mat4f.m[8] + m[5] * p_mat4f.m[9] + m[9] * p_mat4f.m[10] + m[13] * p_mat4f.m[11];
			m[10] = m[2] * p_mat4f.m[8] + m[6] * p_mat4f.m[9] + m[10] * p_mat4f.m[10] + m[14] * p_mat4f.m[11];
			m[11] = m[3] * p_mat4f.m[8] + m[7] * p_mat4f.m[9] + m[11] * p_mat4f.m[10] + m[15] * p_mat4f.m[11];
			m[12] = m[0] * p_mat4f.m[12] + m[4] * p_mat4f.m[13] + m[8] * p_mat4f.m[14] + m[12] * p_mat4f.m[15];
			m[13] = m[1] * p_mat4f.m[12] + m[5] * p_mat4f.m[13] + m[9] * p_mat4f.m[14] + m[13] * p_mat4f.m[15];
			m[14] = m[2] * p_mat4f.m[12] + m[6] * p_mat4f.m[13] + m[10] * p_mat4f.m[14] + m[14] * p_mat4f.m[15];
			m[15] = m[3] * p_mat4f.m[12] + m[7] * p_mat4f.m[13] + m[11] * p_mat4f.m[14] + m[15] * p_mat4f.m[15];
			return *this;
		}
	};

	const inline Mat4f operator*(const Mat4f& p_left, const Mat4f p_right)
	{
		// Multiplication can be done more stylish with a nested loop, but this should be slightly faster
		return Mat4f(p_left.m[0] * p_right.m[0] + p_left.m[4] * p_right.m[1] + p_left.m[8] * p_right.m[2] + p_left.m[12] * p_right.m[3],
					 p_left.m[1] * p_right.m[0] + p_left.m[5] * p_right.m[1] + p_left.m[9] * p_right.m[2] + p_left.m[13] * p_right.m[3],
					 p_left.m[2] * p_right.m[0] + p_left.m[6] * p_right.m[1] + p_left.m[10] * p_right.m[2] + p_left.m[14] * p_right.m[3],
					 p_left.m[3] * p_right.m[0] + p_left.m[7] * p_right.m[1] + p_left.m[11] * p_right.m[2] + p_left.m[15] * p_right.m[3],
					 p_left.m[0] * p_right.m[4] + p_left.m[4] * p_right.m[5] + p_left.m[8] * p_right.m[6] + p_left.m[12] * p_right.m[7],
					 p_left.m[1] * p_right.m[4] + p_left.m[5] * p_right.m[5] + p_left.m[9] * p_right.m[6] + p_left.m[13] * p_right.m[7],
					 p_left.m[2] * p_right.m[4] + p_left.m[6] * p_right.m[5] + p_left.m[10] * p_right.m[6] + p_left.m[14] * p_right.m[7],
					 p_left.m[3] * p_right.m[4] + p_left.m[7] * p_right.m[5] + p_left.m[11] * p_right.m[6] + p_left.m[15] * p_right.m[7],
					 p_left.m[0] * p_right.m[8] + p_left.m[4] * p_right.m[9] + p_left.m[8] * p_right.m[10] + p_left.m[12] * p_right.m[11],
					 p_left.m[1] * p_right.m[8] + p_left.m[5] * p_right.m[9] + p_left.m[9] * p_right.m[10] + p_left.m[13] * p_right.m[11],
					 p_left.m[2] * p_right.m[8] + p_left.m[6] * p_right.m[9] + p_left.m[10] * p_right.m[10] + p_left.m[14] * p_right.m[11],
					 p_left.m[3] * p_right.m[8] + p_left.m[7] * p_right.m[9] + p_left.m[11] * p_right.m[10] + p_left.m[15] * p_right.m[11],
					 p_left.m[0] * p_right.m[12] + p_left.m[4] * p_right.m[13] + p_left.m[8] * p_right.m[14] + p_left.m[12] * p_right.m[15],
					 p_left.m[1] * p_right.m[12] + p_left.m[5] * p_right.m[13] + p_left.m[9] * p_right.m[14] + p_left.m[13] * p_right.m[15],
					 p_left.m[2] * p_right.m[12] + p_left.m[6] * p_right.m[13] + p_left.m[10] * p_right.m[14] + p_left.m[14] * p_right.m[15],
					 p_left.m[3] * p_right.m[12] + p_left.m[7] * p_right.m[13] + p_left.m[11] * p_right.m[14] + p_left.m[15] * p_right.m[15]);
	}

	inline Vec3f toRadian(const Vec3f p_vec3)
	{
		return Vec3f((p_vec3.x * (float)PI / 180.0f),
					 (p_vec3.y * (float)PI / 180.0f), 
					 (p_vec3.z * (float)PI / 180.0f));
	}
	inline Vec3f toDegree(const Vec3f p_vec3) 
	{ 
		return (p_vec3.x * 180.0f / (float)PI,
				p_vec3.y * 180.0f / (float)PI,
				p_vec3.z * 180.0f / (float)PI);
	}
	inline float toRadian(const float p_float) { return (p_float * (float) PI / 180.0f); }
	inline float toDegree(const float p_float) { return (p_float * 180.0f / (float) PI); }
	inline float getMax(const float p_left, const float p_right) { return p_left > p_right ? p_left : p_right; }
	//inline void clampMinMax(float &p_value, float p_clampMin, float p_clampMax) { if(p_value > p_clampMax) p_value = p_clampMax; else if(p_value < p_clampMin) p_value = p_clampMin; }

	template <typename T>
	inline T clip(T p_in, T p_low, T p_high) { return std::min(std::max(p_in, p_low), p_high); }
}