#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "include/PipelineManager.h"
#include "glm/glm.hpp"
#include <Vulkan/Utils/DepthStencilOptions.h>
#include <Logger.h>

namespace Client {

	void PipelineManager::init(VkDevice& device, std::optional<VkDescriptorSetLayout> descriptorSetLayout, VkRenderPass& renderPass)
	{
		pipeline.setShaderPaths(vertPath, fragPath, std::optional<std::string>(std::nullopt));

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(glm::vec4);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		StageConstants stageConstants;
		DepthStencilOptions depthStencilOptions(VK_TRUE, VK_TRUE, VK_TRUE,
			VK_TRUE);
		pipeline.init(
			device, descriptorSetLayout, renderPass,
			bindingDescription, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
			stageConstants, depthStencilOptions);

		Logger::getInstance().LogInfo("Pipeline created");
	}

	void PipelineManager::cleanUp(VkDevice& device)
	{
		pipeline.cleanUp(device);
	}

}