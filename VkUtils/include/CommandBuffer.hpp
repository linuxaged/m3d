#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
#include "VulkanSwapchain.hpp"

namespace m3d {
	class CommandBuffer
	{
	public:
		CommandBuffer(vk::Device &, vk::Queue& , VulkanSwapChain &);

		uint32_t Create(vk::CommandBufferLevel level, bool begin);
		void Flush(uint32_t index);
		void Build();

	private:
		~CommandBuffer();

		void createCommandPool();

	private:
		vk::Device						&device;
		vk::Queue						&queue;
		VulkanSwapChain					&swapChain;
		vk::CommandPool					cmdPool;
		std::vector<vk::CommandBuffer> tempCmdBuffers;
		std::vector<vk::CommandBuffer> drawCmdBuffers;
	};
}