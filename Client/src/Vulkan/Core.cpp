#include "include/Core.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"
#include "imgui_impl_vulkan.h"
#include <filesystem>


namespace Client {

	void Core::init() {
		logger = &(Logger::getInstance());
		logger->LogInfo("Init Client Core");

		compileShaders();
		
		window.init("Onallo Laboratorium - Client");
		
		createInstance(enableValidationLayers);
		setupDebugMessenger(enableValidationLayers);
		createSurface(window.get_GLFW_Window());
		
		deviceManager.init(instance, surface, enableValidationLayers);
		auto& device = deviceManager.getLogicalDevice();
		
		swapChainManager.init(surface, renderPass, deviceManager, window.get_GLFW_Window());
		
		createRenderPass(swapChainManager.getSwapChainImageFormat());
		
		swapChainManager.createFrameBuffers();

		commandPoolManager.createCommandPool(device, deviceManager.getIndices());
		commandPoolManager.createCommandBuffers(device);
		commandPoolManager.createImGuiCommandBuffers(device);

		resourceManager.createImageResources(deviceManager, commandPoolManager);
		resourceManager.createBuffers(deviceManager, commandPoolManager.getCommandPool());
		resourceManager.fillImageWithZeros(deviceManager, commandPoolManager);
		
		descriptorManager.initDescriptors(resourceManager, device);
		descriptorManager.createDescriptors(device);

		pipelineManager.init(device, descriptorManager[0].getDescriptorSetLayout(), renderPass);
		
		renderer = new Renderer(window, deviceManager, swapChainManager,
			descriptorManager, pipelineManager, commandPoolManager, resourceManager, renderPass);
		renderer->createSyncObjects();
		initImGui();

		ClientNetworking::InitSteamDatagramConnectionSockets();
	}

	/* Validation Error: [ VUID-vkCmdDraw-None-08600 ] Object 0: handle = 0x967dd1000000000e, type = VK_OBJECT_TYPE_PIPELINE; |
	MessageID = 0x4768cf39 | vkCmdDraw() : The VkPipeline 0x967dd1000000000e[](created with VkPipelineLayout
		0xec4bec000000000b[]) statically uses descriptor set(index #0) which is not compatible with the currently bound descriptor
		set's pipeline layout (VkPipelineLayout 0x0[]). The Vulkan spec states: For each set n that is statically used by a bound
		shader, a descriptor set must have been bound to n at the same pipeline bind point, with a VkPipelineLayout that is
		compatible for set n, with the VkPipelineLayout used to create the current VkPipeline or the VkDescriptorSetLayout array
		used to create the current VkShaderEXT, as described in Pipeline Layout Compatibility
		(https ://vulkan.lunarg.com/doc/view/1.3.275.0/windows/1.3-extensions/vkspec.html#VUID-vkCmdDraw-None-08600)*/

	void Core::run() {

		mainLoop();
		if (reciever.joinable())
			reciever.join();
		cleanUp();
	}

	void Core::recieve() {

		SteamNetworkingIPAddr addrServer;
		addrServer.Clear();
		addrServer.ParseString(uiInput.ip_address);
		addrServer.m_port = DEFAULT_SERVER_PORT;
		networkMessage.state = Connecting;
		bool valid = client.connect(addrServer);
		// TODO: store the data in an array
		while (!glfwWindowShouldClose(window.get_GLFW_Window()) && valid && networkMessage.state != Failed && !uiInput.disconnect)
		{
			networkMessage = client.run();
			{
				std::unique_lock<std::mutex> imageLock(m);
				recieved = true;
				cv.notify_one();
				cv.wait(imageLock, [this] {return imageRendered; });
				imageRendered = false;
			}
		}

		client.closeConnection();

		if (networkMessage.state == Failed)
			logger->LogError("Reciever loop stopped due to network issues!");

		logger->LogInfo("Thread is stopping", "[enginge - reciever]");
	}

	void Core::cleanUp()
	{
		check_vk_result(vkDeviceWaitIdle(deviceManager.getLogicalDevice()));

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		swapChainManager.cleanUp();

		pipelineManager.cleanUp(deviceManager.getLogicalDevice());
		vkDestroyRenderPass(deviceManager.getLogicalDevice(), renderPass, nullptr);
		descriptorManager.cleanUp(deviceManager.getLogicalDevice());
		resourceManager.cleanUp(deviceManager.getLogicalDevice());
		commandPoolManager.cleanUp(deviceManager.getLogicalDevice());

		renderer->cleanUp();
		delete renderer;

		deviceManager.cleanup();
		window.cleanup();

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT();
		}
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);


	}

	void Core::mainLoop() {
		while (!glfwWindowShouldClose(window.get_GLFW_Window()))
		{
			glfwPollEvents();

			if (networkMessage.state == Failed)
			{
				if (reciever.joinable())
					reciever.join();
				networkMessage.state = Idle;
			}

			if (networkMessage.state == Connected) {
				uiInput.connected = true;
				std::unique_lock<std::mutex> imageLock(m);
				cv.wait(imageLock, [this] {return recieved; });
				recieved = false;
			}
			else
			{
				networkMessage.pImage = nullptr;
			}

			if (uiInput.disconnect) {
				if (reciever.joinable())
					reciever.join();
				networkMessage.state = Idle;
				uiInput.disconnect = false;
				uiInput.connected = false;
			}

			if (uiInput.tryToConnect && networkMessage.state != Connected && networkMessage.state != Connecting) {
				reciever = std::thread{ &Core::recieve, this };
				logger->LogInfo(std::format("Given ip address: {}", uiInput.ip_address));
				uiInput.tryToConnect = false;
			}

			renderer->drawFrame(lastFrameTime, uiInput, networkMessage.pImage);
			double currentTime = glfwGetTime();
			lastFrameTime = (currentTime - Window::lastTime) * 1000.0;
			Window::lastTime = currentTime;

			{
				std::unique_lock<std::mutex> imageLock(m);
				imageRendered = true;
				cv.notify_one();
			}
		}
	}

	void Core::compileShaders() {
#ifdef NDEBUG
		const std::string compileCommand = "start cmd.exe /K" +
			std::filesystem::current_path().string() + "\\..\\..\\..\\..\\Client\\shaders\\compileRelease.bat";
#else
		const std::string compileCommand = "start cmd.exe /K" +
			std::filesystem::current_path().string() + "\\..\\..\\..\\..\\Client\\shaders\\compileDebug.bat";
#endif

		system(compileCommand.c_str());
		// Wait for shaders to compile
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
			Logger::getInstance().LogInfo(message, prefix, "\x1b[34m");

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
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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
			throw std::runtime_error("Failed to create simulation render pass!");
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

	void Core::createSurface(GLFWwindow* window) {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
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
			throw std::runtime_error("Failed to set up debug messenger!");
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

	void Core::initImGui() {
		logger = &(Logger::getInstance());
		logger->LogInfo("Setting up ImGui");
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui_ImplGlfw_InitForVulkan(window.get_GLFW_Window(), true);

		ImGui::GetIO().FontGlobalScale = 1.0f;

		ImGui::StyleColorsDark();

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = instance;
		init_info.Device = deviceManager.getLogicalDevice();
		init_info.PhysicalDevice = deviceManager.getPhysicalDevice();
		init_info.QueueFamily = deviceManager.getQueueFamily();
		init_info.Queue = deviceManager.getGraphicsQueue();
		init_info.DescriptorPool = descriptorManager[0].getDescriptorPool();
		init_info.MinImageCount = swapChainManager.getMinImageCount();
		init_info.ImageCount = swapChainManager.getSwapchainImageCount();
		init_info.Subpass = 0;
		VkPipelineCache m_pipeline_cache{ nullptr };
		init_info.PipelineCache = m_pipeline_cache;
		init_info.Allocator = nullptr;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.CheckVkResultFn = check_vk_result;
		init_info.RenderPass = renderPass;
		ImGui_ImplVulkan_Init(&init_info);

		VkCommandBuffer commandBuffer = commandPoolManager.beginSingleTimeCommands(deviceManager.getLogicalDevice());
		ImGui_ImplVulkan_CreateFontsTexture();
		commandPoolManager.endSingleTimeCommands(deviceManager.getLogicalDevice(), commandBuffer,
			deviceManager.getGraphicsQueue());

		vkDeviceWaitIdle(deviceManager.getLogicalDevice());
	}
}