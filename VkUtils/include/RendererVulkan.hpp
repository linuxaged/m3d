/*
* Copyright (C) 2016 Tracy Ma
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <vulkan/vulkan.hpp>
#include <Matrix.h>

#define DEFAULT_FENCE_TIMEOUT 100000000000

class Scene;

class RendererVulkan
{
private:
	bool CreateInstance();

	bool CreateSurface();

	bool CreateDevice();

	bool GetDeviceQueue();

	bool CreateSemaphores();

public:
	bool Init(Scene* scene);

private:
	/* Render Pass */
	bool CreateRenderPass();

	bool CreateSwapChain();

	bool CreateDepthStencil();

	bool CreateFramebuffers();

	bool CreateCommandPool();

	bool CreatePipeline();

	bool CreateVertices();
	bool CreateUniformBuffers();
	bool CreateDescriptorPool();
	bool CreateDescriptorSet();

	bool CreateCommandBuffers();

	bool RecordCommandBuffers();

public:
	/* Window Event */
	bool OnWindowSizeChanged();

public:
	/* Draw Loop */
	bool Draw();
	void DrawLoop();
	uint32_t frameCounter;
	float frameTimer;

public:

	vk::Device				getDevice() const;
	vk::PhysicalDevice		getPhysicalDevice() const;
	vk::CommandPool			getCommandPool() const;
	vk::Queue				getQueue() const;
private:
	struct SwapChainImage {
		vk::Image image;
		vk::ImageView view;
		vk::Fence fence;
	};
	/* Utils */
	std::vector<const char*> RendererVulkan::getAvailableWSIExtensions();
	void createWin32Window();
	vk::SurfaceKHR createVulkanSurface();
	vk::PipelineShaderStageCreateInfo loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage);

	/* Windows window */
	static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		RendererVulkan *renderer = reinterpret_cast<RendererVulkan *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		// called from constructor, CreateWindowEx specifically.  But why?
		if (!renderer)
			renderer->handle_message(uMsg, wParam, lParam);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam);
	
	HINSTANCE hinstance_;
	HWND hwnd_;
	HMODULE hmodule_;

private:
	vk::Instance						instance;
	vk::SurfaceKHR						surface;
	vk::PhysicalDevice					physicalDevice;
	vk::Device							device;
	vk::Queue							queue;
	uint32_t							graphicsQueueIndex;

	vk::Semaphore						presentComplete;
	vk::Semaphore						renderComplete;

	vk::SwapchainKHR					swapChain;
	vk::SwapchainKHR					oldSwapChain;
	std::vector<SwapChainImage>			images;
	uint32_t							imageCount;
	/* Descriptor Set */
	vk::DescriptorPool					descriptorPool;
	vk::DescriptorSet					descriptorSet;
	vk::DescriptorSetLayout				descriptorSetLayout;
	/* Command Buffer */
	vk::CommandPool						cmdPool;
	std::vector<vk::CommandBuffer>		cmdBuffers;
	/* Vertex Data */
	struct {
		vk::Buffer buffer;
		vk::DeviceMemory memory;
	} vertices;

	struct {
		vk::Buffer buffer;
		vk::DeviceMemory memory;
	} indices;

	struct {
		vk::Buffer buffer;
		vk::DeviceMemory memory;
		vk::DescriptorBufferInfo descriptor;
	}  uniformDataVS;

	struct {
#ifdef USE_GLM
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
#else
		m3d::math::Matrix4x4 projectionMatrix;
		m3d::math::Matrix4x4 modelMatrix;
		m3d::math::Matrix4x4 viewMatrix;
#endif
	} uboVS;

	uint32_t							indexCount;
	vk::PipelineVertexInputStateCreateInfo inputState;
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
	/* Render Pass */
	struct
	{
		vk::Image image;
		vk::DeviceMemory mem;
		vk::ImageView view;
	} depthStencil;
	vk::Format							depthFormat;

	uint32_t							currentImage;
	vk::PresentInfoKHR					presentInfo;
	std::vector<vk::ShaderModule>		shaderModules;
	vk::RenderPass						renderPass;
	std::vector<vk::Framebuffer>		framebuffers;
	vk::Pipeline						pipeline;
	vk::PipelineLayout					pipelineLayout;

private:
	Scene* scene;
};