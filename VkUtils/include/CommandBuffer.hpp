#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

// TODO: move into namespace ::m3d

namespace m3d {
class VulkanSwapChain;
class Scene;
class Pipeline;

class CommandBuffer {
public:
    CommandBuffer(vk::Device&, vk::PhysicalDevice&, vk::Queue&, VulkanSwapChain&);
    ~CommandBuffer();

    /* frame buffer */
    void CreateDepthStencil();
    void CreateFramebuffers(Pipeline&);

    /* Vertex */
    void CreateBuffer(vk::BufferUsageFlags, vk::MemoryPropertyFlags, vk::DeviceSize, void* data, vk::Buffer& buffer, vk::DeviceMemory& memory);
    void CreateVertices(std::vector<float>& vertices, std::vector<uint32_t> &indices);

    uint32_t Create(vk::CommandBufferLevel level, bool begin);
    void Flush(uint32_t index);
    void Build(Pipeline&);

private:
    void createCommandPool();

private:
    vk::Device& device;
    vk::PhysicalDevice& physicalDevice;
    vk::Queue& queue;
    VulkanSwapChain& swapChain;
    /* Vertices */
    struct StagingBuffer {
        vk::DeviceMemory mem;
        vk::Buffer buf;
    };
    struct {
        StagingBuffer vertices;
        StagingBuffer indices;
        uint32_t indexCount;
    } meshBuffer;

    /* frame buffers */
    std::vector<vk::Framebuffer> frameBuffers;
    struct
    {
        vk::Image image;
        vk::DeviceMemory mem;
        vk::ImageView view;
    } depthStencil;

    vk::CommandPool cmdPool;
    std::vector<vk::CommandBuffer> tempCmdBuffers;
    std::vector<vk::CommandBuffer> drawCmdBuffers;
};
}