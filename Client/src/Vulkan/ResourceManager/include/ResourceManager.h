#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <Vulkan/DeviceManager/include/DeviceManager.h>

namespace Client {

	class ResourceManager {

		VkBuffer quadBuffer;
		VkDeviceMemory quadMemory;

	public:

		void init(DeviceManager& deviceManager, VkCommandPool& commandPool);

		void createVertexBuffer(DeviceManager& deviceManager, VkCommandPool& commandPool, const void* src, VkBuffer& buffer, VkDeviceMemory& deviceMemory, VkDeviceSize bufferSize);

		void createBuffer(DeviceManager& deviceManager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);

		VkBuffer& getQuadBuffer() { return quadBuffer; }

		void cleanUp(VkDevice& device) {
			vkDestroyBuffer(device, quadBuffer, nullptr);
			vkFreeMemory(device, quadMemory, nullptr);
		}

	};

}