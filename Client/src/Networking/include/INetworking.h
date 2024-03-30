#pragma once

#include "NetworkUtils.h"

namespace Client {

	class INetworking {

	public:

		virtual void init() = 0;

		virtual void connect(const NetworkAddress& address) = 0;

		virtual void run(size_t i) = 0;

		virtual void closeConnection() = 0;

	};

}