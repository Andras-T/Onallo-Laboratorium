#include "include/Renderer.h"
#include <Vulkan/Utils/Constants.h>
#include <stdexcept>

namespace Server {

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

	void Renderer::cleanUp()
	{
		VkDevice& device = deviceManager.getLogicalDevice();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);

			vkDestroyFence(device, displayInFlightFences[i], nullptr);
		}
	}
}