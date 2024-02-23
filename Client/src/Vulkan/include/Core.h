#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include "GLFW/glfw3.h"
#include "Logger.h"
#include "Vulkan/Window/include/Window.h"

namespace Client {

	class Core {
		Logger* logger;
		Window window;

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation" };

		void mainLoop();

	public:
		void init();

		void cleanUp();

		void createInstance(bool enableValidationLayers);
		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
		void createSurface(GLFWwindow& window);
		void setupDebugMessenger(bool enableValidationLayers);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void DestroyDebugUtilsMessengerEXT();
		VkResult CreateDebugUtilsMessengerEXT(VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		static void check_vk_result(VkResult err);
		static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};

}