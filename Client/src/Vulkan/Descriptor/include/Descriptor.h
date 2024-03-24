#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>

#include "Vulkan/Utils/DescriptorProperties.h"
#include <Vulkan/Utils/Constants.h>
#include <stdexcept>

namespace Client {

	class Descriptor {
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos;
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		std::vector<VkDescriptorPoolSize> poolSizes{};
		std::vector<DescriptorProperties> descriptorProperties;
		int descriptorCount = 0;

	public:

		template<typename... Args>
			requires (std::is_same_v<Args, DescriptorProperties> && ...)
		Descriptor(Args... descriptorProperties_) {
			((this->descriptorProperties).push_back(descriptorProperties_), ...);
			layoutBindings.reserve(descriptorProperties.size());
			poolSizes.reserve(descriptorProperties.size());

			bufferInfos.resize(descriptorProperties.size());
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				bufferInfos.reserve(descriptorProperties.size());
			}
			descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		}

		void createDescriptorSetLayout(VkDevice& device);
		void createDescriptorPool(VkDevice& device);
		void createDescriptorSets(VkDevice& device);

		void cleanUp(VkDevice& device);

		VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout; }
		VkDescriptorPool& getDescriptorPool() { return descriptorPool; }
		std::vector<VkDescriptorSet>& getDescriptorSets() { return descriptorSets; }
	};

}