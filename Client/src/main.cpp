#include "Logger.h"
#include "Vulkan/Core.h"

int main() {
	Logger::getInstance().LogInfo("Client");
	
	Client::Core core;
	core.init();
}