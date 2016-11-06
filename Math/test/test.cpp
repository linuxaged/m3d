#include <cstdio>
#include "../MathBase.h"
int main(int argc, char const *argv[])
{
	float X = 3.0f;
	float result = M3D::Math::MapRange(X, M3D::Math::Range{1.0f, 3.0f}, M3D::Math::Range{1.0f, 10.0f});

	printf("convert %f from range{1.0f, 3.0f} to range {1.0f, 10.f}, get: %f\n", X, result);

	result = M3D::Math::MapRange(X, M3D::Math::Range{-1.0f, 4.0f}, M3D::Math::Range{1.0f, 6.0f});
	printf("convert %f from range{-1.0f, 4.0f} to range {1.0f, 6.0f}, get: %f\n", X, result);
	return 0;
}