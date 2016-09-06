#pragma once

#include <cmath>

namespace M3D
{
	namespace Math
	{
		static inline float InvSqrt(float f)
		{
			// TODO: quake 3, neon
			return sqrt(1.0f / f);
		}
	}
}
