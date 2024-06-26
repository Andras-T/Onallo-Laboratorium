#include "include/ServerNetworking.h"
#include <thread>
#include <array>

namespace Server {

	SteamNetworkingMicroseconds g_logTimeZero;
	ServerNetworking* ServerNetworking::s_pCallbackInstance = nullptr;

	void ServerNetworking::InitSteamDatagramConnectionSockets() {
		SteamDatagramErrMsg errMsg;
		if (!GameNetworkingSockets_Init(nullptr, errMsg))
			Logger::getInstance().LogError(std::format("GameNetworkingSockets_Init failed. {}", errMsg));
		else
			Logger::getInstance().LogInfo("GameNetworkingSockets_Init succeded");

		g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
		SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
	}

	void ServerNetworking::DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
	{
		SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
		printf("%10.6f %s\n", time * 1e-6, pszMsg);
		Logger::getInstance().LogInfo(pszMsg);
		fflush(stdout);
		if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
		{
			fflush(stdout);
			fflush(stderr);
			NukeProcess(1);
		}
	}

	void ServerNetworking::start(uint16 nPort) {
		m_pInterface = SteamNetworkingSockets();

		// Start listening
		SteamNetworkingIPAddr serverLocalAddr;
		serverLocalAddr.Clear();
		serverLocalAddr.m_port = nPort;

		std::array<SteamNetworkingConfigValue_t, 2> opts;
		opts[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
		opts[1].SetInt32(k_ESteamNetworkingConfig_SendBufferSize, 524288 * 1000);
		m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 2, opts.data());

		if (m_hListenSock == k_HSteamListenSocket_Invalid)
			Logger::getInstance().LogError("Failed to listen on port " + std::to_string(nPort));
		m_hPollGroup = m_pInterface->CreatePollGroup();
		if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
			Logger::getInstance().LogError("Failed to listen on port " + std::to_string(nPort));
		Logger::getInstance().LogInfo("Server listening on port  " + std::to_string(nPort));
	}

	NetworkMessage ServerNetworking::run(uint8_t* imgage, uint32 size) {
		std::this_thread::sleep_for(std::chrono::milliseconds(13));

		//PollIncomingMessages();
		PollConnectionStateChanges();

		if (networkMessage.state == Connected)
			SendMessageToAllClients(imgage, size, Reliable);//k_nSteamNetworkingSend_UseCurrentThread

		return networkMessage;
	}

	void ServerNetworking::closeConnetions() {
		//CSteamNetworkConnectionBase::Think
		// Close all the connections
		Logger::getInstance().LogInfo("Closing connections...\n");
		//SendMessageToAllClients("Goodbye!", sizeof(char) * 9, Unreliable);
		for (auto it : m_mapClients)
		{
			// Note that we also have the
			// connection close reason as a place to send final data.  However,
			// that's usually best left for more diagnostic/debug text not actual
			// protocol strings.
			// Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
			// to flush this out and close gracefully.
			m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
		}
		m_mapClients.clear();

		m_pInterface->CloseListenSocket(m_hListenSock);
		m_hListenSock = k_HSteamListenSocket_Invalid;

		m_pInterface->DestroyPollGroup(m_hPollGroup);
		m_hPollGroup = k_HSteamNetPollGroup_Invalid;
	}

	//void freeMsg(SteamNetworkingMessage_t* pMsg) {
	//	delete[]
	//}

	void ServerNetworking::SendMessageToAllClients(const void* data, uint32 size, MessageFlags flag) {
		//Logger::getInstance().LogInfo(std::format("Sending {} bytes", size));
		for (auto& [connection, name] : m_mapClients)
		{
			//auto result = m_pInterface->SendMessageToConnection(connection, data, size, flag, nullptr);
			Logger::getInstance().LogInfo(std::format("Sending message with {} bytes", size));

			SteamNetworkingMessage_t* pMessage = SteamNetworkingUtils()->AllocateMessage(size);
			pMessage->m_conn = connection;
			pMessage->m_nFlags = flag;
			//pMessage->m_cbSize = size;
			memcpy(pMessage->m_pData, data, size);
			m_pInterface->SendMessages(1, &pMessage, nullptr);
			m_pInterface->FlushMessagesOnConnection(connection);
			PrintConnectionStatus(connection);
		}
	}

	void ServerNetworking::PollIncomingMessages()
	{
		// TODO: handle inputs from the client side
	}

	void ServerNetworking::PollConnectionStateChanges()
	{
		s_pCallbackInstance = this;
		m_pInterface->RunCallbacks();
	}

	void ServerNetworking::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
	{
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}

	void ServerNetworking::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
	{
		char temp[1024];

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_None:
			networkMessage.state = Idle;
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
			networkMessage.state = Idle;
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			networkMessage.state = Failed;
			// Ignore if they were not previously connected.  (If they disconnected
			// before we accepted the connection.)
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
			{

				// Locate the client.  Note that it should have been found, because this
				// is the only codepath where we remove clients (except on shutdown),
				// and connection change callbacks are dispatched in queue order.
				auto itClient = m_mapClients.find(pInfo->m_hConn);
				assert(itClient != m_mapClients.end());

				// Select appropriate log messages
				const char* pszDebugLogAction;
				if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					pszDebugLogAction = "problem detected locally";
					sprintf(temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug);
				}
				else
				{
					// Note that here we could check the reason code to see if
					// it was a "usual" connection or an "unusual" one.
					pszDebugLogAction = "closed by peer";
					sprintf(temp, "%s hath departed", itClient->second.m_sNick.c_str());
				}

				auto msg = ("Connection %s %s, reason %d: %s\n",
					pInfo->m_info.m_szConnectionDescription,
					pszDebugLogAction,
					pInfo->m_info.m_eEndReason,
					pInfo->m_info.m_szEndDebug);
				// Spew something to our own log.  Note that because we put their nick
				// as the connection description, it will show up, along with their
				// transport-specific data (e.g. their IP address)
				Logger::getInstance().LogInfo(msg);

				m_mapClients.erase(itClient);
			}
			else
			{
				assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
		{
			networkMessage.state = Connecting;
			// This must be a new connection
			assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

			Logger::getInstance().LogInfo(std::format("Connection request from {}", pInfo->m_info.m_szConnectionDescription));

			// A client is attempting to connect
			// Try to accept the connection.
			if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
			{
				// This could fail.  If the remote host tried to connect, but then
				// disconnected, the connection may already be half closed.  Just
				// destroy whatever we have on our side.
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				Logger::getInstance().LogInfo("Can't accept connection.  (It was already closed?)");
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			// Assign the poll group
			if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
			{
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				Logger::getInstance().LogInfo("Failed to set poll group?");
				break;
			}

			// Add them to the client list, using std::map wacky syntax
			m_mapClients[pInfo->m_hConn];
			break;
		}

		case k_ESteamNetworkingConnectionState_Connected:
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			networkMessage.state = Connected;
			// We will get a callback immediately after accepting the connection.
			// Since we are the server, we can ignore this, it's not news to us.
			break;

		default:
			// Silences -Wswitch
			break;
		}
	}
	void ServerNetworking::PrintConnectionStatus(HSteamNetConnection connection)
	{
		SteamNetConnectionRealTimeStatus_t pStatus;
		m_pInterface->GetConnectionRealTimeStatus(connection, &pStatus, 0, nullptr);
		Logger::getInstance().LogInfo(std::format("SentUnacked: {} m_cbPendingUnreliable, pending: rel {} bytes, unrel {} bytes, queuetime: {} bytes",
			pStatus.m_cbSentUnackedReliable, pStatus.m_cbPendingReliable, pStatus.m_cbPendingUnreliable, pStatus.m_usecQueueTime));
	}
}