#include "stdafx.h"
#include "CppUnitTest.h"

#include "../Math/Matrix.h"

using namespace M3D::Math;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestMath
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
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