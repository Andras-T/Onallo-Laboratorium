#pragma once

#include <cstdint>
#include <optional>

namespace Server {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsAndComputeFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete(bool hasSurface) {
			return graphicsAndComputeFamily.has_value() && (presentFamily.has_value() || !hasSurface);
		}
	};
}