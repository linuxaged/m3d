#include "../include/Pipeline.hpp"
#include "../include/File.hpp"
#include "../include/VulkanHelper.hpp"
#include "Matrix.h"
#define VERTEX_BUFFER_BIND_ID 0
namespace m3d {
	void Pipeline::SetupVertexInputs()
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
	void Pipeline::CreateUniformBuffers()
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
		uboVS.viewMatrix = m3d::math::Matrix4x4::Translation(m3d::math::Vector3(0.0f, 0.0f, -10.0f));
		uboVS.viewMatrix.ToString(buf, 512);
		printf("vmat = %s\n", buf);
		uboVS.modelMatrix = m3d::math::Matrix4x4();

		// Map uniform buffer and update it
		// If you want to keep a handle to the memory and not unmap it afer updating,
		// create the memory with the vk::MemoryPropertyFlagBits::eHostCoherent
		void* pData = device.mapMemory(uniformDataVS.memory, 0, sizeof(uboVS), vk::MemoryMapFlags());
		// TODO:
		size_t uboSize = sizeof(uboVS);
		printf("sizeof ubo = %d", uboSize);
		memcpy(pData, &uboVS, sizeof(uboVS));
		device.unmapMemory(uniformDataVS.memory);
	}

	void Pipeline::CreateRenderPass()
	{
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
		vk::Format depthFormat = {};
		vkhelper::getSupportedDepthFormat(physicalDevice, depthFormat);
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

		std::array<vk::SubpassDescription, 1> subpasses = { {} };

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
	}

	void Pipeline::CreateDescriptorPool()
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
	}

	void Pipeline::CreateDescriptorSetLayout()
	{
		// Setup layout of descriptors used in this example
		// Basically connects the different shader stages to descriptors
		// for binding uniform buffers, image samplers, etc.
		// So every shader binding should map to one descriptor set layout
		// binding

		// Binding 0 : Uniform buffer (Vertex shader)
		vk::DescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		layoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutCreateInfo descriptorLayout = {};
		descriptorLayout.bindingCount = 1;
		descriptorLayout.pBindings = &layoutBinding;

		descriptorSetLayout = device.createDescriptorSetLayout(descriptorLayout);
	}

	void Pipeline::CreatePipelineLayout()
	{
		// Create the pipeline layout that is used to generate the rendering pipelines that
		// are based on this descriptor set layout
		// In a more complex scenario you would have different pipeline layouts for different
		// descriptor set layouts that could be reused
		vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
		pPipelineLayoutCreateInfo.setLayoutCount = 1;
		pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

		pipelineLayout = device.createPipelineLayout(pPipelineLayoutCreateInfo);
	}

	void Pipeline::CreateDescriptorSet()
	{
		// Allocate a new descriptor set from the global descriptor pool
		vk::DescriptorSetAllocateInfo allocInfo = {};
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

	vk::PipelineShaderStageCreateInfo Pipeline::loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage)
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
		return shaderStage;
	}

	Pipeline::Pipeline(vk::Device &Device, vk::PhysicalDevice &PhysicalDevice) : device(Device), physicalDevice(PhysicalDevice)
	{
		CreateDescriptorPool();
		CreateDescriptorSetLayout();
		CreateUniformBuffers();
		CreateDescriptorSet();

		CreateRenderPass();
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

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo = {};
	    // The layout used for this pipeline
	    pipelineCreateInfo.layout = pipelineLayout;
	    // Renderpass this pipeline is attached to
	    pipelineCreateInfo.renderPass = renderPass;

	    // Vertex input state
	    // Describes the topoloy used with this pipeline
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	    inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

	    // Rasterization state
		vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
	    rasterizationState.polygonMode = vk::PolygonMode::eFill;
	    rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
	    rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;
	    rasterizationState.depthClampEnable = VK_FALSE;
	    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	    rasterizationState.depthBiasEnable = VK_FALSE;
	    rasterizationState.lineWidth = 1.0f;

	    // Color blend state
	    // Describes blend modes and color masks
		
	    // One blend attachment state
	    // Blending is not used in this example
	    vk::PipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	    blendAttachmentState[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	    blendAttachmentState[0].blendEnable = VK_FALSE;
	    
		vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.attachmentCount = 1;
	    colorBlendState.pAttachments = blendAttachmentState;

	    // vk::Viewport state
		vk::PipelineViewportStateCreateInfo viewportState = {};
	    // One viewport
	    viewportState.viewportCount = 1;
	    // One scissor rectangle
	    viewportState.scissorCount = 1;

	    // Enable dynamic states
	    // Describes the dynamic states to be used with this pipeline
	    // Dynamic states can be set even after the pipeline has been created
	    // So there is no need to create new pipelines just for changing
	    // a viewport's dimensions or a scissor box
		vk::PipelineDynamicStateCreateInfo dynamicState = {};
	    // The dynamic state properties themselves are stored in the command buffer
	    std::vector<vk::DynamicState> dynamicStateEnables;
	    dynamicStateEnables.push_back(vk::DynamicState::eViewport);
	    dynamicStateEnables.push_back(vk::DynamicState::eScissor);
	    dynamicState.pDynamicStates = dynamicStateEnables.data();
	    dynamicState.dynamicStateCount = dynamicStateEnables.size();

	    // Depth and stencil state
	    // Describes depth and stenctil test and compare ops
		vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
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
		vk::PipelineMultisampleStateCreateInfo multisampleState = {};
	    multisampleState.pSampleMask = nullptr;
	    multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;

	    // Load shaders
	    // Shaders are loaded from the SPIR-V format, which can be generated from glsl
	    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
	    shaderStages[0] = loadShader("G:\\workspace\\m3d\\data\\shaders\\camera\\triangle.vert.spv", vk::ShaderStageFlagBits::eVertex);
	    shaderStages[1] = loadShader("G:\\workspace\\m3d\\data\\shaders\\camera\\triangle.frag.spv", vk::ShaderStageFlagBits::eFragment);

	    // Assign states
	    // Assign pipeline state create information
		SetupVertexInputs();
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
	}

	Pipeline::~Pipeline()
	{
		// TODO
	}
}