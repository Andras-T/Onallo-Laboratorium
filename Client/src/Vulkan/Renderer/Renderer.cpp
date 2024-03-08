#include "include/Renderer.h"
#include <Vulkan/Utils/Constants.h>
#include <stdexcept>

namespace Client {

	void Renderer::createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

		displayInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		auto& d = deviceManager;
		auto device = d.getLogicalDevice();
		auto ph = d.getPhysicalDevice();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(deviceManager.getLogicalDevice(), &semaphoreInfo, nullptr,
				&imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(deviceManager.getLogicalDevice(), &semaphoreInfo, nullptr,
					&renderFinishedSemaphores[i]) != VK_SUCCESS) {
				throw std::runtime_error(
					"Failed to create semaphore synchronization objects!");
			}
			if (vkCreateFence(deviceManager.getLogicalDevice(), &fenceInfo, nullptr, &displayInFlightFences[i]) !=
				VK_SUCCESS) {
				throw std::runtime_error(
					"Failed to create fence synchronization objects!");
			}
		}
	}

	void Renderer::drawFrame(uint32_t lastFrameTime)
	{
		auto& device = deviceManager.getLogicalDevice();

		vkWaitForFences(device, 1, &displayInFlightFences[currentFrame], VK_TRUE,
			UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(
			device, swapChainManager.getSwapChain(), UINT64_MAX,
			imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			swapChainManager.recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		vkResetFences(device, 1, &displayInFlightFences[currentFrame]);

		auto& commandBuffer = commandPoolManager.getCommandBuffers()[currentFrame];
		vkResetCommandBuffer(commandBuffer, 0);
		recordCommandBuffer(commandBuffer, imageIndex);

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

		// TODO: check waitStages
		VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(deviceManager.getGraphicsQueue(), 1, &submitInfo,
			displayInFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkSwapchainKHR swapChains[] = { swapChainManager.getSwapChain() };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(deviceManager.getPresentQueue(), &presentInfo);
		
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			Window::framebufferResized) {
			Window::framebufferResized = false;
			swapChainManager.recreateSwapChain();
			// TODO: create recreateDescriptorSets function
			//descriptorManager.recreateDescriptorSets();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	// TODO
	void Renderer::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording blur command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainManager.getFrameBuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainManager.getSwapChainExtent();

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
		viewport.width = (float)swapChainManager.getSwapChainExtent().width;
		viewport.height = (float)swapChainManager.getSwapChainExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainManager.getSwapChainExtent();
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