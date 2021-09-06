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

	void Send(std::string message, SendTo sendTo = SendTo::EVENT_SOURCE);

	std::string ReadMessage();
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

	char* buffer = nullptr;
	int bufferSize = 1024;
	WSADATA wsaData;
};

