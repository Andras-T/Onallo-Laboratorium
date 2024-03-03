#include "include/Core.h"
#include <thread>

namespace Client {

	void Core::init() {
		logger = &(Logger::getInstance());
		logger->LogInfo("Init Core");

		window.init("Onallo Laboratorium");
		createInstance(enableValidationLayers);
		setupDebugMessenger(enableValidationLayers);
		createSurface(*window.get_GLFW_Window());
		deviceManager.init(instance, surface, enableValidationLayers);
		swapChainManager.init(surface, &deviceManager, window.get_GLFW_Window());
		createRenderPass(swapChainManager.getSwapChainImageFormat());
		descriptorManager.init();
		pipelineManager.init(deviceManager.getLogicalDevice(), std::nullopt/*descriptorManager[0].getDescriptorSetLayout()*/,
			renderPass);
		commandPoolManager.createCommandPool(deviceManager.getLogicalDevice(), deviceManager.getIndices());
		swapChainManager.createImages(deviceManager.getLogicalDevice(), deviceManager.getPhysicalDevice());
		swapChainManager.createFrameBuffers(renderPass);

		std::thread reciever(&Core::recieve, this);

		mainLoop();
		reciever.join();
	}

	void Core::recieve() {
		std::unique_lock lk(m);
		cv.wait(lk, [&] { return ready || windowShouldClose; });

		// TODO: store the data in an array
		while (!glfwWindowShouldClose(window.get_GLFW_Window()))
		{
			std::cout << "--- ";
			//...recieve data
		}
		std::cout << "\nwindow should close\n";
	}

	void Core::cleanUp()
	{
		swapChainManager.cleanUp();
		pipelineManager.cleanUp(deviceManager.getLogicalDevice());
		vkDestroyRenderPass(deviceManager.getLogicalDevice(), renderPass, nullptr);
		descriptorManager.cleanUp(deviceManager.getLogicalDevice());
		commandPoolManager.cleanUp(deviceManager.getLogicalDevice());
		deviceManager.cleanup();
		window.cleanup();

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT();
		}
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	void Core::mainLoop() {
		//while not connected render only the UI
		while (!glfwWindowShouldClose(window.get_GLFW_Window()))
		{
			glfwPollEvents();
			//...
			Window::lastTime = glfwGetTime();

			if (connected)
			{
				// Signals to the reciever that it is ready to display the shared images
				{
					std::lock_guard lk(m);
					ready = true;
					cv.notify_one();
					logger->LogInfo("Ready for processing");
				}
				break;
			}

		}

		while (!glfwWindowShouldClose(window.get_GLFW_Window()))
		{
			// wait for the reciever
			{
				std::unique_lock lk(m);
				cv.wait(lk, [&] { return processed; });
				if (glfwWindowShouldClose(window.get_GLFW_Window()))
					break;
			}
			glfwPollEvents();
			//...
			Window::lastTime = glfwGetTime();
		}

		// Signals to the reciever that the window is being closed
		{
			std::lock_guard lk(m);
			windowShouldClose = true;
			cv.notify_one();
		}
		cleanUp();
	}

	void Core::check_vk_result(VkResult err) {
		if (err == 0)
			return;

		auto prefix = "[vulkan]";
		Logger::getInstance().LogError("Error: VkResult = " + std::to_string(err),
			prefix);

		if (err < 0) {
			Logger::getInstance().LogError("Aborting", prefix);
			abort();
		}
	}

	VkBool32 Core::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::string message = pCallbackData->pMessage;
		auto prefix = "[validation layer]";

		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			Logger::getInstance().LogInfo(message, prefix, "\x1b[31m");
		else
			Logger::getInstance().LogInfo(message, prefix, "\x1b[33m");

		return VK_FALSE;
	}

	VkResult Core::CreateDebugUtilsMessengerEXT(
		VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void Core::createRenderPass(VkFormat imageformat)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = imageformat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo simulationRenderPassInfo{};
		simulationRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		simulationRenderPassInfo.attachmentCount = 1;
		simulationRenderPassInfo.pAttachments = &colorAttachment;
		simulationRenderPassInfo.subpassCount = 1;
		simulationRenderPassInfo.pSubpasses = &subpass;
		simulationRenderPassInfo.dependencyCount = 1;
		simulationRenderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(deviceManager.getLogicalDevice(), &simulationRenderPassInfo, nullptr,
			&renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create simulation render pass!");
		}
	}

	void Core::createInstance(bool enableValidationLayers) {

		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Onallo Laboratorium";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions(enableValidationLayers);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount =
				static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create instance!");
		}
		logger->LogInfo("Instance created successfully");
	}

	bool Core::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> Core::getRequiredExtensions(bool enableValidationLayers) {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions,
			glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void Core::createSurface(GLFWwindow& window) {
		if (glfwCreateWindowSurface(instance, &window, nullptr, &surface) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
		logger->LogInfo("Surface created successfully");
	}

	void Core::setupDebugMessenger(bool enableValidationLayers) {
		if (!enableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
			&debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void Core::populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void Core::DestroyDebugUtilsMessengerEXT() {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, nullptr);
		}
	}
}