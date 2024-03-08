#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include <optional>
#include "Descriptor.h"

namespace Client {

	class DescriptorManager
	{
		std::vector<std::pair<Descriptor, std::string>> descriptors;
	public:

		void init() {
			Descriptor desc;
			DescriptorProperties prop;
			prop.descriptorCount = 1;
			prop.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			prop.poolType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			prop.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			prop.buffer = VK_NULL_HANDLE;
			desc.init(prop);
			descriptors.push_back(std::pair(desc,"ImGui"));
		}

		void createDescriptorPool(VkDevice& device) {
			for (auto& [desc, name] : descriptors)
				desc.createDescriptorPool(device);
		}


		void createDescriptorSets(VkDevice& device) {
			for (auto& [desc, name] : descriptors)
				desc.createDescriptorSets(device);
		}

		void cleanUp(VkDevice& device) {
			vkDestroyDescriptorPool(device, getDescriptor("ImGui").value().getDescriptorPool(), nullptr);
			//for (auto& [desc, name] : descriptors) {
			//	desc.cleanUp(device);
			//}
		}

		Descriptor& operator[](size_t i) { return descriptors[i].first; }

		std::optional<Descriptor> getDescriptor(std::string name) {
			for (auto& [desc, descName] : descriptors) {
				if (name._Equal(descName))
					return  std::make_optional(desc);
			}
			return std::nullopt;
		}
	};
}