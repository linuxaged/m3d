#include "RendererVulkan.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <iostream>

/*
 * Utils
 */
std::vector<const char*> RendererVulkan::getAvailableWSIExtensions()
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

vk::SurfaceKHR RendererVulkan::createVulkanSurface(const vk::Instance& instance, SDL_Window* window)
{
	SDL_SysWMinfo windowInfo;
	SDL_VERSION(&windowInfo.version);
	if (!SDL_GetWindowWMInfo(window, &windowInfo)) {
		throw std::system_error(std::error_code(), "SDK window manager info is not available.");
	}

	switch (windowInfo.subsystem) {

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

/*
 * Setup Vulkan
 */
bool RendererVulkan::CreateInstance()
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
	try {
		instance = vk::createInstance(instInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
		return false;
	}
	return true;
}

bool RendererVulkan::CreateSurface()
{
	// Create an SDL window that supports Vulkan and OpenGL rendering.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Could not initialize SDL." << std::endl;
		return 1;
	}
	SDL_Window* window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		std::cout << "Could not create SDL window." << std::endl;
		return 1;
	}

	// Create a Vulkan surface for rendering
	try {
		surface = createVulkanSurface(instance, window);
	}
	catch (const std::exception& e) {
		std::cout << "Failed to create Vulkan surface: " << e.what() << std::endl;
		instance.destroy();
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
	}
	catch (const std::exception& e)
	{
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

	// Create Vulkan Device
	device = physicalDevice.createDevice(deviceCreateInfo);
	return true;
}

bool RendererVulkan::GetDeviceQueue()
{
	// TODO:
	return true;
}

bool RendererVulkan::CreateSemaphores()
{
	// Create semaphores
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	presentComplete = device.createSemaphore(semaphoreCreateInfo);
	renderComplete = device.createSemaphore(semaphoreCreateInfo);
	return true;
}

bool RendererVulkan::SetupVulkan()
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
bool RendererVulkan::CreateRenderPass()
{
	if (renderPass) {
		device.destroyRenderPass(renderPass);
	}

	std::array<vk::AttachmentDescription, 1> attachments;
	std::array<vk::AttachmentReference, 1> attachmentReferences;

	// Color attachment
	attachments[0].format = swapChain.colorFormat;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// Only one depth attachment, so put it first in the references
	vk::AttachmentReference& colorReference = attachmentReferences[0];
	colorReference.attachment = 0;
	colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

	std::array<vk::SubpassDescription, 1> subpasses;
	{
		vk::SubpassDescription& subpass = subpasses[0];
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = attachmentReferences.data();
	}

	std::array<vk::SubpassDependency, 1> subpassDependencies;
	{
		vk::SubpassDependency& dependency = subpassDependencies[0];
		dependency.srcSubpass = 0;
		dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	}

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

bool RendererVulkan::CreateSwapChain()
{
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

	swapChain = device.createSwapchainKHR(swapchainCI);

	return true;
}

bool RendererVulkan::CreateFramebuffers()
{
	std::array<vk::ImageView, 1> attachments;

	vk::FramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = attachments.size();
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = 1280;//size.width;
	framebufferCreateInfo.height = 720;// size.height;
	framebufferCreateInfo.layers = 1;

	// Create frame buffers for every swap chain image
	framebuffers = swapChain.createFramebuffers(framebufferCreateInfo);

	// Verify that the first attachment is null
	assert(framebufferCreateInfo.pAttachments[0] == vk::ImageView());

	std::vector<vk::Framebuffer> framebuffers;
	// TODO:
	const size_t imageCount = 3;
	framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		framebuffers[i] = device.createFramebuffer(framebufferCreateInfo);
	}

	return true;
}

bool RendererVulkan::CreatePipeline()
{
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
	// This pipeline renders vertex data as triangle lists
	inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

	// Rasterization state
	vk::PipelineRasterizationStateCreateInfo rasterizationState;
	// Solid polygon mode
	rasterizationState.polygonMode = vk::PolygonMode::eFill;
	// No culling
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
	multisampleState.pSampleMask = NULL;
	// No multi sampling used in this example
	multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;

	// Load shaders
	// Shaders are loaded from the SPIR-V format, which can be generated from glsl
	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = loadShader(vkx::getAssetPath() + "shaders/triangle/triangle.vert", vk::ShaderStageFlagBits::eVertex);
	shaderStages[1] = loadShader(vkx::getAssetPath() + "shaders/triangle/triangle.frag", vk::ShaderStageFlagBits::eFragment);

	// Assign states
	// Assign pipeline state create information
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	// Create rendering pipeline
	pipeline = device.createGraphicsPipelines(pipelineCache, pipelineCreateInfo, nullptr)[0];
	return true;
}

bool RendererVulkan::CreateCommandBuffers()
{
	return true;
}

bool RendererVulkan::RecordCommandBuffers()
{
	return true;
}

bool RendererVulkan::OnWindowSizeChanged()
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
bool RendererVulkan::Draw()
{

	return false;
}