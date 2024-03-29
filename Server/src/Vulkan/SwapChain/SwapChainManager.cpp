#include "include/SwapChainManager.h"

namespace Server {
	void SwapChainManager::init(VkSurfaceKHR& surface, DeviceManager& deviceManager, GLFWwindow* window) {
		this->indices = indices;
		this->window = window;
		this->deviceManager = &deviceManager;
		this->surface = &surface;
		indices = deviceManager.getIndices();

		createSwapChain();
		createImageViews();
	}

	void SwapChainManager::cleanUp() {
		for (auto& imageView : swapChainImageViews) {
			vkDestroyImageView(deviceManager->getLogicalDevice(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(deviceManager->getLogicalDevice(), swapChain, nullptr);
	}

	void SwapChainManager::createSwapChain() {
		SwapChainSupportDetails swapChainSupport =
			querySwapChainSupport(*surface);

		VkSurfaceFormatKHR surfaceFormat =
			chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode =
			chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = *surface;
		minImageCount = imageCount;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		uint32_t queueFamilyIndices[] = { indices.graphicsAndComputeFamily.value(),
																 indices.presentFamily.value() };

		if (indices.graphicsAndComputeFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(deviceManager->getLogicalDevice(), &createInfo, nullptr, &swapChain) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(deviceManager->getLogicalDevice(), swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(deviceManager->getLogicalDevice(), swapChain, &imageCount,
			swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void SwapChainManager::createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(deviceManager->getLogicalDevice(), &createInfo, nullptr,
				&swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image views!");
			}
		}
	}

	void SwapChainManager::recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(deviceManager->getLogicalDevice());

		cleanUp();

		createSwapChain();
		createImageViews();
	}

	SwapChainSupportDetails SwapChainManager::querySwapChainSupport(VkSurfaceKHR& surface) {
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			deviceManager->getPhysicalDevice(), surface, &swapChainSupportDetails.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(deviceManager->getPhysicalDevice(), surface, &formatCount,
			nullptr);

		if (formatCount != 0) {
			swapChainSupportDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				deviceManager->getPhysicalDevice(), surface, &formatCount,
				swapChainSupportDetails.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(deviceManager->getPhysicalDevice(), surface,
			&presentModeCount, nullptr);

		if (presentModeCount != 0) {
			swapChainSupportDetails.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				deviceManager->getPhysicalDevice(), surface, &presentModeCount,
				swapChainSupportDetails.presentModes.data());
		}

		return swapChainSupportDetails;
	}

	VkSurfaceFormatKHR SwapChainManager::chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == defaultFormat &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	// TODO: change present mode to the correct one!!!
	VkPresentModeKHR SwapChainManager::chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChainManager::chooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width !=
			std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width =
				std::clamp(actualExtent.width, capabilities.minImageExtent.width,
					capabilities.maxImageExtent.width);
			actualExtent.height =
				std::clamp(actualExtent.height, capabilities.minImageExtent.height,
					capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}