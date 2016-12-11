#include <vulkan/vulkan.hpp>

namespace vkhelper
{
	vk::Bool32 checkDeviceExtensionPresent(vk::PhysicalDevice physicalDevice, const char* extensionName) {
		uint32_t extensionCount = 0;
		std::vector<vk::ExtensionProperties> extensions = physicalDevice.enumerateDeviceExtensionProperties();
		for (auto& ext : extensions) {
			if (!strcmp(extensionName, ext.extensionName)) {
				return true;
			}
		}
		return false;
	}

	uint32_t findQueue(vk::PhysicalDevice& physicalDevice, const vk::QueueFlags& flags, const vk::SurfaceKHR& presentSurface = vk::SurfaceKHR()) {
		std::vector<vk::QueueFamilyProperties> queueProps = physicalDevice.getQueueFamilyProperties();
		size_t queueCount = queueProps.size();
		for (uint32_t i = 0; i < queueCount; i++) {
			if (queueProps[i].queueFlags & flags) {
				if (presentSurface && !physicalDevice.getSurfaceSupportKHR(i, presentSurface)) {
					continue;
				}
				return i;
			}
		}
		throw std::runtime_error("No queue matches the flags " + vk::to_string(flags));
	}

	vk::Bool32 getMemoryType(vk::PhysicalDevice& device, uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t * typeIndex) {
		for (uint32_t i = 0; i < 32; i++) {
			if ((typeBits & 1) == 1) {
				if ((device.getMemoryProperties().memoryTypes[i].propertyFlags & properties) == properties) {
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}
		return false;
	}

	uint32_t getMemoryType(vk::PhysicalDevice& device, uint32_t typeBits, const vk::MemoryPropertyFlags& properties) {
		uint32_t result = 0;
		if (!getMemoryType(device, typeBits, properties, &result)) {
			// todo : throw error
		}
		return result;
	}
}