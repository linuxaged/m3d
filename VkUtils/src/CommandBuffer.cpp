#include "../include/CommandBuffer.hpp"
#include "../include/Pipeline.hpp"
#include "../include/Scene.hpp"
#include "../include/VulkanHelper.hpp"
#include "../include/VulkanSwapchain.hpp"

static const uint32_t width = 1280;
static const uint32_t height = 720;

namespace m3d {
CommandBuffer::CommandBuffer(vk::Device& Device, vk::PhysicalDevice& PhysicalDevice, vk::Queue& Queue, VulkanSwapChain& swapChain)
    : device(Device)
    , physicalDevice(PhysicalDevice)
    , queue(Queue)
    , swapChain(swapChain)
{
    createCommandPool();

    drawCmdBuffers.resize(swapChain.images.size());

    vk::CommandBufferAllocateInfo cmdBufAllocateInfo = {};
    cmdBufAllocateInfo.commandPool = cmdPool;
    cmdBufAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdBufAllocateInfo.commandBufferCount = drawCmdBuffers.size();

    drawCmdBuffers = device.allocateCommandBuffers(cmdBufAllocateInfo);
}

/* Create Frame Buffer */
void CommandBuffer::CreateDepthStencil()
{
    vk::Format depthFormat;
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
}

void CommandBuffer::CreateFramebuffers(Pipeline& pipeline)
{
    // Create frame buffers for every swap chain image
    frameBuffers.resize(swapChain.images.size());

    std::array<vk::ImageView, 2> attachments;
    attachments[1] = depthStencil.view;

    for (size_t i = 0; i < frameBuffers.size(); i++) {
        attachments[0] = swapChain.buffers[i].view;

        vk::FramebufferCreateInfo frameBufferCreateInfo = {};
        //frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // All frame buffers use the same renderpass setup
        frameBufferCreateInfo.renderPass = pipeline.GetRenderPass();
        frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        frameBufferCreateInfo.pAttachments = attachments.data();
        frameBufferCreateInfo.width = width;
        frameBufferCreateInfo.height = height;
        frameBufferCreateInfo.layers = 1;
        // Create the framebuffer
        frameBuffers[i] = device.createFramebuffer(frameBufferCreateInfo);
    }
}

void CommandBuffer::CreateBuffer(vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize size, void* data, vk::Buffer& buffer, vk::DeviceMemory& memory)
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
}

void CommandBuffer::CreateVertices(std::vector<float>& vertices, std::vector<uint32_t> &indices)
{
    size_t vertexBufferSize = vertices.size() * sizeof(float);
    size_t indexBufferSize = indices.size() * sizeof(uint32_t);
    // TODO
    meshBuffer.indexCount = indices.size();

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
        vertices.data(),
        vertexStaging.buffer,
        vertexStaging.memory);
    // Index data
    CreateBuffer(
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible,
        indexBufferSize,
        indices.data(),
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
    uint32_t copyCmdIndex = Create(vk::CommandBufferLevel::ePrimary, true);

    vk::BufferCopy copyRegion = {};

    copyRegion.size = vertexBufferSize;

    tempCmdBuffers[copyCmdIndex].copyBuffer(vertexStaging.buffer, meshBuffer.vertices.buf, copyRegion);

    copyRegion.size = indexBufferSize;

    tempCmdBuffers[copyCmdIndex].copyBuffer(indexStaging.buffer, meshBuffer.indices.buf, copyRegion);

    Flush(copyCmdIndex);

    device.destroyBuffer(vertexStaging.buffer);
    device.freeMemory(vertexStaging.memory);
    device.destroyBuffer(indexStaging.buffer);
    device.freeMemory(indexStaging.memory);
}

void CommandBuffer::Build(Pipeline& pipeline)
{
    {
        CreateDepthStencil();
        CreateFramebuffers(pipeline);
    }

    vk::CommandBufferBeginInfo cmdBufInfo = {};

    vk::ClearValue clearValues[2];
    std::array<float, 4> tmpColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[0].color = vk::ClearColorValue(tmpColor);
    clearValues[1].depthStencil = { 1.0f, 0 };

    vk::RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.renderPass = pipeline.GetRenderPass();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i) {
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        //VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffers[i], &cmdBufInfo));
        drawCmdBuffers[i].begin(cmdBufInfo);

        //vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        drawCmdBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		vk::Viewport viewport = {0, 0, (float)width, (float)height, 0.0f, 1.0f};
        drawCmdBuffers[i].setViewport(0, 1, &viewport);
		vk::Rect2D rect2d = { (0, 0), (width, height) };
        drawCmdBuffers[i].setScissor(0, 1, &rect2d);

        vk::DeviceSize offsets[1] = { 0 };

        drawCmdBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, pipeline.GetDescriptorSet(), nullptr);
        drawCmdBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());

        drawCmdBuffers[i].bindVertexBuffers(0, 1, &meshBuffer.vertices.buf, offsets);
        drawCmdBuffers[i].bindIndexBuffer(meshBuffer.indices.buf, 0, vk::IndexType::eUint32);
        drawCmdBuffers[i].drawIndexed(meshBuffer.indexCount, 1, 0, 0, 0);
        drawCmdBuffers[i].endRenderPass();
        drawCmdBuffers[i].end();
    }
}

uint32_t CommandBuffer::Create(vk::CommandBufferLevel level, bool begin)
{
    vk::CommandBuffer _cmdBuffer;

    vk::CommandBufferAllocateInfo cmdBufAllocateInfo = {};
    cmdBufAllocateInfo.commandPool = cmdPool;
    cmdBufAllocateInfo.level = level;
    cmdBufAllocateInfo.commandBufferCount = 1;

    device.allocateCommandBuffers(&cmdBufAllocateInfo, &_cmdBuffer);

    if (begin) {
        vk::CommandBufferBeginInfo cmdBufInfo = {};
        _cmdBuffer.begin(cmdBufInfo);
    }

    tempCmdBuffers.emplace_back(_cmdBuffer);

    return tempCmdBuffers.size() - 1;
}

void CommandBuffer::Flush(uint32_t index)
{

    tempCmdBuffers[index].end();

    vk::SubmitInfo _submitInfo = {};
    _submitInfo.commandBufferCount = 1;
    _submitInfo.pCommandBuffers = &tempCmdBuffers[index];

    vk::FenceCreateInfo fenceCreateInfo = {};
    vk::Fence fence;
    fence = device.createFence(fenceCreateInfo);

    queue.submit(_submitInfo, fence);
    queue.waitIdle();

    device.destroyFence(fence);
    //if (free) {
    //	device.freeCommandBuffers(cmdPool, 1, &tempCmdBuffers[index]);
    //}
}

void CommandBuffer::createCommandPool()
{
    vk::CommandPoolCreateInfo cmdPoolInfo;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    cmdPool = device.createCommandPool(cmdPoolInfo);
}

CommandBuffer::~CommandBuffer()
{
    // clear temp command buffers
    for (auto& cmdbuffer : tempCmdBuffers) {
        device.freeCommandBuffers(cmdPool, cmdbuffer);
    }
    tempCmdBuffers.clear();

    // clear draw command buffers
    for (auto& cmdbuffer : drawCmdBuffers) {
        device.freeCommandBuffers(cmdPool, cmdbuffer);
    }
    drawCmdBuffers.clear();

    // destroy command pool
    device.destroyCommandPool(cmdPool);
}
} // End of namespace m3d
