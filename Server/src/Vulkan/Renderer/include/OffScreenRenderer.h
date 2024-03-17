#pragma once
#include "Renderer.h"

namespace Server {

	class OffScreenRenderer : public Renderer {

	public:

		OffScreenRenderer(DeviceManager& deviceManager,
						  DescriptorManager& descriptorManager,
						  PipelineManager& pipelineManager,
						  CommandPoolManager& commandPoolManager,
						  ResourceManager& resourceManager, VkRenderPass& renderPass) :
					Renderer(deviceManager, descriptorManager, pipelineManager,
							commandPoolManager, resourceManager, renderPass) {}

		void drawFrame(uint32_t lastFrameTime) override;

		void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex) override;
	};
}