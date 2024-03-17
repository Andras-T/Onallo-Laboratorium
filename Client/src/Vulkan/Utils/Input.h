#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

namespace Client {

	struct Input {

		char ip_address[SteamNetworkingIPAddr::k_cchMaxString] = "127.0.0.1.";
		bool tryToConnect = false;
		bool disconnect = false;
		bool connected = false;
	};

}