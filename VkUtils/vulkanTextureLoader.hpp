/*
* Texture loader for Vulkan
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <vulkan/vulkan.hpp>
#include <gli/gli.hpp>
#include <vector>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

namespace vkext
{
	/**
	* @brief Encapsulates a Vulkan texture object (including view, sampler, descriptor, etc.)
	*/
	struct VulkanTexture
	{
		vk::Sampler				sampler;
		vk::Image				image;
		vk::ImageLayout			imageLayout;
		vk::DeviceMemory		deviceMemory;
		vk::ImageView			view;
		uint32_t				width, height;
		uint32_t				mipLevels;
		uint32_t				layerCount;
		vk::DescriptorImageInfo	descriptor;
	};

	/**
	* @brief A simple Vulkan texture uploader for getting images into GPU memory
	*/
	class VulkanTextureLoader
	{
	private:
		vk::Device&		device;
		vk::PhysicalDevice& physicalDevice;
		vk::Queue queue;
		vk::CommandBuffer cmdBuffer;
		vk::CommandPool cmdPool;
	public:
#if defined(__ANDROID__)
		AAssetManager* assetManager = nullptr;
#endif

		/**
		* Default constructor
		*
		* @param vulkanDevice Pointer to a valid VulkanDevice
		* @param queue Queue for the copy commands when using staging (queue must support transfers)
		* @param cmdPool Commandpool used to get command buffers for copies and layout transitions
		*/
		VulkanTextureLoader(vk::Device& device, vk::PhysicalDevice& physicalDevice, vk::Queue& queue, vk::CommandPool& cmdPool)
			:device(device), physicalDevice(physicalDevice)
		{
			this->device = device;
			this->physicalDevice = physicalDevice;
			this->queue = queue;
			this->cmdPool = cmdPool;

			// Create command buffer for submitting image barriers
			// and converting tilings
			vk::CommandBufferAllocateInfo cmdBufInfo = {};
			cmdBufInfo.setSType(vk::StructureType::eCommandBufferAllocateInfo);
			cmdBufInfo.setCommandPool(cmdPool);
			cmdBufInfo.setLevel(vk::CommandBufferLevel::ePrimary);
			cmdBufInfo.setCommandBufferCount(1);
			// TODO:
			this->cmdBuffer = device.allocateCommandBuffers(cmdBufInfo)[0];
		}

		/**
		* Default destructor
		*
		* @note Does not free texture resources
		*/
		~VulkanTextureLoader()
		{
			this->device.freeCommandBuffers(cmdPool, 1, &cmdBuffer);
		}

		/**
		* Load a 2D texture including all mip levels
		*
		* @param filename File to load
		* @param format Vulkan format of the image data stored in the file
		* @param texture Pointer to the texture object to load the image into
		* @param (Optional) forceLinear Force linear tiling (not advised, defaults to false)
		* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
		*
		* @note Only supports .ktx and .dds
		*/
		void loadTexture(std::string filename, vk::Format format, VulkanTexture *texture, bool forceLinear = false, vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled)
		{
#if defined(__ANDROID__)
			assert(assetManager != nullptr);

			// Textures are stored inside the apk on Android (compressed)
			// So they need to be loaded via the asset manager
			AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
			assert(asset);
			size_t size = AAsset_getLength(asset);
			assert(size > 0);

			void *textureData = malloc(size);
			AAsset_read(asset, textureData, size);
			AAsset_close(asset);

			gli::texture2D tex2D(gli::load((const char*)textureData, size));

			free(textureData);
#else
			gli::texture2D tex2D(gli::load(filename.c_str()));
#endif		
			assert(!tex2D.empty());

			texture->width = static_cast<uint32_t>(tex2D[0].dimensions().x);
			texture->height = static_cast<uint32_t>(tex2D[0].dimensions().y);
			texture->mipLevels = static_cast<uint32_t>(tex2D.levels());

			// Get device properites for the requested texture format
			vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);

			// Only use linear tiling if requested (and supported by the device)
			// Support for linear tiling is mostly limited, so prefer to use
			// optimal tiling instead
			// On most implementations linear tiling will only support a very
			// limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
			VkBool32 useStaging = !forceLinear;

			vk::MemoryAllocateInfo memAllocInfo;
			vk::MemoryRequirements memReqs;

			// Use a separate command buffer for texture loading
			vk::CommandBufferBeginInfo cmdBufInfo;
			cmdBuffer.begin(cmdBufInfo);

			if (useStaging)
			{
				// Create a host-visible staging buffer that contains the raw image data
				vk::Buffer stagingBuffer;
				vk::DeviceMemory stagingMemory;

				vk::BufferCreateInfo bufferCreateInfo;
				bufferCreateInfo.setSize(tex2D.size());
				// This buffer is used as a transfer source for the buffer copy
				bufferCreateInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
				bufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);

				stagingBuffer = device.createBuffer(bufferCreateInfo);

				// Get memory requirements for the staging buffer (alignment, memory type bits)
				memReqs = device.getBufferMemoryRequirements(stagingBuffer);

				memAllocInfo.allocationSize = memReqs.size;
				// Get memory type index for a host visible buffer
				memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

				stagingMemory = device.allocateMemory(memAllocInfo);
				device.bindBufferMemory(stagingBuffer, stagingMemory, 0);

				// Copy texture data into staging buffer
				void* data = device.mapMemory(stagingMemory, 0, memReqs.size, vk::MemoryMapFlags());
				memcpy(data, tex2D.data(), tex2D.size());
				device.unmapMemory(stagingMemory);

				// Setup buffer copy regions for each mip level
				std::vector<vk::BufferImageCopy> bufferCopyRegions;
				uint32_t offset = 0;

				for (uint32_t i = 0; i < texture->mipLevels; i++)
				{
					vk::BufferImageCopy bufferCopyRegion = {};
					bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
					bufferCopyRegion.imageSubresource.mipLevel = i;
					bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
					bufferCopyRegion.imageSubresource.layerCount = 1;
					bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].dimensions().x);
					bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].dimensions().y);
					bufferCopyRegion.imageExtent.depth = 1;
					bufferCopyRegion.bufferOffset = offset;

					bufferCopyRegions.push_back(bufferCopyRegion);

					offset += static_cast<uint32_t>(tex2D[i].size());
				}

				// Create optimal tiled target image
				vk::ImageCreateInfo imageCreateInfo;
				imageCreateInfo.imageType = vk::ImageType::e2D;
				imageCreateInfo.format = format;
				imageCreateInfo.mipLevels = texture->mipLevels;
				imageCreateInfo.arrayLayers = 1;
				imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
				imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
				imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
				imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
				imageCreateInfo.extent = { texture->width, texture->height, 1 };
				imageCreateInfo.usage = imageUsageFlags;
				// Ensure that the TRANSFER_DST bit is set for staging
				if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst ))
				{
					imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
				}
				texture->image = device.createImage(imageCreateInfo);

				memReqs = device.getImageMemoryRequirements(texture->image);

				memAllocInfo.allocationSize = memReqs.size;

				memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
				texture->deviceMemory = device.allocateMemory(memAllocInfo);
				device.bindImageMemory(texture->image, texture->deviceMemory, 0);

				vk::ImageSubresourceRange subresourceRange;
				subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
				subresourceRange.baseMipLevel = 0;
				subresourceRange.levelCount = texture->mipLevels;
				subresourceRange.layerCount = 1;

				// Image barrier for optimal image (target)
				// Optimal image will be used as destination for the copy
				vkhelper::setImageLayout(
					cmdBuffer,
					texture->image,
					vk::ImageAspectFlagBits::eColor,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eTransferDstOptimal,
					subresourceRange);

				// Copy mip levels from staging buffer
				cmdBuffer.copyBufferToImage(stagingBuffer, texture->image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);

				// Change texture image layout to shader read after all mip levels have been copied
				texture->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				vkhelper::setImageLayout(
					cmdBuffer,
					texture->image,
					vk::ImageAspectFlagBits::eColor,
					vk::ImageLayout::eUndefined,
					texture->imageLayout,
					subresourceRange);

				// Submit command buffer containing copy and image layout commands
				cmdBuffer.end();

				// Create a fence to make sure that the copies have finished before continuing
				vk::Fence copyFence;
				vk::FenceCreateInfo fenceCreateInfo;
				copyFence = device.createFence(fenceCreateInfo);

				vk::SubmitInfo submitInfo;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &cmdBuffer;

				queue.submit(submitInfo, copyFence);
				device.waitForFences(copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
				device.destroyFence(copyFence);
				// Clean up staging resources
				device.freeMemory(stagingMemory);
				device.destroyBuffer(stagingBuffer);
			}
			else
			{
				// Prefer using optimal tiling, as linear tiling 
				// may support only a small set of features 
				// depending on implementation (e.g. no mip maps, only one layer, etc.)

				// Check if this support is supported for linear tiling
				assert(formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);

				vk::Image mappableImage;
				vk::DeviceMemory mappableMemory;

				vk::ImageCreateInfo imageCreateInfo;

				imageCreateInfo.imageType = vk::ImageType::e2D;
				imageCreateInfo.format = format;
				imageCreateInfo.extent = { texture->width, texture->height, 1 };
				imageCreateInfo.mipLevels = 1;
				imageCreateInfo.arrayLayers = 1;
				imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
				imageCreateInfo.tiling = vk::ImageTiling::eLinear;
				imageCreateInfo.usage = imageUsageFlags;
				imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
				imageCreateInfo.initialLayout = vk::ImageLayout::ePreinitialized;

				// Load mip map level 0 to linear tiling image
				mappableImage = device.createImage(imageCreateInfo);

				// Get memory requirements for this image 
				// like size and alignment
				memReqs = device.getImageMemoryRequirements(mappableImage);
				// Set memory allocation size to required memory size
				memAllocInfo.allocationSize = memReqs.size;

				// Get memory type that can be mapped to host memory
				memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

				// Allocate host memory
				mappableMemory = device.allocateMemory(memAllocInfo);

				// Bind allocated image for use
				device.bindImageMemory(mappableImage, mappableMemory, 0 );

				// Get sub resource layout
				// Mip map count, array layer, etc.
				vk::ImageSubresource subRes;
				subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
				subRes.mipLevel = 0;

				vk::SubresourceLayout subResLayout;

				// Get sub resources layout 
				// Includes row pitch, size offsets, etc.
				subResLayout = device.getImageSubresourceLayout(mappableImage, subRes);

				// Map image memory
				void* data = device.mapMemory(mappableMemory, 0, memReqs.size, vk::MemoryMapFlags());

				// Copy image data into memory
				memcpy(data, tex2D[subRes.mipLevel].data(), tex2D[subRes.mipLevel].size());
				device.unmapMemory(mappableMemory);

				// Linear tiled images don't need to be staged
				// and can be directly used as textures
				texture->image = mappableImage;
				texture->deviceMemory = mappableMemory;
				texture->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

				// Setup image memory barrier

				vkhelper::setImageLayout(
					cmdBuffer,
					texture->image,
					vk::ImageAspectFlagBits::eColor,
					vk::ImageLayout::ePreinitialized,
					texture->imageLayout);

				// Submit command buffer containing copy and image layout commands
				cmdBuffer.end();

				// TODO: 
				vk::Fence nullFence;

				vk::SubmitInfo submitInfo;
				submitInfo.waitSemaphoreCount = 0;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &cmdBuffer;

				queue.submit(submitInfo, nullFence);
				queue.waitIdle();
			}

			// Create sampler
			vk::SamplerCreateInfo sampler = {};
			sampler.magFilter = vk::Filter::eLinear;
			sampler.minFilter = vk::Filter::eLinear;
			sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
			sampler.addressModeU = vk::SamplerAddressMode::eRepeat;
			sampler.addressModeV = vk::SamplerAddressMode::eRepeat;
			sampler.addressModeW = vk::SamplerAddressMode::eRepeat;
			sampler.mipLodBias = 0.0f;
			sampler.compareOp = vk::CompareOp::eNever;
			sampler.minLod = 0.0f;
			// Max level-of-detail should match mip level count
			sampler.maxLod = (useStaging) ? (float)texture->mipLevels : 0.0f;
			// Enable anisotropic filtering
			sampler.maxAnisotropy = 8;
			sampler.anisotropyEnable = VK_TRUE;
			sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;
			texture->sampler = device.createSampler(sampler);

			// Create image view
			// Textures are not directly accessed by the shaders and
			// are abstracted by image views containing additional
			// information and sub resource ranges
			vk::ImageViewCreateInfo view;
			view.pNext = nullptr;
			view.viewType = vk::ImageViewType::e2D;
			view.format = format;
			view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			// Linear tiling usually won't support mip maps
			// Only set mip map count if optimal tiling is used
			view.subresourceRange.levelCount = (useStaging) ? texture->mipLevels : 1;
			view.image = texture->image;
			texture->view = device.createImageView(view);

			// Fill descriptor image info that can be used for setting up descriptor sets
			texture->descriptor.imageLayout = vk::ImageLayout::eGeneral;
			texture->descriptor.imageView = texture->view;
			texture->descriptor.sampler = texture->sampler;
		}

		/**
		* Load a cubemap texture including all mip levels from a single file
		*
		* @param filename File to load
		* @param format Vulkan format of the image data stored in the file
		* @param texture Pointer to the texture object to load the image into
		*
		* @note Only supports .ktx and .dds
		*/
		void loadCubemap(std::string filename, vk::Format format, VulkanTexture *texture, vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled)
		{
#if defined(__ANDROID__)
			assert(assetManager != nullptr);

			// Textures are stored inside the apk on Android (compressed)
			// So they need to be loaded via the asset manager
			AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
			assert(asset);
			size_t size = AAsset_getLength(asset);
			assert(size > 0);

			void *textureData = malloc(size);
			AAsset_read(asset, textureData, size);
			AAsset_close(asset);

			gli::textureCube texCube(gli::load((const char*)textureData, size));

			free(textureData);
#else
			gli::textureCube texCube(gli::load(filename));
#endif	
			assert(!texCube.empty());

			texture->width = static_cast<uint32_t>(texCube.dimensions().x);
			texture->height = static_cast<uint32_t>(texCube.dimensions().y);
			texture->mipLevels = static_cast<uint32_t>(texCube.levels());

			vk::MemoryAllocateInfo memAllocInfo;
			vk::MemoryRequirements memReqs;

			// Create a host-visible staging buffer that contains the raw image data
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingMemory;

			vk::BufferCreateInfo bufferCreateInfo;
			bufferCreateInfo.size = texCube.size();
			// This buffer is used as a transfer source for the buffer copy
			bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
			bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

			stagingBuffer = device.createBuffer(bufferCreateInfo);

			// Get memory requirements for the staging buffer (alignment, memory type bits)
			memReqs = device.getBufferMemoryRequirements(stagingBuffer);

			memAllocInfo.allocationSize = memReqs.size;
			// Get memory type index for a host visible buffer
			memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			stagingMemory = device.allocateMemory(memAllocInfo);
			device.bindBufferMemory(stagingBuffer, stagingMemory, 0);

			// Copy texture data into staging buffer
			void* data = device.mapMemory(stagingMemory, 0, memReqs.size, vk::MemoryMapFlags());
			memcpy(data, texCube.data(), texCube.size());
			device.unmapMemory(stagingMemory);

			// Setup buffer copy regions for each face including all of it's miplevels
			std::vector<vk::BufferImageCopy> bufferCopyRegions;
			size_t offset = 0;

			for (uint32_t face = 0; face < 6; face++)
			{
				for (uint32_t level = 0; level < texture->mipLevels; level++)
				{
					vk::BufferImageCopy bufferCopyRegion = {};
					bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
					bufferCopyRegion.imageSubresource.mipLevel = level;
					bufferCopyRegion.imageSubresource.baseArrayLayer = face;
					bufferCopyRegion.imageSubresource.layerCount = 1;
					bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(texCube[face][level].dimensions().x);
					bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(texCube[face][level].dimensions().y);
					bufferCopyRegion.imageExtent.depth = 1;
					bufferCopyRegion.bufferOffset = offset;

					bufferCopyRegions.push_back(bufferCopyRegion);

					// Increase offset into staging buffer for next level / face
					offset += texCube[face][level].size();
				}
			}

			// Create optimal tiled target image
			vk::ImageCreateInfo imageCreateInfo;
			imageCreateInfo.imageType = vk::ImageType::e2D;
			imageCreateInfo.format = format;
			imageCreateInfo.mipLevels = texture->mipLevels;
			imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
			imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
			imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
			imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
			imageCreateInfo.extent = { texture->width, texture->height, 1 };
			imageCreateInfo.usage = imageUsageFlags;
			// Ensure that the TRANSFER_DST bit is set for staging
			if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
			{
				imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
			}
			// Cube faces count as array layers in Vulkan
			imageCreateInfo.arrayLayers = 6;
			// This flag is required for cube map images
			imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;

			texture->image = device.createImage(imageCreateInfo);

			memReqs = device.getImageMemoryRequirements(texture->image);

			memAllocInfo.allocationSize = memReqs.size;
			memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

			texture->deviceMemory = device.allocateMemory(memAllocInfo);
			device.bindImageMemory(texture->image, texture->deviceMemory, 0);

			vk::CommandBufferBeginInfo cmdBufInfo;
			cmdBuffer.begin(cmdBufInfo);

			// Image barrier for optimal image (target)
			// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
			vk::ImageSubresourceRange subresourceRange;
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = texture->mipLevels;
			subresourceRange.layerCount = 6;

			vkhelper::setImageLayout(
				cmdBuffer,
				texture->image,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal,
				subresourceRange);

			// Copy the cube map faces from the staging buffer to the optimal tiled image
			cmdBuffer.copyBufferToImage(stagingBuffer, texture->image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);

			// Change texture image layout to shader read after all faces have been copied
			texture->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			vkhelper::setImageLayout(
				cmdBuffer,
				texture->image,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageLayout::eTransferDstOptimal,
				texture->imageLayout,
				subresourceRange);

			cmdBuffer.end();

			// Create a fence to make sure that the copies have finished before continuing
			vk::Fence copyFence;
			vk::FenceCreateInfo fenceCreateInfo;
			copyFence = device.createFence(fenceCreateInfo);

			vk::SubmitInfo submitInfo;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;

			queue.submit(submitInfo, copyFence);
			device.waitForFences(copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
			device.destroyFence(copyFence);

			// Create sampler
			vk::SamplerCreateInfo sampler;
			sampler.magFilter = vk::Filter::eLinear;
			sampler.minFilter = vk::Filter::eLinear;
			sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
			sampler.addressModeU = vk::SamplerAddressMode::eClampToEdge;
			sampler.addressModeV = sampler.addressModeU;
			sampler.addressModeW = sampler.addressModeU;
			sampler.mipLodBias = 0.0f;
			sampler.maxAnisotropy = 8;
			sampler.compareOp = vk::CompareOp::eNever;
			sampler.minLod = 0.0f;
			sampler.maxLod = (float)texture->mipLevels;
			sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;
			texture->sampler = device.createSampler(sampler);

			// Create image view
			vk::ImageViewCreateInfo view;
			view.viewType = vk::ImageViewType::eCube;
			view.format = format;
			view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			view.subresourceRange.layerCount = 6;
			view.subresourceRange.levelCount = texture->mipLevels;
			view.image = texture->image;
			texture->view = device.createImageView(view);

			// Clean up staging resources
			device.freeMemory(stagingMemory);
			device.destroyBuffer(stagingBuffer);

			// Fill descriptor image info that can be used for setting up descriptor sets
			texture->descriptor.imageLayout = vk::ImageLayout::eGeneral;
			texture->descriptor.imageView = texture->view;
			texture->descriptor.sampler = texture->sampler;
		}

		/**
		* Load a texture array including all mip levels from a single file
		*
		* @param filename File to load
		* @param format Vulkan format of the image data stored in the file
		* @param texture Pointer to the texture object to load the image into
		*
		* @note Only supports .ktx and .dds
		*/
		void loadTextureArray(std::string filename, vk::Format format, VulkanTexture *texture, vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled)
		{
#if defined(__ANDROID__)
			assert(assetManager != nullptr);

			// Textures are stored inside the apk on Android (compressed)
			// So they need to be loaded via the asset manager
			AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
			assert(asset);
			size_t size = AAsset_getLength(asset);
			assert(size > 0);

			void *textureData = malloc(size);
			AAsset_read(asset, textureData, size);
			AAsset_close(asset);

			gli::texture2DArray tex2DArray(gli::load((const char*)textureData, size));

			free(textureData);
#else
			gli::texture2DArray tex2DArray(gli::load(filename));
#endif	

			assert(!tex2DArray.empty());

			texture->width = static_cast<uint32_t>(tex2DArray.dimensions().x);
			texture->height = static_cast<uint32_t>(tex2DArray.dimensions().y);
			texture->layerCount = static_cast<uint32_t>(tex2DArray.layers());
			texture->mipLevels = static_cast<uint32_t>(tex2DArray.levels());

			vk::MemoryAllocateInfo memAllocInfo ;
			vk::MemoryRequirements memReqs;

			// Create a host-visible staging buffer that contains the raw image data
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingMemory;

			vk::BufferCreateInfo bufferCreateInfo;
			bufferCreateInfo.size = tex2DArray.size();
			// This buffer is used as a transfer source for the buffer copy
			bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
			bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
			stagingBuffer = device.createBuffer(bufferCreateInfo);

			// Get memory requirements for the staging buffer (alignment, memory type bits)
			memReqs = device.getBufferMemoryRequirements(stagingBuffer);

			memAllocInfo.allocationSize = memReqs.size;
			// Get memory type index for a host visible buffer
			memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			
			stagingMemory = device.allocateMemory(memAllocInfo);
			device.bindBufferMemory(stagingBuffer, stagingMemory, 0);
			// Copy texture data into staging buffer
			void *data;
			data = device.mapMemory(stagingMemory, 0, memReqs.size, vk::MemoryMapFlags());
			memcpy(data, tex2DArray.data(), static_cast<size_t>(tex2DArray.size()));
			device.unmapMemory(stagingMemory);

			// Setup buffer copy regions for each layer including all of it's miplevels
			std::vector<vk::BufferImageCopy> bufferCopyRegions;
			size_t offset = 0;

			for (uint32_t layer = 0; layer < texture->layerCount; layer++)
			{
				for (uint32_t level = 0; level < texture->mipLevels; level++)
				{
					vk::BufferImageCopy bufferCopyRegion = {};
					bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
					bufferCopyRegion.imageSubresource.mipLevel = level;
					bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
					bufferCopyRegion.imageSubresource.layerCount = 1;
					bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2DArray[layer][level].dimensions().x);
					bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2DArray[layer][level].dimensions().y);
					bufferCopyRegion.imageExtent.depth = 1;
					bufferCopyRegion.bufferOffset = offset;

					bufferCopyRegions.push_back(bufferCopyRegion);

					// Increase offset into staging buffer for next level / face
					offset += tex2DArray[layer][level].size();
				}
			}

			// Create optimal tiled target image
			vk::ImageCreateInfo imageCreateInfo;
			imageCreateInfo.imageType = vk::ImageType::e2D;
			imageCreateInfo.format = format;
			imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
			imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
			imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
			imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
			imageCreateInfo.extent = { texture->width, texture->height, 1 };
			imageCreateInfo.usage = imageUsageFlags;
			// Ensure that the TRANSFER_DST bit is set for staging
			if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
			{
				imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
			}
			imageCreateInfo.arrayLayers = texture->layerCount;
			imageCreateInfo.mipLevels = texture->mipLevels;

			texture->image = device.createImage(imageCreateInfo);
			memReqs = device.getImageMemoryRequirements(texture->image);

			memAllocInfo.allocationSize = memReqs.size;
			memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
			texture->deviceMemory = device.allocateMemory(memAllocInfo);
			device.bindImageMemory(texture->image, texture->deviceMemory,0);

			vk::CommandBufferBeginInfo cmdBufInfo;
			cmdBuffer.begin(cmdBufInfo);

			// Image barrier for optimal image (target)
			// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
			vk::ImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = texture->mipLevels;
			subresourceRange.layerCount = texture->layerCount;

			vkhelper::setImageLayout(
				cmdBuffer,
				texture->image,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageLayout::eTransferDstOptimal,
				texture->imageLayout,
				subresourceRange);

			// Copy the layers and mip levels from the staging buffer to the optimal tiled image
			cmdBuffer.copyBufferToImage(stagingBuffer, texture->image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);

			// Change texture image layout to shader read after all faces have been copied
			texture->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			vkhelper::setImageLayout(
				cmdBuffer,
				texture->image,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageLayout::eTransferDstOptimal,
				texture->imageLayout,
				subresourceRange);

			cmdBuffer.end();
			// Create a fence to make sure that the copies have finished before continuing
			vk::Fence copyFence;
			vk::FenceCreateInfo fenceCreateInfo;
			copyFence = device.createFence(fenceCreateInfo);
			vk::SubmitInfo submitInfo;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;
			queue.submit(submitInfo, copyFence);

			device.waitForFences(copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
			device.destroyFence(copyFence);
			// Create sampler
			vk::SamplerCreateInfo sampler;
			sampler.magFilter = vk::Filter::eLinear;
			sampler.minFilter = vk::Filter::eLinear;
			sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
			sampler.addressModeU = vk::SamplerAddressMode::eClampToEdge;
			sampler.addressModeV = sampler.addressModeU;
			sampler.addressModeW = sampler.addressModeU;
			sampler.mipLodBias = 0.0f;
			sampler.maxAnisotropy = 8;
			sampler.compareOp = vk::CompareOp::eNever;
			sampler.minLod = 0.0f;
			sampler.maxLod = (float)texture->mipLevels;
			sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;
			texture->sampler = device.createSampler(sampler);
			// Create image view
			vk::ImageViewCreateInfo view;
			view.viewType = vk::ImageViewType::e2D;
			view.format = format;
			view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			view.subresourceRange.layerCount = texture->layerCount;
			view.subresourceRange.levelCount = texture->mipLevels;
			view.image = texture->image;
			texture->view = device.createImageView(view);
			// Clean up staging resources
			device.freeMemory(stagingMemory);
			device.destroyBuffer(stagingBuffer);
			// Fill descriptor image info that can be used for setting up descriptor sets
			texture->descriptor.imageLayout = vk::ImageLayout::eGeneral;
			texture->descriptor.imageView = texture->view;
			texture->descriptor.sampler = texture->sampler;
		}

		/**
		* Creates a 2D texture from a buffer
		*
		* @param buffer Buffer containing texture data to upload
		* @param bufferSize Size of the buffer in machine units
		* @param width Width of the texture to create
		* @param hidth Height of the texture to create
		* @param format Vulkan format of the image data stored in the file
		* @param texture Pointer to the texture object to load the image into
		* @param (Optional) filter Texture filtering for the sampler (defaults to VK_FILTER_LINEAR)
		* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
		*/
		void createTexture(void* buffer, vk::DeviceSize bufferSize, vk::Format format, uint32_t width, uint32_t height, VulkanTexture *texture, vk::Filter filter = vk::Filter::eLinear, vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled)
		{
			assert(buffer);

			texture->width = width;
			texture->height = height;
			texture->mipLevels = 1;

			vk::MemoryAllocateInfo memAllocInfo;
			vk::MemoryRequirements memReqs;

			// Use a separate command buffer for texture loading
			vk::CommandBufferBeginInfo cmdBufInfo;
			cmdBuffer.begin(cmdBufInfo);
			// Create a host-visible staging buffer that contains the raw image data
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingMemory;

			vk::BufferCreateInfo bufferCreateInfo;
			bufferCreateInfo.size = bufferSize;
			// This buffer is used as a transfer source for the buffer copy
			bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
			bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

			stagingBuffer=device.createBuffer(bufferCreateInfo);

			// Get memory requirements for the staging buffer (alignment, memory type bits)
			memReqs = device.getBufferMemoryRequirements(stagingBuffer);

			memAllocInfo.allocationSize = memReqs.size;
			// Get memory type index for a host visible buffer
			memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			stagingMemory = device.allocateMemory(memAllocInfo);
			device.bindBufferMemory(stagingBuffer, stagingMemory, 0);
			// Copy texture data into staging buffer
			void *data;
			data = device.mapMemory(stagingMemory, 0, memReqs.size, vk::MemoryMapFlags());
			memcpy(data, buffer, bufferSize);
			device.unmapMemory(stagingMemory);

			vk::BufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			bufferCopyRegion.imageSubresource.mipLevel = 0;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = width;
			bufferCopyRegion.imageExtent.height = height;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = 0;

			// Create optimal tiled target image
			vk::ImageCreateInfo imageCreateInfo;
			imageCreateInfo.imageType = vk::ImageType::e2D;
			imageCreateInfo.format = format;
			imageCreateInfo.mipLevels = texture->mipLevels;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
			imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
			imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
			imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
			imageCreateInfo.extent = { texture->width, texture->height, 1 };
			imageCreateInfo.usage = imageUsageFlags;
			// Ensure that the TRANSFER_DST bit is set for staging
			if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
			{
				imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
			}
			texture->image = device.createImage(imageCreateInfo);
			memReqs = device.getImageMemoryRequirements(texture->image);

			memAllocInfo.allocationSize = memReqs.size;

			memAllocInfo.memoryTypeIndex = vkhelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
			texture->deviceMemory = device.allocateMemory(memAllocInfo);
			device.bindImageMemory(texture->image, texture->deviceMemory, 0);

			vk::ImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = texture->mipLevels;
			subresourceRange.layerCount = 1;

			// Image barrier for optimal image (target)
			// Optimal image will be used as destination for the copy
			vkhelper::setImageLayout(
				cmdBuffer,
				texture->image,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal,
				subresourceRange);

			// Copy mip levels from staging buffer
			cmdBuffer.copyBufferToImage(stagingBuffer, texture->image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegion);

			// Change texture image layout to shader read after all mip levels have been copied
			texture->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			vkhelper::setImageLayout(
				cmdBuffer,
				texture->image,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageLayout::eTransferDstOptimal,
				texture->imageLayout,
				subresourceRange);

			// Submit command buffer containing copy and image layout commands
			cmdBuffer.end();
			// Create a fence to make sure that the copies have finished before continuing
			vk::Fence copyFence;
			vk::FenceCreateInfo fenceCreateInfo;
			copyFence =  device.createFence(fenceCreateInfo);

			vk::SubmitInfo submitInfo;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;

			queue.submit(submitInfo, copyFence);
			device.waitForFences(copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
			device.destroyFence(copyFence);
			// Clean up staging resources
			device.freeMemory(stagingMemory);
			device.destroyBuffer(stagingBuffer);

			// Create sampler
			vk::SamplerCreateInfo sampler;
			sampler.magFilter = vk::Filter::eLinear;
			sampler.minFilter = vk::Filter::eLinear;
			sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
			sampler.addressModeU = vk::SamplerAddressMode::eRepeat;
			sampler.addressModeV = vk::SamplerAddressMode::eRepeat;
			sampler.addressModeW = vk::SamplerAddressMode::eRepeat;
			sampler.mipLodBias = 0.0f;
			sampler.compareOp = vk::CompareOp::eNever;
			sampler.minLod = 0.0f;
			sampler.maxLod = 0.0f;
			texture->sampler = device.createSampler(sampler);

			// Create image view
			vk::ImageViewCreateInfo view;
			view.pNext = nullptr;
			view.viewType = vk::ImageViewType::e2D;
			view.format = format;
			view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			view.subresourceRange.levelCount = 1;
			view.image = texture->image;
			texture->view = device.createImageView(view);

			// Fill descriptor image info that can be used for setting up descriptor sets
			texture->descriptor.imageLayout = vk::ImageLayout::eGeneral;
			texture->descriptor.imageView = texture->view;
			texture->descriptor.sampler = texture->sampler;
		}

		/**
		* Free all Vulkan resources used by a texture object
		*
		* @param texture Texture object whose resources are to be freed
		*/
		void destroyTexture(VulkanTexture texture)
		{
			device.destroyImageView(texture.view);
			device.destroyImage(texture.image);
			device.destroySampler(texture.sampler);
			device.freeMemory(texture.deviceMemory);
		}
	};
};
