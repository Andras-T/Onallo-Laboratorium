#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include <optional>
#include "Descriptor.h"
#include <Vulkan/ResourceManager/include/ResourceManager.h>

namespace Client {

	class DescriptorManager
	{
		std::vector<Descriptor> descriptors;
		VkSampler sampler;

	public:

		void initDescriptors(ResourceManager& resourceManager, VkDevice& device) {
			// 2D sampler info
			{
				VkSamplerCreateInfo samplerInfo{};
				samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerInfo.magFilter = VK_FILTER_LINEAR;
				samplerInfo.minFilter = VK_FILTER_LINEAR;
				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.anisotropyEnable = VK_FALSE;
				samplerInfo.maxAnisotropy = 1.0f;
				samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
				samplerInfo.unnormalizedCoordinates = VK_FALSE;
				samplerInfo.compareEnable = VK_TRUE;
				samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
				samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

				if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) !=
					VK_SUCCESS) {
					throw std::runtime_error("Failed to create sampler!");
				}
			}

			DescriptorProperties imageDescriptorProperties{};
			imageDescriptorProperties.descriptorCount = 1;
			imageDescriptorProperties.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageDescriptorProperties.poolType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageDescriptorProperties.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			imageDescriptorProperties.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageDescriptorProperties.sampler = &sampler;
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				imageDescriptorProperties.imageViews[i] = &resourceManager.getImageView(i);
			}

			DescriptorProperties imguiDescriptorProperties{};
			imguiDescriptorProperties.descriptorCount = 1;
			imguiDescriptorProperties.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imguiDescriptorProperties.poolType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imguiDescriptorProperties.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			imguiDescriptorProperties.withDescriptorPoolOnly = true;

			descriptors.push_back(Descriptor(imageDescriptorProperties, imguiDescriptorProperties));
		}

		void createDescriptors(VkDevice& device) {
			for (auto& desc : descriptors) {
				desc.createDescriptorSetLayout(device);
				desc.createDescriptorPool(device);
				desc.createDescriptorSets(device);
			}
		}

		void cleanUp(VkDevice& device) {
			vkDestroySampler(device, sampler, nullptr);
			for (auto& desc : descriptors) {
				desc.cleanUp(device);
			}
		}

		Descriptor& getDescriptor() {
			return descriptors[0];
		}

		Descriptor& operator[](size_t i) {
			if (i < 0 || i >= descriptors.size())
			{
				Logger::getInstance().LogError(std::format("Couldn't find #{} descriptor", i));
				throw std::runtime_error(std::format("Couldn't find #{} descriptor", i));
			}
			return descriptors[i];
		}
	};
}