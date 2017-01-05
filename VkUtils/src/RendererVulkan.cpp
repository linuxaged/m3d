/*
* Copyright (C) 2017 Tracy Ma
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "RendererVulkan.hpp"
#include "File.hpp"
#include "Matrix.h"
#include "Scene.hpp"
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "VulkanHelper.hpp"
#include "VulkanSwapchain.hpp"
#include "vulkanTextureLoader.hpp"
#include <chrono>
#include <iostream>

#define VERTEX_BUFFER_BIND_ID 0

/*
 * Utils
 */
std::vector<const char*> RendererVulkan::getAvailableWSIExtensions()
{
	std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };

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

//#if defined(_DEBUG)
//	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
//#endif

    return extensions;
}

void RendererVulkan::createWin32Window(HINSTANCE hinstance, WNDPROC wndproc, uint32_t w, uint32_t h)
{
    const std::string class_name("RendererVulkanWindowClass");

    hinstance_ = hinstance;

    WNDCLASSEX win_class = {};
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = wndproc;
    win_class.hInstance = hinstance_;
    win_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    win_class.lpszMenuName = nullptr;
    win_class.lpszClassName = class_name.c_str();
    //win_class.hIconSm = LoadIcon(win_class.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    RegisterClassEx(&win_class);

    const DWORD win_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_OVERLAPPEDWINDOW;

    RECT win_rect = { 0, 0, w, h };
    AdjustWindowRect(&win_rect, win_style, false);

    hwnd_ = CreateWindowEx(WS_EX_APPWINDOW,
        class_name.c_str(),
        "RendererVulkan",
        win_style,
        0,
        0,
        win_rect.right - win_rect.left,
        win_rect.bottom - win_rect.top,
        nullptr,
        nullptr,
        hinstance_,
        nullptr);

    if (!hwnd_) {
        printf("could NOT create window!\n");
        fflush(stdout);
        exit(1);
    }

    ShowWindow(hwnd_, SW_SHOW);
    SetForegroundWindow(hwnd_);
    SetFocus(hwnd_);

    // TODO:
    SetWindowLongPtr(hwnd_, GWLP_USERDATA, (LONG_PTR)this);
}

LRESULT RendererVulkan::handle_message(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_CLOSE:
        inited = false;
        DestroyWindow(hwnd_);
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        ValidateRect(hwnd_, NULL);
        break;
    case WM_KEYDOWN:

        break;
    case WM_KEYUP:

        break;
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
        break;
    case WM_MOUSEWHEEL: {

        break;
    }
    case WM_MOUSEMOVE:

        break;
    case WM_SIZE:
        OnWindowSizeChanged();
        break;
    case WM_ENTERSIZEMOVE:

        break;
    case WM_EXITSIZEMOVE:
        break;
    }
    return 0;
}

/*
 * Setup Vulkan
 */
bool RendererVulkan::CreateInstance()
{
    // Use validation layers if this is a debug build, and use WSI extensions regardless
    std::vector<const char*> extensions = getAvailableWSIExtensions();
    
//#if defined(_DEBUG)
//	std::vector<const char*> layers;
//    layers.push_back("VK_LAYER_LUNARG_standard_validation");
//#endif

    // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
    // program, which can be useful for layers and tools to provide more debug information.
    vk::ApplicationInfo appInfo = vk::ApplicationInfo()
                                      .setPApplicationName("m3d example")
                                      .setApplicationVersion(1)
                                      .setPEngineName("m3d")
                                      .setEngineVersion(1)
                                      .setApiVersion(VK_API_VERSION_1_0);

    // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
    // are needed.
    vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
                                          .setFlags(vk::InstanceCreateFlags())
                                          .setPApplicationInfo(&appInfo)
                                          .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
                                          .setPpEnabledExtensionNames(extensions.data())
//#if defined(_DEBUG)
//                                          .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
//                                          .setPpEnabledLayerNames(layers.data())
//#endif
		;

    // Create the Vulkan instance.
    try {
        instance = vk::createInstance(instInfo);
    } catch (const std::exception& e) {
        std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool RendererVulkan::CreateDevice()
{
    // Create physical device
    std::vector<vk::PhysicalDevice> physicalDevices;

    try {
        physicalDevices = instance.enumeratePhysicalDevices();
    } catch (const std::exception& e) {
        std::cout << "Failed to create physical device: " << e.what() << std::endl;
        instance.destroy();
        return 1;
    }

    physicalDevice = physicalDevices[0];
    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures = physicalDevice.getFeatures();

    // TODO:
    // check queue family from a physical device support swapchain
    //physicalDevice.getSurfaceSupportKHR();

    // Find a queue that supports graphics operations
    graphicsQueueIndex = vkhelper::findQueue(physicalDevice, vk::QueueFlagBits::eGraphics);
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
    if (vkhelper::checkDeviceExtensionPresent(physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    if (enabledExtensions.size() > 0) {
        deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }

    // Create Vulkan Device
    device = physicalDevice.createDevice(deviceCreateInfo);

    swapChain.connect(instance, physicalDevice, device);

    queue = device.getQueue(graphicsQueueIndex, 0);

    // SubmitInfo
    vk::SemaphoreCreateInfo semaphoreCreateInfo;
    presentComplete = device.createSemaphore(semaphoreCreateInfo);
    renderComplete = device.createSemaphore(semaphoreCreateInfo);

    //submitInfo = vkTools::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderComplete;

    return true;
}

/*************** Swapchain ****************/
void RendererVulkan::CreateSwapChain()
{
    swapChain.initSurface(hinstance_, hwnd_);
    swapChain.create(&width, &height, false);
}

void RendererVulkan::CreatePipelineCache()
{
    //vk::PipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    //pipelineCache = device.createPipelineCache(pipelineCacheCreateInfo);
}

void RendererVulkan::InitCommon()
{
    CreateSwapChain();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateDepthStencil();
    CreateRenderPass();
    CreatePipelineCache();
    CreateFramebuffers();
}

//void RendererVulkan::SetupVertexInputs()
//{
//   
//}

bool RendererVulkan::Init(Scene* scene)
{
    if (!CreateInstance()) {
        return false;
    }

    if (!CreateDevice()) {
        return false;
    }

    this->scene = scene;

    InitCommon();

    // spec init
    {
        CreateVertices();
        SetupVertexInputs();
        CreateUniformBuffers();
		CreatePipelineLayout();
        CreatePipeline();
        CreateDescriptorPool();
        CreateDescriptorSet();
        BuildCommandBuffers();
    }

    // TODO:
    //OnWindowSizeChanged();

    return true;
}

/* Render Pass */
//bool RendererVulkan::CreateRenderPass()
//{
//    
//    return true;
//}

bool RendererVulkan::CreateDepthStencil()
{
    assert(vkhelper::getSupportedDepthFormat(physicalDevice, depthFormat));

    vk::ImageCreateInfo image = {};
    image.setSType(vk::StructureType::eImageCreateInfo);
    image.setPNext(nullptr);
    image.imageType = vk::ImageType::e2D;
    image.format = depthFormat;
    image.extent = { width, height, 1 };
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = vk::SampleCountFlagBits::e1;
    image.tiling = vk::ImageTiling::eOptimal;
    image.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;

    vk::MemoryAllocateInfo memAlloc = {};
    memAlloc.setSType(vk::StructureType::eMemoryAllocateInfo);
    memAlloc.pNext = nullptr;
    memAlloc.setAllocationSize(0);
    memAlloc.memoryTypeIndex = 0;

    vk::ImageViewCreateInfo depthStencilView = {};
    depthStencilView.setSType(vk::StructureType::eImageViewCreateInfo);
    depthStencilView.setViewType(vk::ImageViewType::e2D);
    depthStencilView.format = depthFormat;

    depthStencilView.subresourceRange = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
        0,
        1,
        0,
        1
    };

    vk::MemoryRequirements memReqs;
    depthStencil.image = device.createImage(image, nullptr);
    memReqs = device.getImageMemoryRequirements(depthStencil.image);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    depthStencil.mem = device.allocateMemory(memAlloc, nullptr);
    device.bindImageMemory(depthStencil.image, depthStencil.mem, 0);

    depthStencilView.image = depthStencil.image;

    depthStencil.view = device.createImageView(depthStencilView, nullptr);

    return true;
}

bool RendererVulkan::CreateFramebuffers()
{
    // Create frame buffers for every swap chain image
    framebuffers.resize(swapChain.images.size());

	std::array<vk::ImageView, 2> attachments;
	attachments[1] = depthStencil.view;

	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		attachments[0] = swapChain.buffers[i].view;	

		vk::FramebufferCreateInfo frameBufferCreateInfo = {};
		//frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		// All frame buffers use the same renderpass setup
		frameBufferCreateInfo.renderPass = renderPass;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;
		// Create the framebuffer
		framebuffers[i] = device.createFramebuffer(frameBufferCreateInfo);
	}

    return true;
}

//void RendererVulkan::CreatePipelineLayout()
//{
//	
//}
//
//bool RendererVulkan::CreatePipeline()
//{
//    
//    
//    return true;
//}

bool RendererVulkan::CreateBuffer(vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize size, void* data, vk::Buffer& buffer, vk::DeviceMemory& memory)
{
	vk::MemoryRequirements memReqs = {};
	vk::MemoryAllocateInfo memAlloc = {};
    vk::BufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.setUsage(usageFlags);
    bufferCreateInfo.setSize(size);

    //VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer));
    buffer = device.createBuffer(bufferCreateInfo);
    //vkGetBufferMemoryRequirements(device, buffer, &memReqs);
    memReqs = device.getBufferMemoryRequirements(buffer);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, memoryPropertyFlags);

    //VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, memory));
    memory = device.allocateMemory(memAlloc);
    if (data != nullptr) {
        void* mapped;
        //VK_CHECK_RESULT(vkMapMemory(device, *memory, 0, size, 0, &mapped));
        mapped = device.mapMemory(memory, 0, size);
        memcpy(mapped, data, size);
        //vkUnmapMemory(device, *memory);
        device.unmapMemory(memory);
    }
    //VK_CHECK_RESULT(vkBindBufferMemory(device, *buffer, *memory, 0));
    device.bindBufferMemory(buffer, memory, 0);
    return true;
}


bool RendererVulkan::CreateVertices()
{
    size_t vertexBufferSize = scene->meshes[0].vertices.size() * sizeof(float);
    size_t indexBufferSize = scene->meshes[0].indices.size() * sizeof(uint32_t);
    // TODO
    meshBuffer.indexCount = scene->meshes[0].indices.size();

    struct {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
    } vertexStaging, indexStaging;

    // Create staging buffers
    // Vertex data
    CreateBuffer(
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible,
        vertexBufferSize,
        scene->meshes[0].vertices.data(),
        vertexStaging.buffer,
        vertexStaging.memory);
    // Index data
    CreateBuffer(
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible,
        indexBufferSize,
        scene->meshes[0].indices.data(),
        indexStaging.buffer,
        indexStaging.memory);

    // Create device local buffers
    // Vertex buffer
    CreateBuffer(
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vertexBufferSize,
        nullptr,
        meshBuffer.vertices.buf,
        meshBuffer.vertices.mem);
    // Index buffer
    CreateBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        indexBufferSize,
        nullptr,
        meshBuffer.indices.buf,
        meshBuffer.indices.mem);

    // Copy from staging buffers
    vk::CommandBuffer copyCmd = CreateCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

    vk::BufferCopy copyRegion = {};

    copyRegion.size = vertexBufferSize;
    //vkCmdCopyBuffer(
    //	copyCmd,
    //	vertexStaging.buffer,
    //	skinnedMesh->meshBuffer.vertices.buf,
    //	1,
    //	&copyRegion);

    copyCmd.copyBuffer(vertexStaging.buffer, meshBuffer.vertices.buf, copyRegion);

    copyRegion.size = indexBufferSize;
    //vkCmdCopyBuffer(
    //	copyCmd,
    //	indexStaging.buffer,
    //	skinnedMesh->meshBuffer.indices.buf,
    //	1,
    //	&copyRegion);

    copyCmd.copyBuffer(indexStaging.buffer, meshBuffer.indices.buf, copyRegion);

    FlushCommandBuffer(copyCmd, queue, true);

    //vkDestroyBuffer(device, vertexStaging.buffer, nullptr);
    //vkFreeMemory(device, vertexStaging.memory, nullptr);
    //vkDestroyBuffer(device, indexStaging.buffer, nullptr);
    //vkFreeMemory(device, indexStaging.memory, nullptr);
    device.destroyBuffer(vertexStaging.buffer);
    device.freeMemory(vertexStaging.memory);
    device.destroyBuffer(indexStaging.buffer);
    device.freeMemory(indexStaging.memory);

    return true;
}
//
//bool RendererVulkan::CreateUniformBuffers()
//{
//    
//    return true;
//}

//bool RendererVulkan::CreateDescriptorSet()
//{
//    
//    return true;
//}

bool RendererVulkan::CreateCommandPool()
{
    vk::CommandPoolCreateInfo cmdPoolInfo;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    cmdPool = device.createCommandPool(cmdPoolInfo);
    return true;
}



bool RendererVulkan::OnWindowSizeChanged()
{
    if (!inited) {
        return false;
    }
    inited = false;

    // Recreate swapChain
    CreateSwapChain();
    // Recreate framebuffers
    device.destroyImageView(depthStencil.view);
    device.destroyImage(depthStencil.image);
    device.freeMemory(depthStencil.mem);
    CreateDepthStencil();

    for (uint32_t i = 0; i < framebuffers.size(); i++) {
        device.destroyFramebuffer(framebuffers[i]);
    }
    CreateFramebuffers();

    DestroyCommandBuffers();
    CreateCommandBuffers();
    BuildCommandBuffers();

    queue.waitIdle();
    device.waitIdle();

    inited = true;
}

void RendererVulkan::PrepareFrame()
{
    swapChain.acquireNextImage(presentComplete, &currentImage);
}

void RendererVulkan::SubmitFrame()
{
    swapChain.queuePresent(queue, currentImage, renderComplete);
    //queue.waitIdle();
}

/* Draw Loop */
void RendererVulkan::Draw()
{
    PrepareFrame();

    //device.waitForFences(1, &vk::Fence()/*&waitFences[currentImage]*/, true, UINT64_MAX);
    //device.resetFences(1, &vk::Fence() /*&waitFences[currentImage]*/);

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffers[currentImage];
    queue.submit(submitInfo, vk::Fence()/*waitFences[currentImage]*/);

    SubmitFrame();
}

void RendererVulkan::DrawLoop()
{
    while (1) {
        auto tStart = std::chrono::high_resolution_clock::now();
        Draw();
        frameCounter++;

        auto tEnd = std::chrono::high_resolution_clock::now();
        double tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        //frameTimer = (float)tDiff / 1000.0f;
        printf("%f\n", tDiff);
    }
    device.waitIdle();
}

RendererVulkan::~RendererVulkan()
{
    device.destroyPipeline(pipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyDescriptorSetLayout(descriptorSetLayout);

    // TODO: destroy texture, Mesh resources
}
