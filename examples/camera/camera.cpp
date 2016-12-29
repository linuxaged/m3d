
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>

#include "Scene.hpp"
#include "RendererVulkan.hpp"

int main()
{
	Scene scene;
	scene.Init();

	// fill the Scene
	std::vector<uint32_t> loadedMeshIds;
	LoadMeshes(&scene, &loadedMeshIds);
	for (auto &meshId : loadedMeshIds)
	{
		uint32_t instanceId;
		AddInstance(scene, meshId, &instanceId);
		// do some translation
	}

	RendererVulkan renderer;
	renderer.Init(&scene);
	renderer.DrawLoop();
	//while (1)
	//{
	//	renderer.Draw();
	//}

    return 0;
}

