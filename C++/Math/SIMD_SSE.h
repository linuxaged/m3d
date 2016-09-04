#pragma once

#include <emmintrin.h> // SSE2

namespace M3D
{
	namespace Math
	{
		using VectorSIMD = __m128;
		
		inline VectorSIMD MakeVectorSIMD(float fX, float fY, float fZ, float fW)
		{
			return _mm_setr_ps(fX, fY, fZ, fW);
		}
		
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )
		
#define VectorMultiply( v0, v1 )	_mm_mul_ps( v0, v1 )
#define VectorMultiplyAdd( v0, v1, v2 )	_mm_add_ps( _mm_mul_ps(v0, v1), v2 )
#define VectorReplicate( v, index )	_mm_shuffle_ps( v, v, SHUFFLEMASK(index, index, index, index) )
		
		inline void MatrixMultiply(void *result, const void* left, const void* right)
		{
			const VectorSIMD *_left	= (const VectorSIMD *) left;
			const VectorSIMD *_right	= (const VectorSIMD *) right;
			VectorSIMD *_result		= (VectorSIMD *) result;
			VectorSIMD temp, row0, row1, row2, row3;

			// row 0
			temp	= VectorMultiply(VectorReplicate(_left[0], 0), _right[0]);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[0], 1), _right[1], temp);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[0], 2), _right[2], temp);
			row0		= VectorMultiplyAdd(VectorReplicate(_left[0], 3), _right[3], temp);

			// row 1
			temp	= VectorMultiply(VectorReplicate(_left[1], 0), _right[0]);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[1], 1), _right[1], temp);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[1], 2), _right[2], temp);
			row1	= VectorMultiplyAdd(VectorReplicate(_left[1], 3), _right[3], temp);

			// row 2
			temp	= VectorMultiply(VectorReplicate(_left[2], 0), _right[0]);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[2], 1), _right[1], temp);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[2], 2), _right[2], temp);
			row2	= VectorMultiplyAdd(VectorReplicate(_left[2], 3), _right[3], temp);

			// row 3
			temp	= VectorMultiply(VectorReplicate(_left[3], 0), _right[0]);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[3], 1), _right[1], temp);
			temp	= VectorMultiplyAdd(VectorReplicate(_left[3], 2), _right[2], temp);
			row3	= VectorMultiplyAdd(VectorReplicate(_left[3], 3), _right[3], temp);

			_result[0] = row0;
			_result[1] = row1;
			_result[2] = row2;
			_result[3] = row3;
		}
	}
}