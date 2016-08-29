#ifndef M3D_MATH_MATRIX_H
#define M3D_MATH_MATRIX_H

#include <cmath>
#include <cstring>
#include <cstdio>

namespace M3D {
	namespace Math {
		//-------------------------------------------------------------
		// Vector3
		//-------------------------------------------------------------
		struct Vector3
		{
		public:
			float x;
			float y;
			float z;

			inline Vector3(float InX, float InY, float InZ);
			inline Vector3 operator+(const Vector3& Other) const;
			inline Vector3 operator-(const Vector3& Other) const;

			inline Vector3 operator+(float Bias) const;
			inline Vector3 operator-(float Bias) const;
			inline Vector3 operator*(float Scale) const;

			inline float operator|(const Vector3& Other) const;
			inline Vector3 operator^(const Vector3& Other) const;

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

		inline Vector3 Vector3::operator+(const Vector3& Other) const
		{
			return Vector3(x + Other.x, y + Other.y, z + Other.z);
		}

		inline Vector3 Vector3::operator-(const Vector3& Other) const
		{
			return Vector3(x - Other.x, y - Other.y, z - Other.z);
		}

		inline Vector3 Vector3::operator+(float Bias) const
		{
			return Vector3(x + Bias, y + Bias, z + Bias);
		}

		inline Vector3 Vector3::operator-(float Bias) const
		{
			return Vector3(x - Bias, y - Bias, z - Bias);
		}

		inline Vector3 Vector3::operator*(float Scale) const
		{
			return Vector3(x * Scale, y * Scale, z * Scale);
		}

		inline float Vector3::operator|(const Vector3& Other) const
		{
			return x * Other.x + y * Other.y + z * Other.z;
		}

		inline Vector3 Vector3::operator^(const Vector3& Other) const
		{
			return Vector3(   y * Other.z - z * Other.y,
				z * Other.x - x * Other.z,
				x * Other.y - y * Other.x);
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
			float m[4][4];

			inline Matrix4x4();
			inline Matrix4x4(const float* array);

			inline void SetIdentity();

			inline Matrix4x4 operator+(Matrix4x4& Other);
			inline Matrix4x4 operator-(Matrix4x4& Other);
			inline Matrix4x4 operator*(Matrix4x4& Other);
			inline Matrix4x4& operator=(Matrix4x4& Other);
			inline Matrix4x4 operator+=(Matrix4x4& Other);
			inline Matrix4x4 operator*=(Matrix4x4& Other);

			void print();
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

		inline Matrix4x4 Matrix4x4::operator+(Matrix4x4& Other)
		{
			Matrix4x4 result;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					result.m[i][j] = m[i][j] + Other.m[i][j];
				}
			}
			return result;
		}

		inline Matrix4x4 Matrix4x4::operator-(Matrix4x4& Other)
		{
			Matrix4x4 result;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					result.m[i][j] = m[i][j] - Other.m[i][j];
				}
			}
			return result;
		}

		inline Matrix4x4 Matrix4x4::operator*(Matrix4x4& Other)
		{
			Matrix4x4 result;
			for (int i = 0; i < 4; i++)
			{
				float accumulator;
				for (int j = 0; j < 4; j++)
				{
					for (int k = 0; k < 4; k++)
					{
						accumulator += m[i][k] * Other.m[k][j];
					}

					result.m[i][j] = accumulator;
				}
			}
			return result;
		}

		inline Matrix4x4& Matrix4x4::operator=(Matrix4x4& Other)
		{
			std::memcpy(m, &Other.m[0][0], 16 * sizeof(float));
		}

		inline Matrix4x4 Matrix4x4::operator+=(Matrix4x4& Other)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					m[i][j] += Other.m[i][j];
				}
			}
		}

		inline Matrix4x4 Matrix4x4::operator*=(Matrix4x4& Other)
		{
			Matrix4x4 result;
			for (int i = 0; i < 4; i++)
			{
				float accumulator;
				for (int j = 0; j < 4; j++)
				{
					for (int k = 0; k < 4; k++)
					{
						accumulator += m[i][k] * Other.m[k][j];
					}

					result.m[i][j] = accumulator;
				}
			}
			*this = result;
		}
	} // namespace Math
} // namespace M3D

#endif // M3D_MATH_MATRIX_H