#pragma once

// SIMD
#if defined __arm__
#include "SIMD_NEON.h"
#else
#include "SIMD_SSE.h"
#endif

namespace M3D
{
	namespace Math
	{
		struct Vector3;
		struct Matrix4x4;
		
		struct alignas(16) Quaternion
		{
		public:
			float x, y, z, w;
		public:
			Quaternion() {};
			inline Quaternion(float fX, float fY, float fZ, float fW);
			inline Quaternion(const Quaternion& other);
			explicit Quaternion(const Matrix4x4& mat);
			Quaternion(Vector3 axis, float angleInRad);
			
			inline Quaternion Inverse() const;
			
			inline Quaternion operator=(const Quaternion& other);
			inline Quaternion operator+(const Quaternion& other) const;
			inline Quaternion operator+=(const Quaternion& other);
			inline Quaternion operator-(const Quaternion& other) const;
			inline Quaternion operator-=(const Quaternion& other);
			inline Quaternion operator*(const Quaternion& other) const;
			inline Quaternion operator*=(const Quaternion& other);
			float operator|(const Quaternion& other) const; // dot product
			
			Vector3 operator*(const Vector3& v) const;
			Matrix4x4 operator*(const Matrix4x4& mat) const;
			
			inline Quaternion operator*(const float scale) const;
			inline Quaternion operator*=(const float scale);
			inline Quaternion operator/(const float scale) const;
			inline Quaternion operator/=(const float scale);
			
			
		};
		
		inline Quaternion::Quaternion(float fX, float fY, float fZ, float fW) :
			x(fX), y(fY), z(fZ), w(fW)
		{
			
		}
		
		inline Quaternion::Quaternion(const Quaternion& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
		}
		
		inline Quaternion Quaternion::Inverse() const
		{
			return Quaternion(-x, -y, -z, w);
		}
		
		/* Operators */
		inline Quaternion Quaternion::operator=(const Quaternion& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
			return *this;
		}
		
		inline Quaternion Quaternion::operator+(const Quaternion& other) const
		{
			return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
		}
		
		inline Quaternion Quaternion::operator+=(const Quaternion& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}
		
		inline Quaternion Quaternion::operator-(const Quaternion& other) const
		{
			return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
		}
		
		inline Quaternion Quaternion::operator-=(const Quaternion& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}
		
		inline Quaternion Quaternion::operator*(const Quaternion& other) const
		{
			Quaternion result;
			QuaternionMultiply(&result, this, &other);
			return result;
		}
		
		inline Quaternion Quaternion::operator*=(const Quaternion& other)
		{
			VectorSIMD A = VectorLoad4f(this);
			VectorSIMD B = VectorLoad4f(&other);
			VectorSIMD Result;
			QuaternionMultiply(&Result, &A, &B);
			VectorStore4f(Result, this);

			return *this; 
		}
		
		float Quaternion::operator|(const Quaternion& other) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}
		
		
		/* Scale */
		inline Quaternion Quaternion::operator*(const float scale) const
		{
			return Quaternion(x * scale, y * scale, z * scale, w * scale);
		}
		
		inline Quaternion Quaternion::operator*=(const float scale)
		{
			x *= scale;
			y *= scale;
			z *= scale;
			w *= scale;
			return *this;
		}
		
		inline Quaternion Quaternion::operator/(const float scale) const
		{
			return Quaternion(x / scale, y / scale, z / scale, w / scale);
		}
		
		inline Quaternion Quaternion::operator /= (const float scale)
		{
			x /= scale;
			y /= scale;
			z /= scale;
			w /= scale;
			return *this;
		}
	}
}
