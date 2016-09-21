#ifndef M3D_MATH_MATRIX_H
#define M3D_MATH_MATRIX_H

#include <cstring>
#include <cstdio>

#include "MathBase.h"
// SIMD
#if defined __arm__
#include "SIMD_NEON.h"
#else
#include "SIMD_SSE.h"
#endif

#define USE_SIMD 1

namespace M3D {
	namespace Math {
		
		/*
		Vector2
		*/
		struct Vector2
		{
		public:
			float x;
			float y;
			
			inline Vector2() {}
			;
			
			inline Vector2(float fX, float fY)
				: x(fX)
				, y(fY)
			{
			}
			
			/// Arithmetic operators
			inline Vector2 operator+(const Vector2& other) const
			{
				return Vector2 { x + other.x, y + other.y };
			}
			
			inline Vector2 operator-(const Vector2& other) const
			{
				return Vector2 { x - other.x, y - other.y };
			}

			inline Vector2 operator*(const Vector2& other) const
			{
				return Vector2 { x * other.x, y * other.y };
			}
			
			inline Vector2 operator/(const Vector2& other) const
			{
				return Vector2 { x / other.x, y / other.y };
			}

			inline Vector2 operator+(const float bias) const
			{
				return Vector2 { x + bias, y + bias };
			}
			
			inline Vector2 operator-(const float bias) const
			{
				return Vector2 { x - bias, y - bias };
			}

			inline Vector2 operator*(const float scale) const
			{
				return Vector2 { x * scale, y * scale };
			}
			
			inline Vector2 operator/(const float scale) const
			{
				return Vector2 { x / scale, y / scale };
			}

			/// Compound assignment operators
			inline Vector2& operator+=(const Vector2& other)
			{
				this->x += other.x;
				this->y += other.y;
				return *this;
			}
			
			inline Vector2& operator-=(const Vector2& other)
			{
				this->x -= other.x;
				this->y -= other.y;
				return *this;
			}
			
			inline Vector2& operator+=(const float bias)
			{
				this->x += bias;
				this->y +=bias;
				return *this;
			}
			
			inline Vector2& operator-=(const float bias)
			{
				this->x -= bias;
				this->y -= bias;
				return *this;
			}
			
			inline Vector2& operator*=(const float scale)
			{
				this->x *= scale;
				this->y *= scale;
				return *this;
			}
			
			inline Vector2& operator/=(const float scale)
			{
				this->x /= scale;
				this->y /= scale;
				return *this;
			}
			
			/// dot multiply with other
			inline float operator|(const Vector2& other) const
			{
				return x * other.x + y * other.y;
			}
			
			inline static float DotProduct(const Vector2& left, const Vector2& right)
			{
				return left | right;
			}
		};
		
		//-------------------------------------------------------------
		// Vector3
		//-------------------------------------------------------------
		struct Vector3
		{
		public:
			float x;
			float y;
			float z;

			inline Vector3() {}
			;
			inline Vector3(float InX, float InY, float InZ);
			inline Vector3(Vector2& v, float fZ);
			
			inline Vector3 operator+(const Vector3& other) const;
			inline Vector3 operator-(const Vector3& other) const;
			inline Vector3 operator*(const Vector3& other) const;
			inline Vector3 operator/(const Vector3& other) const;

			inline Vector3 operator+(const float bias) const;
			inline Vector3 operator-(const float bias) const;
			inline Vector3 operator*(const float Scale) const;
			inline Vector3 operator/(const float Scale) const;
			inline float operator|(const Vector3& other) const;
			inline Vector3 operator^(const Vector3& other) const;

			inline Vector3& operator+=(const Vector3& other);
			inline Vector3& operator-=(const Vector3& other);
			inline Vector3& operator*=(const Vector3& other);
			inline Vector3& operator/=(const Vector3& other);

			inline Vector3& operator+=(const float bias);
			inline Vector3& operator-=(const float bias);
			inline Vector3& operator*=(const float Scale);
			inline Vector3& operator/=(const float Scale);

			
			inline static Vector3 CrossProduct(const Vector3& Left, const Vector3& Right);
			inline static float DotProduct(const Vector3& Left, const Vector3& Right);

			inline void Normalize();

			void print();
			void ToString(char* const str, size_t size);
		};
		
		inline Vector3::Vector3(float InX, float InY, float InZ)
			: x(InX)
			, y(InY)
			, z(InZ)
		{

		}

		inline Vector3::Vector3(Vector2& v, float fZ)
			: x(v.x)
			, y(v.y)
			, z(fZ)
		{
			
		}
		
		inline Vector3 Vector3::operator+(const Vector3& other) const
		{
			return Vector3(x + other.x, y + other.y, z + other.z);
		}

		inline Vector3 Vector3::operator-(const Vector3& other) const
		{
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		inline Vector3 Vector3::operator*(const Vector3& other) const
		{
			return Vector3(x * other.x, y * other.y, z * other.z);
		}

		inline Vector3 Vector3::operator/(const Vector3& other) const
		{
			return Vector3(x / other.x, y / other.y, z / other.z);
		}

		inline Vector3 Vector3::operator+(const float bias) const
		{
			return Vector3(x + bias, y + bias, z + bias);
		}

		inline Vector3 Vector3::operator-(const float bias) const
		{
			return Vector3(x - bias, y - bias, z - bias);
		}

		inline Vector3 Vector3::operator*(const float Scale) const
		{
			return Vector3(x * Scale, y * Scale, z * Scale);
		}

		inline Vector3 Vector3::operator/(const float Scale) const
		{
			return Vector3(x / Scale, y / Scale, z / Scale);
		}
		
		inline float Vector3::operator|(const Vector3& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		inline Vector3 Vector3::operator^(const Vector3& other) const
		{
			return Vector3(   y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x);
		}

		// Compound assignment operators
		inline Vector3& Vector3::operator+=(const Vector3& other)
		{
			this->x += other.x;
			this->y += other.y;
			this->z += other.z;
			return *this;
		}
		inline Vector3& Vector3::operator-=(const Vector3& other)
		{
			this->x -= other.x;
			this->y -= other.y;
			this->z -= other.z;
			return *this;
		}

		inline Vector3& Vector3::operator*=(const Vector3& other)
		{
			this->x *= other.x;
			this->y *= other.y;
			this->z *= other.z;
			return *this;
		}

		inline Vector3& Vector3::operator/=(const Vector3& other)
		{
			this->x /= other.x;
			this->y /= other.y;
			this->z /= other.z;
			return *this;
		}

		inline Vector3& Vector3::operator+=(const float bias)
		{
			this->x += bias;
			this->y += bias;
			this->z += bias;
			return *this;
		}
		inline Vector3& Vector3::operator-=(const float bias)
		{
			this->x -= bias;
			this->y -= bias;
			this->z -= bias;
			return *this;
		}
		inline Vector3& Vector3::operator*=(const float Scale)
		{
			this->x *= Scale;
			this->y *= Scale;
			this->z *= Scale;
			return *this;
		}
		inline Vector3& Vector3::operator/=(const float Scale)
		{
			this->x /= Scale;
			this->y /= Scale;
			this->z /= Scale;
			return *this;
		}


		inline float Vector3::DotProduct(const Vector3& Left, const Vector3& Right)
		{
			return Left | Right;
		}

		inline Vector3 Vector3::CrossProduct(const Vector3& Left, const Vector3& Right)
		{
			return Left ^ Right;
		}

		inline void Vector3::Normalize()
		{
			float length = sqrt(x * x + y * y + z * z);
			x /= length;
			y /= length;
			z /= length;
		}
		
		//-------------------------------------------------------------
		// Matrix4x4
		//-------------------------------------------------------------
		struct Matrix4x4
		{
		public:
			alignas(16) float m[4][4];

			inline Matrix4x4();
			inline Matrix4x4(const float* array);

			inline void SetIdentity();

			inline Matrix4x4 operator+(Matrix4x4& other);
			inline Matrix4x4 operator-(Matrix4x4& other);
			inline Matrix4x4 operator*(Matrix4x4& other);
			inline void operator+=(Matrix4x4& other);
			inline void operator*=(Matrix4x4& other);

			void print();
			void ToString(char* const str, size_t size);
		};

		inline void Matrix4x4::SetIdentity()
		{
			m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
			m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
			m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
			m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
		}

		inline Matrix4x4::Matrix4x4()
		{
		  // SetIdentity();
		}

		inline Matrix4x4::Matrix4x4(const float* array)
		{
			std::memcpy(m, array, 16 * sizeof(float));
		}

		inline Matrix4x4 Matrix4x4::operator+(Matrix4x4& other)
		{
			Matrix4x4 result;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					result.m[i][j] = m[i][j] + other.m[i][j];
				}
			}
			return result;
		}

		inline Matrix4x4 Matrix4x4::operator-(Matrix4x4& other)
		{
			Matrix4x4 result;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					result.m[i][j] = m[i][j] - other.m[i][j];
				}
			}
			return result;
		}

		inline Matrix4x4 Matrix4x4::operator*(Matrix4x4& other)
		{
			Matrix4x4 result;
#if USE_SIMD
			MatrixMultiply(&result, this, &other);
#else
			
			for (int i = 0; i < 4; i++)
			{
				float accumulator;
				for (int j = 0; j < 4; j++)
				{
					for (int k = 0; k < 4; k++)
					{
						accumulator += m[i][k] * other.m[k][j];
					}

					result.m[i][j] = accumulator;
				}
			}
#endif
			return result;
		}

		inline void Matrix4x4::operator+=(Matrix4x4& other)
		{
			*this = *this + other;
		}

		inline void Matrix4x4::operator*=(Matrix4x4& other)
		{
#if USE_SIMD
			MatrixMultiply(this, this, &other);
#else
			*this = *this * other;
#endif
		}
	} // namespace Math
} // namespace M3D

#endif // M3D_MATH_MATRIX_H