#include "TCPLinux.h"


TCP::TCP(std::string port, std::string targetAddress)
{
	isServer = (targetAddress == "0.0.0.0");

	buffer = new char[bufferSize];
	memset(buffer, 0, sizeof(mySocketHint));

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

			if (mySocket == -1) {
				std::cerr << "TCP::socket() : " << errno;
				abort();
			}

			if (connect(mySocket, iter->ai_addr, (int)iter->ai_addrlen) == -1) {
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
	close(mySocket);

	for (size_t i = 0; i < fdArray.size(); i++)
	{
		shutdown(fdArray[i].fd, SHUT_RDWR);
		close(fdArray[i].fd);
	}

	delete buffer;
}

TCP::WaitEventType TCP::WaitEvent(int timeoutMicroSecond)
{
	if (poll(fdArray.data(), fdArray.size(), timeoutMicroSecond) == 0)
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
			{
				memset(buffer, 0, sizeof(mySocketHint));
				recv(pfd.fd, buffer, bufferSize, 0);

				if (buffer[0] == '\\')
					return WaitEventType::NONE;

				return WaitEventType::MESSAGE;
			}
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

void TCP::Send(std::string message, SendTo sendTo)
{
	switch (sendTo)
	{
	case SendTo::EVENT_SOURCE:

		send(sender, message.data(), (int)message.size() + 1, 0);
		break;

	case SendTo::ALL:

		for (size_t i = 0; i < fdArray.size(); i++)
			send(fdArray[i].fd, message.data(), (int)message.size() + 1, 0);
		break;

	case SendTo::OTHERS:

		for (size_t i = 0; i < fdArray.size(); i++)
		{
			if (fdArray[i].fd != sender)
				send(fdArray[i].fd, message.data(), (int)message.size() + 1, 0);
		}
		break;
	}
}

std::string TCP::ReadMessage()
{
	return buffer;
}

std::string TCP::ReadSenderID()
{
	return std::to_string(sender);
}