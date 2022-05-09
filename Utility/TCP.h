#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")


class TCP
{
public:

	enum class WaitEventType
	{
		NONE,
		NEWCLIENT,
		DISCONNECT,
		MESSAGE,
	};

	enum class SendTo
	{
		EVENT_SOURCE,
		ALL,
		OTHERS,
	};

	TCP(std::string port, std::string targetIP = "0.0.0.0");
	~TCP();

	void AddClient();
	void CloseClient();

	WaitEventType WaitEvent(int timeoutMilliseconds = -1);

	void Send(const char* message, int size, SendTo sendTo = SendTo::EVENT_SOURCE);
	void SendMsg(std::string message, SendTo sendTo = SendTo::EVENT_SOURCE);

	const char* ReadData(int size);
	std::string ReadMsg();
	std::string ReadSenderID();


private:

	bool isServer;

	SOCKET mySocket = INVALID_SOCKET;
	addrinfo mySocketHint;
	SOCKET sender = INVALID_SOCKET;

	std::vector<WSAPOLLFD> fdArray;
	size_t fdArrayCurrentIndex = 0;

	char* cache = nullptr;
	char* buffer = nullptr;
	int cacheSize = 8192;
	int bufferSize = 8388608;
	int bufferValidDataSize = 0;

	WSADATA wsaData;
};

