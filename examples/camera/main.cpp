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

/*
 * Vulkan
 */

bool CreateInstance()
{
	return true;
}

bool CreateSurface()
{
	return true;
}

bool CreateDevice()
{
	return true;
}

bool GetDeviceQueue()
{
	return true;
}

bool CreateSemaphores()
{
	return true;
}

bool SetupVulkan()
{
	if (!CreateInstance())
	{
		return false;
	}
	if (!CreateSurface())
	{
		return false;
	}
	if (!CreateDevice())
	{
		return false;
	}
	if (!GetDeviceQueue())
	{
		return false;
	}
	if (!CreateSemaphores())
	{
		return false;
	}
}

/* Render Pass */
bool CreateRenderPass()
{
	return true;
}

bool CreateFramebuffers()
{
	return true;
}

bool CreatePipeline()
{
	return true;
}

bool CreateCommandBuffers()
{
	return true;
}

bool RecordCommandBuffers()
{
	return true;
}

bool OnWindowSizeChanged()
{
	if (!CreateRenderPass())
	{
		return false;
	}
	if (!CreateFramebuffers())
	{
		return false;
	}
	if (!CreatePipeline())
	{
		return false;
	}
	if (!CreateCommandBuffers())
	{
		return false;
	}
	if (!RecordCommandBuffers())
	{
		return false;
	}
	return true;
}

/* Draw Loop */
bool Draw()
{

	return false;
}

vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
std::vector<const char*> getAvailableWSIExtensions();





int main()
{
	SetupVulkan();

    

	

	

	/*
	 * SwapChain
	 */
	vk::SurfaceCapabilitiesKHR surfaceCapabilitiesKHR = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	size_t formatCount = surfaceFormats.size();
	vk::Format colorFormat;
	vk::ColorSpaceKHR colorSpace;
	// If the surface format list only includes one entry with  vk::Format::eUndefined,
	// there is no preferered format, so we assume  vk::Format::eB8G8R8A8Unorm
	if ((formatCount == 1) && (surfaceFormats[0].format == vk::Format::eUndefined)) {
		colorFormat = vk::Format::eB8G8R8A8Unorm;
	}
	else {
		// Always select the first available color format
		// If you need a specific format (e.g. SRGB) you'd need to
		// iterate over the list of available surface format and
		// check for it's presence
		colorFormat = surfaceFormats[0].format;
	}
	colorSpace = surfaceFormats[0].colorSpace;

	// Determine the number of images
	uint32_t desiredNumberOfSwapchainImages = surfaceCapabilitiesKHR.minImageCount + 1;
	if ((surfaceCapabilitiesKHR.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCapabilitiesKHR.maxImageCount)) {
		desiredNumberOfSwapchainImages = surfaceCapabilitiesKHR.maxImageCount;
	}
	// Select the size of Swap Chain Images
	vk::Extent2D size{ 1280, 720 };
	vk::Extent2D swapchainExtent;
	// width and height are either both -1, or both not -1.
	if (surfaceCapabilitiesKHR.currentExtent.width == -1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent = size;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfaceCapabilitiesKHR.currentExtent;
		size = surfaceCapabilitiesKHR.currentExtent;
	}
	// pre transform
	vk::SurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCapabilitiesKHR.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else {
		preTransform = surfaceCapabilitiesKHR.currentTransform;
	}
	// Prefer mailbox mode if present, it's the lowest latency non-tearing present  mode
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

	vk::SwapchainCreateInfoKHR swapchainCI;
	swapchainCI.surface = surface;
	swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
	swapchainCI.imageFormat = colorFormat;
	swapchainCI.imageColorSpace = colorSpace;
	swapchainCI.imageExtent = vk::Extent2D{ swapchainExtent.width, swapchainExtent.height };
	swapchainCI.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	swapchainCI.preTransform = preTransform;
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.imageSharingMode = vk::SharingMode::eExclusive;
	swapchainCI.queueFamilyIndexCount = 0;
	swapchainCI.pQueueFamilyIndices = NULL;
	swapchainCI.presentMode = swapchainPresentMode;
	// TODO:
	//swapchainCI.oldSwapchain = oldSwapchain;
	swapchainCI.clipped = true;
	swapchainCI.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

	vk::SwapchainKHR swapChain = device.createSwapchainKHR(swapchainCI);


	// DRAW
	uint32_t currentImage;

	auto resultValue = device.acquireNextImageKHR(swapChain, UINT64_MAX, presentComplete, vk::Fence());
	vk::Result result = resultValue.result;
	if (result != vk::Result::eSuccess) {
		// TODO handle eSuboptimalKHR
		std::cerr << "Invalid acquire result: " << vk::to_string(result);
		throw std::error_code(result);
	}
	currentImage = resultValue.value;
	// Present the current image to the queue
	//vk::PresentInfoKHR presentInfo;
	//vk::Result queuePresent(vk::Semaphore waitSemaphore) {
	//	presentInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
	//	presentInfo.pWaitSemaphores = &waitSemaphore;
	//	return context.queue.presentKHR(presentInfo);
	//}
	
	/*
	 * Create Command Buffer Memory Pool
	 */
	vk::CommandPool cmdPool;
	vk::CommandPoolCreateInfo cmdPoolInfo;
	cmdPoolInfo.queueFamilyIndex = graphicsQueueIndex;
	cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	cmdPool = device.createCommandPool(cmdPoolInfo);
	/*
	 * Allocate Command Buffers
	 */
	uint32_t swapChainImagesCount;
	device.getSwapchainImagesKHR(swapChain, &swapChainImagesCount, nullptr);

	vk::CommandBufferAllocateInfo cmdBufferAllocInfo;
	cmdBufferAllocInfo.setSType(vk::StructureType::eCommandBufferAllocateInfo);
	cmdBufferAllocInfo.setCommandPool(cmdPool);
	cmdBufferAllocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	cmdBufferAllocInfo.setCommandBufferCount(swapChainImagesCount);

	vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers(cmdBufferAllocInfo)[0];
	/*
	 * Record Comand Buffers
	 */
	vk::CommandBufferBeginInfo cmdBufferBeginInfo;
	vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    // This is where most initializtion for a program should be performed



    // Poll for user input.
    bool stillRunning = true;
    while(stillRunning) {

		/*
		* Draw
		*/

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

    /*
	 * Clean up
	 */
	// destroy swap chain
	device.destroySwapchainKHR(swapChain);
	// 
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
