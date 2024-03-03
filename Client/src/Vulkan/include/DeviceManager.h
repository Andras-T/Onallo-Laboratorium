#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>
#include <set>
#include <stdexcept>

#include "Logger.h"
#include <Vulkan/Utils/QueueFamilyIndicies.h>
#include <Vulkan/Utils/SwapChainSupportDetails.h>

namespace Client {

	class DeviceManager {
		VkDevice device;
		VkPhysicalDevice physicalDevice;

		SwapChainSupportDetails swapChainSupportDetails;
		QueueFamilyIndices indices;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		// TODO: check if they are needed
		const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
		VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME };

		const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation" };

		Logger* logger;

	public:
		void init(VkInstance& instance, VkSurfaceKHR& surface,
			bool enableValidationLayers);

		void createLogicalDevice(VkSurfaceKHR& surface, bool enableValidationLayers);

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

		void pickPhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);

		bool isDeviceSuitable(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

		bool checkDeviceExtensionSupport(VkPhysicalDevice& physicalDevice);

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& physicalDevice,
			VkSurfaceKHR& surface);

		void cleanup();

		VkPhysicalDevice& getPhysicalDevice() {
			return physicalDevice;
		}

		VkDevice& getLogicalDevice() {
			return device;
		}

		QueueFamilyIndices& getIndices() {
			return indices;
		}
	};
}