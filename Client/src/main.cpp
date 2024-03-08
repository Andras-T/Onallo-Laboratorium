#include "Logger.h"
#include "Vulkan/include/Core.h"

int main() {
	Logger::getInstance().LogInfo("Client");
#ifdef DEBUG_MACRO
	Logger::getInstance().setSeverity(Trace);
#endif
	Client::Core core;
	core.init();
	core.run();
}