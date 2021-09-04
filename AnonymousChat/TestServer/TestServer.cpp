#include <iostream>
#include <string>
#include <thread>
#include <map>
#include "../../Utility/TCP.h"

using namespace std;
using namespace chrono_literals;


int main()
{
	TCP* tcp = new TCP(9510);

	cout << "****************" << endl;
	cout << "* Server Start *" << endl;
	cout << "****************" << endl;

	map<string, string> users;

	for (bool exit = false; !exit;)
	{
		switch (tcp->WaitEvent(false))
		{
		case TCP::WaitEventType::NEWCLIENT:
			tcp->AddClient();
			tcp->Send("[SERVER] Your SocketID is #" + tcp->CurrentSocketID() + "\r\n");
			tcp->Send("[SERVER] #" + tcp->CurrentSocketID() + " is connected" + "\r\n", TCP::SendRange::OTHERS);
			cout << "[SERVER] #" + tcp->CurrentSocketID() + " is connected." << endl;
			break;

		case TCP::WaitEventType::DISCONNECT:
			tcp->CloseClient();
			tcp->Send("[SERVER] #" + tcp->CurrentSocketID() + " is disconnected" + "\r\n", TCP::SendRange::OTHERS);
			cout << "[SERVER] #" + tcp->CurrentSocketID() + " is disconnected." << endl;
			break;

		case TCP::WaitEventType::MESSAGE:

			string message = tcp->ReadMessage();

			if (message.substr(0, 10) == "[SHUTDOWN]")
				exit = true;
			else if (message.substr(0, 10) == "[USERNAME]")
				users.emplace(tcp->CurrentSocketID(), message.substr(10));
			else
			{
				tcp->Send("[" + users[tcp->CurrentSocketID()] + "] : " + tcp->ReadMessage() + "\r\n", TCP::SendRange::OTHERS);
				cout << "[CLIENT] #" + tcp->CurrentSocketID() + " send a message" << endl;
			}

			break;
		}

		this_thread::sleep_for(100us);
	}

	tcp->Send("[SERVER] Server is closed\r\n", TCP::SendRange::ALL);
	delete tcp;

	return 0;
}