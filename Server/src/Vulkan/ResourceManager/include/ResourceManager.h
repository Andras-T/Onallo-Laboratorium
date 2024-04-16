#pragma once

#define GLFW_INCLUDE_VULKAN
#include "cmp_core.h"
#include "Compressonator.h"
#include "GLFW/glfw3.h"
#include "Images.h"
#include <Vulkan/DeviceManager/include/DeviceManager.h>
#include <Vulkan/Utils/Constants.h>
#include <Vulkan/CommandPool/include/CommandPoolManager.h>

namespace Server {

	class ResourceManager {
		VkBuffer quadBuffer;
		VkDeviceMemory quadMemory;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		uint32_t width = DEFAULT_WIDTH;
		uint32_t height = DEFAULT_HEIGHT;
		VkDeviceSize size;
		VkDeviceSize compressedSize;
		void* mappedData;

		CMP_BYTE* compressedImage;
		uint8_t* cpuImage;

		Images presentImages;
		std::vector<VkFramebuffer> frameBuffers;

	public:

		void createBuffers(DeviceManager& deviceManager, VkCommandPool& commandPool, std::optional<GLFWwindow*> window);

		void recreateStagingBuffer(DeviceManager& deviceManager, std::optional<GLFWwindow*> window);

		void copyImageToStagingBuffer(VkImage& image, DeviceManager& deviceManager, VkCommandBuffer& commandBuffer);

		void createFrameBuffers(std::vector<VkImageView>& imageViews, VkRenderPass& renderPass, VkDevice& device, std::optional<VkExtent2D> extent);

		void createImages(VkDevice& device, VkPhysicalDevice& physicalDevice) {
			presentImages.init(3);
			presentImages.createImage(DEFAULT_WIDTH, DEFAULT_HEIGHT, defaultFormat, VK_IMAGE_TILING_LINEAR,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device, physicalDevice);
			presentImages.createImageView(defaultFormat, VK_IMAGE_ASPECT_COLOR_BIT, device);
		}

		void destroyFrameBuffers(VkDevice& device) {
			for (auto& framebuffer : frameBuffers)
				vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		void cleanUp(VkDevice& device) {
			vkDestroyBuffer(device, quadBuffer, nullptr);
			vkDestroyBuffer(device, stagingBuffer, nullptr);

			vkFreeMemory(device, quadMemory, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

			delete[] cpuImage;
			delete[] compressedImage;
		}

		inline uint8_t* getCPUImage() {
			return cpuImage;
		}

		inline uint8_t* getCompressedImage() {
			return compressedImage;
		}

		VkBuffer& getQuadBuffer() { return quadBuffer; }

		std::vector<VkFramebuffer>& getFrameBuffers() { return frameBuffers; }

		Images& getPresentImages() {
			return presentImages;
		}

		inline VkDeviceSize getCPUImageSize() { return size; }

		inline VkDeviceSize getCompressedImageSize() { return compressedSize; }

	private:

		void compressImage(size_t row) {
			// compressing the result using BC1 compression
			//unsigned int stride = width * 4;
			//auto blocksInRow = width / 4;
			//auto blocksInColumn = height / 4;
			//int index = 0;
			//for (size_t row = 0; row < blocksInRow; row++)
			//{
			//	for (size_t column = 0; column < blocksInColumn; column++)
			//	{
			//		uint8_t* cmpImageLocation = &compressedImage[index * 8];
			//		int cellIndex = 4 * (column * 4 + row * 4 * width);
			//		uint8_t* imageLocation = &cpuImage[cellIndex];
			//		//CompressBlockBC1(imageLocation, stride, cmpImageLocation);
			//		index++;
			//	}
			//}
		}

		void createVertexBuffer(DeviceManager& deviceManager, VkCommandPool& commandPool, const void* src, VkBuffer& buffer, VkDeviceMemory& deviceMemory, VkDeviceSize bufferSize);

		void createBuffer(DeviceManager& deviceManager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);
	};
}