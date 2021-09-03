#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>

#pragma comment (lib, "ws2_32.lib")


class TCP
{
public:

	enum class WaitEventType
	{
		none,
		connect,
		disconnect,
		message,
	};

	TCP(int port, bool isReceiveBlock = false, std::string targetIP = "")
	{
		this->port = port;
		this->targetIP = targetIP;

		bufferSize = 1024;
		buffer = new char[bufferSize];

		ClearBuffer();

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			std::cerr << "TCP::WSAStartup() : " << WSAGetLastError();
			abort();
		}

		if ((mySocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			std::cerr << "TCP::Socket() : " << WSAGetLastError();
			abort();
		}

		memset(&mySocketHint, 0, sizeof(mySocketHint));
		mySocketHint.sin_family = AF_INET;
		mySocketHint.sin_port = htons(port);
		mySocketHint.sin_addr.S_un.S_addr = INADDR_ANY; // OR inet_pton()

		if (bind(mySocket, (sockaddr*)&mySocketHint, sizeof(mySocketHint)) != 0)
		{
			std::cerr << "TCP::bind() : " << WSAGetLastError();
			abort();
		}

		if (listen(mySocket, SOMAXCONN) != 0)
		{
			std::cerr << "TCP::listen() : " << WSAGetLastError();
			abort();
		}

		fdArray.emplace_back(WSAPOLLFD{ mySocket, POLLRDNORM, 0 });
	}

	~TCP()
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

	WaitEventType WaitEvent(bool timeout = false, int microsecond = -1)
	{
		if (WSAPoll(fdArray.data(), fdArray.size(), microsecond) == 0)
			return WaitEventType::none;

		size_t prevPos = fdArrayCurrentIndex;

		do
		{
			if (fdArrayCurrentIndex >= fdArray.size())
				fdArrayCurrentIndex = 0;

			WSAPOLLFD& pollfd = fdArray[fdArrayCurrentIndex];

			if (pollfd.revents & POLLIN)
			{
				if (pollfd.fd == mySocket)
					return WaitEventType::connect;
				else
				{
					currentClient = pollfd.fd;

					ClearBuffer();
					recv(pollfd.fd, buffer, bufferSize, 0);

					if (buffer[0] == '\\')
						return WaitEventType::none;

					return WaitEventType::message;
				}
			}
			else if (pollfd.revents & POLLHUP)
				return WaitEventType::disconnect;

			fdArrayCurrentIndex++;

			if (fdArrayCurrentIndex >= fdArray.size())
				fdArrayCurrentIndex = 0;

		} while (fdArrayCurrentIndex != prevPos);

		return WaitEventType::none;
	}

	void AddClient()
	{
		currentClient = accept(mySocket, (sockaddr*)&currentClient, nullptr);
		fdArray.emplace_back(WSAPOLLFD{ currentClient, POLLRDNORM , 0 });
	}

	void CloseClient()
	{
		WSAPOLLFD& pollfd = fdArray[fdArrayCurrentIndex];

		shutdown(pollfd.fd, SD_BOTH);
		closesocket(pollfd.fd);

		fdArray.erase(fdArray.begin() + fdArrayCurrentIndex);
	}

	void Send(std::string message)
	{
		send(currentClient, message.data(), message.size() + 1, 0);
	}

	void SendAll(std::string message)
	{
		for (size_t i = 0; i < fdArray.size(); i++)
		{
			if (fdArray[i].fd != currentClient)
				send(fdArray[i].fd, message.data(), message.size() + 1, 0);
		}
	}

	std::string ReadMessage()
	{
		return buffer;
	}

	std::string CurrentSocketID()
	{
		return std::to_string(currentClient);
	}

private:

	void ClearBuffer()
	{
		memset(buffer, 0, bufferSize);
	}

	int port;
	std::string targetIP;

	SOCKET mySocket;
	sockaddr_in mySocketHint;
	SOCKET currentClient = INVALID_SOCKET;

	size_t fdArrayCurrentIndex = 0;
	std::vector<WSAPOLLFD> fdArray;

	char* buffer;
	int bufferSize;
	WSADATA wsaData;
};

