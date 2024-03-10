#include "Logger.h"
#include <Vulkan/include/Core.h>

int main() {
	Logger::getInstance().LogInfo("Server");
	Server::Core core;
	core.init();
	core.run();
}