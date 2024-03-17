#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>
#include <optional>
#include <vector>
#include <Vulkan/Utils/DepthStencilOptions.h>
#include <Vulkan/Utils/StageConstants.h>

namespace Server {

	class Pipeline {

		VkPipelineLayout layout = {};
		VkPipeline pipeline = {};

		std::optional<std::string> vertPath;
		std::optional<std::string> fragPath;
		std::optional<std::string> compPath;

		std::vector<char> readFile(const std::string& filename);

		VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice& device);

		VkPipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint);


	public:

		void setShaderPaths(std::optional<std::string> vertPath, std::optional<std::string> fragPath,
			std::optional<std::string> compPath);

		void init(VkDevice& device, std::optional<VkDescriptorSetLayout> descriptorSetLayout,
			VkRenderPass& renderPass,
			VkVertexInputBindingDescription bindingDescription,
			VkPrimitiveTopology topology,
			StageConstants& stageConstants,
			DepthStencilOptions& depthStencilOptions,
			VkBool32 alphaBlendingEnable = VK_FALSE);

		void cleanUp(VkDevice& device) {
			vkDestroyPipeline(device, pipeline, nullptr);
			vkDestroyPipelineLayout(device, layout, nullptr);
		}

		VkPipeline& getVkPipeline() { return pipeline; }

		VkPipelineLayout& getPipelineLayout() { return layout; }

	};

}