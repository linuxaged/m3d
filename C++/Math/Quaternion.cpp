#include "Quaternion.h"
#include "MathBase.h"
#include "Matrix.h"

namespace M3D
{
	namespace Math
	{
		Quaternion::Quaternion(const Matrix4x4& mat4x4)
		{
			float s;

			const float trace = mat4x4.m[0][0] + mat4x4.m[1][1] + mat4x4.m[2][2];

			if (trace > 0.0f) 
			{
				float invsqrt = InvSqrt(trace + 1.f);
				w = 0.5f * (1.0f / invsqrt);
				s = 0.5f * invsqrt;

				x = (mat4x4.m[1][2] - mat4x4.m[2][1]) * s;
				y = (mat4x4.m[2][0] - mat4x4.m[0][2]) * s;
				z = (mat4x4.m[0][1] - mat4x4.m[1][0]) * s;
			} 
			else 
			{
				int i = 0;

				if (mat4x4.m[1][1] > mat4x4.m[0][0])
					i = 1;

				if (mat4x4.m[2][2] > mat4x4.m[i][i])
					i = 2;

				static const int nxt[3] = { 1, 2, 0 };
				const int j = nxt[i];
				const int k = nxt[j];
 
				s = mat4x4.m[i][i] - mat4x4.m[j][j] - mat4x4.m[k][k] + 1.0f;

				float invsqrt = InvSqrt(s);

				float qt[4];
				qt[i] = 0.5f * (1.f / invsqrt);

				s = 0.5f * invsqrt;

				qt[3] = (mat4x4.m[j][k] - mat4x4.m[k][j]) * s;
				qt[j] = (mat4x4.m[i][j] + mat4x4.m[j][i]) * s;
				qt[k] = (mat4x4.m[i][k] + mat4x4.m[k][i]) * s;

				x = qt[0];
				y = qt[1];
				z = qt[2];
				w = qt[3];
			}
		}
		
		Vector3 Quaternion::operator*(const Vector3& v0) const
		{
			const Vector3 v1(x, y, z);
			const Vector3 normal = Vector3::CrossProduct(v1, v0) * 2.0f;
			const Vector3 result = v0 + (normal * w) + Vector3::CrossProduct(v1, normal);
			return result;
		}
		
		Matrix4x4 Quaternion::operator*(const Matrix4x4& mat) const
		{
			Matrix4x4 result;
			Quaternion quat0, quat1;
			Quaternion inverse = Inverse();
			for (int I = 0; I < 4; ++I)
			{
				Quaternion quat_mat(mat.m[I][0], mat.m[I][1], mat.m[I][2], mat.m[I][3]);
				QuaternionMultiply(&quat0, this, &quat_mat);
				QuaternionMultiply(&quat1, &quat0, &inverse);
				result.m[I][0] = quat1.x;
				result.m[I][1] = quat1.y;
				result.m[I][2] = quat1.z;
				result.m[I][3] = quat1.w;
			}

			return result;
		}
		
	} // namespace Math
} // namespace M3D