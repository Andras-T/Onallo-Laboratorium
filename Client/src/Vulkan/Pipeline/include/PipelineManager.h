#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Pipeline.h"

namespace Client {

	class PipelineManager {

		Pipeline pipeline;

		std::string vertPath = "\\shaders\\vert.spv";
		std::string fragPath = "\\shaders\\frag.spv";

	public:

		void init(VkDevice& device, std::optional<VkDescriptorSetLayout> descriptorSetLayout, VkRenderPass& renderPass);

		void cleanUp(VkDevice& device);

	};
}