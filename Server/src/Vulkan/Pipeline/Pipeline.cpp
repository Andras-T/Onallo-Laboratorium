#include "include/Pipeline.h"

#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <Logger.h>

namespace Server {
	void Pipeline::setShaderPaths(std::optional<std::string> vertPath, std::optional<std::string> fragPath,
		std::optional<std::string> compPath) {
		this->vertPath = vertPath;
		this->fragPath = fragPath;
		this->compPath = compPath;
	}

	void Pipeline::init(VkDevice& device, std::optional<VkDescriptorSetLayout> descriptorSetLayout,
		VkRenderPass& renderPass,
		VkVertexInputBindingDescription bindingDescription,
		VkPrimitiveTopology topology,
		StageConstants& stageConstants,
		DepthStencilOptions& depthStencilOptions,
		VkBool32 alphaBlendingEnable) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = stageConstants.descriptorSetLayoutCount;
		if (pipelineLayoutInfo.setLayoutCount > 0 && descriptorSetLayout.has_value())
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.value();
		pipelineLayoutInfo.pushConstantRangeCount = stageConstants.pushConstantsCount;
		pipelineLayoutInfo.pPushConstantRanges = &stageConstants.pushConstantRange;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}
		std::string currentPath(std::filesystem::current_path().string());

		if (fragPath.has_value() && vertPath.has_value()) {
			VkPipelineShaderStageCreateInfo shaderStages[2];
			VkShaderModule vertShaderModule =
				createShaderModule(readFile(currentPath + vertPath.value()), device);
			shaderStages[0] = getPipelineShaderStageCreateInfo(
				VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule, "main");

			VkShaderModule fragShaderModule =
				createShaderModule(readFile(currentPath + fragPath.value()), device);
			shaderStages[1] = getPipelineShaderStageCreateInfo(
				VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule, "main");

			VkVertexInputAttributeDescription attributeDescription{};
			attributeDescription.binding = 0;
			attributeDescription.location = 0;
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = 0;

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType =
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType =
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = topology;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType =
				VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_NONE;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType =
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.sType =
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = depthStencilOptions.enabledepthTest;
			depthStencil.depthWriteEnable = depthStencilOptions.enabledepthWrite;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable =
				depthStencilOptions.enabledepthBoundsTest;
			depthStencil.stencilTestEnable = depthStencilOptions.enableStencilTest;
			depthStencil.minDepthBounds = 0.0f;
			depthStencil.maxDepthBounds = 1.0f;
			depthStencil.front.failOp = VK_STENCIL_OP_KEEP;      // Action if the stencil test fails
			depthStencil.front.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; // Increment the stencil value and clamp
			depthStencil.front.depthFailOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; // Action if the stencil test passes, but depth fails
			depthStencil.front.compareOp = VK_COMPARE_OP_ALWAYS; // Always pass the stencil test
			// Stencil operations for back-facing fragments
			depthStencil.back.failOp = VK_STENCIL_OP_KEEP;       // Action if the stencil test fails
			depthStencil.back.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; // Increment the stencil value and clamp
			depthStencil.back.depthFailOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;  // Action if the stencil test passes, but depth fails
			depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;  // Always pass the stencil test

			depthStencil.front.compareMask = 0xFF;  // Stencil comparison mask for front-facing fragments
			depthStencil.front.writeMask = 0xFF;    // Stencil write mask for front-facing fragments
			depthStencil.front.reference = 0;       // Reference value for the stencil test

			depthStencil.back.compareMask = 0xFF;  // Stencil comparison mask for back-facing fragments
			depthStencil.back.writeMask = 0xFF;    // Stencil write mask for back-facing fragments
			depthStencil.back.reference = 0;       // Reference value for the stencil test

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = alphaBlendingEnable;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor =
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor =
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType =
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,
														 VK_DYNAMIC_STATE_SCISSOR };

			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount =
				static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = layout;
			pipelineInfo.renderPass = renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
				nullptr, &pipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics pipeline!");
			}

			vkDestroyShaderModule(device, fragShaderModule, nullptr);
			vkDestroyShaderModule(device, vertShaderModule, nullptr);
		}
		else if (compPath.has_value()) {
			VkShaderModule compShaderModule =
				createShaderModule(readFile(currentPath + compPath.value()), device);
			VkPipelineShaderStageCreateInfo shaderStages =
				getPipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT,
					compShaderModule, "main");

			VkComputePipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipelineInfo.layout = layout;
			pipelineInfo.stage = shaderStages;

			if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
				nullptr, &pipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create compute pipeline!");
			}

			vkDestroyShaderModule(device, compShaderModule, nullptr);
		}
		else {
			Logger::getInstance().LogError("Wrong pipeline stages were given!");
		}
	}

	std::vector<char> Pipeline::readFile(const std::string& filename) {
		std::string normalizedPath = std::filesystem::canonical(filename).string();
		Logger::getInstance().LogInfo("Reading file from: " + normalizedPath);
		std::ifstream file(normalizedPath, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code, VkDevice& device) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	VkPipelineShaderStageCreateInfo
		Pipeline::getPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
			VkShaderModule module,
			const char* entryPoint) {
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = stage;
		shaderStageInfo.module = module;
		shaderStageInfo.pName = entryPoint;
		return shaderStageInfo;
	}
}