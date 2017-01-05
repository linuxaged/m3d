#pragma once
#include <vulkan/vulkan.hpp>

namespace m3d {
	class Pipeline
	{
	public:
		Pipeline(vk::Device&);
		~Pipeline();
		// create uniform buffer
		void CreateUniformBuffers();
		// create render pass
		void CreateRenderPass();
		// set vertex data format
		void SetupVertexInputs();
		//
		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSet();
		// shader
		vk::PipelineShaderStageCreateInfo loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage);

	private:
		vk::Device							&device;
		vk::Pipeline						pipeline;
		// uniform buffer
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
		// render pass
		vk::RenderPass						renderPass;
		//
		vk::DescriptorPool					descriptorPool;
		vk::DescriptorSetLayout				descriptorSetLayout;
		vk::DescriptorSet					descriptorSet;

		vk::PipelineLayout					pipelineLayout;

		struct {
			vk::PipelineVertexInputStateCreateInfo inputState;
			std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
		} vertexInputs;
	};
}