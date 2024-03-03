#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>

#include "Vulkan/Utils/DescriptorProperties.h"
#include <stdexcept>

namespace Client {

	class Descriptor {
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		std::vector<VkDescriptorPoolSize> poolSizes{};

		int n = 0;

	public:

		template<typename... Args>
			requires (std::is_same_v<Args, DescriptorProperties> && ...)
		void init(Args... descriptorProperties) {
			n = sizeof...(descriptorProperties);
			layoutBindings.reserve(n);
			poolSizes.reserve(n);
			(addLayout(descriptorProperties), ...);
			(addPoolSize(descriptorProperties), ...);
		}

		void addLayout(DescriptorProperties descriptorProperties);
		void addPoolSize(DescriptorProperties descriptorProperties);

		void createDescriptorSetLayout(VkDevice& device);
		void createDescriptorPool(VkDevice& device);
		void createDescriptorSets(VkDevice& device);

		void cleanUp(VkDevice& device);

		VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout; }
		VkDescriptorPool& getDescriptorPool() { return descriptorPool; }
	};

}