/*
* Copyright (C) 2017 Tracy Ma
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "RendererVulkan.hpp"
#include "File.hpp"
#include "Matrix.h"
#include "Scene.hpp"
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

void RendererVulkan::SetupVertexInputs()
{
    // Binding description
    vertexInputs.bindingDescriptions.resize(1);
    vertexInputs.bindingDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    vertexInputs.bindingDescriptions[0].stride = sizeof(float) * 4;
    vertexInputs.bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

    // Attribute descriptions
    // Describes memory layout and shader positions
    vertexInputs.attributeDescriptions.resize(1);
    // Location 0 : Position
    vertexInputs.attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    vertexInputs.attributeDescriptions[0].location = 0;
    vertexInputs.attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
    vertexInputs.attributeDescriptions[0].offset = 0;
    //vkTools::initializers::vertexInputAttributeDescription(
    //	VERTEX_BUFFER_BIND_ID,
    //	0,
    //	VK_FORMAT_R32G32B32_SFLOAT,
    //	0);

    // Location 1 : Normal
    // Location 2 : Texture coordinates
    // Location 3 : Color
    // Location 4 : Bone weights
    // Location 5 : Bone IDs

    vertexInputs.inputState.vertexBindingDescriptionCount = vertexInputs.bindingDescriptions.size();
    vertexInputs.inputState.pVertexBindingDescriptions = vertexInputs.bindingDescriptions.data();
    vertexInputs.inputState.vertexAttributeDescriptionCount = vertexInputs.attributeDescriptions.size();
    vertexInputs.inputState.pVertexAttributeDescriptions = vertexInputs.attributeDescriptions.data();
}

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
bool RendererVulkan::CreateRenderPass()
{
    if (renderPass) {
        device.destroyRenderPass(renderPass);
    }

    std::array<vk::AttachmentDescription, 2> attachments;

    // Color attachment
    attachments[0].format = vk::Format::eB8G8R8A8Unorm;
    attachments[0].samples = vk::SampleCountFlagBits::e1;
    attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
    // Depth attachment
    attachments[1].format = depthFormat;
    attachments[1].samples = vk::SampleCountFlagBits::e1;
    attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout = vk::ImageLayout::eUndefined;
    attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    std::array<vk::SubpassDescription, 1> subpasses;

    subpasses[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &colorReference;
    subpasses[0].pDepthStencilAttachment = &depthReference;

    std::array<vk::SubpassDependency, 2> subpassDependencies;

    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
    subpassDependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    subpassDependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    subpassDependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpassDependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
    subpassDependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    subpassDependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    subpassDependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpassDependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();

    renderPass = device.createRenderPass(renderPassInfo);
    return true;
}

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
    std::array<vk::ImageView, 2> attachments;

    attachments[1] = depthStencil.view;

    vk::FramebufferCreateInfo framebufferCreateInfo;

    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = width;
    framebufferCreateInfo.height = height;
    framebufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    framebuffers.resize(swapChain.images.size());
    for (uint32_t i = 0; i < framebuffers.size(); i++) {
        attachments[0] = swapChain.buffers[i].view;
        framebuffers[i] = device.createFramebuffer(framebufferCreateInfo);
    }

    return true;
}

vk::ShaderModule _loadShader(const std::string& filename, vk::Device device, vk::ShaderStageFlagBits stage)
{
    std::vector<uint8_t> binaryData; // = readBinaryFile(filename);
    m3d::file::readBinary(filename.c_str(), binaryData);
    vk::ShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.codeSize = binaryData.size();
    moduleCreateInfo.pCode = (uint32_t*)binaryData.data();
    return device.createShaderModule(moduleCreateInfo);
}

vk::PipelineShaderStageCreateInfo RendererVulkan::loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage)
{
    vk::PipelineShaderStageCreateInfo shaderStage;
    shaderStage.stage = stage;
#if defined(__ANDROID__)
    shaderStage.module = _loadShader(androidApp->activity->assetManager, fileName.c_str(), device, stage);
#else
    shaderStage.module = _loadShader(fileName.c_str(), device, stage);
#endif
    shaderStage.pName = "main"; // todo : make param
    assert(shaderStage.module);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

bool RendererVulkan::CreatePipeline()
{
    /* Pipeline Layout */

    // Setup layout of descriptors used in this example
    // Basically connects the different shader stages to descriptors
    // for binding uniform buffers, image samplers, etc.
    // So every shader binding should map to one descriptor set layout
    // binding

    // Binding 0 : Uniform buffer (Vertex shader)
    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    layoutBinding.pImmutableSamplers = NULL;

    vk::DescriptorSetLayoutCreateInfo descriptorLayout;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    descriptorSetLayout = device.createDescriptorSetLayout(descriptorLayout, NULL);

    // Create the pipeline layout that is used to generate the rendering pipelines that
    // are based on this descriptor set layout
    // In a more complex scenario you would have different pipeline layouts for different
    // descriptor set layouts that could be reused
    vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    pipelineLayout = device.createPipelineLayout(pPipelineLayoutCreateInfo);

    // Create our rendering pipeline used in this example
    // Vulkan uses the concept of rendering pipelines to encapsulate
    // fixed states
    // This replaces OpenGL's huge (and cumbersome) state machine
    // A pipeline is then stored and hashed on the GPU making
    // pipeline changes much faster than having to set dozens of
    // states
    // In a real world application you'd have dozens of pipelines
    // for every shader set used in a scene
    // Note that there are a few states that are not stored with
    // the pipeline. These are called dynamic states and the
    // pipeline only stores that they are used with this pipeline,
    // but not their states

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    // The layout used for this pipeline
    pipelineCreateInfo.layout = pipelineLayout;
    // Renderpass this pipeline is attached to
    pipelineCreateInfo.renderPass = renderPass;

    // Vertex input state
    // Describes the topoloy used with this pipeline
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
    inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

    // Rasterization state
    vk::PipelineRasterizationStateCreateInfo rasterizationState;
    rasterizationState.polygonMode = vk::PolygonMode::eFill;
    rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
    rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.lineWidth = 1.0f;

    // Color blend state
    // Describes blend modes and color masks
    vk::PipelineColorBlendStateCreateInfo colorBlendState;
    // One blend attachment state
    // Blending is not used in this example
    vk::PipelineColorBlendAttachmentState blendAttachmentState[1] = {};
    blendAttachmentState[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    blendAttachmentState[0].blendEnable = VK_FALSE;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = blendAttachmentState;

    // vk::Viewport state
    vk::PipelineViewportStateCreateInfo viewportState;
    // One viewport
    viewportState.viewportCount = 1;
    // One scissor rectangle
    viewportState.scissorCount = 1;

    // Enable dynamic states
    // Describes the dynamic states to be used with this pipeline
    // Dynamic states can be set even after the pipeline has been created
    // So there is no need to create new pipelines just for changing
    // a viewport's dimensions or a scissor box
    vk::PipelineDynamicStateCreateInfo dynamicState;
    // The dynamic state properties themselves are stored in the command buffer
    std::vector<vk::DynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(vk::DynamicState::eViewport);
    dynamicStateEnables.push_back(vk::DynamicState::eScissor);
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = dynamicStateEnables.size();

    // Depth and stencil state
    // Describes depth and stenctil test and compare ops
    vk::PipelineDepthStencilStateCreateInfo depthStencilState;
    // Basic depth compare setup with depth writes and depth test enabled
    // No stencil used
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp = vk::StencilOp::eKeep;
    depthStencilState.back.passOp = vk::StencilOp::eKeep;
    depthStencilState.back.compareOp = vk::CompareOp::eAlways;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = depthStencilState.back;

    // Multi sampling state
    vk::PipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.pSampleMask = nullptr;
    multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;

    // Load shaders
    // Shaders are loaded from the SPIR-V format, which can be generated from glsl
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = loadShader("G:\\workspace\\m3d\\data\\shaders\\camera\\triangle.vert.spv", vk::ShaderStageFlagBits::eVertex);
    shaderStages[1] = loadShader("G:\\workspace\\m3d\\data\\shaders\\camera\\triangle.frag.spv", vk::ShaderStageFlagBits::eFragment);

    // Assign states
    // Assign pipeline state create information
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputs.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Create rendering pipeline
    // TODO: release
    vk::PipelineCache pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());
    pipeline = device.createGraphicsPipelines(pipelineCache, pipelineCreateInfo, nullptr)[0];
    return true;
}

bool RendererVulkan::CreateBuffer(vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize size, void* data, vk::Buffer& buffer, vk::DeviceMemory& memory)
{
    vk::MemoryRequirements memReqs;
    vk::MemoryAllocateInfo memAlloc;
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

vk::CommandBuffer RendererVulkan::CreateCommandBuffer(vk::CommandBufferLevel level, bool begin)
{
    std::vector<vk::CommandBuffer> _cmdBuffers(1);

    vk::CommandBufferAllocateInfo cmdBufAllocateInfo = {};
    cmdBufAllocateInfo.commandPool = cmdPool;
    cmdBufAllocateInfo.level = level;
    cmdBufAllocateInfo.commandBufferCount = 1;

    //VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &cmdBuffer));
    _cmdBuffers = device.allocateCommandBuffers(cmdBufAllocateInfo);
    // If requested, also start the new command buffer
    if (begin) {
        vk::CommandBufferBeginInfo cmdBufInfo = {};
        //VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
        _cmdBuffers[0].begin(cmdBufInfo);
    }

    return _cmdBuffers[0];
}

void RendererVulkan::FlushCommandBuffer(vk::CommandBuffer commandBuffer, vk::Queue queue, bool free)
{
    if (!commandBuffer) {
        return;
    }

    commandBuffer.end();

    vk::SubmitInfo _submitInfo = {};
    _submitInfo.commandBufferCount = 1;
    _submitInfo.pCommandBuffers = &commandBuffer;

	vk::FenceCreateInfo fenceCreateInfo = {};
	vk::Fence fence;
	fence = device.createFence(fenceCreateInfo);
	//VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &vk::Fence()));


    queue.submit(_submitInfo, fence);
    queue.waitIdle();

	device.destroyFence(fence);
    if (free) {
        device.freeCommandBuffers(cmdPool, 1, &commandBuffer);
    }
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

bool RendererVulkan::CreateUniformBuffers()
{
    // Prepare and initialize a uniform buffer block containing shader uniforms
    // In Vulkan there are no more single uniforms like in GL
    // All shader uniforms are passed as uniform buffer blocks
    vk::MemoryRequirements memReqs;

    // Vertex shader uniform buffer block
    vk::BufferCreateInfo bufferInfo;
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = 0;
    allocInfo.memoryTypeIndex = 0;
    bufferInfo.size = sizeof(uboVS);
    bufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;

    // Create a new buffer
    uniformDataVS.buffer = device.createBuffer(bufferInfo);
    // Get memory requirements including size, alignment and memory type
    memReqs = device.getBufferMemoryRequirements(uniformDataVS.buffer);
    allocInfo.allocationSize = memReqs.size;
    // Get the memory type index that supports host visibile memory access
    // Most implementations offer multiple memory tpyes and selecting the
    // correct one to allocate memory from is important
    allocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
    // Allocate memory for the uniform buffer
    (uniformDataVS.memory) = device.allocateMemory(allocInfo);
    // Bind memory to buffer
    device.bindBufferMemory(uniformDataVS.buffer, uniformDataVS.memory, 0);

    // Store information in the uniform's descriptor
    uniformDataVS.descriptor.buffer = uniformDataVS.buffer;
    uniformDataVS.descriptor.offset = 0;
    uniformDataVS.descriptor.range = sizeof(uboVS);

// Update matrices
#ifdef USE_GLM
    uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)size.width / (float)size.height, 0.1f, 256.0f);
    std::cout << "pMat: " << glm::to_string(uboVS.projectionMatrix) << std::endl;
    uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));
    std::cout << "vMat: " << glm::to_string(uboVS.viewMatrix) << std::endl;
    uboVS.modelMatrix = glm::mat4();
    std::cout << "mMat: " << glm::to_string(uboVS.modelMatrix) << std::endl;
#else
    float pMat[16] = {
        2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.22f, -2.22f,
        0.0f, 0.0f, -1.0f, 0.0f
    };
    uboVS.projectionMatrix = m3d::math::Matrix4x4::Perspective(60.0f, 1.0f, 0.1f, 256.0f);
    char buf[512];
    uboVS.projectionMatrix.ToString(buf, 512);
    printf("pmat = %s\n", buf);
    float vMat[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, -10.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    uboVS.viewMatrix = m3d::math::Matrix4x4::Translation(m3d::math::Vector3(0.0f, 0.0f, -100.0f));
    uboVS.viewMatrix.ToString(buf, 512);
    printf("vmat = %s\n", buf);
    uboVS.modelMatrix = m3d::math::Matrix4x4();
#endif

    // Map uniform buffer and update it
    // If you want to keep a handle to the memory and not unmap it afer updating,
    // create the memory with the vk::MemoryPropertyFlagBits::eHostCoherent
    void* pData = device.mapMemory(uniformDataVS.memory, 0, sizeof(uboVS), vk::MemoryMapFlags());
    // TODO:
    size_t uboSize = sizeof(uboVS);
    printf("sizeof ubo = %d", uboSize);
    memcpy(pData, &uboVS, sizeof(uboVS));
    device.unmapMemory(uniformDataVS.memory);
    return true;
}

bool RendererVulkan::CreateDescriptorPool()
{
    // We need to tell the API the number of max. requested descriptors per type
    vk::DescriptorPoolSize typeCounts[1];
    // This example only uses one descriptor type (uniform buffer) and only
    // requests one descriptor of this type
    typeCounts[0].type = vk::DescriptorType::eUniformBuffer;
    typeCounts[0].descriptorCount = 1;
    // For additional types you need to add new entries in the type count list
    // E.g. for two combined image samplers :
    // typeCounts[1].type = vk::DescriptorType::eCombinedImageSampler;
    // typeCounts[1].descriptorCount = 2;

    // Create the global descriptor pool
    // All descriptors used in this example are allocated from this pool
    vk::DescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;
    // Set the max. number of sets that can be requested
    // Requesting descriptors beyond maxSets will result in an error
    descriptorPoolInfo.maxSets = 1;

    descriptorPool = device.createDescriptorPool(descriptorPoolInfo);
    return true;
}

bool RendererVulkan::CreateDescriptorSet()
{
    // Allocate a new descriptor set from the global descriptor pool
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    descriptorSet = device.allocateDescriptorSets(allocInfo)[0];

    // Update the descriptor set determining the shader binding points
    // For every binding point used in a shader there needs to be one
    // descriptor set matching that binding point

    vk::WriteDescriptorSet writeDescriptorSet;

    // Binding 0 : Uniform buffer
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &uniformDataVS.descriptor;
    // Binds this uniform buffer to binding point 0
    writeDescriptorSet.dstBinding = 0;

    device.updateDescriptorSets(writeDescriptorSet, nullptr);
    return true;
}

bool RendererVulkan::CreateCommandPool()
{
    vk::CommandPoolCreateInfo cmdPoolInfo;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    cmdPool = device.createCommandPool(cmdPoolInfo);
    return true;
}

void RendererVulkan::DestroyCommandBuffers()
{
    device.freeCommandBuffers(cmdPool, cmdBuffers);
}

// depend on swapchain
void RendererVulkan::CreateCommandBuffers()
{
    cmdBuffers.resize(swapChain.images.size());

    vk::CommandBufferAllocateInfo cmdBufAllocateInfo;
    cmdBufAllocateInfo.commandPool = cmdPool;
    cmdBufAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdBufAllocateInfo.commandBufferCount = cmdBuffers.size();

    cmdBuffers = device.allocateCommandBuffers(cmdBufAllocateInfo);

    vk::FenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    waitFences.resize(cmdBuffers.size());
    for (auto& fence : waitFences) {
        fence = device.createFence(fenceCreateInfo);
    }
}

void RendererVulkan::BuildCommandBuffers()
{
    vk::CommandBufferBeginInfo cmdBufInfo = {};

    vk::ClearValue clearValues[2];
    std::array<float, 4> tmpColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[0].color = vk::ClearColorValue(tmpColor);
    clearValues[1].depthStencil = { 1.0f, 0 };

    vk::RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < cmdBuffers.size(); ++i) {
        renderPassBeginInfo.framebuffer = framebuffers[i];

        //VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffers[i], &cmdBufInfo));
        cmdBuffers[i].begin(cmdBufInfo);

        //vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        //std::array<vk::Viewport, 1> viewports = { vk::Viewport{ 1280.0f, 720.0f, 0.0f, 1.0f } };
        //vkCmdSetViewport(drawCmdBuffers[i], 0, 1, viewports);
        //cmdBuffers[i].setViewport(0, 1, &viewports[0]);
        cmdBuffers[i].setViewport(0, vk::Viewport{ (float)width, (float)height, 0.0f, 1.0f });
        //cmdBuffers[i].setViewport(0, viewports);

        //std::array<vk::Rect2D, 1> scissors = { vk::Rect2D{ (0, 0), ((uint32_t)1280, (uint32_t)720) } };
        //vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
        cmdBuffers[i].setScissor(0, { vk::Rect2D{ (0, 0), (width, height) } });

        vk::DeviceSize offsets[1] = { 0 };

        // Skinned mesh

        //vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
        //vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.skinning);
        cmdBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);
        cmdBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

        //vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &skinnedMesh->meshBuffer.vertices.buf, offsets);
        //vkCmdBindIndexBuffer(drawCmdBuffers[i], skinnedMesh->meshBuffer.indices.buf, 0, VK_INDEX_TYPE_UINT32);
        //vkCmdDrawIndexed(drawCmdBuffers[i], skinnedMesh->meshBuffer.indexCount, 1, 0, 0, 0);

        cmdBuffers[i].bindVertexBuffers(0, 1, &meshBuffer.vertices.buf, offsets);
        cmdBuffers[i].bindIndexBuffer(meshBuffer.indices.buf, 0, vk::IndexType::eUint32);
        cmdBuffers[i].drawIndexed(meshBuffer.indexCount, 1, 0, 0, 0);
        //vkCmdEndRenderPass(drawCmdBuffers[i]);
        cmdBuffers[i].endRenderPass();
        //VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        cmdBuffers[i].end();
    }
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

    device.waitForFences(1, &waitFences[currentImage], true, UINT64_MAX);
    device.resetFences(1, &waitFences[currentImage]);

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffers[currentImage];
    queue.submit(submitInfo, waitFences[currentImage]);

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
