#include "include/Core.h"
#include <thread>

namespace Server {

	void Core::init() {
		//...
		// set up Vulkan environment

		ServerNetworking::InitSteamDatagramConnectionSockets();
	}

	void Core::run() {
		std::thread sender(&Core::send, this);

		mainLoop();
		sender.join();
		cleanUp();
	}

	void Core::send() {
		server.start(27020);
		while (true)
		{
			server.run();
		}
		server.closeConnetions();
	}

	void Core::mainLoop() {

	}

	void Core::cleanUp() {

	}
}