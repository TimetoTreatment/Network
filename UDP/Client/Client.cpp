#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "../Assets/UDP.h"
#include <thread>

#define SERVER "192.168.219.101"
#define BUFLEN 512
#define PORT 9510

using namespace std::chrono_literals;


char value[20];


void getKeyboard()
{
	for (;;)
	{
		std::cin >> value;
	}
}



int main(void)
{
	char buffer[100];

	UDP* udp = new UDP(9510);

	udp->Initialize();

	std::thread t(getKeyboard);

	for (;;)
	{
		std::this_thread::sleep_for(100ms);
		std::cout << "loop\n";
		memcpy(buffer, value, 20);

		if (buffer[0] != '\0')
		{
			std::cout << "Transmit : " << buffer << std::endl;
			udp->Transmit(value);
		}

		if (udp->ReceiveNonBlock())
			std::cout << "Receive : " << udp->ReadFromBuffer() << std::endl;

		memset(buffer, '\0', 100);
		memset(value, '\0', 20);
	}



	return 0;
}