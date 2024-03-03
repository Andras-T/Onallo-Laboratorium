#include "include/Images.h"
#include <stdexcept>
#include <Vulkan/Utils/Constants.h>

void Images::createImage(uint32_t width, uint32_t height, VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties, VkDevice& device,
	VkPhysicalDevice& physicalDevice) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	for (int i = 0; i < images.size(); i++) {
		if (vkCreateImage(device, &imageInfo, nullptr, &images[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, images[i], &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			memRequirements.memoryTypeBits, properties, physicalDevice);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &memories[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(device, images[i], memories[i], 0);
	}
}


void Images::createImageView(VkFormat format, VkImageAspectFlags aspectFlags,
	VkDevice& device) {
	for (int i = 0; i < images.size(); i++) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = images[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		if (vkCreateImageView(device, &viewInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture image view!");
		}
	}
}

uint32_t Images::findMemoryType(uint32_t typeFilter,
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

void Images::cleanUp(VkDevice& device) {
	for (size_t i = 0; i < images.size(); i++) {
		vkDestroyImage(device, images[i], nullptr);
		vkDestroyImageView(device, imageViews[i], nullptr);
		vkFreeMemory(device, memories[i], nullptr);
	}
}