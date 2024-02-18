#pragma once

#define GLFW_INCLUDE_VULKAN

#include "Logger.h"
#include "GLFW/glfw3.h"
#include <vector>

namespace Client {

	constexpr const char* validationLayer =
		"VK_LAYER_KHRONOS_validation";

	class VulkanObject {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;

		void createInstance(bool enableValidationLayers);

		void setupDebugMessenger(bool enableValidationLayers);

		void createSurface(GLFWwindow& window);

		bool checkValidationLayerSupport();

		std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

		static VkBool32
			debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData);

		void populateDebugMessengerCreateInfo(
			VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void DestroyDebugUtilsMessengerEXT();

	public:
		void init(GLFWwindow& window, bool enableValidationLayers);

		void cleanup(bool enableValidationLayers);

		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance& instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);
	};

}