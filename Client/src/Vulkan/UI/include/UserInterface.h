#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <imgui_impl_vulkan.h>
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"
#include <Vulkan/Window/include/Window.h>

namespace Client {
	
	class UserInterface {
		
		static void createFPSCounter();
		static void createMenuBar(Window& window);
		static void menuShortcuts(Window& window);

	public:
		static void draw(Window& window, VkCommandBuffer& commandBuffer);
	};
}