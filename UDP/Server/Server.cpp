#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#pragma comment (lib, "ws2_32.lib")

using namespace std;


int main()
{
	WSADATA wsaData;

	int wsOk = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return 0;
	}

	// Create a socket
	SOCKET mySocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mySocket == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return 0;
	}

	// Bind the ip address and port to a socket
	sockaddr_in mySocketHint;
	mySocketHint.sin_family = AF_INET;
	mySocketHint.sin_port = htons(9510);
	mySocketHint.sin_addr.S_un.S_addr = INADDR_ANY; // inet_pton() 

	bind(mySocket, (sockaddr*)&mySocketHint, sizeof(mySocketHint));
	listen(mySocket, SOMAXCONN);

	const int clientMaxCount = 10;
	const int timeout = 100;

	// Create the master file descriptor set and zero it
	WSAPOLLFD fdArray[clientMaxCount];

	for (int i = 0; i < clientMaxCount; i++)
	{
		fdArray[i].fd = INVALID_SOCKET;
		fdArray[i].events = 0;
	}

	fdArray[0].fd = mySocket;
	fdArray[0].events = POLLRDNORM;

	for (bool exit = false; !exit;)
	{
		int eventNum = WSAPoll(fdArray, clientMaxCount, -1);

		if (eventNum == 0)
			continue;

		else if (eventNum < 0)
		{
			cerr << "WSAPoll() : " << WSAGetLastError();
			abort();
		}

		for (int i = 0; i < clientMaxCount; i++)
		{
			WSAPOLLFD& pfd = fdArray[i];

			if (pfd.revents & POLLIN)
			{
				if (pfd.fd == mySocket)
				{
					SOCKET client = accept(mySocket, nullptr, nullptr);

					for (int i = 0; i < clientMaxCount; i++)
					{
						if (fdArray[i].fd == INVALID_SOCKET)
						{
							fdArray[i].fd = client;
							fdArray[i].events = POLLRDNORM;

							break;
						}
					}

					string connectMsg = "SERVER : Connected\r\n";
					send(client, connectMsg.c_str(), connectMsg.size() + 1, 0);
				}
				else
				{
					char buf[4096];
					memset(buf, 0, 4096);

					// Receive message
					int bytesIn = recv(pfd.fd, buf, 4096, 0);

					if (buf[0] == '\\')
					{
						cout << bytesIn << endl;
						continue;
					}
					else if (strncmp(buf, "kaka", 4) == 0)
					{
						exit = true;
						continue;
					}

					for (int i = 0; i < clientMaxCount; i++)
					{
						SOCKET outSock = fdArray[i].fd;

						if (outSock != mySocket && outSock != pfd.fd)
						{
							ostringstream ss;
							ss << "SOCKET #" << pfd.fd << ": " << buf << "\r\n";
							string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
			else if (pfd.revents & POLLHUP)
			{
				shutdown(pfd.fd, SD_BOTH);
				closesocket(pfd.fd);
				pfd.fd = INVALID_SOCKET;
				pfd.events = 0;
			}
		}
	}

	closesocket(mySocket);

	string msg = "SERVER is shutdown.";

	for (int i = 1; i < clientMaxCount; i++)
	{
		SOCKET sock = fdArray[i].fd;

		if (sock != INVALID_SOCKET)
		{
			send(sock, msg.c_str(), msg.size() + 1, 0);
			closesocket(sock);
		}
	}

	WSACleanup();

	return 0;
}