#include "Matrix.h"
using namespace M3D::Math;

int main(int argc, char const *argv[])
{
	  Vector3 v0(1.0f, 2.0f, 3.0f);
	  Vector3 v1(4.0f, 5.0f, 6.0f);

	  Vector3 v2 = v0 + v1;
	  v2 = v0 ^ v1;

	  v2.print();

	  Matrix4x4 m0;
	  m0.SetIdentity();
	  m0.print();
	  return 0;
}