#include "include/ClientNetworking.h"
#include <thread>
#include <array>
#include <Vulkan/Utils/Constants.h>

namespace Client {

	bool ClientNetworking::quit = false;
	SteamNetworkingMicroseconds g_logTimeZero;
	ClientNetworking* ClientNetworking::s_pCallbackInstance = nullptr;

	NetworkUtils* ClientNetworking::init()
	{
		SteamDatagramErrMsg errMsg;
		if (!GameNetworkingSockets_Init(nullptr, errMsg))
			Logger::getInstance().LogError(std::format("GameNetworkingSockets_Init failed. {}", errMsg));
		else
			Logger::getInstance().LogInfo("GameNetworkingSockets_Init succeded");

		g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
		SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
		return &networkUtils;
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
		Logger::getInstance().LogInfo(std::format("m_hConnection: {}", uint32_t(m_hConnection)));
		Logger::getInstance().LogInfo(std::format("pInfo->m_hConn: {}", uint32_t(pInfo->m_hConn)));
		if (pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid)
		{

			// What's the state of the connection?
			switch (pInfo->m_info.m_eState)
			{
			case k_ESteamNetworkingConnectionState_None:
				// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				networkUtils.state = Failed;

				// Print an appropriate message
				if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
				{
					// Note: we could distinguish between a timeout, a rejected connection,
					// or some other transport problem.
					Logger::getInstance().LogInfo(std::format("We sought the remote host, yet our efforts were met with defeat. {}", pInfo->m_info.m_szEndDebug));
				}
				else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					Logger::getInstance().LogInfo(std::format("Alas, troubles beset us; we have lost contact with the host. {}", pInfo->m_info.m_szEndDebug));
				}
				else
				{
					// NOTE: We could check the reason code for a normal disconnection
					Logger::getInstance().LogInfo(std::format("The host hath bidden us farewell. {}", pInfo->m_info.m_szEndDebug));
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
				networkUtils.state = Connecting;
				// We will get this callback when we start connecting.
				// We can ignore this.
				break;

			case k_ESteamNetworkingConnectionState_Connected:
				networkUtils.state = Connected;
				Logger::getInstance().LogInfo("OnSteamNetConnectionStatusChanged: Connected to server OK");
				break;

			default:
				// Silences -Wswitch
				break;
			}
		}
		else {
			Logger::getInstance().LogWarning(std::format("Invalid Connection handle with ESteamNetworkingConnectionState: {}", int(pInfo->m_info.m_eState)));
		}
	}

	void ClientNetworking::connect(const NetworkAddress& address) {
		SteamNetworkingIPAddr serverAddr;
		serverAddr.Clear();
		serverAddr.ParseString(address.address);
		serverAddr.m_port = address.port;

		m_pInterface = SteamNetworkingSockets();

		// Start connecting
		char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
		serverAddr.ToString(szAddr, sizeof(szAddr), true);
		Logger::getInstance().LogInfo(std::format("Connecting to server at: {}", szAddr));
		constexpr int NOPTS = 4;
		std::array<SteamNetworkingConfigValue_t, NOPTS> opts;
		opts[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
		opts[1].SetInt32(k_ESteamNetworkingConfig_TimeoutInitial, 5000);
		opts[2].SetInt32(k_ESteamNetworkingConfig_TimeoutConnected, 6000);
		opts[3].SetInt32(k_ESteamNetworkingConfig_Unencrypted, 2);

		m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, NOPTS, opts.data());

		if (m_hConnection == k_HSteamNetConnection_Invalid) {
			Logger::getInstance().LogError("Failed to create connection");
			networkUtils.state = Failed;
			return;
		}
		Logger::getInstance().LogInfo(std::format("m_hConnection: {}", uint32_t(m_hConnection)));

		networkUtils.pImage = new uint8_t[DEFAULT_IMAGE_WIDTH * DEFAULT_IMAGE_HEIGHT * DEFAULT_PIXEL_SIZE];
		if (networkUtils.pImage == nullptr)
		{
			Logger::getInstance().LogError("Failed to create pImage");
		}
		else {
			Logger::getInstance().LogInfo("Allocated pImage");
		}

		PollConnectionStateChanges();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		PollConnectionStateChanges();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	void ClientNetworking::run() {
		if (networkUtils.state == Connected)
			PollIncomingMessages();

		PollConnectionStateChanges();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	void ClientNetworking::PollIncomingMessages() {
		while (!quit)
		{
			ISteamNetworkingMessage* pIncomingMsg = nullptr;
			int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			if (numMsgs > 0) {
				Logger::getInstance().LogInfo(std::format("Got  messages: {}", numMsgs));
				if (networkUtils.pImage != nullptr)
					memcpy(networkUtils.pImage, pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);

				pIncomingMsg->Release();
			}
			else if (numMsgs == 0) {
				break;
			}
			else {
				Logger::getInstance().LogError("Error checking for messages");
			}
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
		delete networkUtils.pImage;
	}
}