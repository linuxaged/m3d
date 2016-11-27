#include <vulkan/vulkan.hpp>

class RendererVulkan
{
private:
	bool CreateInstance();

	bool CreateSurface();

	bool CreateDevice();

	bool GetDeviceQueue();

	bool CreateSemaphores();

public:
	bool SetupVulkan();

private:
	/* Render Pass */
	bool CreateRenderPass();

	bool CreateSwapChain();

	bool CreateFramebuffers();

	bool CreatePipeline();

	bool CreateCommandBuffers();

	bool RecordCommandBuffers();

public:
	/* Window Event */
	bool OnWindowSizeChanged();

public:
	/* Draw Loop */
	bool Draw();

private:
	/* Utils */
	std::vector<const char*> RendererVulkan::getAvailableWSIExtensions();
	vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
private:
	vk::Instance						instance;
	vk::SurfaceKHR						surface;
	vk::PhysicalDevice					physicalDevice;
	vk::Device							device;

	vk::Semaphore						presentComplete;
	vk::Semaphore						renderComplete;

	vk::SwapChain						swapChain;
	vk::RenderPass						renderPass;
	std::vector<vk::Framebuffer>		framebuffers;
	vk::Pipeline						pipeline;
};