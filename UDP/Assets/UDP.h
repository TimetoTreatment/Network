#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>

using std::string;

class UDP
{
public:

	UDP(int port, bool isReceiveBlock = true, string targetIP = "")
	{
		this->port = port;
		this->isReceiveBlock = isReceiveBlock;
		this->targetIP = targetIP;

		buffer = new char[bufferLength];
		targetAddressLength = sizeof(this->targetAddress);
	}

	~UDP()
	{
		delete buffer;
	}

	void Initialize()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			std::cout << "[ERROR] WSAStartup : " << WSAGetLastError();
			abort();
		}

		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			std::cout << "[ERROR] socket : " << WSAGetLastError();
			abort();
		}

		myAddress.sin_family = AF_INET;
		myAddress.sin_addr.s_addr = INADDR_ANY;
		myAddress.sin_port = htons(port);

		memset((char*)&targetAddress, 0, sizeof(targetAddress));
		targetAddress.sin_family = AF_INET;
		targetAddress.sin_port = htons(port);

		if (bind(s, (struct sockaddr*)&myAddress, sizeof(myAddress)) == SOCKET_ERROR)
		{
			std::cout << "[ERROR] bind : " << WSAGetLastError();
			abort();
		}

		if (isReceiveBlock)
		{
			u_long mode = 1;
			ioctlsocket(s, FIONBIO, &mode);
		}
	}

	void Transmit(const char* message)
	{
		inet_pton(AF_INET, targetIP.c_str(), &(targetAddress.sin_addr.s_addr));

		if (sendto(s, message, strlen(message), 0, (struct sockaddr*)&targetAddress, targetAddressLength) == SOCKET_ERROR)
		{
			std::cout << "[ERROR] Transmit::sendto : " << WSAGetLastError();
			abort();
		}
	}

	void Receive()
	{
		memset(buffer, '\0', bufferLength);

		if ((recv_len = recvfrom(s, buffer, bufferLength, 0, (struct sockaddr*)&targetAddress, &targetAddressLength)) == SOCKET_ERROR)
		{
			std::cout << "[ERROR] Receive::recvfrom() : " << WSAGetLastError();
			abort();
		}
	}

	bool ReceiveNonBlock()
	{
		if (!isBufferCleared)
		{
			memset(buffer, '\0', bufferLength);
			isBufferCleared = true;
		}

		if ((recv_len = recvfrom(s, buffer, bufferLength, 0, (struct sockaddr*)&targetAddress, &targetAddressLength)) < 0)
			return false;
		else
		{
			isBufferCleared = false;
			return true;
		}
	}

	char* ReadFromBuffer()
	{
		return buffer;
	}

	void Terminate()
	{
		closesocket(s);
		WSACleanup();
	}


private:

	void PacketFrom()
	{
		char str[20];
		std::cout << " - Received packet from " << inet_ntop(AF_INET, &targetAddress.sin_addr, str, sizeof(str)) << ':' << ntohs(targetAddress.sin_port) << '\n';
	}

	char* buffer = nullptr;
	const int bufferLength = 1024;
	bool isBufferCleared = false;
	SOCKET s;
	sockaddr_in myAddress;
	sockaddr_in targetAddress;
	int targetAddressLength;

	int port;
	bool isReceiveBlock;
	string targetIP;
	int recv_len;


	WSADATA wsa;
};

