/*
* Copyright (C) 2017 Tracy Ma
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

namespace m3d {
	class Mesh {
	public:
		Mesh() {}
		virtual ~Mesh();

		std::vector<float> getVetices();
		std::vector<uint32_t> getIndices();

		/* Rectangular box or Cube */
		void CreateBox(Vector3f dimensions, uint32_t segments);

		/* Ellipsoid or Sphere */
		void CreateEllipsoid(float radii, uint32_t radialSegments,
			uint32_t verticalSegments, bool hemisphere);

		/* */
		void CreateCylinder(float height, float radii, uint32_t radialSegments,
			uint32_t verticalSegments);

	private:
		std::vector<float> vertices;
		std::vector<uint32_t> indices;
	}
}