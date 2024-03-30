#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <signal.h>
#endif

#include <map>
#include <cassert>

#include <Logger.h>
#include "INetworking.h"

namespace Client {

	static void NukeProcess(int rc)
	{
#ifdef _WIN32
		ExitProcess(rc);
#else
		(void)rc; // Unused formal parameter
		kill(getpid(), SIGKILL);
#endif
	}

	class ClientNetworking : public INetworking {
		static bool quit;
		NetworkUtils networkUtils;
		ISteamNetworkingMessage** pIncomingMsg;
		static ClientNetworking* s_pCallbackInstance;
		ISteamNetworkingSockets* m_pInterface = nullptr;
		HSteamNetConnection m_hConnection;

		std::map<HSteamNetConnection, std::string> m_mapClients;

	public:

		ClientNetworking(NetworkUtils& n) :networkUtils(n) {}

		void init() override;
		
		void connect(const NetworkAddress& address) override;

		void run(size_t i) override;

		void closeConnection() override;

	private:

		static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);

		static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

		void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

		void PollIncomingMessages(size_t i);

		void PollConnectionStateChanges();
	};
}