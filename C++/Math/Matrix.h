#ifndef M3D_MATH_MATRIX_H
#define M3D_MATH_MATRIX_H

#include <cmath>
#include <cstring>
#include <cstdio>

namespace M3D {
	namespace Math {

	// #if __has_attribute(always_inline)
#define ALWAYS_INLINE inline //__attribute__((always_inline)) inline
	// #else
	// #define ALWAYS_INLINE
	// #endif

		struct Vector3
		{
		public:
			float x;
			float y;
			float z;

			ALWAYS_INLINE Vector3(float InX, float InY, float InZ);
			ALWAYS_INLINE Vector3 operator+(const Vector3& Other) const;
			ALWAYS_INLINE Vector3 operator-(const Vector3& Other) const;

			ALWAYS_INLINE Vector3 operator+(float Bias) const;
			ALWAYS_INLINE Vector3 operator-(float Bias) const;
			ALWAYS_INLINE Vector3 operator*(float Scale) const;

			ALWAYS_INLINE float operator|(const Vector3& Other) const;
			ALWAYS_INLINE Vector3 operator^(const Vector3& Other) const;

			ALWAYS_INLINE static Vector3 CrossProduct(const Vector3& Left, const Vector3& Right);
			ALWAYS_INLINE static float DotProduct(const Vector3& Left, const Vector3& Right);

			ALWAYS_INLINE void Normalize();

			void print();
			void ToString(char* const str, size_t size);
		};

		ALWAYS_INLINE Vector3::Vector3(float InX, float InY, float InZ)
			: x(InX)
			, y(InY)
			, z(InZ)
		{

		}

		ALWAYS_INLINE Vector3 Vector3::operator+(const Vector3& Other) const
		{
			return Vector3(x + Other.x, y + Other.y, z + Other.z);
		}

		ALWAYS_INLINE Vector3 Vector3::operator-(const Vector3& Other) const
		{
			return Vector3(x - Other.x, y - Other.y, z - Other.z);
		}

		ALWAYS_INLINE Vector3 Vector3::operator+(float Bias) const
		{
			return Vector3(x + Bias, y + Bias, z + Bias);
		}

		ALWAYS_INLINE Vector3 Vector3::operator-(float Bias) const
		{
			return Vector3(x - Bias, y - Bias, z - Bias);
		}

		ALWAYS_INLINE Vector3 Vector3::operator*(float Scale) const
		{
			return Vector3(x * Scale, y * Scale, z * Scale);
		}

		ALWAYS_INLINE float Vector3::operator|(const Vector3& Other) const
		{
			return x * Other.x + y * Other.y + z * Other.z;
		}

		ALWAYS_INLINE Vector3 Vector3::operator^(const Vector3& Other) const
		{
			return Vector3(   y * Other.z - z * Other.y,
				z * Other.x - x * Other.z,
				x * Other.y - y * Other.x);
		}

		ALWAYS_INLINE float Vector3::DotProduct(const Vector3& Left, const Vector3& Right)
		{
			return Left | Right;
		}

		ALWAYS_INLINE Vector3 Vector3::CrossProduct(const Vector3& Left, const Vector3& Right)
		{
			return Left ^ Right;
		}

		ALWAYS_INLINE void Vector3::Normalize()
		{
			float length = sqrt(x * x + y * y + z * z);
			x /= length;
			y /= length;
			z /= length;
		}

		void Vector3::print()
		{
			printf("(%.9g, %.9g, %.9g)\n", x, y, z);
		}

		void Vector3::ToString(char* const str, size_t size)
		{
			snprintf(str, size, "(%f, %f, %f)", x, y, z);
		}
		
		struct Matrix4x4
		{
		public:
			float m[4][4];

			ALWAYS_INLINE Matrix4x4();
			ALWAYS_INLINE Matrix4x4(const float* array);

			ALWAYS_INLINE void SetIdentity();

			ALWAYS_INLINE Matrix4x4 operator+(Matrix4x4& Other);
			ALWAYS_INLINE Matrix4x4 operator-(Matrix4x4& Other);
			ALWAYS_INLINE Matrix4x4 operator*(Matrix4x4& Other);
			ALWAYS_INLINE Matrix4x4& operator=(Matrix4x4& Other);
			ALWAYS_INLINE Matrix4x4 operator+=(Matrix4x4& Other);
			ALWAYS_INLINE Matrix4x4 operator*=(Matrix4x4& Other);

			void print();
		};

		ALWAYS_INLINE void Matrix4x4::SetIdentity()
		{
			m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
			m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
			m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
			m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
		}

		ALWAYS_INLINE Matrix4x4::Matrix4x4()
		{
		  // SetIdentity();
		}

		ALWAYS_INLINE Matrix4x4::Matrix4x4(const float* array)
		{
			std::memcpy(m, array, 16 * sizeof(float));
		}

		ALWAYS_INLINE Matrix4x4 Matrix4x4::operator+(Matrix4x4& Other)
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

		ALWAYS_INLINE Matrix4x4 Matrix4x4::operator-(Matrix4x4& Other)
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

		ALWAYS_INLINE Matrix4x4 Matrix4x4::operator*(Matrix4x4& Other)
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

		ALWAYS_INLINE Matrix4x4& Matrix4x4::operator=(Matrix4x4& Other)
		{
			std::memcpy(m, &Other.m[0][0], 16 * sizeof(float));
		}

		ALWAYS_INLINE Matrix4x4 Matrix4x4::operator+=(Matrix4x4& Other)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					m[i][j] += Other.m[i][j];
				}
			}
		}

		ALWAYS_INLINE Matrix4x4 Matrix4x4::operator*=(Matrix4x4& Other)
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

		void Matrix4x4::print()
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					printf("%f,", m[i][j]);
				}
				printf("\n");
			}
		}

	} // namespace Math
} // namespace M3D

#endif // M3D_MATH_MATRIX_H