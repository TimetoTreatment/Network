#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>

#define BUFLEN 512
#define PORT 9510


int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	// Initialise winsock
	std::cout << "Initializing Winsock... ";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed. Error Code : " << WSAGetLastError();
		exit(EXIT_FAILURE);
	}
	std::cout << "Initialized.\n";

	// Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		std::cout << "Could not create socket : " << WSAGetLastError();
	}
	std::cout << "Socket created.\n";

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	// Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		std::cout << "Bind failed with error code : " << WSAGetLastError();
		exit(EXIT_FAILURE);
	}

	std::cout << "Bind done\n\n";

	// Keep listening for data
	while (1)
	{
		std::cout << "* Waiting for data..." << std::endl;

		// clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		// try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
		{
			std::cout << "recvfrom() failed with error code : " << WSAGetLastError();
			exit(EXIT_FAILURE);
		}

		char str[20];

		// print details of the client/peer and the data received

		std::cout << " - Received packet from " << inet_ntop(AF_INET, &si_other.sin_addr, str, sizeof(str)) << ':' << ntohs(si_other.sin_port) << '\n';
		std::cout << " - Data : " << buf << "\n\n";

		// now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			std::cout << "sendto() failed with error code : " << WSAGetLastError();
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}