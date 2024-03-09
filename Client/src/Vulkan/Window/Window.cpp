#include "include/Window.h"
#include "stb_image.h"
#include <filesystem>
#include <iostream>
#include "Logger.h"

bool Window::framebufferResized = false;
double Window::lastTime = 0.0f;

void Window::initIcon() {
	const std::string iconPath =
		std::filesystem::current_path().string() + "\\..\\..\\..\\..\\res\\logo.png";
	std::string normalizedPath = std::filesystem::canonical(iconPath).string();

	Logger::getInstance().LogInfo("Loading icon from: " + normalizedPath);

	icon.pixels =
		stbi_load(normalizedPath.c_str(), &icon.width, &icon.height, 0, 0);

	if (icon.pixels != nullptr) {
		GLFWimage images[1];
		images[0] = icon;

		glfwSetWindowIcon(window, 1, images);
	}
	else {
		Logger::getInstance().LogError("Failed to load icon!");
	}
}

void Window::init(std::string_view title) {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	window = glfwCreateWindow(mode->width, mode->height,
		title.data(), nullptr, nullptr);

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	int xpos, ypos, width, height;
	glfwGetMonitorWorkarea(primaryMonitor, &xpos, &ypos, &width, &height);

	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

	glfwSetWindowMonitor(window, nullptr, xpos + 50, ypos + 50,
		1920, 1080,
		mode->refreshRate);

	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	initIcon();

	lastTime = glfwGetTime();
}

void Window::changeDisplayMode() {
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	int xpos, ypos, width, height;
	glfwGetMonitorWorkarea(primaryMonitor, &xpos, &ypos, &width, &height);

	if (!fullScreen) {
		glfwSetWindowMonitor(window, primaryMonitor, xpos, ypos, mode->width,
			mode->height, mode->refreshRate);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	}
	else {
		glfwSetWindowMonitor(window, nullptr, xpos + 50, ypos + 50,
			1920, 1080,
			mode->refreshRate);
	}

	GLFWimage images[1];
	images[0] = icon;
	if (images != nullptr)
		glfwSetWindowIcon(window, 1, images);

	// TODO: change it to
	//    glfwSetWindowIcon(window, 1, &icon);

	fullScreen = !fullScreen;
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width,
	int height) {
	Logger::getInstance().LogTrace("Window resize callback");
	auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Window::cleanup() {
	glfwDestroyWindow(window);
	glfwTerminate();
	stbi_image_free(icon.pixels);
}