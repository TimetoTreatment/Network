#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <string>

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

	WaitEventType WaitEvent(int timeoutMicroSecond = -1);

	void Send(const char* message, int size, SendTo sendTo = SendTo::EVENT_SOURCE);

	const char* ReadMessage();
	const char* ReadBuffer(int size);
	std::string ReadSenderID();

	void AddClient();
	void CloseClient();


private:

	bool isServer;

	SOCKET mySocket;
	addrinfo mySocketHint;
	SOCKET sender = INVALID_SOCKET;

	std::vector<WSAPOLLFD> fdArray;
	size_t fdArrayCurrentIndex = 0;

	char* cache = nullptr;
	char* buffer = nullptr;
	int cacheSize = 8192;
	int bufferSize = 8388608;

	WSADATA wsaData;
};

