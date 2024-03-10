#include "include/ClientNetworking.h"
#include <thread>

namespace Client {

	bool ClientNetworking::quit = false;
	SteamNetworkingMicroseconds g_logTimeZero;
	ClientNetworking* ClientNetworking::s_pCallbackInstance = nullptr;

	void ClientNetworking::InitSteamDatagramConnectionSockets() {
		SteamDatagramErrMsg errMsg;
		if (!GameNetworkingSockets_Init(nullptr, errMsg))
			Logger::getInstance().LogError(std::string("GameNetworkingSockets_Init failed. ") + errMsg);
		else
			Logger::getInstance().LogInfo("GameNetworkingSockets_Init succeded");

		g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
		SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
	}

	void ClientNetworking::DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
	{
		SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
		printf("%10.6f %s\n", time * 1e-6, pszMsg);
		fflush(stdout);
		if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
		{
			fflush(stdout);
			fflush(stderr);
			NukeProcess(1);
		}
	}

	void ClientNetworking::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
	{
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}

	void ClientNetworking::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
	{
		//assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			quit = true;

			// Print an appropriate message
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
			{
				// Note: we could distinguish between a timeout, a rejected connection,
				// or some other transport problem.
				Logger::getInstance().LogInfo("We sought the remote host, yet our efforts were met with defeat. " + std::string(pInfo->m_info.m_szEndDebug));
			}
			else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				Logger::getInstance().LogInfo("Alas, troubles beset us; we have lost contact with the host. " + std::string(pInfo->m_info.m_szEndDebug));
			}
			else
			{
				// NOTE: We could check the reason code for a normal disconnection
				Logger::getInstance().LogInfo("The host hath bidden us farewell. " + std::string(pInfo->m_info.m_szEndDebug));
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			m_hConnection = k_HSteamNetConnection_Invalid;
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
			// We will get this callback when we start connecting.
			// We can ignore this.
			break;

		case k_ESteamNetworkingConnectionState_Connected:
			Logger::getInstance().LogInfo("Connected to server OK");
			break;

		default:
			// Silences -Wswitch
			break;
		}
	}

	bool ClientNetworking::connect(const SteamNetworkingIPAddr& serverAddr) {
		m_pInterface = SteamNetworkingSockets();

		// Start connecting
		char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
		serverAddr.ToString(szAddr, sizeof(szAddr), true);
		Logger::getInstance().LogInfo(std::string("Connecting to chat server at: ") + szAddr);
		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
		m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
		if (m_hConnection == k_HSteamNetConnection_Invalid) {
			Logger::getInstance().LogError("Failed to create connection");
			return false;
		}
		Logger::getInstance().LogInfo("m_hConnection: " + std::to_string(m_hConnection));
		return true;
	}

	bool ClientNetworking::run() {
		PollIncomingMessages();
		PollConnectionStateChanges();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return quit;
	}

	void ClientNetworking::PollIncomingMessages() {
		while (!quit)
		{
			ISteamNetworkingMessage* pIncomingMsg = nullptr;
			int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			if (numMsgs == 0)
				break;
			if (numMsgs < 0)
				Logger::getInstance().LogError("Error checking for messages");
		}
	}

	void ClientNetworking::PollConnectionStateChanges()
	{
		s_pCallbackInstance = this;
		m_pInterface->RunCallbacks();
	}

	void ClientNetworking::closeConnection() {
		Logger::getInstance().LogInfo("Disconnecting from server");
		m_pInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
	}
}