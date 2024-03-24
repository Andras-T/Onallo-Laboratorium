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

	enum NetworkState {
		Idle,
		Connecting,
		Connected,
		Failed
	};
	
	struct NetworkMessage {
		NetworkState state = Idle;
		uint8_t* pImage = nullptr;
	};

	class ClientNetworking {
		static bool quit;
		NetworkMessage networkMessage;
		ISteamNetworkingMessage* pIncomingMsg;
		static ClientNetworking* s_pCallbackInstance;
		ISteamNetworkingSockets* m_pInterface = nullptr;
		HSteamNetConnection m_hConnection;

		std::map<HSteamNetConnection, std::string> m_mapClients;

	public:

		static void InitSteamDatagramConnectionSockets();
		
		bool connect(const SteamNetworkingIPAddr& serverAddr);

		NetworkMessage run();

		void closeConnection();

	private:

		static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);

		static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

		void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

		void PollIncomingMessages();

		void PollConnectionStateChanges();
	};
}