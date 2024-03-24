#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <array>
#include <Vulkan/Utils/Constants.h>

namespace Client {

	struct DescriptorProperties {
		VkDescriptorType descriptorType;
		VkDescriptorType poolType;
		VkShaderStageFlagBits stageFlags;
		uint32_t descriptorCount;
		VkBuffer* buffer = nullptr;
		VkImageLayout imageLayout;
		VkImageView* imageViews[MAX_FRAMES_IN_FLIGHT];
		VkSampler* sampler;
		bool withDescriptorPoolOnly = false;
	};

}