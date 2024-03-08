#include "include/Descriptor.h"
#include <Logger.h>

namespace Client {

	void Descriptor::addLayout(DescriptorProperties descriptorProperties) {
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = layoutBindings.size();
		layoutBinding.descriptorCount = descriptorProperties.descriptorCount;
		layoutBinding.descriptorType = descriptorProperties.descriptorType;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = descriptorProperties.stageFlags;

		layoutBindings.push_back(layoutBinding);
	}

	void Descriptor::addPoolSize(DescriptorProperties descriptorProperties) {
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = descriptorProperties.poolType;
		poolSize.descriptorCount = descriptorProperties.descriptorCount;

		poolSizes.push_back(poolSize);
	}

	void Descriptor::createDescriptorSetLayout(VkDevice& device)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
			&descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error(
				"Failed to create simulation descriptor set layout!");
		}
	}

	void Descriptor::createDescriptorPool(VkDevice& device)
	{
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1024;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create simulation descriptor pool!");
		}
	}

	// TODO
	void Descriptor::createDescriptorSets(VkDevice& device)
	{
		Logger::getInstance().LogWarning("Descriptor::createDescriptorSets function is not implemented yet!");
	}

	void Descriptor::cleanUp(VkDevice& device)
	{
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}

}