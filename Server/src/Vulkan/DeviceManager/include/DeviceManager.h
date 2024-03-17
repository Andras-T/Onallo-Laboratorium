#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "Logger.h"
#include <Vulkan/Utils/QueueFamilyIndices.h>
#include <Vulkan/Utils/SwapChainSupportDetails.h>

namespace Server {

	class DeviceManager {
		Logger* logger;

		VkDevice device;
		VkPhysicalDevice physicalDevice;

		SwapChainSupportDetails swapChainSupportDetails;
		QueueFamilyIndices indices;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		uint32_t queueFamily;

		//VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
		//VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME 
		const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation" };

	public:

		void init(VkInstance& instance, bool enableValidationLayers, std::optional<VkSurfaceKHR*> surface);

		void createLogicalDevice(std::optional<VkSurfaceKHR*> surface, bool enableValidationLayers);

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& physicalDevice, std::optional<VkSurfaceKHR*> surface);

		void pickPhysicalDevice(VkInstance& instance, std::optional<VkSurfaceKHR*> surface);

		bool isDeviceSuitable(VkPhysicalDevice& physicalDevice, std::optional<VkSurfaceKHR*> surface);

		bool checkDeviceExtensionSupport(VkPhysicalDevice& physicalDevice);

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& physicalDevice, VkSurfaceKHR* surface);

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

		VkQueue& getGraphicsQueue() {
			return graphicsQueue;
		}

		VkQueue& getPresentQueue() {
			return presentQueue;
		}

		uint32_t getQueueFamily() { return queueFamily; }
	};

}