#include "include/OffScreenRenderer.h"

namespace Server {

	void OffScreenRenderer::drawFrame(uint32_t lastFrameTime)
	{
		auto& device = deviceManager.getLogicalDevice();

		vkWaitForFences(device, 1, &displayInFlightFences[currentFrame], VK_TRUE,
			UINT64_MAX);

		static uint32_t imageIndex = 0;

		vkResetFences(device, 1, &displayInFlightFences[currentFrame]);

		auto& commandBuffer = commandPoolManager.getCommandBuffers()[currentFrame];
		vkResetCommandBuffer(commandBuffer, 0);
		recordCommandBuffer(commandBuffer, imageIndex);

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

		VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(deviceManager.getGraphicsQueue(), 1, &submitInfo,
			displayInFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		imageIndex = (imageIndex + 1) % 3;
	}

	void OffScreenRenderer::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording blur command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = resourceManager.getFrameBuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { DEFAULT_WIDTH, DEFAULT_HEIGHT };

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { 1.0f, 0.5f, 0.5f, 1.0f };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineManager.getPipeline().getVkPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;

		viewport.width = (float)DEFAULT_WIDTH;
		viewport.height = (float)DEFAULT_HEIGHT;

		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { DEFAULT_WIDTH, DEFAULT_HEIGHT };

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(resourceManager.getQuadBuffer()),
			offsets);

		//auto descriptorSet = &instance->getBlurDescriptorSets()[currentFrame];
		//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		//	pipelineManager->getBlurPipelineLayout(), 0, 1,
		//	descriptorSet, 0, nullptr);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}