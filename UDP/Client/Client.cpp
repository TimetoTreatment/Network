#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512
#define PORT 9510


int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	WSADATA wsa;

	// Initialize
	std::cout << "Initializing Winsock... ";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed. Error Code : " << WSAGetLastError();
		exit(EXIT_FAILURE);
	}
	std::cout << "Initialized.\n\n";

	// Create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		std::cout << "socket() failed with error code : " << WSAGetLastError();
		exit(EXIT_FAILURE);
	}

	// Setup address structure
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);

	inet_pton(AF_INET, SERVER, &(si_other.sin_addr.s_addr));

	// Start communication
	while (1)
	{
		std::cout << " * Enter message : ";
		std::cin >> message;

		// send the message
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			std::cout << "sendto() failed with error code : " << WSAGetLastError();
			exit(EXIT_FAILURE);
		}

		// receive a reply and print it
		// clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		// try to receive some data, this is a blocking call
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen) == SOCKET_ERROR)
		{
			std::cout << "recvfrom() failed with error code : " << WSAGetLastError();
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}