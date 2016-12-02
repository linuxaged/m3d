#include "tests/gtest/gtest.h"

#include "Matrix.h"

using namespace m3d::math;

TEST(Math, Matrix)
{
	Matrix m0;
	EXPECT_EQ(m0[0][0] == 0.0f);
}