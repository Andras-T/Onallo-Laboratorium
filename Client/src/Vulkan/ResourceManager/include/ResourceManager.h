#pragma once

#define GLFW_INCLUDE_VULKAN
#include "cmp_core.h"
#include "GLFW/glfw3.h"
#include "Images.h"
#include <Vulkan/CommandPool/include/CommandPoolManager.h>
#include <Vulkan/DeviceManager/include/DeviceManager.h>
#include <Vulkan/Utils/Constants.h>

namespace Client {

	class ResourceManager {

		VkBuffer quadBuffer;
		VkDeviceMemory quadMemory;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Images images;
		uint8_t *decompressedImage;

		void createVertexBuffer(DeviceManager& deviceManager, VkCommandPool& commandPool, const void* src, VkBuffer& buffer, VkDeviceMemory& deviceMemory, VkDeviceSize bufferSize);

		void createBuffer(DeviceManager& deviceManager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);

	public:

		void createBuffers(DeviceManager& deviceManager, VkCommandPool& commandPool);

		void createImageResources(DeviceManager& deviceManager, CommandPoolManager& commandPoolManager);

		void fillImageWithZeros(DeviceManager& deviceManager, CommandPoolManager& commandPoolManager) {
			VkDeviceSize size = getCompressedImageSize(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT);
			uint8_t* ptr = new uint8_t[size];
			std::memset(ptr, 255, size);
			auto commandBuffer = commandPoolManager.beginSingleTimeCommands(deviceManager.getLogicalDevice());
			copyToImage(ptr, size, deviceManager, commandBuffer, 0);
			copyToImage(ptr, size, deviceManager, commandBuffer, 1);
			commandPoolManager.endSingleTimeCommands(deviceManager.getLogicalDevice(), commandBuffer, deviceManager.getGraphicsQueue());
			delete[] ptr;
		}

		uint8_t* decompressImage(uint8_t* compressedImage, uint64_t imageSize) {
			unsigned int stride = DEFAULT_IMAGE_WIDTH * 4;
			auto blocksInRow = DEFAULT_IMAGE_WIDTH / 4;
			auto blocksInColumn = DEFAULT_IMAGE_HEIGHT / 4;
			int index = 0;
			for (size_t row = 0; row < blocksInRow; row++)
			{
				for (size_t column = 0; column < blocksInColumn; column++)
				{
					uint8_t srcBlock[64] = { 0 };
					uint8_t* cmpImageLocation = &compressedImage[index * 8];
					DecompressBlockBC1(cmpImageLocation, srcBlock);
					int cellIndex = 4 * (column * 4 + row * 4 * DEFAULT_IMAGE_WIDTH);

					for (size_t smallRow = 0; smallRow < 4; smallRow++)
					{
						for (size_t smallColumn = 0; smallColumn < 16; smallColumn++)
						{
							int blockIndex = smallRow * 16 + smallColumn;
							int imageIndex = cellIndex + smallRow * DEFAULT_IMAGE_WIDTH * 4 + smallColumn;
							decompressedImage[imageIndex] = srcBlock[blockIndex];
						}
					}
					
					index++;
				}
			}

			return decompressedImage;
		}

		void copyToImage(uint8_t* pImage, VkDeviceSize imageSize, DeviceManager& deviceManager, VkCommandBuffer& commandBuffer, uint32_t imageIndex) {
			void* data;
			vkMapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pImage, imageSize);
			vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);

			VkImageMemoryBarrier2 barrier1 = {};
			barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
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
			barrier1.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
			barrier1.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
			barrier1.pNext = nullptr;						   
															   
			VkDependencyInfo dependencyInfo1 = {};			   
			dependencyInfo1.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			dependencyInfo1.dependencyFlags = 0;			   
			dependencyInfo1.memoryBarrierCount = 0;			   
			dependencyInfo1.pMemoryBarriers = nullptr;		   
			dependencyInfo1.bufferMemoryBarrierCount = 0;	   
			dependencyInfo1.pBufferMemoryBarriers = nullptr;   
			dependencyInfo1.imageMemoryBarrierCount = 1;	   
			dependencyInfo1.pImageMemoryBarriers = &barrier1;  
															   
			vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo1);
															   
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
															   
			VkImageMemoryBarrier2 barrier2 = {};			   
			barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
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
			barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT; //VK_PIPELINE_STAGE_2_COPY_BIT
			barrier2.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			barrier2.pNext = nullptr;

			VkDependencyInfo dependencyInfo2 = {};
			dependencyInfo2.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			dependencyInfo2.dependencyFlags = 0;
			dependencyInfo2.memoryBarrierCount = 0;
			dependencyInfo2.pMemoryBarriers = nullptr;
			dependencyInfo2.bufferMemoryBarrierCount = 0;
			dependencyInfo2.pBufferMemoryBarriers = nullptr;
			dependencyInfo2.imageMemoryBarrierCount = 1;
			dependencyInfo2.pImageMemoryBarriers = &barrier2;


			vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo2);
		}

		VkBuffer& getQuadBuffer() { return quadBuffer; }

		VkImageView& getImageView(size_t i) { return images.getImageView(i); }

		void cleanUp(VkDevice& device) {
			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

			vkDestroyBuffer(device, quadBuffer, nullptr);
			vkFreeMemory(device, quadMemory, nullptr);

			images.cleanUp(device);
			delete[] decompressedImage;
		}

	};

}