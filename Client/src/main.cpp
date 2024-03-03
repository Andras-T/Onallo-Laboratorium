#include "Logger.h"
#include "Vulkan/include/Core.h"

int main() {
	Logger::getInstance().LogInfo("Client");
	//Logger::getInstance().setSeverity(Trace);
	Client::Core core;
	core.init();
}