#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include <optional>
#include "Descriptor.h"

namespace Server {

	class DescriptorManager {

	public:

		void init();

		void createDescriptorPool(VkDevice& device);

		void createDescriptorSets(VkDevice& device);

		void cleanUp(VkDevice& device);
	};
}