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
#include <Vulkan/Utils/Input.h>

namespace Client {
	
	class Renderer {

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;

		std::vector<VkFence> displayInFlightFences;

		Window& window;
		DeviceManager& deviceManager;
		SwapChainManager& swapChainManager;
		DescriptorManager& descriptorManager;
		PipelineManager& pipelineManager;
		CommandPoolManager& commandPoolManager;
		ResourceManager& resourceManager;		
		VkRenderPass& renderPass;

		size_t currentFrame = 0;
	public:


		Renderer(Window& window,
			DeviceManager& deviceManager,
			SwapChainManager& swapChainManager,
			DescriptorManager& descriptorManager,
			PipelineManager& pipelineManager,
			CommandPoolManager& commandPoolManager,
			ResourceManager& resourceManager, VkRenderPass& renderPass) :
			window(window), deviceManager(deviceManager),
			swapChainManager(swapChainManager), descriptorManager(descriptorManager), 
			pipelineManager(pipelineManager), commandPoolManager(commandPoolManager),
			resourceManager(resourceManager), renderPass(renderPass) {}

		void createSyncObjects();

		void drawFrame(uint32_t lastFrameTime, Input& uiInput, uint8_t* pImage);

		void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Input& uiInput, uint8_t* pImage);
	
		void cleanUp();
	};
}