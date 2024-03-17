#pragma once
#include "Renderer.h"

namespace Server {

	class WindowedRenderer : public Renderer {

		Window& window;
		SwapChainManager& swapChainManager;

	public:

		WindowedRenderer(Window& window, 
			DeviceManager& deviceManager,
			SwapChainManager& swapChainManager,
			DescriptorManager& descriptorManager,
			PipelineManager& pipelineManager,
			CommandPoolManager& commandPoolManager,
			ResourceManager& resourceManager, VkRenderPass& renderPass) :
			window(window), swapChainManager(swapChainManager),
			Renderer(deviceManager, descriptorManager, pipelineManager,
				commandPoolManager, resourceManager, renderPass) {}

		void drawFrame(uint32_t lastFrameTime) override;

		void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex) override;
	};

}