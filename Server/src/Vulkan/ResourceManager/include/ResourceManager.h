#pragma once

#define GLFW_INCLUDE_VULKAN
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
		void* mappedData;
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
		}

		inline uint8_t* getCPUpuImage() {
			return cpuImage;
		}

		VkBuffer& getQuadBuffer() { return quadBuffer; }

		std::vector<VkFramebuffer>& getFrameBuffers() { return frameBuffers; }

		Images& getPresentImages() {
			return presentImages;
		}

		inline VkDeviceSize getCPUImageSize() { return size; }

	private:

		void createVertexBuffer(DeviceManager& deviceManager, VkCommandPool& commandPool, const void* src, VkBuffer& buffer, VkDeviceMemory& deviceMemory, VkDeviceSize bufferSize);

		void createBuffer(DeviceManager& deviceManager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);
	};
}