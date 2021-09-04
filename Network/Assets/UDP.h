#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment (lib, "ws2_32.lib")

using std::string;


class UDP
{
public:

	UDP(int port, bool isReceiveBlock = false, string targetIP = "");
	~UDP();

	void Send(const char* message);

	void Receive();
	bool ReceiveNonBlock();
	char* ReadFromBuffer();
	void Close();


private:

	void ClearBuffer();
	void PacketFrom();

	int port;
	string targetIP;
	int recvLen;

	char* buffer = nullptr;
	int bufferLength = 1024;
	bool isBufferCleared = false;
	SOCKET sock;
	sockaddr_in myAddress;
	sockaddr_in targetAddress;
	int targetAddressLength;
	WSADATA WSAData;
};

