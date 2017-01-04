/*
* Copyright (C) 2017 Tracy Ma
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <Matrix.h>
#include <vulkan/vulkan.hpp>

#include "VulkanSwapchain.hpp"

#define DEFAULT_FENCE_TIMEOUT 100000000000

class Scene;

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
    void InitCommon();

    bool CreateBuffer(vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize size, void* data, vk::Buffer& buffer, vk::DeviceMemory& memory);
    vk::CommandBuffer CreateCommandBuffer(vk::CommandBufferLevel level, bool begin);
    void FlushCommandBuffer(vk::CommandBuffer commandBuffer, vk::Queue queue, bool free);

public:
    bool Init(Scene* scene);

private:
    /* Render Pass */
    bool CreateRenderPass();

    bool CreateDepthStencil();

    bool CreateFramebuffers();

    bool CreateCommandPool();

    void SetupVertexInputs();

    // pipeline
    bool CreateDescriptorPool();
    bool CreateDescriptorSet();
	void CreatePipelineLayout();
    bool CreatePipeline();

    bool CreateVertices();
    bool CreateUniformBuffers();

    void DestroyCommandBuffers();
    void BuildCommandBuffers();
    void CreateCommandBuffers();

    bool RecordCommandBuffers();

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
    //struct {
    //	vk::Buffer buffer;
    //	vk::DeviceMemory memory;
    //} vertices;

    //struct {
    //	vk::Buffer buffer;
    //	vk::DeviceMemory memory;
    //} indices;

    struct {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::DescriptorBufferInfo descriptor;
    } uniformDataVS;

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

    uint32_t indexCount;
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
    vk::Format depthFormat;

    uint32_t currentImage;
    vk::PipelineStageFlags submitPipelineStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo;
    vk::PresentInfoKHR presentInfo;
    std::vector<vk::ShaderModule> shaderModules;
    vk::RenderPass renderPass;
    std::vector<vk::Framebuffer> framebuffers;
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;

private:
    Scene* scene;
};