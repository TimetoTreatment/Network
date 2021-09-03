#include <iostream>
#include <string>
#include "../Assets/TCP.h"

using namespace std;


int main()
{
	TCP* tcp = new TCP(9510);

	for (bool exit = false; !exit;)
	{
		switch (tcp->WaitEvent())
		{
		case TCP::WaitEventType::connect:
			tcp->AddClient();
			tcp->Send("[SERVER] Your ID is #" + tcp->CurrentSocketID() + "\r\n");
			break;

		case TCP::WaitEventType::disconnect:
			tcp->CloseClient();
			break;

		case TCP::WaitEventType::message:

			string message = tcp->ReadMessage();

			if (message.substr(0, 4) == "EXIT")
				exit = true;
			else
				tcp->SendAll("#" + tcp->CurrentSocketID() + " : " + tcp->ReadMessage());

			break;
		}
	}

	tcp->SendAll("[SERVER] Server is closed");
	delete tcp;

	return 0;
}