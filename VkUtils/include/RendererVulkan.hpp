/*
* Copyright (C) 2017 Tracy Ma
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <Matrix.h>
#include <vulkan/vulkan.hpp>

#include "VulkanSwapchain.hpp"

#define DEFAULT_FENCE_TIMEOUT 100000000000

namespace m3d {
	class Pipeline;
	class RenderPass;
	class CommandBuffer;

	class RendererVulkan {
	public:
		~RendererVulkan();

	private:
		bool CreateInstance();

		bool CreateSurface();

		bool CreateDevice();

		bool GetDeviceQueue();

		bool CreateSemaphores();

		void CreateSwapChain();
		void CreatePipelineCache();

		bool CreateBuffer(vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize size, void* data, vk::Buffer& buffer, vk::DeviceMemory& memory);

	public:
		bool Init();

	private:


		bool CreateCommandPool();




		//void DestroyCommandBuffers();
		//void BuildCommandBuffers();
		//void CreateCommandBuffers();

		//bool RecordCommandBuffers();

		void PrepareFrame();
		void SubmitFrame();

	public:
		/* Window Event */
		bool OnWindowSizeChanged();

	public:
		/* Draw Loop */
		void Draw();
		void DrawLoop();
		uint32_t frameCounter;
		float frameTimer;

	public:
		vk::Device getDevice() const;
		vk::PhysicalDevice getPhysicalDevice() const;
		vk::CommandPool getCommandPool() const;
		vk::Queue getQueue() const;

	private:
		struct SwapChainImage {
			vk::Image image;
			vk::ImageView view;
			vk::Fence fence;
		};
		/* Utils */
		std::vector<const char*> RendererVulkan::getAvailableWSIExtensions();

		vk::SurfaceKHR createVulkanSurface();
		vk::PipelineShaderStageCreateInfo loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage);

		/* Windows window */
	public:
		void createWin32Window(HINSTANCE hinstance, WNDPROC wndproc, uint32_t width, uint32_t height);
		LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam);

	private:
		HINSTANCE hinstance_;
		HWND hwnd_;
		HMODULE hmodule_;

	private:
		vk::Instance instance;
		vk::SurfaceKHR surface;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue queue;
		uint32_t graphicsQueueIndex;

		vk::Semaphore presentComplete;
		vk::Semaphore renderComplete;
		std::vector<vk::Fence> waitFences;

		bool inited;
		uint32_t width, height;
		VulkanSwapChain swapChain;

		//vk::SwapchainKHR					swapChain;
		//vk::SwapchainKHR					oldSwapChain;
		std::vector<SwapChainImage> images;
		uint32_t imageCount;
		/* Descriptor Set */
		vk::DescriptorPool descriptorPool;
		vk::DescriptorSet descriptorSet;
		vk::DescriptorSetLayout descriptorSetLayout;
		/* Command Buffer */
		vk::CommandPool cmdPool;
		std::vector<vk::CommandBuffer> cmdBuffers;
		/* Vertex Data */
		struct StagingBuffer {
			vk::DeviceMemory mem;
			vk::Buffer buf;
		};

		struct {
			StagingBuffer vertices;
			StagingBuffer indices;
			uint32_t indexCount;
		} meshBuffer;

		struct {
			vk::PipelineVertexInputStateCreateInfo inputState;
			std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
		} vertexInputs;

		struct {
			vk::Buffer buffer;
			vk::DeviceMemory memory;
			vk::DescriptorBufferInfo descriptor;
		} uniformDataVS;

		struct {
			m3d::math::Matrix4x4 projectionMatrix;
			m3d::math::Matrix4x4 modelMatrix;
			m3d::math::Matrix4x4 viewMatrix;
		} uboVS;

		/* Submit */
		uint32_t currentImage;
		vk::PipelineStageFlags submitPipelineStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo;

		/* Render Pass */
		Pipeline		*pipeLine;
		CommandBuffer *commandBuffer;

	};
}
