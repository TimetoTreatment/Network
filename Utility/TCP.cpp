#include "TCP.h"

TCP::TCP(int port, std::string targetIP)
{
	this->port = port;
	this->targetIP = targetIP;

	isServer = (targetIP == "0.0.0.0");

	bufferSize = 1024;
	buffer = new char[bufferSize];

	ClearBuffer();

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "TCP::WSAStartup() : " << WSAGetLastError();
		abort();
	}

	addrinfo* result = nullptr;

	memset(&mySocketHint, 0, sizeof(mySocketHint));
	mySocketHint.ai_family = AF_INET;
	mySocketHint.ai_socktype = SOCK_STREAM;
	mySocketHint.ai_protocol = IPPROTO_TCP;
	mySocketHint.ai_flags = AI_PASSIVE;

	if (isServer)
	{
		getaddrinfo(nullptr, std::to_string(port).c_str(), &mySocketHint, &result);

		if ((mySocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
		{
			std::cerr << "TCP::Socket() : " << WSAGetLastError();
			abort();
		}

		if (bind(mySocket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
		{
			std::cerr << "TCP::bind() : " << WSAGetLastError();
			abort();
		}

		if (listen(mySocket, SOMAXCONN) != 0)
		{
			std::cerr << "TCP::listen() : " << WSAGetLastError();
			abort();
		}
	}
	else
	{
		getaddrinfo(targetIP.c_str(), std::to_string(port).c_str(), &mySocketHint, &result);

		for (addrinfo* iter = result; iter != NULL; iter = iter->ai_next) {

			// Create a SOCKET for connecting to server
			mySocket = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);

			if (mySocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				abort();
			}

			// Connect to server.

			if (connect(mySocket, iter->ai_addr, (int)iter->ai_addrlen) == SOCKET_ERROR) {
				closesocket(mySocket);
				mySocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		if (mySocket == INVALID_SOCKET)
		{
			std::cerr << "TCP::connect()";
			abort();
		}

		currentClient = mySocket;
	}

	freeaddrinfo(result);

	fdArray.emplace_back(WSAPOLLFD{ mySocket, POLLRDNORM, 0 });
}

TCP::~TCP()
{
	closesocket(mySocket);

	for (size_t i = 0; i < fdArray.size(); i++)
	{
		shutdown(fdArray[i].fd, SD_BOTH);
		closesocket(fdArray[i].fd);
	}

	WSACleanup();

	delete buffer;
}

TCP::WaitEventType TCP::WaitEvent(int timeoutMicroSecond)
{
	if (WSAPoll(fdArray.data(), fdArray.size(), timeoutMicroSecond) == 0)
		return WaitEventType::NONE;

	size_t prevPos = fdArrayCurrentIndex;

	do
	{
		WSAPOLLFD& pollfd = fdArray[fdArrayCurrentIndex];

		if (pollfd.revents & POLLIN)
		{
			currentClient = pollfd.fd;

			if (isServer && pollfd.fd == mySocket)
				return WaitEventType::NEWCLIENT;
			else
			{
				ClearBuffer();
				recv(pollfd.fd, buffer, bufferSize, 0);

				if (buffer[0] == '\\')
					return WaitEventType::NONE;

				return WaitEventType::MESSAGE;
			}
		}
		else if (pollfd.revents & POLLHUP)
		{
			currentClient = pollfd.fd;

			return WaitEventType::DISCONNECT;
		}

		fdArrayCurrentIndex++;

		if (fdArrayCurrentIndex >= fdArray.size())
			fdArrayCurrentIndex = 0;

	} while (fdArrayCurrentIndex != prevPos);

	return WaitEventType::NONE;
}

void TCP::AddClient()
{
	currentClient = accept(mySocket, (sockaddr*)&currentClient, nullptr);
	fdArray.emplace_back(WSAPOLLFD{ currentClient, POLLRDNORM , 0 });
}

void TCP::CloseClient()
{
	WSAPOLLFD& pollfd = fdArray[fdArrayCurrentIndex];

	shutdown(pollfd.fd, SD_BOTH);
	closesocket(pollfd.fd);

	fdArray.erase(fdArray.begin() + fdArrayCurrentIndex);

	if (fdArrayCurrentIndex >= fdArray.size())
		fdArrayCurrentIndex = 0;
}

void TCP::Send(std::string message, SendRange sendRange)
{
	switch (sendRange)
	{
	case SendRange::EVENT_SENDER:

		send(currentClient, message.data(), message.size() + 1, 0);
		break;

	case SendRange::ALL:

		for (size_t i = 0; i < fdArray.size(); i++)
			send(fdArray[i].fd, message.data(), message.size() + 1, 0);
		break;

	case SendRange::OTHERS:

		for (size_t i = 0; i < fdArray.size(); i++)
		{
			if (fdArray[i].fd != currentClient)
				send(fdArray[i].fd, message.data(), message.size() + 1, 0);
		}
		break;
	}
}

std::string TCP::ReadMessage()
{
	return buffer;
}

std::string TCP::CurrentSocketID()
{
	return std::to_string(currentClient);
}

void TCP::ClearBuffer()
{
	memset(buffer, 0, bufferSize);
}