#include "Logger.h"
#include "Vulkan/include/Core.h"

int main() {
	Logger::getInstance().LogInfo("Client");
	
	Client::Core core;
	core.init();
}