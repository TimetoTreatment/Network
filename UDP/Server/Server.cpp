#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include "../Assets/UDP.h"

using namespace std::chrono_literals;


int main()
{
	UDP* udp = new UDP(9510);

	udp->Initialize();

	for (;;)
	{
		if (udp->Receive(false))
		{
			std::cout << udp->ReadFromBuffer();
		}

		std::this_thread::sleep_for(100ms);

	}


	udp->Close();




	return 0;
}