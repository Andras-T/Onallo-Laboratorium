#include "Core.h"

namespace Client {

	void Core::init() {
		logger = &(Logger::getInstance());
		logger->LogInfo("Init Core");

		window.init("Onallo Laboratorium");
		mainLoop();
	}

	void Core::check_vk_result(VkResult err) {
		if (err == 0)
			return;
		Logger::getInstance().LogError("Error: VkResult = " + std::to_string(err),
			"[vulkan]");

		if (err < 0) {
			Logger::getInstance().LogError("Aborting", "[vulkan]");
			abort();
		}
	}

}