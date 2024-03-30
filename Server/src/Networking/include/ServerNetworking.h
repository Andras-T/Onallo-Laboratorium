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
#include <thread>

#include <Logger.h>

namespace Server {

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
	};

	enum MessageFlags {
		Unreliable = 0,
		NoNagle = 1,
		UnreliableNoNagle = Unreliable | NoNagle,
		NoDelay = 4,
		UnreliableNoDelay = Unreliable | NoDelay | NoNagle,
		Reliable = 8,
		ReliableNoNagle = Reliable | NoNagle
	};

	class ServerNetworking {
		static bool quit;
		ISteamNetworkingSockets* m_pInterface;
		HSteamListenSocket m_hListenSock;
		HSteamNetPollGroup m_hPollGroup;
		static ServerNetworking* s_pCallbackInstance;

		struct Client_t
		{
			std::string m_sNick;
		};

		NetworkMessage networkMessage;
		std::map< HSteamNetConnection, Client_t > m_mapClients;

	public:

		static void InitSteamDatagramConnectionSockets();
		
		void start(uint16 nPort);

		NetworkMessage run(uint8_t* imgage, uint32 size);

		void closeConnetions();

	private:

		void SendMessageToAllClients(const void* data, uint32 size, MessageFlags flag);

		static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);
		
		void PollIncomingMessages();
		
		void PollConnectionStateChanges();

		static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
		void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

		void PrintConnectionStatus(HSteamNetConnection connection);
	};

}