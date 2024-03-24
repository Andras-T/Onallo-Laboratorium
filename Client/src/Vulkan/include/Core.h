#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include <mutex>
#include <thread>
#include <condition_variable>
#include "GLFW/glfw3.h"

#include "Logger.h"
#include <Networking/include/ClientNetworking.h>
#include <Vulkan/CommandPool/include/CommandPoolManager.h>
#include <Vulkan/Descriptor/include/DescriptorManager.h>
#include <Vulkan/DeviceManager/include/DeviceManager.h>
#include <Vulkan/Pipeline/include/PipelineManager.h>
#include <Vulkan/Renderer/include/Renderer.h>
#include <Vulkan/ResourceManager/include/ResourceManager.h>
#include <Vulkan/Swapchain/include/SwapchainManager.h>
#include <Vulkan/Window/include/Window.h>
#include <Vulkan/Utils/Input.h>

namespace Client {

	class Core {
		Logger* logger;

		Window window;
		DeviceManager deviceManager;
		SwapChainManager swapChainManager;
		DescriptorManager descriptorManager;
		PipelineManager pipelineManager;
		CommandPoolManager commandPoolManager;
		ResourceManager resourceManager;
		Renderer* renderer;

		Input uiInput;
		NetworkMessage networkMessage;
		ClientNetworking client;

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkRenderPass renderPass;

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation" };

		double lastFrameTime = 0.0f;

		std::thread reciever;
		std::mutex m;
		std::condition_variable cv;
		bool imageRendered = false;
		bool recieved = false;

	public:

		void init();

		void run();

	private:

		void mainLoop();

		void compileShaders();

		void recieve();

		void cleanUp();

		void createInstance(bool enableValidationLayers);
		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
		void createSurface(GLFWwindow* window);
		void setupDebugMessenger(bool enableValidationLayers);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void DestroyDebugUtilsMessengerEXT();
		void initImGui();
		VkResult CreateDebugUtilsMessengerEXT(VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void createRenderPass(VkFormat imageformat);

		static void check_vk_result(VkResult err);
		static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};

}