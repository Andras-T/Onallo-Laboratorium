#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include <Vulkan/Swapchain/include/SwapchainManager.h>
#include "Vulkan/Window/include/Window.h"
#include <Vulkan/Descriptor/include/DescriptorManager.h>
#include <Vulkan/Pipeline/include/PipelineManager.h>
#include <Vulkan/CommandPool/include/CommandPoolManager.h>
#include <Vulkan/ResourceManager/include/ResourceManager.h>

namespace Server {
	
	class Renderer {
	protected:
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;

		std::vector<VkFence> displayInFlightFences;

		DeviceManager& deviceManager;
		DescriptorManager& descriptorManager;
		PipelineManager& pipelineManager;
		CommandPoolManager& commandPoolManager;
		ResourceManager& resourceManager;		
		VkRenderPass& renderPass;

		size_t currentFrame = 0;

	public:

		Renderer(DeviceManager& deviceManager,
			DescriptorManager& descriptorManager,
			PipelineManager& pipelineManager,
			CommandPoolManager& commandPoolManager,
			ResourceManager& resourceManager, VkRenderPass& renderPass) :
			deviceManager(deviceManager),
			descriptorManager(descriptorManager), pipelineManager(pipelineManager),
			commandPoolManager(commandPoolManager), resourceManager(resourceManager),
			renderPass(renderPass) {}

		void createSyncObjects();

		void cleanUp();

		virtual void drawFrame(uint32_t lastFrameTime) = 0;

		virtual void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex) = 0;
	};
}