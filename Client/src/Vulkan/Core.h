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
#include "Vulkan/Window/Window.h"

namespace Client {

	class Core {
		Logger* logger;
		Window window;

		void mainLoop() {
			while (!glfwWindowShouldClose(window.get_GLFW_Window()))
			{
				glfwPollEvents();
				//...
				Window::lastTime = glfwGetTime();
			}
			cleanUp();
		}

	public:
		void init();

		void cleanUp() {
			window.cleanup();
		}

		static void check_vk_result(VkResult err);
	};

}