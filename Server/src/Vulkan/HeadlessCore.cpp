#include "include/HeadlessCore.h"
#include <Vulkan/Renderer/include/OffScreenRenderer.h>
#include <thread>
#include <filesystem>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace Server {

	void HeadlessCore::init() {
		logger = &(Logger::getInstance());
		logger->LogInfo("Init Server Core");

		compileShaders();
		createInstance(enableValidationLayers);
		setupDebugMessenger(enableValidationLayers);

		deviceManager.init(instance, enableValidationLayers, std::nullopt);
		logger->LogInfo("creating images");
		resourceManager.createImages(deviceManager.getLogicalDevice(), deviceManager.getPhysicalDevice());
		logger->LogInfo("creating renderpass");
		createRenderPass(VK_FORMAT_R8G8B8A8_UNORM);

		VkDevice& device = deviceManager.getLogicalDevice();
		descriptorManager.init();
		pipelineManager.init(device, std::nullopt, renderPass);
		commandPoolManager.createCommandPool(device, deviceManager.getIndices());

		resourceManager.createFrameBuffers(resourceManager.getPresentImages().getImageViews(), renderPass, device, std::nullopt);

		descriptorManager.createDescriptorPool(device);
		resourceManager.createBuffers(deviceManager, commandPoolManager.getCommandPool());
		descriptorManager.createDescriptorSets(device);
		commandPoolManager.createCommandBuffers(device);

		renderer = new OffScreenRenderer(deviceManager,
			descriptorManager, pipelineManager, commandPoolManager, resourceManager, renderPass);

		renderer->createSyncObjects();

		ServerNetworking::InitSteamDatagramConnectionSockets();
	}

	void HeadlessCore::mainLoop() {
		bool run = true;
		while (run)
		{
			glfwPollEvents();
			renderer->drawFrame(lastFrameTime);
			double currentTime = glfwGetTime();
			lastFrameTime = (currentTime - Window::lastTime) * 1000.0;
			Window::lastTime = currentTime;
		}
	}

	void HeadlessCore::cleanUp() {
		VkDevice& device = deviceManager.getLogicalDevice();
		check_vk_result(vkDeviceWaitIdle(device));

		resourceManager.getPresentImages().cleanUp(device);

		for (auto& framebuffer : resourceManager.getFrameBuffers())
			vkDestroyFramebuffer(device, framebuffer, nullptr);

		pipelineManager.cleanUp(device);
		vkDestroyRenderPass(device, renderPass, nullptr);
		descriptorManager.cleanUp(device);
		resourceManager.cleanUp(device);
		commandPoolManager.cleanUp(device);

		renderer->cleanUp();
		delete renderer;
		deviceManager.cleanup();

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT();
		}
		vkDestroyInstance(instance, nullptr);

	}

	void HeadlessCore::createRenderPass(VkFormat imageformat)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = imageformat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

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
}