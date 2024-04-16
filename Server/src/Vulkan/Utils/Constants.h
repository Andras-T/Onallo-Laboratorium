#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

typedef unsigned int       uint32_t;

namespace Server {
	constexpr VkFormat defaultFormat = VK_FORMAT_R8G8B8A8_UNORM;
	constexpr uint32_t pixelSize = 4;
	// We can only send 524288 bytes so the max width*heigth is 362*362 with 4bytes/pixel (8bit r/g/b/a)
	// but it's pretty slow to send max byte packages
	// for example even with 200*200 it takes around 150ms/package for the reciever
	// with 100*100 it's much better 20-50 ms/package but it takes 4x more package :/
	// with 50*50 it takes 13-14ms/package
	constexpr uint32_t DEFAULT_WIDTH = 720;
	constexpr uint32_t DEFAULT_HEIGHT = 480;
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	constexpr int DEFAULT_SERVER_PORT = 27020;
}