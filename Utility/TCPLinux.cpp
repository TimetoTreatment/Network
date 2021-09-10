#include "TCPLinux.h"


TCP::TCP(std::string port, std::string targetAddress)
{
	isServer = (targetAddress == "0.0.0.0");

	cache = new char[cacheSize];
	memset(cache, 0, cacheSize);
	buffer = new char[bufferSize];
	memset(buffer, 0, bufferSize);

	memset(&mySocketHint, 0, sizeof(mySocketHint));
	mySocketHint.ai_family = AF_INET;
	mySocketHint.ai_socktype = SOCK_STREAM;
	mySocketHint.ai_protocol = IPPROTO_TCP;

	addrinfo* addrinfoResult = nullptr;

	if (isServer)
	{
		mySocketHint.ai_flags = AI_PASSIVE;

		getaddrinfo(nullptr, port.c_str(), &mySocketHint, &addrinfoResult);

		if ((mySocket = socket(addrinfoResult->ai_family, addrinfoResult->ai_socktype, addrinfoResult->ai_protocol)) == -1)
		{
			std::cerr << "TCP::Socket() : " << errno;
			abort();
		}

		if (bind(mySocket, addrinfoResult->ai_addr, (int)addrinfoResult->ai_addrlen) == -1)
		{
			std::cerr << "TCP::bind() : " << errno;
			abort();
		}

		if (listen(mySocket, SOMAXCONN) != 0)
		{
			std::cerr << "TCP::listen() : " << errno;
			abort();
		}
	}
	else // (isClient)
	{
		getaddrinfo(targetAddress.c_str(), port.c_str(), &mySocketHint, &addrinfoResult);

		for (addrinfo* iter = addrinfoResult; iter != NULL; iter = iter->ai_next)
		{
			mySocket = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);

			if (mySocket == -1)
			{
				std::cerr << "TCP::socket() : " << errno;
				abort();
			}

			if (connect(mySocket, iter->ai_addr, (int)iter->ai_addrlen) == -1)
			{
				close(mySocket);
				mySocket = -1;
			}
			else break;
		}

		if (mySocket == -1)
		{
			std::cerr << "TCP::connect()";
			abort();
		}

		sender = mySocket;
	}

	freeaddrinfo(addrinfoResult);

	fdArray.emplace_back(pollfd{ mySocket, POLLRDNORM, 0 });
}


TCP::~TCP()
{
	shutdown(mySocket, SHUT_RDWR);
	close(mySocket);

	for (size_t i = 0; i < fdArray.size(); i++)
	{
		shutdown(fdArray[i].fd, SHUT_RDWR);
		close(fdArray[i].fd);
	}

	delete cache;
}

void TCP::AddClient()
{
	sender = accept(mySocket, nullptr, nullptr);
	fdArray.emplace_back(pollfd{ sender, POLLRDNORM , 0 });
}


void TCP::CloseClient()
{
	pollfd pfd = fdArray[fdArrayCurrentIndex];

	shutdown(pfd.fd, SHUT_RDWR);
	close(pfd.fd);

	fdArray.erase(fdArray.begin() + fdArrayCurrentIndex);

	if (fdArrayCurrentIndex >= fdArray.size())
		fdArrayCurrentIndex = 0;
}


TCP::WaitEventType TCP::WaitEvent(int timeoutMilliseconds)
{
	if (poll(fdArray.data(), fdArray.size(), timeoutMilliseconds) == 0)
		return WaitEventType::NONE;

	size_t prevPos = fdArrayCurrentIndex;

	do
	{
		pollfd& pfd = fdArray[fdArrayCurrentIndex];

		if (pfd.revents & POLLHUP)
		{
			sender = pfd.fd;

			return WaitEventType::DISCONNECT;
		}
		else if (pfd.revents & POLLIN || pfd.revents & POLLRDNORM)
		{
			sender = pfd.fd;

			if (isServer && pfd.fd == mySocket)
				return WaitEventType::NEWCLIENT;
			else
				return WaitEventType::MESSAGE;
		}

		fdArrayCurrentIndex++;

		if (fdArrayCurrentIndex >= fdArray.size())
			fdArrayCurrentIndex = 0;

	} while (fdArrayCurrentIndex != prevPos);

	return WaitEventType::NONE;
}


void TCP::SendMsg(std::string message, SendTo sendTo)
{
	Send(message.c_str(), message.size() + 1, sendTo);
}


void TCP::Send(const char* message, int size, SendTo sendTo)
{
	auto SendStream = [&](int sendTo)
	{
		int sendSize = 0;
		int iResult;

		for (; sendSize < size;)
		{
			if (sendSize + cacheSize <= size)
				iResult = send(sendTo, message + sendSize, cacheSize, 0);
			else
				iResult = send(sendTo, message + sendSize, size - sendSize, 0);

			sendSize += iResult;
		}

		if (sendSize != size)
			std::cout << "?\n";
	};

	switch (sendTo)
	{
	case SendTo::EVENT_SOURCE:
		SendStream(sender);
		break;

	case SendTo::ALL:
		for (size_t i = 0; i < fdArray.size(); i++)
			SendStream(fdArray[i].fd);
		break;

	case SendTo::OTHERS:
		for (size_t i = 0; i < fdArray.size(); i++)
		{
			if (fdArray[i].fd != sender)
				SendStream(fdArray[i].fd);
		}
		break;
	}
}


std::string TCP::ReadMsg()
{
	std::string message;

	if (bufferValidDataSize > 0)
	{
		message = std::string(buffer);

		memmove(buffer, buffer + message.size() + 1, bufferValidDataSize - message.size() - 1);

		bufferValidDataSize -= message.size() + 1;
	}
	else
	{
		int iResult = recv(fdArray[fdArrayCurrentIndex].fd, cache, cacheSize, 0);

		message = std::string(cache);

		bufferValidDataSize = iResult - message.size() - 1;

		if (bufferValidDataSize > 0)
		{
			memmove(cache, cache + message.size() + 1, bufferValidDataSize);
			memcpy(buffer, cache, bufferValidDataSize);
		}
	}

	return message;
}


const char* TCP::ReadData(int size)
{
	for (;;)
	{
		int iResult;

		if (size - bufferValidDataSize > cacheSize)
			iResult = recv(fdArray[fdArrayCurrentIndex].fd, cache, cacheSize, 0);
		else
			iResult = recv(fdArray[fdArrayCurrentIndex].fd, cache, size - bufferValidDataSize, 0);

		if (iResult > 0)
		{
			if (bufferValidDataSize + iResult <= size)
			{
				memcpy(buffer + bufferValidDataSize, cache, iResult);
				bufferValidDataSize += iResult;

				if (bufferValidDataSize >= size)
					break;
			}
			else
			{
				memcpy(buffer + bufferValidDataSize, cache, (size_t)size - bufferValidDataSize);
				bufferValidDataSize += size - bufferValidDataSize;
				break;
			}
		}
		else if (iResult == 0 || bufferValidDataSize >= size)
			break;
	}

	bufferValidDataSize = 0;
	return buffer;
}


std::string TCP::ReadSenderID()
{
	return std::to_string(sender);
}