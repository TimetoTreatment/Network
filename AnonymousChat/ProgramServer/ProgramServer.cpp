#include <iostream>
#include <string>
#include <thread>
#include <map>
#include "../../Utility/TCP.h"
#include "../../Utility/Random.h"

using namespace std;
using namespace chrono_literals;


struct Operation
{
public:

	Operation(string line)
	{
		size_t whiteSpace = line.find(' ');

		command = line.substr(0, whiteSpace);

		if (whiteSpace != string::npos)
			argument = line.substr(whiteSpace + 1);
	}

	string command;
	string argument;
};


int main()
{
	TCP* tcp = new TCP("9510");

	cout << "****************" << endl;
	cout << "* Server Start *" << endl;
	cout << "****************" << endl;

	map<string, string> users;
	int anonymCount = 0;

	for (bool exit = false; !exit;)
	{
		switch (tcp->WaitEvent())
		{
		case TCP::WaitEventType::NEWCLIENT:

			tcp->AddClient();
			tcp->Send("[SERVER] Your SocketID is #" + tcp->ReadSenderID());
			tcp->Send("[SERVER] #" + tcp->ReadSenderID() + " is connected", TCP::SendRange::OTHERS);
			cout << "[SERVER] #" + tcp->ReadSenderID() + " is connected." << endl;
			break;

		case TCP::WaitEventType::DISCONNECT:

			tcp->CloseClient();
			tcp->Send("[SERVER] " + users[tcp->ReadSenderID()] + "님이 방을 나가셨습니다.", TCP::SendRange::OTHERS);
			users.erase(tcp->ReadSenderID());
			cout << "[SERVER] #" + tcp->ReadSenderID() + " is disconnected." << endl;
			break;

		case TCP::WaitEventType::MESSAGE:

			if (tcp->ReadMessage().substr(0, 2) == "--")
			{
				Operation operation(tcp->ReadMessage());

				if (operation.command == "--shutdown")
					exit = true;
				else if (operation.command == "--username")
				{
					string userName = operation.argument;

					if (userName.empty() || userName.find("SERVER") != string::npos)
					{
						anonymCount++;
						userName = "익명" + to_string(anonymCount);
					}

					if (users.find(tcp->ReadSenderID()) == users.end())
						users.emplace(tcp->ReadSenderID(), userName);
					else
						users[tcp->ReadSenderID()] = userName;
				}
				else if (operation.command == "--dice")
				{
					string messageOut = "[SERVER] " + users[tcp->ReadSenderID()] + "님이 주사위를 굴려 " + std::to_string(Random::Integer(0, 100)) + "이(가) 나왔습니다!";
					tcp->Send(messageOut, TCP::SendRange::ALL);
				}
			}
			else
			{
				tcp->Send("[" + users[tcp->ReadSenderID()] + "] : " + tcp->ReadMessage(), TCP::SendRange::ALL);
				cout << "[CLIENT] #" + tcp->ReadSenderID() + " send a message" << endl;
			}

			break;
		}

		this_thread::sleep_for(100us);
	}

	tcp->Send("[SERVER] Server is closed", TCP::SendRange::ALL);
	delete tcp;

	return 0;
}