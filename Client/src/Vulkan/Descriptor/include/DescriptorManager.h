#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include "Descriptor.h"

namespace Client {

	class DescriptorManager
	{
		std::vector<Descriptor> descriptors;
	public:

		void init() {
			//Descriptor desc;
			//DescriptorProperties prop;
			//desc.init();
			//descriptors.push_back(desc);
		}

		void cleanUp(VkDevice& device) {
			for (auto& desc : descriptors) {
				desc.cleanUp(device);
			}
		}

		Descriptor& operator[](size_t i) { return descriptors[i]; }
	};
}