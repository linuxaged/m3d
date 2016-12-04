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

	bool CreateVertices();
	bool CreateUniformBuffers();
	bool CreateDescriptorPool();
	bool CreateDescriptorSetLayout();
	bool CreateDescriptorSet();

	bool CreateCommandBuffers();

	bool RecordCommandBuffers();

public:
	/* Window Event */
	bool OnWindowSizeChanged();

public:
	/* Draw Loop */
	bool Draw();

private:
	struct SwapChainImage {
		vk::Image image;
		vk::ImageView view;
		vk::Fence fence;
	};
	/* Utils */
	std::vector<const char*> RendererVulkan::getAvailableWSIExtensions();
	vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
	vk::PipelineShaderStageCreateInfo loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage);

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
		M3D::Math::Matrix4x4 projectionMatrix;
		M3D::Math::Matrix4x4 modelMatrix;
		M3D::Math::Matrix4x4 viewMatrix;
#endif
	} uboVS;

	uint32_t							indexCount;
	vk::PipelineVertexInputStateCreateInfo inputState;
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
	/* Render Pass */
	uint32_t							currentImage;
	uint32_t							currentBuffer;
	vk::PresentInfoKHR					presentInfo;
	std::vector<vk::ShaderModule>		shaderModules;
	vk::RenderPass						renderPass;
	std::vector<vk::Framebuffer>		framebuffers;
	vk::Pipeline						pipeline;
	vk::PipelineLayout					pipelineLayout;
	vk::PipelineVertexInputStateCreateInfo inputState;
};