#include "TCP.h"


TCP::TCP(std::string port, std::string targetAddress)
{
	isServer = (targetAddress == "0.0.0.0");

	cache = new char[cacheSize];
	memset(cache, 0, cacheSize);
	buffer = new char[bufferSize];
	memset(buffer, 0, bufferSize);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "TCP::WSAStartup() : " << WSAGetLastError();
		abort();
	}

	memset(&mySocketHint, 0, sizeof(mySocketHint));
	mySocketHint.ai_family = AF_INET;
	mySocketHint.ai_socktype = SOCK_STREAM;
	mySocketHint.ai_protocol = IPPROTO_TCP;

	addrinfo* addrinfoResult = nullptr;

	if (isServer)
	{
		mySocketHint.ai_flags = AI_PASSIVE;

		getaddrinfo(nullptr, port.c_str(), &mySocketHint, &addrinfoResult);

		if ((mySocket = socket(addrinfoResult->ai_family, addrinfoResult->ai_socktype, addrinfoResult->ai_protocol)) == INVALID_SOCKET)
		{
			std::cerr << "TCP::Socket() : " << WSAGetLastError();
			abort();
		}

		if (bind(mySocket, addrinfoResult->ai_addr, (int)addrinfoResult->ai_addrlen) == SOCKET_ERROR)
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
	else // (isClient)
	{
		getaddrinfo(targetAddress.c_str(), port.c_str(), &mySocketHint, &addrinfoResult);

		for (addrinfo* iter = addrinfoResult; iter != NULL; iter = iter->ai_next)
		{
			mySocket = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);

			if (mySocket == INVALID_SOCKET) {
				std::cerr << "TCP::socket() : " << WSAGetLastError();
				WSACleanup();
				abort();
			}

			if (connect(mySocket, iter->ai_addr, (int)iter->ai_addrlen) == SOCKET_ERROR) {
				closesocket(mySocket);
				mySocket = INVALID_SOCKET;
			}
			else break;
		}

		if (mySocket == INVALID_SOCKET)
		{
			std::cerr << "TCP::connect()";
			abort();
		}

		sender = mySocket;
	}

	freeaddrinfo(addrinfoResult);

	fdArray.emplace_back(WSAPOLLFD{ mySocket, POLLRDNORM, 0 });
}

TCP::~TCP()
{
	shutdown(mySocket, SD_BOTH);
	closesocket(mySocket);

	for (size_t i = 0; i < fdArray.size(); i++)
	{
		shutdown(fdArray[i].fd, SD_BOTH);
		closesocket(fdArray[i].fd);
	}

	WSACleanup();

	delete cache;
}

TCP::WaitEventType TCP::WaitEvent(int timeoutMicroSecond)
{
	if (WSAPoll(fdArray.data(), (ULONG)fdArray.size(), timeoutMicroSecond) == 0)
		return WaitEventType::NONE;

	size_t prevPos = fdArrayCurrentIndex;

	do
	{
		WSAPOLLFD& pollfd = fdArray[fdArrayCurrentIndex];

		if (pollfd.revents & POLLIN)
		{
			sender = pollfd.fd;

			if (isServer && pollfd.fd == mySocket)
				return WaitEventType::NEWCLIENT;
			else
			{
				memset(cache, 0, cacheSize);
				recv(pollfd.fd, cache, cacheSize, 0);

				if (cache[0] == '\\')
					return WaitEventType::NONE;

				return WaitEventType::MESSAGE;
			}
		}
		else if (pollfd.revents & POLLHUP)
		{
			sender = pollfd.fd;

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
	sender = accept(mySocket, nullptr, nullptr);
	fdArray.emplace_back(WSAPOLLFD{ sender, POLLRDNORM , 0 });
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


void TCP::Send(const char* message, int size, SendTo sendTo)
{
	if (size < cacheSize)
	{
		switch (sendTo)
		{
		case SendTo::EVENT_SOURCE:

			send(sender, message, cacheSize, 0);
			break;

		case SendTo::ALL:

			for (size_t i = 0; i < fdArray.size(); i++)
				send(fdArray[i].fd, message, cacheSize, 0);
			break;

		case SendTo::OTHERS:

			for (size_t i = 0; i < fdArray.size(); i++)
			{
				if (fdArray[i].fd != sender)
					send(fdArray[i].fd, message, cacheSize, 0);
			}
			break;
		}

		return;
	}

	auto RecvAll = [&]()
	{
		int iterCount = size / cacheSize;
		int remainder = size % cacheSize;
		int pos = 0;

		for (int count = 0; count < iterCount; count++)
		{
			pos = count * cacheSize;
			memcpy(cache, message + pos, cacheSize);
			send(sender, cache, cacheSize, 0);
		}

		if (remainder != 0)
		{
			pos = iterCount * cacheSize;
			memcpy(cache, message + pos, remainder);
			send(sender, cache, remainder, 0);
		}
	};

	switch (sendTo)
	{
	case SendTo::EVENT_SOURCE:
		RecvAll();
		break;

	case SendTo::ALL:

		for (size_t i = 0; i < fdArray.size(); i++)
			RecvAll();
		break;

	case SendTo::OTHERS:

		for (size_t i = 0; i < fdArray.size(); i++)
		{
			if (fdArray[i].fd != sender)
				RecvAll();
		}
		break;
	}
}

const char* TCP::ReadMessage()
{
	return cache;
}

const char* TCP::ReadBuffer(int size)
{
	if (size <= cacheSize)
		return cache;

	int iterCount = size / cacheSize;
	int remainder = size % cacheSize;
	int pos = 0;

	memcpy(buffer, cache, cacheSize);

	for (int count = 1; count < iterCount; count++)
	{
		pos = count * cacheSize;
		recv(fdArray[fdArrayCurrentIndex].fd, cache, cacheSize, 0);
		memcpy(buffer + pos, cache, cacheSize);
	}

	if (remainder != 0)
	{
		pos = iterCount * cacheSize;
		recv(fdArray[fdArrayCurrentIndex].fd, cache, remainder, 0);
		memcpy(buffer + pos, cache, cacheSize);
	}

	return buffer;
}


std::string TCP::ReadSenderID()
{
	return std::to_string(sender);
}