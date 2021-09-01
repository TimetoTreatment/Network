#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>

#include <iostream>


class UDP
{
public:

	UDP(int port)
	{
		buffer = new char[bufferLength];
		slen = sizeof(clientAddress);

		this->port = port;
	}

	void Initialize()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			std::cout << "Failed. Error Code : " << WSAGetLastError();
			abort();
		}

		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			std::cout << "Could not create socket : " << WSAGetLastError();
		}

		// Prepare the sockaddr_in structure
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = INADDR_ANY;
		serverAddress.sin_port = htons(port);

		if (bind(s, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
		{
			std::cout << "Bind failed with error code : " << WSAGetLastError();
			abort();
		}
	}


	bool Transmit()
	{


	}


	bool Receive(bool debugMode = false)
	{
		// clear the buffer by filling null, it might have previously received data
		memset(buffer, '\0', bufferLength);

		// try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buffer, bufferLength, 0, (struct sockaddr*)&clientAddress, &slen)) == SOCKET_ERROR)
		{
			std::cout << "recvfrom() failed with error code : " << WSAGetLastError();
			exit(EXIT_FAILURE);
		}

		// print details of the clientAddress/peer and the data received
		if (debugMode)
		{
			char str[20];
			std::cout << " - Received packet from " << inet_ntop(AF_INET, &clientAddress.sin_addr, str, sizeof(str)) << ':' << ntohs(clientAddress.sin_port) << '\n';
		}

		return true;
	}

	bool ReceiveNonBlock(bool block, bool debugMode = false)
	{
		// clear the buffer by filling null, it might have previously received data
		memset(buffer, '\0', bufferLength);

		u_long mode = 1;
		ioctlsocket(s, FIONBIO, &mode);

		// try to receive some data, this is a blocking call

		if ((recv_len = recvfrom(s, buffer, bufferLength, 0, (struct sockaddr*)&clientAddress, &slen)) < 0)
			return false;
		else
			return true;

		if (debugMode)
		{
			char str[20];
			std::cout << " - Received packet from " << inet_ntop(AF_INET, &clientAddress.sin_addr, str, sizeof(str)) << ':' << ntohs(clientAddress.sin_port) << '\n';
		}

		return true;
	}


	char* ReadFromBuffer()
	{
		return buffer;
	}

	void Close()
	{
		closesocket(s);
		WSACleanup();
	}


private:


	char* buffer = nullptr;
	const int bufferLength = 1024;
	SOCKET s;
	sockaddr_in serverAddress;
	sockaddr_in clientAddress;

	int port;
	int slen, recv_len;

	WSADATA wsa;
};

