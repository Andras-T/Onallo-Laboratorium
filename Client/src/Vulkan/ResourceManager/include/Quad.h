#pragma once

#include <array>

namespace Client {

	struct Quad {

		static constexpr std::array<float, 32> quadVertices = {
			-1.0f, -1.0f, 0.0f, 1.0f, // Vertex 1
			1.0f,  -1.0f, 0.0f, 1.0f, // Vertex 2
			-1.0f, 1.0f,  0.0f, 1.0f, // Vertex 3

			1.0f,  -1.0f, 0.0f, 1.0f, // Vertex 2
			1.0f,  1.0f,  0.0f, 1.0f, // Vertex 4
			-1.0f, 1.0f,  0.0f, 1.0f  // Vertex 3
		};

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(float) * 4;
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}
	};
}