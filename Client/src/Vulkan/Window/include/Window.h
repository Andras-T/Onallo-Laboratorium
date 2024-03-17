#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string_view>
#include <imgui.h>
namespace Client {

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

		ImVec2 getSize() {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			return ImVec2(static_cast<float>(width), static_cast<float>(height));
		}
	};
}