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
			for (int i = 0; i < 8; i++)
			{
				printf("%f\n", dr(dre));
			}
			Vector3 v0(1.0f, 2.0f, 3.0f);
			Vector3 v1(4.0f, 5.0f, 6.0f);

			Vector3 v2 = v0 + v1;
			v2 = v0 ^ v1;

			v2.print();

			Matrix4x4 m0;
			m0.SetIdentity();
			m0.print();

			char buffer[128];
			v2.ToString(buffer, 128);
			Logger::WriteMessage(buffer);
		}

	};
}