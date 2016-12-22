
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>

#include "Scene.hpp"
#include "RendererVulkan.hpp"
/*
 * Vulkan
 */

int main()
{
	Scene scene;
	scene.Init();

	// fill the Scene
	std::vector<uint32_t> loadedMeshIds;
	LoadMeshes(&scene, "D:\\app\\FBX_SDK_2017.1\\samples\\ViewScene\\humanoid.fbx", &loadedMeshIds);
	for (auto &meshId : loadedMeshIds)
	{
		uint32_t instanceId;
		AddInstance(scene, meshId, &instanceId);
		// do some translation
	}

	RendererVulkan renderer;
	renderer.Init(&scene);

	while (1)
	{
		renderer.Draw();
	}

    return 0;
}

