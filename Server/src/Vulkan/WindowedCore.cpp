#include "include/WindowedCore.h"
#include <Vulkan/Renderer/include/WindowedRenderer.h>
#include <thread>
#include <filesystem>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace Server {

	void WindowedCore::init() {
		logger = &(Logger::getInstance());
		logger->LogInfo("Init Server Core");

		compileShaders();
		window.init("Onallo Laboratorium - Server");
		createInstance(enableValidationLayers);
		setupDebugMessenger(enableValidationLayers);
		createSurface(*window.get_GLFW_Window());

		deviceManager.init(instance, enableValidationLayers, &surface);
		swapChainManager.init(surface, deviceManager, window.get_GLFW_Window());
		createRenderPass(swapChainManager.getSwapChainImageFormat());

		VkDevice& device = deviceManager.getLogicalDevice();
		descriptorManager.init();
		pipelineManager.init(device, std::nullopt, renderPass);
		commandPoolManager.createCommandPool(device, deviceManager.getIndices());
		//swapChainManager.createImages(device, deviceManager.getPhysicalDevice());

		resourceManager.createFrameBuffers(swapChainManager.getSwapChainImageViews(), renderPass, device, swapChainManager.getSwapChainExtent());

		descriptorManager.createDescriptorPool(device);
		resourceManager.createBuffers(deviceManager, commandPoolManager.getCommandPool(), window.get_GLFW_Window());
		descriptorManager.createDescriptorSets(device);
		commandPoolManager.createCommandBuffers(device);

		renderer = new WindowedRenderer(window, deviceManager, swapChainManager,
			descriptorManager, pipelineManager, commandPoolManager, resourceManager, renderPass);
		renderer->createSyncObjects();

		ServerNetworking::InitSteamDatagramConnectionSockets();
	}

	void WindowedCore::mainLoop() {

		while (!glfwWindowShouldClose(window.get_GLFW_Window()))
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(2));
			glfwPollEvents();
			renderer->drawFrame(lastFrameTime);
			double currentTime = glfwGetTime();
			lastFrameTime = (currentTime - Window::lastTime) * 1000.0;
			Window::lastTime = currentTime;

			{
				std::unique_lock<std::mutex> imageLock(imageProcessing);
				imageRendered = true;
				cv.notify_one();
			}
		}
	}

	void WindowedCore::send() {
		server.start(27020);
		{
			std::unique_lock<std::mutex> imageLock(imageProcessing);
			cv.wait(imageLock, [this] {return imageRendered; });
			imageRendered = false;
		}
		while (!glfwWindowShouldClose(window.get_GLFW_Window()))
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(2));
			networkMessage = server.run(resourceManager.getCompressedImage(), resourceManager.getCompressedImageSize());
			{
				std::unique_lock<std::mutex> imageLock(imageProcessing);
				cv.wait(imageLock, [this] {return imageRendered; });
				imageRendered = false;
			}
		}
		server.closeConnetions();

		logger->LogInfo("Thread is stopping", "[enginge - sender]");
	}

	void WindowedCore::cleanUp() {
		VkDevice& device = deviceManager.getLogicalDevice();
		check_vk_result(vkDeviceWaitIdle(device));

		swapChainManager.cleanUp();
		resourceManager.destroyFrameBuffers(device);

		pipelineManager.cleanUp(device);
		vkDestroyRenderPass(device, renderPass, nullptr);
		descriptorManager.cleanUp(device);
		resourceManager.cleanUp(device);
		commandPoolManager.cleanUp(device);

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

	void WindowedCore::createRenderPass(VkFormat imageformat)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = imageformat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
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

	void WindowedCore::createSurface(GLFWwindow& window) {
		if (glfwCreateWindowSurface(instance, &window, nullptr, &surface) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
		logger->LogInfo("Surface created successfully!");

	}
}