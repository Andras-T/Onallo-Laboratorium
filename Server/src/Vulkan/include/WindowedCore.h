#pragma once
#include "Core.h"
#include <Vulkan/Window/include/Window.h>

namespace Server {
	class WindowedCore : public Core {
	public:

		virtual void init();

	protected:
		
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