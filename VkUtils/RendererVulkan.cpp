#include "RendererVulkan.h"
#include "Matrix.h"
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_syswm.h>

#include <iostream>

#define VERTEX_BUFFER_BIND_ID 0
#define DEFAULT_FENCE_TIMEOUT 100000000000
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

void RendererVulkan::createWin32Window()
{
	const std::string class_name("RendererVulkanWindowClass");

	hinstance_ = GetModuleHandle(nullptr);

	WNDCLASSEX win_class = {};
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = window_proc;
	win_class.hInstance = hinstance_;
	win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	win_class.lpszClassName = class_name.c_str();
	RegisterClassEx(&win_class);

	const DWORD win_style =
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_OVERLAPPEDWINDOW;

	RECT win_rect = { 0, 0, 1280, 720 };
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

	SetForegroundWindow(hwnd_);
	SetWindowLongPtr(hwnd_, GWLP_USERDATA, (LONG_PTR) this);
}

LRESULT RendererVulkan::handle_message(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_SIZE:
	{
		UINT w = LOWORD(lparam);
		UINT h = HIWORD(lparam);
		// TODO:
		//resize_swapchain(w, h);
		OnWindowSizeChanged();
	}
	break;
	case WM_KEYDOWN:
	{
		//Game::Key key;

		switch (wparam) {
		case VK_ESCAPE:
			//key = Game::KEY_ESC;
			break;
		case VK_UP:
			//key = Game::KEY_UP;
			break;
		case VK_DOWN:
			//key = Game::KEY_DOWN;
			break;
		case VK_SPACE:
			//key = Game::KEY_SPACE;
			break;
		default:
			//key = Game::KEY_UNKNOWN;
			break;
		}

		//game_.on_key(key);
	}
	break;
	case WM_CLOSE:
		//game_.on_key(Game::KEY_SHUTDOWN);
		break;
	case WM_DESTROY:
		//quit();
		break;
	default:
		return DefWindowProc(hwnd_, msg, wparam, lparam);
		break;
	}

	return 0;
}

vk::SurfaceKHR RendererVulkan::createVulkanSurface()
{
	createWin32Window();

	vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(hinstance_)
		.setHwnd(hwnd_);
	return instance.createWin32SurfaceKHR(surfaceInfo);
//
//	SDL_SysWMinfo windowInfo;
//	SDL_VERSION(&windowInfo.version);
//	if (!SDL_GetWindowWMInfo(window, &windowInfo)) {
//		throw std::system_error(std::error_code(), "SDK window manager info is not available.");
//	}
//
//	switch (windowInfo.subsystem) {
//
//#if defined(SDL_VIDEO_DRIVER_ANDROID) && defined(VK_USE_PLATFORM_ANDROID_KHR)
//	case SDL_SYSWM_ANDROID: {
//		vk::AndroidSurfaceCreateInfoKHR surfaceInfo = vk::AndroidSurfaceCreateInfoKHR()
//			.setWindow(windowInfo.info.android.window);
//		return instance.createAndroidSurfaceKHR(surfaceInfo);
//	}
//#endif
//
//#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
//	case SDL_SYSWM_MIR: {
//		vk::MirSurfaceCreateInfoKHR surfaceInfo = vk::MirSurfaceCreateInfoKHR()
//			.setConnection(windowInfo.info.mir.connection)
//			.setMirSurface(windowInfo.info.mir.surface);
//		return instance.createMirSurfaceKHR(surfaceInfo);
//	}
//#endif
//
//#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
//	case SDL_SYSWM_WAYLAND: {
//		vk::WaylandSurfaceCreateInfoKHR surfaceInfo = vk::WaylandSurfaceCreateInfoKHR()
//			.setDisplay(windowInfo.info.wl.display)
//			.setSurface(windowInfo.info.wl.surface);
//		return instance.createWaylandSurfaceKHR(surfaceInfo);
//	}
//#endif
//
//#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
//	case SDL_SYSWM_WINDOWS: {
//		vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
//			.setHinstance(GetModuleHandle(NULL))
//			.setHwnd(windowInfo.info.win.window);
//		return instance.createWin32SurfaceKHR(surfaceInfo);
//	}
//#endif
//
//#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
//	case SDL_SYSWM_X11: {
//		vk::XlibSurfaceCreateInfoKHR surfaceInfo = vk::XlibSurfaceCreateInfoKHR()
//			.setDpy(windowInfo.info.x11.display)
//			.setWindow(windowInfo.info.x11.window);
//		return instance.createXlibSurfaceKHR(surfaceInfo);
//	}
//#endif
//
//	default:
//		throw std::system_error(std::error_code(), "Unsupported window manager is in use.");
//	}
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

vk::Bool32 getMemoryType(vk::PhysicalDevice& device, uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t * typeIndex) {
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			if ((device.getMemoryProperties().memoryTypes[i].propertyFlags & properties) == properties) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

uint32_t getMemoryType(vk::PhysicalDevice& device, uint32_t typeBits, const vk::MemoryPropertyFlags& properties) {
	uint32_t result = 0;
	if (!getMemoryType(device, typeBits, properties, &result)) {
		// todo : throw error
	}
	return result;
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
	// Create a Vulkan surface for rendering
	try {
		surface = createVulkanSurface();
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
	graphicsQueueIndex = findQueue(physicalDevice, vk::QueueFlagBits::eGraphics);
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

	queue = device.getQueue(graphicsQueueIndex, 0);
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
	if (!CreateSwapChain())
	{
		return false;
	}
	if (!CreateSemaphores())
	{
		return false;
	}
	// TODO:
	OnWindowSizeChanged();
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
	attachments[0].format = vk::Format::eR8G8B8A8Uint;
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

	// present info
	presentInfo.setSwapchainCount(1);
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &currentImage;

	// If an existing sawp chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapChain) {
		for (uint32_t i = 0; i < imageCount; i++) {
			device.destroyImageView(images[i].view);
		}
		device.destroySwapchainKHR(oldSwapChain);
	}

	vk::ImageViewCreateInfo colorAttachmentView;
	colorAttachmentView.format = colorFormat;
	colorAttachmentView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	colorAttachmentView.subresourceRange.levelCount = 1;
	colorAttachmentView.subresourceRange.layerCount = 1;
	colorAttachmentView.viewType = vk::ImageViewType::e2D;

	// Get the swap chain images
	auto swapChainImages = device.getSwapchainImagesKHR(swapChain);
	imageCount = (uint32_t)swapChainImages.size();

	// Get the swap chain buffers containing the image and imageview
	images.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		images[i].image = swapChainImages[i];
		colorAttachmentView.image = swapChainImages[i];
		images[i].view = device.createImageView(colorAttachmentView);
		images[i].fence = vk::Fence();
	}

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

	// Verify that the first attachment is null
	assert(framebufferCreateInfo.pAttachments[0] == vk::ImageView());

	std::vector<vk::ImageView> _attachments;
	_attachments.resize(framebufferCreateInfo.attachmentCount);
	for (size_t i = 0; i < framebufferCreateInfo.attachmentCount; ++i) {
		_attachments[i] = framebufferCreateInfo.pAttachments[i];
	}
	framebufferCreateInfo.pAttachments = _attachments.data();

	imageCount = 3;
	framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		_attachments[0] = images[i].view;
		framebuffers[i] = device.createFramebuffer(framebufferCreateInfo);
	}

	return true;
}

void setupDescriptorSetLayout() {
	
}

std::vector<uint8_t> readBinaryFile(const std::string& filename)
{
	std::FILE* fp = std::fopen(filename.c_str(), "rb");
	if (fp)
	{
		std::vector<uint8_t> contents;
		std::fseek(fp, 0, SEEK_END);
		contents.resize(std::ftell(fp));
		std::rewind(fp);
		std::fread(&contents[0], 1, contents.size(), fp);
		std::fclose(fp);
		return (contents);
	}
	throw(errno);
}

vk::ShaderModule _loadShader(const std::string& filename, vk::Device device, vk::ShaderStageFlagBits stage) {
	std::vector<uint8_t> binaryData = readBinaryFile(filename);
	vk::ShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.codeSize = binaryData.size();
	moduleCreateInfo.pCode = (uint32_t*)binaryData.data();
	return device.createShaderModule(moduleCreateInfo);
}

vk::PipelineShaderStageCreateInfo RendererVulkan::loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage) {
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

	vk::DescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout(descriptorLayout, NULL);

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
	shaderStages[0] = loadShader("D:\\workspace\\m3d\\data\\shaders\\camera\\triangle.vert", vk::ShaderStageFlagBits::eVertex);
	shaderStages[1] = loadShader("D:\\workspace\\m3d\\data\\shaders\\camera\\triangle.frag", vk::ShaderStageFlagBits::eFragment);

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
	vk::PipelineCache pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());
	pipeline = device.createGraphicsPipelines(pipelineCache, pipelineCreateInfo, nullptr)[0];
	return true;
}

bool RendererVulkan::CreateVertices()
{
	struct Vertex {
		float pos[3];
		float col[3];
	};

	// Setup vertices
	std::vector<Vertex> vertexBuffer = {
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};
	uint32_t vertexBufferSize = (uint32_t)(vertexBuffer.size() * sizeof(Vertex));

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
	uint32_t indexBufferSize = (uint32_t)(indexBuffer.size() * sizeof(uint32_t));
	indexCount = (uint32_t)indexBuffer.size();

	vk::MemoryAllocateInfo memAlloc;
	vk::MemoryRequirements memReqs;

	void *data;

	// Static data like vertex and index buffer should be stored on the device memory 
	// for optimal (and fastest) access by the GPU
	//
	// To achieve this we use so-called "staging buffers" :
	// - Create a buffer that's visible to the host (and can be mapped)
	// - Copy the data to this buffer
	// - Create another buffer that's local on the device (VRAM) with the same size
	// - Copy the data from the host to the device using a command buffer
	// - Delete the host visible (staging) buffer
	// - Use the device local buffers for rendering

	struct StagingBuffer {
		vk::DeviceMemory memory;
		vk::Buffer buffer;
	};

	struct {
		StagingBuffer vertices;
		StagingBuffer indices;
	} stagingBuffers;

	// vk::Buffer copies are done on the queue, so we need a command buffer for them
	vk::CommandBufferAllocateInfo cmdBufInfo;
	cmdBufInfo.commandPool = cmdPool;
	cmdBufInfo.level = vk::CommandBufferLevel::ePrimary;
	cmdBufInfo.commandBufferCount = 1;

	vk::CommandBuffer copyCommandBuffer = device.allocateCommandBuffers(cmdBufInfo)[0];

	// Vertex buffer
	vk::BufferCreateInfo vertexBufferInfo;
	vertexBufferInfo.size = vertexBufferSize;
	// vk::Buffer is used as the copy source
	vertexBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	// Create a host-visible buffer to copy the vertex data to (staging buffer)
	stagingBuffers.vertices.buffer = device.createBuffer(vertexBufferInfo);
	memReqs = device.getBufferMemoryRequirements(stagingBuffers.vertices.buffer);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
	stagingBuffers.vertices.memory = device.allocateMemory(memAlloc);
	// Map and copy
	data = device.mapMemory(stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, vk::MemoryMapFlags());
	memcpy(data, vertexBuffer.data(), vertexBufferSize);
	device.unmapMemory(stagingBuffers.vertices.memory);
	device.bindBufferMemory(stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0);

	// Create the destination buffer with device only visibility
	// vk::Buffer will be used as a vertex buffer and is the copy destination
	vertexBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	vertices.buffer = device.createBuffer(vertexBufferInfo);
	memReqs = device.getBufferMemoryRequirements(vertices.buffer);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
	vertices.memory = device.allocateMemory(memAlloc);
	device.bindBufferMemory(vertices.buffer, vertices.memory, 0);

	// Index buffer
	vk::BufferCreateInfo indexbufferInfo;
	indexbufferInfo.size = indexBufferSize;
	indexbufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	// Copy index data to a buffer visible to the host (staging buffer)
	stagingBuffers.indices.buffer = device.createBuffer(indexbufferInfo);
	memReqs = device.getBufferMemoryRequirements(stagingBuffers.indices.buffer);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
	stagingBuffers.indices.memory = device.allocateMemory(memAlloc);
	data = device.mapMemory(stagingBuffers.indices.memory, 0, indexBufferSize, vk::MemoryMapFlags());
	memcpy(data, indexBuffer.data(), indexBufferSize);
	device.unmapMemory(stagingBuffers.indices.memory);
	device.bindBufferMemory(stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0);

	// Create destination buffer with device only visibility
	indexbufferInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	indices.buffer = device.createBuffer(indexbufferInfo);
	memReqs = device.getBufferMemoryRequirements(indices.buffer);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
	indices.memory = device.allocateMemory(memAlloc);
	device.bindBufferMemory(indices.buffer, indices.memory, 0);

	vk::CommandBufferBeginInfo cmdBufferBeginInfo;
	vk::BufferCopy copyRegion;

	// Put buffer region copies into command buffer
	// Note that the staging buffer must not be deleted before the copies 
	// have been submitted and executed
	copyCommandBuffer.begin(cmdBufferBeginInfo);

	// Vertex buffer
	copyRegion.size = vertexBufferSize;
	copyCommandBuffer.copyBuffer(stagingBuffers.vertices.buffer, vertices.buffer, copyRegion);
	// Index buffer
	copyRegion.size = indexBufferSize;
	copyCommandBuffer.copyBuffer(stagingBuffers.indices.buffer, indices.buffer, copyRegion);
	copyCommandBuffer.end();

	// Submit copies to the queue
	vk::SubmitInfo copySubmitInfo;
	copySubmitInfo.commandBufferCount = 1;
	copySubmitInfo.pCommandBuffers = &copyCommandBuffer;

	queue.submit(copySubmitInfo, vk::Fence());
	queue.waitIdle();

	device.freeCommandBuffers(cmdPool, copyCommandBuffer);

	// Destroy staging buffers
	device.destroyBuffer(stagingBuffers.vertices.buffer);
	device.freeMemory(stagingBuffers.vertices.memory);
	device.destroyBuffer(stagingBuffers.indices.buffer);
	device.freeMemory(stagingBuffers.indices.memory);

	// Binding description
	bindingDescriptions.resize(1);
	bindingDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	attributeDescriptions.resize(2);
	// Location 0 : Position
	attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = 0;
	// Location 1 : Color
	attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = sizeof(float) * 3;

	// Assign to vertex input state
	inputState.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
	inputState.pVertexBindingDescriptions = bindingDescriptions.data();
	inputState.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	inputState.pVertexAttributeDescriptions = attributeDescriptions.data();
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
	allocInfo.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
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
	uboVS.projectionMatrix = M3D::Math::Matrix4x4::Perspective(120.0f, 1.0f, 0.1f, 256.0f);
	char buf[512];
	uboVS.projectionMatrix.ToString(buf, 512);
	printf("pmat = %s\n", buf);
	float vMat[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, -10.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	uboVS.viewMatrix = M3D::Math::Matrix4x4::Translation(M3D::Math::Vector3(0.0f, 0.0f, -10.0f));
	uboVS.viewMatrix.ToString(buf, 512);
	printf("vmat = %s\n", buf);
	uboVS.modelMatrix = M3D::Math::Matrix4x4();
#endif


	// Map uniform buffer and update it
	// If you want to keep a handle to the memory and not unmap it afer updating, 
	// create the memory with the vk::MemoryPropertyFlagBits::eHostCoherent 
	void *pData = device.mapMemory(uniformDataVS.memory, 0, sizeof(uboVS), vk::MemoryMapFlags());
	// TODO:
	size_t uboSize = sizeof(uboVS);
	printf("sizeof ubo = %d", uboSize);
	memcpy(pData, &uboVS, sizeof(uboVS));
	device.unmapMemory(uniformDataVS.memory);
	return true;
}

bool RendererVulkan::CreateDescriptorSetLayout()
{
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

bool RendererVulkan::CreateCommandBuffers()
{
	vk::CommandPoolCreateInfo cmdPoolInfo;
	cmdPoolInfo.queueFamilyIndex = graphicsQueueIndex;
	cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	cmdPool = device.createCommandPool(cmdPoolInfo);

	// Create one command buffer per image in the swap chain

	// Command buffers store a reference to the
	// frame buffer inside their render pass info
	// so for static usage without having to rebuild
	// them each frame, we use one per frame buffer
	vk::CommandBufferAllocateInfo cmdBufAllocateInfo;
	cmdBufAllocateInfo.commandPool = cmdPool;
	cmdBufAllocateInfo.commandBufferCount = imageCount;
	cmdBuffers = device.allocateCommandBuffers(cmdBufAllocateInfo);

	vk::CommandBufferBeginInfo cmdBufInfo;
	vk::ClearValue clearValues[2];
	clearValues[0].color = std::array<float, 4>{0.025f, 0.025f, 0.025f, 1.0f};

	vk::RenderPassBeginInfo renderPassBeginInfo;
	vk::Extent2D size{ 1280, 720 };
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent = size;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;

	float minDepth = 0;
	float maxDepth = 1;
	vk::Viewport viewport = vk::Viewport{ 0, 0, (float)size.width, (float)size.height, minDepth, maxDepth };
	vk::Rect2D scissor = vk::Rect2D{ vk::Offset2D(), size };
	vk::DeviceSize offsets = 0;
	for (size_t i = 0; i < imageCount; ++i) {
		const auto& cmdBuffer = cmdBuffers[i];
		cmdBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		cmdBuffer.begin(cmdBufInfo);
		renderPassBeginInfo.framebuffer = framebuffers[i];
		cmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		// Update dynamic viewport state
		cmdBuffer.setViewport(0, viewport);
		// Update dynamic scissor state
		cmdBuffer.setScissor(0, scissor);
		// Bind descriptor sets describing shader binding points
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);
		// Bind the rendering pipeline (including the shaders)
		cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		// Bind triangle vertices
		cmdBuffer.bindVertexBuffers(VERTEX_BUFFER_BIND_ID, vertices.buffer, offsets);
		// Bind triangle indices
		cmdBuffer.bindIndexBuffer(indices.buffer, 0, vk::IndexType::eUint32);
		// Draw indexed triangle
		cmdBuffer.drawIndexed(indexCount, 1, 0, 0, 1);
		cmdBuffer.endRenderPass();
		cmdBuffer.end();
	}
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
	// Get next image in the swap chain (back/front buffer)
	auto resultValue = device.acquireNextImageKHR(swapChain, UINT64_MAX, presentComplete, vk::Fence());
	vk::Result result = resultValue.result;
	if (result != vk::Result::eSuccess) {
		// TODO handle eSuboptimalKHR
		std::cerr << "Invalid acquire result: " << vk::to_string(result);
		throw std::error_code(result);
	}

	currentImage = resultValue.value;

	// The submit infor strcuture contains a list of
	// command buffers and semaphores to be submitted to a queue
	// If you want to submit multiple command buffers, pass an array
	vk::PipelineStageFlags pipelineStages = vk::PipelineStageFlagBits::eBottomOfPipe;
	vk::SubmitInfo submitInfo;
	submitInfo.pWaitDstStageMask = &pipelineStages;
	// The wait semaphore ensures that the image is presented 
	// before we start submitting command buffers agein
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentComplete;
	// Submit the currently active command buffer
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffers[currentBuffer];
	// The signal semaphore is used during queue presentation
	// to ensure that the image is not rendered before all
	// commands have been submitted
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderComplete;

	// Submit to the graphics queue
	// TODO explain submit fence
	

	{
		auto& image = images[currentImage];
		while (image.fence) {
			vk::Result fenceRes = device.waitForFences(image.fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
			if (fenceRes == vk::Result::eSuccess) {
				if (false /*destroy*/) {
					device.destroyFence(image.fence);
				}
				image.fence = vk::Fence();
			}
		}

		image.fence = device.createFence(vk::FenceCreateFlags());
		queue.submit(submitInfo, image.fence);
	}
	// Present the current buffer to the swap chain
	// We pass the signal semaphore from the submit info
	// to ensure that the image is not rendered until
	// all commands have been submitted
	presentInfo.waitSemaphoreCount = renderComplete ? 1 : 0;
	presentInfo.pWaitSemaphores = &renderComplete;
	queue.presentKHR(presentInfo);
	return false;
}