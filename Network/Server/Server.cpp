#include <iostream>
#include <string>
#include <thread>
#include "../Assets/TCP.h"

using namespace std;
using namespace chrono_literals;

int main()
{
	TCP* tcp = new TCP(9510);

	for (bool exit = false; !exit;)
	{
		switch (tcp->WaitEvent(false))
		{
		case TCP::WaitEventType::NEWCLIENT:
			tcp->AddClient();
			tcp->Send("[SERVER] Your ID is #" + tcp->CurrentSocketID() + "\r\n");
			tcp->SendAll("[SERVER] #" + tcp->CurrentSocketID() + " is connected" + "\r\n");
			cout << "[CLIENT] #" + tcp->CurrentSocketID() + " is connected" << endl;
			break;

		case TCP::WaitEventType::DISCONNECT:
			tcp->SendAll("[SERVER] #" + tcp->CurrentSocketID() + " is disconnected" + "\r\n");
			tcp->CloseClient();

			break;

		case TCP::WaitEventType::MESSAGE:
			if (tcp->ReadMessage().substr(0, 4) == "EXIT")
				exit = true;
			else
			{
				string msg = "#" + tcp->CurrentSocketID() + " : " + tcp->ReadMessage() + "\r\n";
				tcp->SendAll(msg);
				tcp->Send(msg);
				cout << msg;
			}

			break;
		}

		this_thread::sleep_for(100us);
	}

	tcp->SendAll("[SERVER] Server is closed");
	delete tcp;

	return 0;
}