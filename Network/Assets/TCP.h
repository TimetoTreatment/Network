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

	enum class SendRange
	{
		EVENT_SENDER,
		ALL,
		OTHERS,
	};

	TCP(int port, std::string targetIP = "0.0.0.0");
	~TCP();

	WaitEventType WaitEvent(int timeoutMicroSecond = -1);
	void Send(std::string message, SendRange sendRange = SendRange::EVENT_SENDER);
	std::string ReadMessage();
	std::string CurrentSocketID();

	void AddClient();
	void CloseClient();


private:

	void ClearBuffer();

	int port;
	std::string targetIP;

	bool isServer;

	SOCKET mySocket;
	addrinfo mySocketHint;
	SOCKET currentClient = INVALID_SOCKET;

	size_t fdArrayCurrentIndex = 0;
	std::vector<WSAPOLLFD> fdArray;

	char* buffer;
	int bufferSize;
	WSADATA wsaData;
};

