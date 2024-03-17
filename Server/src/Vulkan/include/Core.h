#pragma once

#include "Logger.h"
#include <Networking/include/ServerNetworking.h>
#include <Vulkan/CommandPool/include/CommandPoolManager.h>
#include <Vulkan/Descriptor/include/DescriptorManager.h>
#include <Vulkan/DeviceManager/include/DeviceManager.h>
#include <Vulkan/Pipeline/include/PipelineManager.h>
#include <Vulkan/ResourceManager/include/ResourceManager.h>
#include <Vulkan/SwapChain/include/SwapChainManager.h>
#include <Vulkan/Renderer/include/Renderer.h>
#include <Vulkan/Utils/Constants.h>

namespace Server {

	class Core {
	protected:
		Logger* logger;
		DeviceManager deviceManager;
		DescriptorManager descriptorManager;
		PipelineManager pipelineManager;
		CommandPoolManager commandPoolManager;
		ResourceManager resourceManager;
		Renderer* renderer;

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkRenderPass renderPass;

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation" };
		
		double lastFrameTime = 0.0f;

		ServerNetworking server;

	public:


		virtual void init() = 0;
		void run();

	protected:

		virtual void send() = 0;
		virtual void cleanUp() = 0;
		virtual void mainLoop() = 0;
		virtual void createRenderPass(VkFormat imageformat) = 0;
		
		void compileShaders();
		bool checkValidationLayerSupport();
		void DestroyDebugUtilsMessengerEXT();
		void createInstance(bool enableValidationLayers);
		void setupDebugMessenger(bool enableValidationLayers);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		
		std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
		VkResult CreateDebugUtilsMessengerEXT(VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		
		static void check_vk_result(VkResult err);
		static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};

}