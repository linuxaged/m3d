#include "../include/CommandBuffer.hpp"

namespace m3d {
CommandBuffer::CommandBuffer(vk::Device& Device, vk::Queue& Queue, VulkanSwapChain& swapChain)
    : device(Device)
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

void CommandBuffer::Build()
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

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i) {
        renderPassBeginInfo.framebuffer = framebuffers[i];

        //VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffers[i], &cmdBufInfo));
        drawCmdBuffers[i].begin(cmdBufInfo);

        //vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        drawCmdBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        drawCmdBuffers[i].setViewport(0, vk::Viewport{ (float)width, (float)height, 0.0f, 1.0f });
        drawCmdBuffers[i].setScissor(0, { vk::Rect2D{ (0, 0), (width, height) } });

        vk::DeviceSize offsets[1] = { 0 };

        drawCmdBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);
        drawCmdBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

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
}