#pragma once
#include "stdint.h"

namespace Client {

	enum NetworkState {
		Idle,
		Connecting,
		Connected,
		Failed
	};

	struct NetworkUtils
	{
		NetworkState state = Idle;
		uint8_t* pImage;
		bool recieved;
	};

	struct NetworkAddress {
		int port;
		const char* address;
	};

}