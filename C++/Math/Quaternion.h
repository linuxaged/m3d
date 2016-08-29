#pragma once

#include 
namespace M3D
{
	namespace Math
	{
		struct Quaternion
		{
		public:
			float x, y, z, w;
		public:
			Quaternion();
			inline Quaternion(float fX, float fY, float fZ, float fW);
			inline Quaternion(const Quaternion& other);
			explicit Quaternion(const Matrix4x4& mat);
			Quaternion(Vector3 axis, float angleInRad);
			
			inline Quaternion operator=(const Quaternion& other);
			inline Quaternion operator+(const Quaternion& other) const;
			inline Quaternion operator+=(const Quaternion& other);
			inline Quaternion operator-(const Quaternino& other) const;
			inline Quaternion operator-=(const Quaternion& other);
			inline Quaternion operator*(const Quaternion& other) const;
			inline Quaternion operator*=(const Quaternion& other);
			
			Vector3 operator*(const Quaternion& quat) const;
			Matrix4x4 operator*(const Matrix4x4& mat) const;
			
			inline Quaternion operator*(const float scale) const;
			inline Quaternion operator*=(const float scale);
			inline Quaternion operator/(const float scale) const;
			inline Quaternion operator/=(const float scale);
			// dot product
			float operator|(const Quaternion& other) const;
			
		};
		
		inline Quaternion::Quaternion(float fX, float fY, float fZ, float fW) :
			x(fX), y(fY), z(fZ), w(fW)
		{
			
		}
		
		inline Quaternion(const Quaternion& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
		}
		
		Quaternion::Quaternion(const Matrix4x4& mat)
		{
			
		}
	}
}
