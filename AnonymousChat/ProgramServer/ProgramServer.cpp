#include <iostream>
#include <string>
#include <thread>
#include <map>
#include "../../Utility/TCP.h"
#include "../../Utility/Random.h"

using namespace std;
using namespace chrono_literals;


class Operation
{
public:

	Operation(string line)
	{
		size_t whiteSpace = line.find(' ');

		command = line.substr(0, whiteSpace);
		argument = line.substr(whiteSpace + 1);
	}

	string command;
	string argument;
};


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
			tcp->Send("[SERVER] Your SocketID is #" + tcp->CurrentSocketID());
			tcp->Send("[SERVER] #" + tcp->CurrentSocketID() + " is connected", TCP::SendRange::OTHERS);
			cout << "[SERVER] #" + tcp->CurrentSocketID() + " is connected." << endl;
			break;

		case TCP::WaitEventType::DISCONNECT:
			tcp->CloseClient();
			tcp->Send("[SERVER] #" + tcp->CurrentSocketID() + " is disconnected", TCP::SendRange::OTHERS);
			users.erase(tcp->CurrentSocketID());
			cout << "[SERVER] #" + tcp->CurrentSocketID() + " is disconnected." << endl;
			break;

		case TCP::WaitEventType::MESSAGE:

			string messageIn = tcp->ReadMessage();

			if (messageIn.substr(0, 2) == "--")
			{
				Operation operation(messageIn);

				if (operation.command == "--shutdown")
					exit = true;

				else if (operation.command == "--username")
				{
					string userName = operation.argument;

					if (userName.empty())
						userName = "익명" + to_string(5);

					if (users.find(tcp->CurrentSocketID()) == users.end())
						users.emplace(tcp->CurrentSocketID(), userName);
					else
						users[tcp->CurrentSocketID()] = userName;
				}

				else if (operation.command == "--dice")
				{
					string messageOut = "[SERVER] " + users[tcp->CurrentSocketID()] + "님이 주사위를 굴려 " + std::to_string(Random::Integer(0, 100)) + "이 나왔습니다!";
					tcp->Send(messageOut, TCP::SendRange::ALL);
				}
			}
			else
			{
				tcp->Send("[" + users[tcp->CurrentSocketID()] + "] : " + tcp->ReadMessage(), TCP::SendRange::ALL);
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