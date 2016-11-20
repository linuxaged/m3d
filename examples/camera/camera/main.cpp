/*
 * Vulkan Windowed Program
 *
 * Copyright (C) 2016 Valve Corporation
 * Copyright (C) 2016 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
Vulkan C++ Windowed Project Template
Create and destroy a Vulkan surface on an SDL window.
*/

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
std::vector<const char*> getAvailableWSIExtensions();

uint32_t findQueue(vk::PhysicalDevice& physicalDevice, const vk::QueueFlags& flags, const vk::SurfaceKHR& presentSurface = vk::SurfaceKHR()) {
	std::vector<vk::QueueFamilyProperties> queueProps = physicalDevice.getQueueFamilyProperties();
	size_t queueCount = queueProps.size();
	for (uint32_t i = 0; i < queueCount; i++) {
		if (queueProps[i].queueFlags & flags) {
			if (presentSurface && !physicalDevice.getSurfaceSupportKHR(i, presentSurface)) {
				continue;
			}
			return i;
		}
	}
	throw std::runtime_error("No queue matches the flags " + vk::to_string(flags));
}

vk::Bool32 checkDeviceExtensionPresent(vk::PhysicalDevice physicalDevice, const char* extensionName) {
	uint32_t extensionCount = 0;
	std::vector<vk::ExtensionProperties> extensions = physicalDevice.enumerateDeviceExtensionProperties();
	for (auto& ext : extensions) {
		if (!strcmp(extensionName, ext.extensionName)) {
			return true;
		}
	}
	return false;
}

int main()
{
    // Use validation layers if this is a debug build, and use WSI extensions regardless
    std::vector<const char*> extensions = getAvailableWSIExtensions();
    std::vector<const char*> layers;
#if defined(_DEBUG)
    layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

    // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
    // program, which can be useful for layers and tools to provide more debug information.
    vk::ApplicationInfo appInfo = vk::ApplicationInfo()
        .setPApplicationName("Vulkan C++ Windowed Program Template")
        .setApplicationVersion(1)
        .setPEngineName("LunarG SDK")
        .setEngineVersion(1)
        .setApiVersion(VK_API_VERSION_1_0);

    // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
    // are needed.
    vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
        .setFlags(vk::InstanceCreateFlags())
        .setPApplicationInfo(&appInfo)
        .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
        .setPpEnabledExtensionNames(extensions.data())
        .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
        .setPpEnabledLayerNames(layers.data());

    // Create the Vulkan instance.
    vk::Instance instance;
    try {
        instance = vk::createInstance(instInfo);
    } catch(const std::exception& e) {
        std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
        return 1;
    }

    // Create an SDL window that supports Vulkan and OpenGL rendering.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "Could not initialize SDL." << std::endl;
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    if(window == NULL) {
        std::cout << "Could not create SDL window." << std::endl;
        return 1;
    }

    // Create a Vulkan surface for rendering
    vk::SurfaceKHR surface;
    try {
        surface = createVulkanSurface(instance, window);
    } catch(const std::exception& e) {
        std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
        instance.destroy();
        return 1;
    }

	// Create physical device
	std::vector<vk::PhysicalDevice> physicalDevices;
	
	try {
		physicalDevices = instance.enumeratePhysicalDevices();
	}
	catch (const std::exception& e)
	{
		std::cout << "Failed to create physical device: " << e.what() << std::endl;
		instance.destroy();
		return 1;
	}

	vk::PhysicalDevice physicalDevice = physicalDevices[0];
	vk::PhysicalDeviceFeatures deviceFeatures;
	deviceFeatures = physicalDevice.getFeatures();

	// Vulkan device
	vk::Device device;
	{
		// Find a queue that supports graphics operations
		uint32_t graphicsQueueIndex = findQueue(physicalDevice, vk::QueueFlagBits::eGraphics);
		std::array<float, 1> queuePriorities = { 0.0f };
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = queuePriorities.data();
		std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		vk::DeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		// enable the debug marker extension if it is present (likely meaning a debugging tool is present)
		if (checkDeviceExtensionPresent(physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
			enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		}
		if (enabledExtensions.size() > 0) {
			deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
		}

		device = physicalDevice.createDevice(deviceCreateInfo);
	}

	// Create semaphores
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	vk::Semaphore presentComplete = device.createSemaphore(semaphoreCreateInfo);
	vk::Semaphore renderComplete = device.createSemaphore(semaphoreCreateInfo);

	// Create swapChain


    // This is where most initializtion for a program should be performed

    // Poll for user input.
    bool stillRunning = true;
    while(stillRunning) {

        SDL_Event event;
        while(SDL_PollEvent(&event)) {

            switch(event.type) {

            case SDL_QUIT:
                stillRunning = false;
                break;

            default:
                // Do nothing.
                break;
            }
        }

        SDL_Delay(10);
    }

    // Clean up.
    instance.destroySurfaceKHR(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
	//
	device.destroySemaphore(presentComplete);
	device.destroySemaphore(renderComplete);
    instance.destroy();

    return 0;
}

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window)
{
    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);
    if(!SDL_GetWindowWMInfo(window, &windowInfo)) {
        throw std::system_error(std::error_code(), "SDK window manager info is not available.");
    }

    switch(windowInfo.subsystem) {

#if defined(SDL_VIDEO_DRIVER_ANDROID) && defined(VK_USE_PLATFORM_ANDROID_KHR)
    case SDL_SYSWM_ANDROID: {
        vk::AndroidSurfaceCreateInfoKHR surfaceInfo = vk::AndroidSurfaceCreateInfoKHR()
            .setWindow(windowInfo.info.android.window);
        return instance.createAndroidSurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
    case SDL_SYSWM_MIR: {
        vk::MirSurfaceCreateInfoKHR surfaceInfo = vk::MirSurfaceCreateInfoKHR()
            .setConnection(windowInfo.info.mir.connection)
            .setMirSurface(windowInfo.info.mir.surface);
        return instance.createMirSurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
    case SDL_SYSWM_WAYLAND: {
        vk::WaylandSurfaceCreateInfoKHR surfaceInfo = vk::WaylandSurfaceCreateInfoKHR()
            .setDisplay(windowInfo.info.wl.display)
            .setSurface(windowInfo.info.wl.surface);
        return instance.createWaylandSurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
    case SDL_SYSWM_WINDOWS: {
        vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
            .setHinstance(GetModuleHandle(NULL))
            .setHwnd(windowInfo.info.win.window);
        return instance.createWin32SurfaceKHR(surfaceInfo);
    }
#endif

#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
    case SDL_SYSWM_X11: {
        vk::XlibSurfaceCreateInfoKHR surfaceInfo = vk::XlibSurfaceCreateInfoKHR()
            .setDpy(windowInfo.info.x11.display)
            .setWindow(windowInfo.info.x11.window);
        return instance.createXlibSurfaceKHR(surfaceInfo);
    }
#endif

    default:
        throw std::system_error(std::error_code(), "Unsupported window manager is in use.");
    }
}

std::vector<const char*> getAvailableWSIExtensions()
{
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MIR_KHR)
    extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

    return extensions;
}
