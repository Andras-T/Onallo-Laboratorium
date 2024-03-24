#pragma once
#include "Core.h"
#include <Vulkan/Window/include/Window.h>
#include <condition_variable>
#include <mutex>

namespace Server {
	class WindowedCore : public Core {
	public:

		virtual void init();

	protected:

		std::mutex imageProcessing;
		std::condition_variable cv;
		bool imageRendered = false;

		NetworkMessage networkMessage;

		Window window;
		VkSurfaceKHR surface;
		SwapChainManager swapChainManager;

		void send() override;
		void cleanUp() override;
		void mainLoop() override;
		void createRenderPass(VkFormat imageformat) override;

		void createSurface(GLFWwindow& window);
	};
}