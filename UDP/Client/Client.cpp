#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "../Assets/UDP.h"
#include <thread>


using namespace std::chrono_literals;
using std::string;

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
	int port;
	string targetAddress;

	std::cout << "PORT : ";
	std::cin >> port;

	std::cout << "Target Address : ";
	std::cin >> targetAddress;

	UDP* udp = new UDP(port, true, targetAddress);

	udp->Initialize();

	std::thread t(getKeyboard);

	for (;;)
	{
		std::this_thread::sleep_for(10ms);
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

	udp->Terminate();
	delete udp;

	return 0;
}