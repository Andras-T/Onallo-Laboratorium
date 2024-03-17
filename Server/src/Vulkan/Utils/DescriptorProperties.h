#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Server {

	struct DescriptorProperties {
		VkDescriptorType descriptorType;
		VkDescriptorType poolType;
		VkShaderStageFlagBits stageFlags;
		uint32_t descriptorCount;
		VkBuffer buffer;
	};
}