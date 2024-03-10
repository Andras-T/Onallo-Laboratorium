#pragma once

#include <Networking/include/ServerNetworking.h>

namespace Server {

	class Core {

		ServerNetworking server;

		void mainLoop();

		void cleanUp();

		void send();

	public:

		void init();

		void run();
	};

}