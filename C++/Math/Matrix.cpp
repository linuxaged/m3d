#include "Matrix.h"

namespace M3D
{
	namespace Math
	{
		//-------------------------------------------------------------
		// Vector3
		//-------------------------------------------------------------
		void Vector3::print()
		{
			printf("(%.9g, %.9g, %.9g)\n", x, y, z);
		}

		void Vector3::ToString(char* const str, size_t size)
		{
			snprintf(str, size, "(%f, %f, %f)", x, y, z);
		}
		
		//-------------------------------------------------------------
		// Matrix4x4
		//-------------------------------------------------------------
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
	}
}