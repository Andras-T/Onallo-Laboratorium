#include "Logger.h"
#include <Vulkan/include/HeadlessCore.h>
#include <Vulkan/include/WindowedCore.h>




int main() {
	Logger::getInstance().LogInfo("Server");
#ifdef NDEBUG
	Server::HeadlessCore core;
#else
	Server::WindowedCore core;
#endif
	core.init();
	core.run();
}