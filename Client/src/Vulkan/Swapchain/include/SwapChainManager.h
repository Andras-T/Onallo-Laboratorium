#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>
#include <array>
#include <algorithm>

#include "Images.h"
#include <Vulkan/Utils/Constants.h>
#include <Vulkan/include/DeviceManager.h>

namespace Client {

	class SwapChainManager {
		VkSwapchainKHR swapChain;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		SwapChainSupportDetails swapChainSupportDetails;

		Images images;
		
		uint32_t minImageCount;

		std::vector<VkFramebuffer> frameBuffers;

		QueueFamilyIndices indices;
		GLFWwindow* window;
		DeviceManager* deviceManager;

	public:

		void init(VkSurfaceKHR& surface, DeviceManager* deviceManager, GLFWwindow* window);

		void cleanUp() {
			images.cleanUp(deviceManager->getLogicalDevice());
			
			for (auto& framebuffer : frameBuffers)
				vkDestroyFramebuffer(deviceManager->getLogicalDevice(), framebuffer, nullptr);

			vkDestroySwapchainKHR(deviceManager->getLogicalDevice(), swapChain, nullptr);
		}

		void createSwapChain(VkSurfaceKHR& surface);

		void createImages(VkDevice& device, VkPhysicalDevice& physicalDevice) {
			images.createImage(swapChainExtent.width, swapChainExtent.height, swapChainImageFormat, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device, physicalDevice);
			images.createImageView(swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, device);
		}

		void createFrameBuffers(VkRenderPass& renderPass) {
			frameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
			for (int i = 0; i < frameBuffers.size(); i++) {
				std::array<VkImageView, 1> attachments = { images.getImageView(i) };

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount =
					static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = swapChainExtent.width;
				framebufferInfo.height = swapChainExtent.height;
				framebufferInfo.layers = 1;
				
				if (vkCreateFramebuffer(deviceManager->getLogicalDevice(), &framebufferInfo, nullptr,
					&frameBuffers[i]) != VK_SUCCESS) {
					throw std::runtime_error("Failed to create framebuffer!");
				}
			}
			Logger::getInstance().LogInfo("Framebuffers created successfully!");
		}

		SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR& surface);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(
			const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat getSwapChainImageFormat() {
			return swapChainImageFormat;
		}
	};

}