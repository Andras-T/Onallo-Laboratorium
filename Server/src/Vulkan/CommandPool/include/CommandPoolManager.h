#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <stdexcept>
#include <vector>
#include <Vulkan/Utils/QueueFamilyIndices.h>

namespace Server {

	class CommandPoolManager {
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

	public:

		void createCommandPool(VkDevice& device, QueueFamilyIndices& queueFamilyIndices);

		void createCommandBuffers(VkDevice& device);

		VkCommandBuffer beginSingleTimeCommands(VkDevice& device);

		void endSingleTimeCommands(VkDevice& device, VkCommandBuffer commandBuffer,
			VkQueue& graphicsQueue);

		void transitionImageLayout(VkDevice& device, VkImage image, VkFormat format,
			VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue& graphicsQueue,
			uint32_t layerCount = 1);

		void cleanUp(VkDevice& device);

		VkCommandPool& getCommandPool() { return commandPool; }

		std::vector<VkCommandBuffer>& getCommandBuffers() {
			return commandBuffers;
		}
	};
}