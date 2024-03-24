#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <Vulkan/DeviceManager/include/DeviceManager.h>
#include "Images.h"
#include <Vulkan/Utils/Constants.h>
#include <Vulkan/CommandPool/include/CommandPoolManager.h>

namespace Client {

	class ResourceManager {

		VkBuffer quadBuffer;
		VkDeviceMemory quadMemory;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Images images;

		void createVertexBuffer(DeviceManager& deviceManager, VkCommandPool& commandPool, const void* src, VkBuffer& buffer, VkDeviceMemory& deviceMemory, VkDeviceSize bufferSize);

		void createBuffer(DeviceManager& deviceManager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);

	public:

		void createBuffers(DeviceManager& deviceManager, VkCommandPool& commandPool);

		void createImageResources(DeviceManager& deviceManager, CommandPoolManager& commandPoolManager) {
			images.init(MAX_FRAMES_IN_FLIGHT);
			images.createImage(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				deviceManager.getLogicalDevice(), deviceManager.getPhysicalDevice());
			images.createImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, deviceManager.getLogicalDevice());

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				commandPoolManager.transitionImageLayout(deviceManager.getLogicalDevice(), images.getImage(i),
					VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, deviceManager.getGraphicsQueue());
			}
		}

		void fillImageWithZeros(DeviceManager& deviceManager, CommandPoolManager& commandPoolManager) {
			VkDeviceSize size = DEFAULT_IMAGE_WIDTH * DEFAULT_IMAGE_HEIGHT * 4;
			uint8_t* ptr = new uint8_t[size];
			std::memset(ptr, 255, size);
			auto commandBuffer = commandPoolManager.beginSingleTimeCommands(deviceManager.getLogicalDevice());
			copyToImage(ptr, size, deviceManager, commandBuffer, 0);
			copyToImage(ptr, size, deviceManager, commandBuffer, 1);
			commandPoolManager.endSingleTimeCommands(deviceManager.getLogicalDevice(), commandBuffer, deviceManager.getGraphicsQueue());
		}

		/*Validation Error : [VUID - VkImageMemoryBarrier - oldLayout - 01197] Object 0 : handle = 0x20c1b5ea9a0, type =
			VK_OBJECT_TYPE_COMMAND_BUFFER; Object 1: handle = 0xe88693000000000c, type = VK_OBJECT_TYPE_IMAGE; | MessageID = 0x124ffb34
			| vkCmdPipelineBarrier() : pImageMemoryBarriers[0].image(VkImage 0xe88693000000000c[]) cannot transition the layout of
			aspect = 1, level = 0, layer = 0 from VK_IMAGE_LAYOUT_GENERAL when the previous known layout is
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.The Vulkan spec states : If srcQueueFamilyIndex and dstQueueFamilyIndex define a queue
			family ownership transfer or oldLayout and newLayout define an image layout transition, oldLayout must be
			VK_IMAGE_LAYOUT_UNDEFINED or the current layout of the image subresources affected by the barrier
			(https ://vulkan.lunarg.com/doc/view/1.3.275.0/windows/1.3-extensions/vkspec.html#VUID-VkImageMemoryBarrier-oldLayout-01197)
				2024 : 03 : 24 19 : 37 42s 396ms[validation layer] Validation Error : [VUID - VkImageMemoryBarrier - oldLayout - 01197] Object 0 : handle = 0x20c1b5ea9a0, type =
				VK_OBJECT_TYPE_COMMAND_BUFFER; Object 1: handle = 0x967dd1000000000e, type = VK_OBJECT_TYPE_IMAGE; | MessageID = 0x124ffb34
				| vkCmdPipelineBarrier() : pImageMemoryBarriers[0].image(VkImage 0x967dd1000000000e[]) cannot transition the layout of
				aspect = 1, level = 0, layer = 0 from VK_IMAGE_LAYOUT_GENERAL when the previous known layout is
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.The Vulkan spec states : If srcQueueFamilyIndex and dstQueueFamilyIndex define a queue
				family ownership transfer or oldLayout and newLayout define an image layout transition, oldLayout must be
				VK_IMAGE_LAYOUT_UNDEFINED or the current layout of the image subresources affected by the barrier
				(https ://vulkan.lunarg.com/doc/view/1.3.275.0/windows/1.3-extensions/vkspec.html#VUID-VkImageMemoryBarrier-oldLayout-01197)*/

		void copyToImage(uint8_t* pImage, VkDeviceSize imageSize, DeviceManager& deviceManager, VkCommandBuffer& commandBuffer, uint32_t imageIndex) {
			void* data;
			vkMapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pImage, imageSize);
			vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);

			VkImageMemoryBarrier barrier1{};
			barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier1.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier1.image = images.getImage(imageIndex);
			barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier1.subresourceRange.baseMipLevel = 0;
			barrier1.subresourceRange.levelCount = 1;
			barrier1.subresourceRange.baseArrayLayer = 0;
			barrier1.subresourceRange.layerCount = 1;
			barrier1.srcAccessMask = 0;
			barrier1.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // After all operations that have been submitted so far
				VK_PIPELINE_STAGE_TRANSFER_BIT,  // Before the transfer stage
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier1
			);


			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT, 1 };

			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, images.getImage(imageIndex), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			VkImageMemoryBarrier barrier2 = {};
			barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier2.image = images.getImage(imageIndex);
			barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier2.subresourceRange.baseMipLevel = 0;
			barrier2.subresourceRange.levelCount = 1;
			barrier2.subresourceRange.baseArrayLayer = 0;
			barrier2.subresourceRange.layerCount = 1;
			barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,  // After the transfer stage
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,  // Before the fragment shader stage
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier2
			);

		}

		VkBuffer& getQuadBuffer() { return quadBuffer; }

		VkImageView& getImageView(size_t i) { return images.getImageView(i); }

		void cleanUp(VkDevice& device) {
			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

			vkDestroyBuffer(device, quadBuffer, nullptr);
			vkFreeMemory(device, quadMemory, nullptr);

			images.cleanUp(device);
		}

	};

}