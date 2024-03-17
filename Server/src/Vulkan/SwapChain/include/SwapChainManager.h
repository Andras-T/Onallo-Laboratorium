#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>
#include <array>
#include <algorithm>

#include <Vulkan/Utils/Constants.h>
#include <Vulkan/DeviceManager/include/DeviceManager.h>


namespace Server {
	class SwapChainManager {
		VkSwapchainKHR swapChain;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		SwapChainSupportDetails swapChainSupportDetails;

		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;

		uint32_t minImageCount;

		QueueFamilyIndices indices;
		GLFWwindow* window;
		DeviceManager* deviceManager;
		VkSurfaceKHR* surface;
		VkRenderPass* renderPass;

	public:

		void init(VkSurfaceKHR& surface, DeviceManager& deviceManager, GLFWwindow* window);

		void recreateSwapChain();

		void cleanUp();

		VkFormat getSwapChainImageFormat() {
			return swapChainImageFormat;
		}

		VkSwapchainKHR& getSwapChain() { return swapChain; }

		uint32_t getMinImageCount() { return minImageCount; }

		uint32_t getSwapchainImageCount() { return swapChainImages.size(); }

		VkExtent2D& getSwapChainExtent() { return swapChainExtent; }

		std::vector<VkImageView>& getSwapChainImageViews() {
			return swapChainImageViews;
		}

	private:

		void createImageViews();

		void createSwapChain();

		SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR& surface);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(
			const VkSurfaceCapabilitiesKHR& capabilities);
	};
}