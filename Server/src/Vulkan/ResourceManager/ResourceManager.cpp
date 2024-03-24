#include "include/ResourceManager.h"
#include "include/Quad.h"

namespace Server {

	void ResourceManager::createBuffers(DeviceManager& deviceManager, VkCommandPool& commandPool, std::optional<GLFWwindow*> window)
	{
		createVertexBuffer(deviceManager, commandPool, Quad::quadVertices.data(),
			quadBuffer, quadMemory,
			Quad::quadVertices.size() * sizeof(float));

		if (window.has_value())
		{

			int width, height;
			glfwGetFramebufferSize(window.value(), &width, &height);
			auto temp = static_cast<uint32_t>(width) * static_cast<uint32_t>(height);
			size = temp * pixelSize;
		}
		else {
			size = DEFAULT_WIDTH * DEFAULT_HEIGHT * pixelSize;
		}

		createBuffer(deviceManager, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
			stagingBuffer, stagingBufferMemory);
		cpuImage = new uint8_t[size];
	}

	void ResourceManager::recreateStagingBuffer(DeviceManager& deviceManager, std::optional<GLFWwindow*> window)
	{
		vkDestroyBuffer(deviceManager.getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, nullptr);

		if (window.has_value())
		{

			int width, height;
			glfwGetFramebufferSize(window.value(), &width, &height);
			this->width = static_cast<uint32_t>(width);
			this->height = static_cast<uint32_t>(height);
			size = width * height * pixelSize;
		}
		else {
			size = DEFAULT_WIDTH * DEFAULT_HEIGHT * pixelSize;
		}

		createBuffer(deviceManager, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
			stagingBuffer, stagingBufferMemory);
		delete[] cpuImage;
		cpuImage = new uint8_t[size];
	}

	void ResourceManager::copyImageToStagingBuffer(VkImage& image, DeviceManager& deviceManager, VkCommandBuffer& commandBuffer) {
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcQueueFamilyIndex = deviceManager.getQueueFamily();
		barrier.dstQueueFamilyIndex = deviceManager.getQueueFamily();
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
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
		region.imageExtent = { width, height, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

		VkImageMemoryBarrier barrier2 = {};
		barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier2.srcQueueFamilyIndex = deviceManager.getQueueFamily();
		barrier2.dstQueueFamilyIndex = deviceManager.getQueueFamily();
		barrier2.image = image;
		barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier2.subresourceRange.baseMipLevel = 0;
		barrier2.subresourceRange.levelCount = 1;
		barrier2.subresourceRange.baseArrayLayer = 0;
		barrier2.subresourceRange.layerCount = 1;

		barrier2.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier2.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier2
		);

		//commandPoolManager.endSingleTimeCommands(deviceManager.getLogicalDevice(), commandBuffer, deviceManager.getGraphicsQueue());

		vkMapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, 0, size, 0, &mappedData);

		memcpy(cpuImage, mappedData, (size_t)size);

		// for testing the "cpu" image
		//int sum = 0;
		//for (size_t i = 0; i < size; i++) {
		//	sum += cpuImage[i] != 0 ? 1 : 0;
		//}
		//
		//std::cout << "\n\n" << sum << "\n\n";

		vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);
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

	void ResourceManager::createFrameBuffers(std::vector<VkImageView>& imageViews, VkRenderPass& renderPass, VkDevice& device, std::optional<VkExtent2D> extent) {
		frameBuffers.resize(imageViews.size());
		for (int i = 0; i < frameBuffers.size(); i++) {
			std::array<VkImageView, 1> attachments = { imageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount =
				static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			if (extent.has_value()) {
				framebufferInfo.width = extent.value().width;
				framebufferInfo.height = extent.value().height;
			}
			else {
				framebufferInfo.width = DEFAULT_WIDTH;
				framebufferInfo.height = DEFAULT_HEIGHT;
			}
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
				&frameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
		Logger::getInstance().LogInfo("Framebuffers created successfully!");
	}
}