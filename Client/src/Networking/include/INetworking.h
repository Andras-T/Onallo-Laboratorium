#pragma once

#include "NetworkUtils.h"

namespace Client {

	class INetworking {

	public:

		virtual NetworkUtils* init() = 0;

		virtual void connect(const NetworkAddress& address) = 0;

		virtual void run() = 0;

		virtual void closeConnection() = 0;

	};

}