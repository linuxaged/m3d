/*
* Copyright (C) 2017 Tracy Ma
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#elif defined(__ANDROID__)
#include "vulkanandroid.h"
#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#elif defined(__linux__)
#include <xcb/xcb.h>
#endif

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>

#include "RendererVulkan.hpp"
#include "Scene.hpp"

//VulkanExample *vulkanExample;
RendererVulkan* renderer;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (renderer != NULL) {
        renderer->handle_message(uMsg, wParam, lParam);
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    //for (size_t i = 0; i < __argc; i++) { VulkanExample::args.push_back(__argv[i]); };
    //vulkanExample = new VulkanExample();
    //vulkanExample->initVulkan();
    //vulkanExample->setupWindow(hInstance, WndProc);
    //vulkanExample->initSwapchain();
    //vulkanExample->prepare();
    //vulkanExample->renderLoop();
    //delete(vulkanExample);

    Scene scene;
    scene.Init();

    // fill the Scene
    std::vector<uint32_t> loadedMeshIds;
    LoadMeshes(&scene, &loadedMeshIds);
    for (auto& meshId : loadedMeshIds) {
        uint32_t instanceId;
        AddInstance(scene, meshId, &instanceId);
        // do some translation
    }

    renderer = new RendererVulkan();
    renderer->createWin32Window(hInstance, WndProc);
    renderer->Init(&scene);
    renderer->DrawLoop();

    delete (renderer);

    return 0;
}

//int main()
//{
//    Scene scene;
//    scene.Init();
//
//    // fill the Scene
//    std::vector<uint32_t> loadedMeshIds;
//    LoadMeshes(&scene, &loadedMeshIds);
//    for (auto& meshId : loadedMeshIds) {
//        uint32_t instanceId;
//        AddInstance(scene, meshId, &instanceId);
//        // do some translation
//    }
//
//    RendererVulkan renderer;
//    renderer.Init(&scene);
//    renderer.DrawLoop();
//
//    return 0;
//}
