#include "Scene.h"
#include "tiny_obj_loader.h"
#include <fbxsdk.h>
 
void Scene::Init()
{
	diffuseMaps = packed_freelist<DiffuseMap>(512);
	materials = packed_freelist<Material>(512);
	meshes = packed_freelist<Mesh>(512);
	transforms = packed_freelist<Transform>(4096);
	instances = packed_freelist<Instance>(4096);
	cameras = packed_freelist<Camera>(32);
}

void LoadMeshes(
	Scene& scene,
	const std::string& filename,
	std::vector<uint32_t>* loadedMeshIDs)
{
	// assume mtl is in the same folder as the obj
	std::string mtl_basepath = filename;
	size_t last_slash = mtl_basepath.find_last_of("/");
	if (last_slash == std::string::npos)
		mtl_basepath = "./";
	else
		mtl_basepath = mtl_basepath.substr(0, last_slash + 1);

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	if (!tinyobj::LoadObj(
		shapes, materials, err,
		filename.c_str(), mtl_basepath.c_str(),
		tinyobj::triangulation | tinyobj::calculate_normals))
	{
		fprintf(stderr, "tinyobj::LoadObj(%s) error: %s\n", filename.c_str(), err.c_str());
		return;
	}

	if (!err.empty())
	{
		fprintf(stderr, "tinyobj::LoadObj(%s) warning: %s\n", filename.c_str(), err.c_str());
	}

	// Add materials to the scene
	std::map<std::string, uint32_t> diffuseMapCache;
	std::vector<uint32_t> newMaterialIDs;
	for (const tinyobj::material_t& materialToAdd : materials)
	{
		Material newMaterial;

		newMaterial.name = materialToAdd.name;

		newMaterial.ambient[0] = materialToAdd.ambient[0];
		newMaterial.ambient[1] = materialToAdd.ambient[1];
		newMaterial.ambient[2] = materialToAdd.ambient[2];
		newMaterial.diffuse[0] = materialToAdd.diffuse[0];
		newMaterial.diffuse[1] = materialToAdd.diffuse[1];
		newMaterial.diffuse[2] = materialToAdd.diffuse[2];
		newMaterial.specular[0] = materialToAdd.specular[0];
		newMaterial.specular[1] = materialToAdd.specular[1];
		newMaterial.specular[2] = materialToAdd.specular[2];
		newMaterial.shininess = materialToAdd.shininess;

		newMaterial.diffuseMapId = -1;

		if (!materialToAdd.diffuse_texname.empty())
		{
			auto cachedTexture = diffuseMapCache.find(materialToAdd.diffuse_texname);

			if (cachedTexture != end(diffuseMapCache))
			{
				newMaterial.diffuseMapId = cachedTexture->second;
			}
			else
			{
				std::string diffuse_texname_full = mtl_basepath + materialToAdd.diffuse_texname;
				int x, y, comp;

				{
					float maxAnisotropy;

					vkext::VulkanTexture newDiffuseMapTO;
					vkext::VulkanTextureLoader vulkanTextureLoader();
					// TODO: load texture

					DiffuseMap newDiffuseMap;
					newDiffuseMap.texture = newDiffuseMapTO;

					uint32_t newDiffuseMapID = scene.diffuseMaps.insert(newDiffuseMap);

					diffuseMapCache.emplace(materialToAdd.diffuse_texname, newDiffuseMapID);

					newMaterial.diffuseMapId = newDiffuseMapID;
					// TODO: release texture data
				}
			}
		}

		uint32_t newMaterialID = scene.materials.insert(newMaterial);

		newMaterialIDs.push_back(newMaterialID);
	}

	// Add meshes (and prototypes) to the scene
	for (const tinyobj::shape_t& shapeToAdd : shapes)
	{
		const tinyobj::mesh_t& meshToAdd = shapeToAdd.mesh;

		Mesh newMesh;

		newMesh.name = shapeToAdd.name;

		if (meshToAdd.positions.empty())
		{
			// should never happen
			assert("there is no vertices in .obj");
		}
		else
		{
			/* TODO: Bind Vertex Buffer */
		}

		if (meshToAdd.texcoords.empty())
		{
			assert("there is no uv in .obj");
		}
		else
		{
			/* TODO: Bind UV */
		}

		if (meshToAdd.normals.empty())
		{
			assert("there is no normal in .obj");
		}
		else
		{
			/* TODO: Bind Normal */
		}

		if (meshToAdd.indices.empty())
		{
			// should never happen
			assert("there is no indies in .obj");
		}
		else
		{
			/* TODO: Bind Indices */
		}

		// split mesh into draw calls with different materials
		int numFaces = (int)meshToAdd.indices.size() / 3;
		int currMaterialFirstFaceIndex = 0;
		for (int faceIdx = 0; faceIdx < numFaces; faceIdx++)
		{
			bool isLastFace = faceIdx + 1 == numFaces;
			bool isNextFaceDifferent = isLastFace || meshToAdd.material_ids[faceIdx + 1] != meshToAdd.material_ids[faceIdx];
			if (isNextFaceDifferent)
			{
				DrawElementsIndirectCommand currDrawCommand;
				currDrawCommand.count = ((faceIdx + 1) - currMaterialFirstFaceIndex) * 3;
				currDrawCommand.primCount = 1;
				currDrawCommand.firstIndex = currMaterialFirstFaceIndex * 3;
				currDrawCommand.baseVertex = 0;
				currDrawCommand.baseInstance = 0;

				uint32_t currMaterialID = newMaterialIDs[meshToAdd.material_ids[faceIdx]];

				newMesh.drawCommands.push_back(currDrawCommand);
				newMesh.materialIds.push_back(currMaterialID);

				currMaterialFirstFaceIndex = faceIdx + 1;
			}
		}

		uint32_t newMeshID = scene.meshes.insert(newMesh);

		if (loadedMeshIDs)
		{
			loadedMeshIDs->push_back(newMeshID);
		}
	}
}

void AddInstance(
	Scene& scene,
	uint32_t meshID,
	uint32_t* newInstanceID)
{
	Transform newTransform;
	newTransform.scale = m3d::math::Vector3(1.0f, 1.0f, 1.0f);

	uint32_t newTransformID = scene.transforms.insert(newTransform);

	Instance newInstance;
	newInstance.meshId = meshID;
	newInstance.transformId = newTransformID;

	uint32_t tmpNewInstanceID = scene.instances.insert(newInstance);
	if (newInstanceID)
	{
		*newInstanceID = tmpNewInstanceID;
	}
}