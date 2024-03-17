#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string_view>

namespace Server {

	class Window {
	private:
		GLFWwindow* window;
		GLFWimage icon;
		bool fullScreen = false;

		void initIcon();

	public:
		static double lastTime;
		static bool framebufferResized;

		void init(std::string_view title);

		void changeDisplayMode();

		static void framebufferResizeCallback(GLFWwindow* window, int width,
			int height);

		void cleanup();

		GLFWwindow* get_GLFW_Window() { return window; }

		VkExtent2D getExtent() {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
			return extent;
		}
	};
}