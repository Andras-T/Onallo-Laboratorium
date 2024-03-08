#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <imgui_impl_vulkan.h>
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"

namespace Client {
	
	class UserInterface {
	public:
		static void draw(GLFWwindow* window, VkCommandBuffer& commandBuffer);
	};
}