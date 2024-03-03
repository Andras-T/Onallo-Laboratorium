#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include <mutex>
#include <condition_variable>
#include "GLFW/glfw3.h"

#include "Logger.h"
#include "DeviceManager.h"
#include <Vulkan/Swapchain/include/SwapchainManager.h>
#include "Vulkan/Window/include/Window.h"
#include <Vulkan/Descriptor/include/DescriptorManager.h>
#include <Vulkan/Pipeline/include/PipelineManager.h>
#include <Vulkan/CommandPool/include/CommandPoolManager.h>

namespace Client {

	class Core {
		Logger* logger;

		Window window;
		DeviceManager deviceManager;
		SwapChainManager swapChainManager;
		DescriptorManager descriptorManager;
		PipelineManager pipelineManager;
		CommandPoolManager commandPoolManager;

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkRenderPass renderPass;

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation" };

		std::mutex m;
		std::condition_variable cv;
		bool ready = false;
		bool windowShouldClose = false;
		bool connected = false;
		bool processed = false;

		void mainLoop();

	public:

		void init();

		void recieve();

		void cleanUp();

		void createInstance(bool enableValidationLayers);
		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
		void createSurface(GLFWwindow& window);
		void setupDebugMessenger(bool enableValidationLayers);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void DestroyDebugUtilsMessengerEXT();
		VkResult CreateDebugUtilsMessengerEXT(VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void createRenderPass(VkFormat imageformat);

		static void check_vk_result(VkResult err);
		static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};

}