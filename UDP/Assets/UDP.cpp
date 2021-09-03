#include "UDP.h"


UDP::UDP(int port, bool isReceiveBlock, string targetIP)
{
	this->port = port;
	this->targetIP = targetIP;

	buffer = new char[bufferLength];
	targetAddressLength = sizeof(this->targetAddress);

	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		std::cout << "[ERROR] WSAStartup : " << WSAGetLastError();
		abort();
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
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

	if (bind(sock, (struct sockaddr*)&myAddress, sizeof(myAddress)) == SOCKET_ERROR)
	{
		std::cout << "[ERROR] bind : " << WSAGetLastError();
		abort();
	}

	if (isReceiveBlock)
	{
		u_long mode = 1;
		ioctlsocket(sock, FIONBIO, &mode);
	}
}

UDP::~UDP()
{
	delete buffer;
}

void UDP::Send(const char* message)
{
	inet_pton(AF_INET, targetIP.c_str(), &(targetAddress.sin_addr.s_addr));

	if (sendto(sock, message, strlen(message), 0, (struct sockaddr*)&targetAddress, targetAddressLength) == SOCKET_ERROR)
	{
		std::cout << "[ERROR] Transmit::sendto : " << WSAGetLastError();
		abort();
	}
}


void UDP::Receive()
{
	ClearBuffer();

	if ((recvLen = recvfrom(sock, buffer, bufferLength, 0, (struct sockaddr*)&targetAddress, &targetAddressLength)) == SOCKET_ERROR)
	{
		std::cout << "[ERROR] Receive::recvfrom() : " << WSAGetLastError();
		abort();
	}
}

bool UDP::ReceiveNonBlock()
{
	ClearBuffer();

	if ((recvLen = recvfrom(sock, buffer, bufferLength, 0, (struct sockaddr*)&targetAddress, &targetAddressLength)) < 0)
		return false;
	else
	{
		isBufferCleared = false;
		return true;
	}
}

char* UDP::ReadFromBuffer()
{
	return buffer;
}

void UDP::Close()
{
	closesocket(sock);
	WSACleanup();
}


void UDP::ClearBuffer()
{
	if (!isBufferCleared)
	{
		memset(buffer, '\0', bufferLength);
		isBufferCleared = true;
	}
}

void UDP::PacketFrom()
{
	char str[20];
	std::cout << " - Received packet from " << inet_ntop(AF_INET, &targetAddress.sin_addr, str, sizeof(str)) << ':' << ntohs(targetAddress.sin_port) << '\n';
}