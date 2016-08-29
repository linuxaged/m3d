#include "stdafx.h"
#include "CppUnitTest.h"

#include "../../Math/Matrix.h"
#include <random>
using namespace M3D::Math;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestMath
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			// TODO: 在此输入测试代码
			std::default_random_engine dre;
			std::uniform_real_distribution<float> dr(-100.0f, 100.0f);

			const size_t length = 100;
			std::vector<Matrix4x4> array_mat(length);
			for (auto& m : array_mat)
			{
				for (int i = 0; i < 4; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						m.m[i][j] = dr(dre);
					}
				}
			}
			
			Matrix4x4 mul_result;
			for (auto& m : array_mat)
			{
				mul_result += m;
			}

			char buffer[256];
			mul_result.ToString(buffer, 256);
			Logger::WriteMessage(buffer);
		}

	};
}