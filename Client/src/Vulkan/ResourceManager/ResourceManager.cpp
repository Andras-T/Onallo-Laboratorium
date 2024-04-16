#include "include/ResourceManager.h"
#include "include/Quad.h"

namespace Client {

	void ResourceManager::createBuffers(DeviceManager& deviceManager, VkCommandPool& commandPool)
	{
		createVertexBuffer(deviceManager, commandPool, Quad::quadVertices.data(),
			quadBuffer, quadMemory,
			Quad::quadVertices.size() * sizeof(float));

		uint32_t pixelSize = DEFAULT_PIXEL_SIZE;
		VkDeviceSize size = pixelSize * static_cast<uint32_t>(DEFAULT_IMAGE_WIDTH) * static_cast<uint32_t>(DEFAULT_IMAGE_HEIGHT);
		createBuffer(deviceManager, size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);
	}

	void ResourceManager::createImageResources(DeviceManager& deviceManager, CommandPoolManager& commandPoolManager)
	{
		decompressedImage = new uint8_t[DEFAULT_IMAGE_WIDTH * DEFAULT_IMAGE_HEIGHT * DEFAULT_PIXEL_SIZE];
		images.init(MAX_FRAMES_IN_FLIGHT);
		images.createImage(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_TILING_LINEAR,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			deviceManager.getLogicalDevice(), deviceManager.getPhysicalDevice());
		images.createImageView(VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT, deviceManager.getLogicalDevice());

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			commandPoolManager.transitionImageLayout(deviceManager.getLogicalDevice(), images.getImage(i),
				VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, deviceManager.getGraphicsQueue());
		}
	}

	void ResourceManager::createVertexBuffer(DeviceManager& deviceManager,
		VkCommandPool& commandPool,
		const void* src, VkBuffer& buffer,
		VkDeviceMemory& deviceMemory,
		VkDeviceSize bufferSize) {
		VkDevice device = deviceManager.getLogicalDevice();
		VkPhysicalDevice physicalDevice = deviceManager.getPhysicalDevice();

		VkBuffer buffer_;
		VkDeviceMemory deviceMemory_;
		createBuffer(deviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer_, deviceMemory_);

		void* bufferData;
		vkMapMemory(device, deviceMemory_, 0, bufferSize, 0, &bufferData);
		memcpy(bufferData, src, (size_t)bufferSize);
		vkUnmapMemory(device, deviceMemory_);

		createBuffer(deviceManager, bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, deviceMemory);
		copyBuffer(device, commandPool, deviceManager.getGraphicsQueue(), buffer_,
			buffer, bufferSize);

		vkDestroyBuffer(device, buffer_, nullptr);
		vkFreeMemory(device, deviceMemory_, nullptr);
	}

	void ResourceManager::createBuffer(DeviceManager& deviceManager,
		VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(deviceManager.getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(deviceManager.getLogicalDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			memRequirements.memoryTypeBits, properties, deviceManager.getPhysicalDevice());

		if (vkAllocateMemory(deviceManager.getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(deviceManager.getLogicalDevice(), buffer, bufferMemory, 0);
	}

	void ResourceManager::copyBuffer(VkDevice& device, VkCommandPool& commandPool,
		VkQueue& graphicsQueue, VkBuffer srcBuffer,
		VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	uint32_t ResourceManager::findMemoryType(uint32_t typeFilter,
		VkMemoryPropertyFlags properties,
		VkPhysicalDevice& physicalDevice) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
				properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}
}