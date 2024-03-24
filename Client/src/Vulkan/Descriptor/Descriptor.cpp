#include "include/Descriptor.h"
#include <Logger.h>

namespace Client {

	void Descriptor::createDescriptorSetLayout(VkDevice& device) {

		for (auto& descriptorProperty : descriptorProperties)
		{
			if (descriptorProperty.withDescriptorPoolOnly)
				continue;

			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = layoutBindings.size();
			layoutBinding.descriptorCount = descriptorProperty.descriptorCount;
			layoutBinding.descriptorType = descriptorProperty.descriptorType;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = descriptorProperty.stageFlags;

			layoutBindings.push_back(layoutBinding);
		}

		if (layoutBindings.size() == 0) {
			Logger::getInstance(). LogError("Couldn't create descriptorSetLayout!");
			return;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
			&descriptorSetLayout) != VK_SUCCESS) {
			Logger::getInstance().LogError("Failed to create simulation descriptor set layout!");
			throw std::runtime_error("Failed to create simulation descriptor set layout!");
		}
	}

	void Descriptor::createDescriptorPool(VkDevice& device)
	{
		for (auto& descriptorProperty : descriptorProperties)
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = descriptorProperty.poolType;
			poolSize.descriptorCount = descriptorProperty.descriptorCount;
			descriptorCount += descriptorProperty.descriptorCount;

			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = descriptorCount * 2;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
			VK_SUCCESS) {
			Logger::getInstance().LogError("Failed to create simulation descriptor pool!");
			throw std::runtime_error("Failed to create simulation descriptor pool!");
		}
	}

	void Descriptor::createDescriptorSets(VkDevice& device)
	{
		Logger::getInstance().LogInfo("Allocating descriptor sets!");

		if (layoutBindings.size() == 0)
			return;

		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
			descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount =
			static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) !=
			VK_SUCCESS) {
			Logger::getInstance().LogError("Failed to allocate descriptor sets!");
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		std::vector<VkWriteDescriptorSet> descriptorWrites;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			int index = 0;
			for (auto& descriptorProperty : descriptorProperties)
			{
				if (descriptorProperty.withDescriptorPoolOnly)
					continue;

				VkWriteDescriptorSet descriptorSet{};
				descriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorSet.dstSet = descriptorSets[i];
				descriptorSet.dstArrayElement = 0;
				descriptorSet.dstBinding = index++;
				descriptorSet.descriptorType = descriptorProperty.descriptorType;
				descriptorSet.descriptorCount = descriptorProperty.descriptorCount;

				if (descriptorProperty.buffer != nullptr)
				{
					size_t last = 0;
					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = descriptorProperty.buffer[i];
					bufferInfo.offset = 0;
					bufferInfo.range = VK_WHOLE_SIZE;
					bufferInfos[i].push_back(bufferInfo);
					last = bufferInfos[i].size() - 1;
					descriptorSet.pBufferInfo = &bufferInfos[i][last];
					descriptorSet.pImageInfo = VK_NULL_HANDLE;
				}
				else {
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = descriptorProperty.imageLayout;
					imageInfo.imageView = *descriptorProperty.imageViews[i];
					imageInfo.sampler = *descriptorProperty.sampler;

					descriptorSet.pImageInfo = &imageInfo;
				}

				descriptorWrites.push_back(descriptorSet);
			}

			vkUpdateDescriptorSets(device,
				static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(), 0, nullptr);
			descriptorWrites.clear();
		}
	}

	void Descriptor::cleanUp(VkDevice& device)
	{
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		if (layoutBindings.size() == 0)
			return;

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}

}