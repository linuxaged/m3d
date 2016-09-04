#pragma once

#include <arm_neon.h>

namespace M3D
{
	namespace Math
	{
		using VectorSIMD = float32x4_t __attribute((aligned(16)));

		inline VectorSIMD MakeVectorSIMD(float fX, float fY, float fZ, float fW)
		{
			union
			{
				VectorSIMD _vectorSIMD;
				float _v[4];
			} vectorSIMD;
	
			vectorSIMD._v[0] = fX;
			vectorSIMD._v[1] = fY;
			vectorSIMD._v[2] = fZ;
			vectorSIMD._v[3] = fW;
	
			return vectorSIMD._vectorSIMD;
		}
		
		/// Add two VectorSIMD
		inline VectorSIMD VectorAdd(VectorSIMD v0, VectorSIMD v1)
		{
			return vaddq_f32(v0, v1);
		}
		
		/// Substract a VectorSIMD from another
		inline VectorSIMD VectorSubstract(VectorSIMD v0, VectorSIMD v1)
		{
			return vsubq_f32(v0, v1);
		}
		
		/// Multiply a VectorSIMD to another
		inline VectorSIMD VectorMultiply(VectorSIMD v0, VectorSIMD v1)
		{
			return vmulq_f32(v0, v1);
		}
		
		inline void MatrixMultiply(void* result, const void* m0, const void* m1)
		{
			const VectorSIMD *left	= (const VectorSIMD *) m0;
			const VectorSIMD *right	= (const VectorSIMD *) m1;
			VectorSIMD *_result		= (VectorSIMD *) result;
			VectorSIMD temp, row0, row1, row2, row3;
			// row 1
			temp    = vmulq_lane_f32(right[0], vget_low_f32(left[0]), 0);
			temp    = vmlaq_lane_f32(temp, right[1], vget_low_f32(left[0]), 1);
			temp    = vmlaq_lane_f32(temp, right[2], vget_high_f32(left[0]), 0);
			row0      = vmlaq_lane_f32(temp, right[3], vget_high_f32(left[0]), 1);
			// row 2
			temp    = vmulq_lane_f32(right[0], vget_low_f32(left[1]), 0);
			temp    = vmlaq_lane_f32(temp, right[1], vget_low_f32(left[1]), 1);
			temp    = vmlaq_lane_f32(temp, right[2], vget_high_f32(left[1]), 0);
			row1      = vmlaq_lane_f32(temp, right[3], vget_high_f32(left[1]), 1);
			// row 3
			temp    = vmulq_lane_f32(right[0], vget_low_f32(left[2]), 0);
			temp    = vmlaq_lane_f32(temp, right[1], vget_low_f32(left[2]), 1);
			temp    = vmlaq_lane_f32(temp, right[2], vget_high_f32(left[2]), 0);
			row2      = vmlaq_lane_f32(temp, right[3], vget_high_f32(left[2]), 1);
			// row 4
			temp    = vmulq_lane_f32(right[0], vget_low_f32(left[3]), 0);
			temp    = vmlaq_lane_f32(temp, right[1], vget_low_f32(left[3]), 1);
			temp    = vmlaq_lane_f32(temp, right[2], vget_high_f32(left[3]), 0);
			row3      = vmlaq_lane_f32(temp, right[3], vget_high_f32(left[3]), 1);

			_result[0] = row0;
			_result[1] = row1;
			_result[2] = row2;
			_result[3] = row3;	
		}
	}
}
