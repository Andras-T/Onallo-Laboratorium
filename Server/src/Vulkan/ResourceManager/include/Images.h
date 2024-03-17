#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>

namespace Server {

	class Images {
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkDeviceMemory> memories;

		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);

	public:

		void init(unsigned int numberOfImages) {
			images.resize(numberOfImages);
			imageViews.resize(numberOfImages);
			memories.resize(numberOfImages);
		}

		void createImage(uint32_t width, uint32_t height, VkFormat format,
			VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkDevice& device,
			VkPhysicalDevice& physicalDevice);

		void createImageView(VkFormat format, VkImageAspectFlags aspectFlags,
			VkDevice& device);

		void cleanUp(VkDevice& device);

		std::vector<VkImage>& getImages() { return images; }
		VkImage& getImage(size_t i) { return images[i]; }

		std::vector<VkDeviceMemory>& getImageMemories() { return memories; }
		VkDeviceMemory& getImageMemory(size_t i) { return memories[i]; }

		std::vector<VkImageView>& getImageViews() { return imageViews; }
		VkImageView& getImageView(size_t i) { return imageViews[i]; }
	};
}