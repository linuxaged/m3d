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
m3d::RendererVulkan* renderer;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (renderer != NULL) {
        renderer->handle_message(uMsg, wParam, lParam);
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    m3d::Scene scene;
    scene.Init();

    // fill the Scene
    std::vector<uint32_t> loadedMeshIds;
    LoadMeshes(&scene, &loadedMeshIds);
    for (auto& meshId : loadedMeshIds) {
        uint32_t instanceId;
        AddInstance(scene, meshId, &instanceId);
        // do some translation
    }

    renderer = new m3d::RendererVulkan();
    renderer->createWin32Window(hInstance, WndProc, 1280, 720);
    renderer->Init(&scene);
    renderer->DrawLoop();

    delete (renderer);

    return 0;
}
